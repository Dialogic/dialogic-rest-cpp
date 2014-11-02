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

#ifndef APPFRAMEWOK_H
#define APPFRAMEWOK_H

/*------------------------------ Dependencies --------------------------------*/

#include <string>
#include <queue>

#include "getoption.h"
#include "call.h"

/*----------------------------------------------------------------------------*/

// Externs for event handler thread
extern std::queue<std::string> eventQueue;
extern pthread_mutex_t queueLock;
extern pthread_cond_t queueNotEmpty;
extern pthread_t evHandlerThread;


/*!
 * \class AppFramework
 * 
 */
class AppFramework
{
  public:
    AppFramework ();
    ~AppFramework ();

    // Entry point - work loop
    bool run (const GetOptions & opts, int signal_fd);

  private:
    static void *eventHandlerThread (void *voidPtr);
    bool initEventHandlerThread ();

    struct MemoryStruct
    {
        char *memory;
        size_t size;
    };
/********************************************
    // JH - can I consolidate this with the one in dispatchXmsCmd.cpp?
    static size_t createEvhandlerReplyContentCallback (void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *) userp;

        mem->memory = (char *) realloc (mem->memory, mem->size + realsize + 1);
        if (mem->memory == NULL)
        {
            // out of memory
            LOGCRIT ("evhandlerReplyContentCallback - not enough memory (realloc returned NULL)");
            return 0;
        }

        memcpy (&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }
*******************************************/

    static size_t longPollReplyContentCallback (void *contents, size_t size, size_t nmemb, void *userp)
    {
	// A typical CURL reply content callback, BUT, this one is used for the logn poll
	// GET that remains open.  Any content from the GET is an event from XMS, and here
	// in the event handling thread, it is enqueued to be read in the main thread where
	// all the action is.
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *) userp;
        mem->memory = (char *) realloc (mem->memory, mem->size + realsize + 1);

        if (mem->memory == NULL)
        {
            // out of memory! 
            LOGCRIT ("longPollReplyContentCallback - not enough memory (realloc returned NULL)");
            return 0;
        }
	// Add event to the event queue
        char * eventPtr = (char *) malloc((size * nmemb) + 1); 
        memcpy(eventPtr, contents, size * nmemb);
        //*(eventPtr + (size * nmemb) + 1) = 0;
        *(eventPtr + size * nmemb) = 0;
        //LOGDEBUG("Event length is " << strlen(eventPtr));
        //LOGDEBUG("Event is " << eventPtr);

        //for (int i=0; i< (int) strlen(eventPtr); i++)
        //{
	//   printf ("%x ", (int) *(eventPtr+i));
        //}
	std::string strToEnqueue = eventPtr + 4;

	// Protected section of code here. Only allow adding to queue
	// if event loop is not removing an entry.
	// In addition, send a singal to the main thread so that it will 
	// read an entry out of the queue
	//LOGDEBUG("Entering protected enqueue section");
	pthread_mutex_lock(&queueLock);
	eventQueue.push(strToEnqueue);
	pthread_mutex_unlock(&queueLock);
	pthread_cond_signal(&queueNotEmpty);
	//LOGDEBUG("Out of protected enqueue section");

        return realsize;
    }
};


#endif // APPFRAMEWOK_H

/* vim:ts=4:set nu:
 * EOF
 */
