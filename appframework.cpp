/*
 * Copyright (C) 2014 Dialogic Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 * Alternatively see <http://www.gnu.org/licenses/>.
 * Or see the LICENSE file included within the source tree.
 *
 */
/*------------------------------ Dependencies --------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <signal.h>

#include "dispatchxmscmd.h"
#include "appframework.h"
#include "logger.h"
#include "conference720p.h"
#include "calls.h"
#include "confvideoplays.h"

#include <curl/curl.h>
#include "XmlDomDocument.h"
#include "xmscmds.h"
#include "xmseventparser.h"
#include "xmsreplyparser.h"
#include "replycontentcallback.h"

/*----------------------------------------------------------------------------*/

//const
//    std::string
//    default_appmanager_tcp_port = "15001";

// Global pointers for a single instance of the active call list and video
// conference play list
Calls *
    Calls::pInstance_ = NULL;
ConfVideoPlays *
    ConfVideoPlays::pInstance_ = NULL;

// REST service address/port
extern
    std::string
    xmsAddr;

// Flags for process termination, log reset
extern int
    term;
extern int
    log_restart;

extern void
sig_terminate (int signo);

static
    std::string
    eventHandlerId;

/*
 * ctor
 */
AppFramework::AppFramework ()
{
    ;
}


/*
 * dtor
 */
AppFramework::~AppFramework ()
{
    ;
}

void *
AppFramework::eventHandlerThread (void *voidPtr)
{
    // A seprate thread that will handle all REST events from XMS
    //
    // What happens here:
    // Initialize cURL subsystem.
    // Request (GET) an event handler from XMS.
    // With the event handler ID, do a long poll GET on the URL
    // formed with the event handler ID. This GET will remain open
    // for the duration of demo, and incoming events will appear
    // in the cURL getEventReplyContent callback function. There they
    // are stuffed in a queue, wehre they are read/processed by the
    // main thread.

    CURL *curl;
    CURLcode res;
    struct MemoryStruct createEvhandlerReplyContent;
    struct MemoryStruct getEventReplyContent;


    // will be grown as needed by realloc above 
    createEvhandlerReplyContent.memory = (char *) malloc (1);
    createEvhandlerReplyContent.size = 0;

    // All incoming events will arrive in this memory buffer
    getEventReplyContent.memory = (char *) malloc (1);
    getEventReplyContent.size = 0;

    LOGDEBUG ("Initializing cURL");
    curl_global_init (CURL_GLOBAL_ALL);

    // Set up curl for POST to create event handler
    curl = curl_easy_init ();
    if (curl)
    {
        std::string evHandlerUrl = "http://" + xmsAddr + "/default/eventhandlers?appid=app";
        curl_easy_setopt (curl, CURLOPT_URL, evHandlerUrl.c_str ());
        // Set callback for request
        //curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, createEvhandlerReplyContentCallback);
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, replyContentCallback);
        //
        // pass our createEvhandlerReplyContent struct to the callback function to get reply to POST
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &createEvhandlerReplyContent);

        // some servers don't like requests that are made without a user-agent
        // field, so we provide one 
        curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Get an event handler from XMS
        LOGDEBUG ("Eventhander POST content is " << createEvhandlerXml ().c_str ());
        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, createEvhandlerXml ().c_str ());

        // if we don't provide POSTFIELDSIZE, libcurl will strlen() by itself 
        curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, (long) createEvhandlerXml ().size ());

        // Perform the request, res will get the return code 
        res = curl_easy_perform (curl);
        if (res != CURLE_OK)
        {
            LOGERROR ("Event handler create - curl_easy_perform() failed: " << curl_easy_strerror (res));
            sig_terminate (SIGTERM);
            return NULL;
        }
        else
        {
            // Get the response
            long respCode;
            res = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &respCode);
            if (res == CURLE_OK)
            {
                LOGDEBUG ("Response code to event handler POST is " << respCode);
                if (respCode != 201)
                {
                    LOGCRIT ("Event handler not available.  Exiting application.");
                    curl_easy_cleanup (curl);
                    curl_global_cleanup ();
                    sig_terminate (SIGTERM);
                    return NULL;
                }
            }

            // Now, our createEvhandlerReplyContent.memory points to a memory block that is 
            // createEvhandlerReplyContent.size bytes big and contains the reply
            LOGDEBUG ("Event handler create returns " << createEvhandlerReplyContent.memory);
            std::string createEvhandlerReplyContentString = createEvhandlerReplyContent.memory;
            xmsReplyParser *parser = new xmsReplyParser (createEvhandlerReplyContentString, createEventhandler);
            //LOGDEBUG("Parsed eventhandler reply is " << eventhandlerHref);
            std::string evHandlerUrl = "http://" + xmsAddr + parser->getEventhandlerHref () + "?appid=app";
            eventHandlerId = parser->getEventhandlerId ();
            delete parser;

            LOGDEBUG ("Initiate long-poll GET for eventhandler URL " << evHandlerUrl);
            curl = curl_easy_init ();
            curl_easy_setopt (curl, CURLOPT_URL, evHandlerUrl.c_str ());
            curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, longPollReplyContentCallback);
            curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &getEventReplyContent);
            curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
            LOGDEBUG ("Entering curl_easy_perform for long poll GET");
            res = curl_easy_perform (curl);

            // How can curl be convinced to return from long poll?
            LOGDEBUG ("Returned from long poll GET. Result from GET is " << res);

            // When done with event handler need to clean up as below
            // Also, should delete event handler
            curl_easy_cleanup (curl);
            if (createEvhandlerReplyContent.memory)
                free (createEvhandlerReplyContent.memory);
            if (getEventReplyContent.memory)
                free (getEventReplyContent.memory);

            // we' re done with libcurl, so clean it up 
            curl_global_cleanup ();
            LOGDEBUG ("Curl cleanup done, exiting");
            return NULL;
        }
    }                           // end if curl not initialized
    LOGCRIT ("Curl cannot be initialized.  Event handler thread.");
    return NULL;
}

bool AppFramework::initEventHandlerThread ()
{
    // All REST client initialization takes place here.
    //
    // First, a separate thread must be spawned, as an event handler
    // It remains open, collecting events.
    int
        nothing = 0;
    if (pthread_create (&evHandlerThread, NULL, eventHandlerThread, (void *) &nothing))
    {
        LOGCRIT ("Cannot create event handler thread");
        return false;
    }

    LOGDEBUG ("Event Handler thread created");
    return true;
}



// This is the application's top level function, in the main thread
//
bool AppFramework::run (const GetOptions & opts, int /*signal_fd */ )
{
    Conference720p *
        conf_test_720p;
    xmsEventParser *
        curEvent;
    std::string eventXml;

    // First, get IP address and port for XMS REST connection
    std::string ipAddr = opts.getValue ("ip-address");
    std::string restPort = opts.getValue ("port");
    if (ipAddr.empty ())
    {
        LOGCRIT ("XMS IP address must be entered");
        return false;
    }
    if (restPort.empty ())
        restPort = "81";
    xmsAddr = ipAddr + ":" + restPort;
    LOGDEBUG ("XMS server's REST connection is at " << xmsAddr);
    // Handler for REST events from XMS will be run in a 2nd thread
    if (!initEventHandlerThread ())
        return false;

    std::string dtmf_mode = opts.getValue ("dtmf-mode");
    if (dtmf_mode != "rfc2833" && dtmf_mode != "sipinfo")
        dtmf_mode = "sipinfo";

    // Create the app object
    conf_test_720p = new Conference720p (dtmf_mode);

    // Loop on events until a term isgnal is received
    LOGDEBUG ("Entering event loop in main thread");
    while (!term)
    {
        // Block until something in the event handler
        // Look here:
        // http://www.dailyfreecode.com/code/solve-producer-consumer-problem-thread-2114.aspx

        // Protected eventQueue access
        //LOGDEBUG ("Entering protected dequeue section");
        pthread_mutex_lock (&queueLock);
        //LOGDEBUG("Event queue has " << eventQueue.size() << " events");
        pthread_cond_wait (&queueNotEmpty, &queueLock);
        // When ^C ing out of app after a hangup, wait above is released. If that's
        // the case, do not further processing and break out of the event loop
        // and do cleanup before shutting down.
        if (term)
            break;

        if (!eventQueue.empty ())
        {
            eventXml = eventQueue.front ();
            LOGDEBUG ("Event from queue is " << eventXml);
            eventQueue.pop ();
        }
        pthread_mutex_unlock (&queueLock);
        //LOGDEBUG("Out of protected dequeue section");
        if (log_restart)
        {
            LOGINFO ("Restarting log file");
            Logger::instance ().restart ();
            log_restart = false;
        }
        curEvent = new xmsEventParser (eventXml);
        // Incoming event gets special treatment
        if (curEvent->getEventType () == "incoming")
        {
            std::string call_id = curEvent->findValByKey ("call_id");

            // Create a new 720p conference call object
            Call
            conf_call_720p ("conf_demo", call_id, conf_test_720p);
            Calls::Instance ()->addNewCall (conf_call_720p);
            conf_test_720p->onEvent (curEvent);
        }                       // end if offer
        else if (curEvent->getEventType () == "keepalive")
        {
            // These can come anytime, and have no call ID
            LOGDEBUG ("Keepalive received");
        }
        else
        {
            // OK, so not an offered of keepalive. Send event directly to the conferencing app
            conf_test_720p->onEvent (curEvent);
        }
        // Done with the event
        delete
            curEvent;
    }                           // end event loop

    LOGDEBUG ("Leaving main processing thread");
    // Done; clean up
    //
    // DELETE event handler. This includes sending a DELETE message to XMS
    destroy_eventhandler (eventHandlerId);
    // And conference object
    delete
        conf_test_720p;
    return true;
}

/* vim:ts=4:set nu:
 * EOF
 */
