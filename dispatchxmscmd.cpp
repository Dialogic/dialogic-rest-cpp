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
#include <stdio.h>
#include <curl/curl.h>

#include "logger.h"
#include "dispatchxmscmd.h"
#include "xmscmds.h"
#include "xmsreplyparser.h"
#include "replycontentcallback.h"

/*----------------------------------------------------------------------------*/

// REST service address/port
extern
    std::string
    xmsAddr;

/********************************  
struct MemoryStruct
{
    char *
        memory;
    size_t
        size;
};

static
    size_t
replyContentCallback (void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *
        mem = (struct MemoryStruct *) userp;
    mem->memory = (char *) realloc (mem->memory, mem->size + realsize + 1);

    if (mem->memory == NULL)
    {
        // out of memory!
        LOGCRIT ("ReplyContentCallback - not enough memory (realloc returned NULL)");
        return 0;
    }

    memcpy (&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
******************************************/

std::string
dispatchPost (std::string resource, std::string xmlContent)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct replyContent;
    replyContent.memory = (char *) malloc (1);
    replyContent.size = 0;
    std::string replyContentString;

    curl = curl_easy_init ();
    if (curl)
    {

        std::string url = "http://" + xmsAddr + resource;
        curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
        // Set callback for request
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, replyContentCallback);
        //
        // pass our replyContent struct to the callback function to get reply to POST
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &replyContent);

        // some servers don't like requests that are made without a user-agent
        // field, so we provide one
        curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Add XML content for POST
        LOGDEBUG ("XML content for resource " << resource << " is " << xmlContent.c_str ());
        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, xmlContent.c_str ());
        curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, (long) xmlContent.size ());

        // Perform the request, res will get the return code
        res = curl_easy_perform (curl);
        LOGDEBUG ("POST result is " << res);
        if (res != CURLE_OK)
        {
            LOGERROR ("POST- curl_easy_perform() failed: " << curl_easy_strerror (res));
            return replyContentString;
        }
        else
        {
            // Get the response
            long respCode;
            res = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &respCode);
            if (res == CURLE_OK)
            {
                LOGDEBUG ("Response code to POST is " << respCode);
                if (respCode == 201)
                {
                    // Now, our replyContent.memory points to a memory block that is replyContent.size
                    // bytes big and contains the reply
                    LOGDEBUG ("POST returns " << replyContent.size << " bytes");
                    LOGDEBUG ("POST reply is " << replyContent.memory);
                    replyContentString = replyContent.memory;
                    return replyContentString;
                }
                else if (respCode >= 400 && respCode <= 499)
                {
                    LOGWARN ("400 series response to POST - " << respCode);
                }
            }
            free (replyContent.memory);
            replyContent.size = 0;
        }
    }
    return replyContentString;
}

std::string
dispatchPut (std::string resource, std::string xmlContent, std::string id)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct replyContent;
    replyContent.memory = (char *) malloc (1);
    replyContent.size = 0;
    std::string replyContentString;

    curl = curl_easy_init ();
    if (curl)
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append (headers, "Accept: application/xml");
        headers = curl_slist_append (headers, "Content-Type: application/xml");
        headers = curl_slist_append (headers, "Connection: keep-alive");
        std::string contentLengthHeader = "Content-Length: " + xmlContent.size ();
        headers = curl_slist_append (headers, contentLengthHeader.c_str ());
        curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);

        std::string url = "http://" + xmsAddr + resource + id + "?appid=app";
        LOGDEBUG ("URL for PUT: " << url);
        curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());

        curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "PUT");

        // send all data to this function  
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, replyContentCallback);

        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, xmlContent.c_str ());
        curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, (long) xmlContent.size ());
        curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // we pass our replyContent struct to the callback function to get reply to PUT
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &replyContent);

        // Perform the request, res will get the return code 
        LOGDEBUG ("Sending HTTP PUT via Curl using URL " << url);
        LOGDEBUG ("Content of PUT is " << xmlContent);
        res = curl_easy_perform (curl);
        // Check for errors 
        if (res != CURLE_OK)
        {
            LOGERROR ("curl_easy_perform() for PUT failed: " << curl_easy_strerror (res));
        }
        else
        {
            // Get the response
            long respCode;
            res = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &respCode);
            if (res == CURLE_OK)
            {
                LOGDEBUG ("Response code to PUT is " << respCode);
                if (respCode == 200)
                {
                    // Now, our replyContent.memory points to a memory block that is replyContent.size
                    // bytes big and contains the reply
                    LOGDEBUG ("PUT returns " << replyContent.size << " bytes");
                    LOGDEBUG ("PUT reply is " << replyContent.memory);
                    replyContentString = replyContent.memory;
                }
                else if (respCode >= 400 && respCode <= 499)
                {
                    LOGWARN ("400 series response to PUT - " << respCode);
                }
            }
            // Done with  replyContent
            free (replyContent.memory);
            replyContent.size = 0;
        }
    }
    else
    {
        LOGERROR ("cur_easy_init failed for PUT");
    }
    return replyContentString;
}

int
dispatchDelete (std::string resource, std::string id)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct replyContent;
    replyContent.memory = (char *) malloc (1);
    replyContent.size = 0;

    curl = curl_easy_init ();
    if (curl)
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append (headers, "Accept: application/xml");
        headers = curl_slist_append (headers, "Content-Type: application/xml");
        headers = curl_slist_append (headers, "Connection: keep-alive");
        //std::string contentLengthHeader = "Content-Length: " + xmlContent.size ();
        //headers = curl_slist_append (headers, contentLengthHeader.c_str ());
        curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);

        std::string url = "http://" + xmsAddr + resource + id + "?appid=app";
        curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());

        curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        // send all data to this function  
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, replyContentCallback);

        //curl_easy_setopt (curl, CURLOPT_POSTFIELDS, xmlContent.c_str ());
        //curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, (long) xmlContent.size ());
        curl_easy_setopt (curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // we pass our replyContent struct to the callback function to get reply to PUT
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &replyContent);

        // Perform the request, res will get the return code 
        LOGDEBUG ("Sending HTTP DELETE via Curl using URL " << url);
        //LOGDEBUG ("Content of PUT is " << xmlContent);
        res = curl_easy_perform (curl);
        // Check for errors 
        if (res != CURLE_OK)
        {
            LOGERROR ("curl_easy_perform() for DELETE failed: " << curl_easy_strerror (res));
        }
        else
        {
            // Get the response
            long respCode;
            res = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &respCode);
            if (res == CURLE_OK)
            {
                LOGDEBUG ("Response code to DELETE is " << respCode);
                if (respCode != 204)
                {
                    // 204 is success for DELETE. No content is returned with it
                    LOGERROR ("DELETE for call/conference ID " << resource + id << " was not successful");
                    return 1;
                }
                else if (respCode >= 400 && respCode <= 499)
                {
                    LOGWARN ("400 series response to DELETE - " << respCode);
                }
            }
        }
    }
    else
    {
        LOGERROR ("cur_easy_init failed for DELETE");
    }
    return 0;
}


int
answer (std::string callId, const char *dtmf_mode)
{

    std::string answerXml = answer_call_xml (dtmf_mode);
    dispatchPut ("/default/calls/", answerXml, callId);
    return 0;
}


/*
 * Wrapper for xms_hangup()
 */
int
hangup (std::string callId)
{
    std::string hangupXml = hangup_xml ();
    dispatchPut ("/default/calls/", hangupXml, callId);
    return 0;
}

std::string
play_into_conf (std::string conf_id, const char *audio_uri, const char *audio_type,
                const char *base_audio_uri, const char *video_uri, const char *video_type,
                const char *base_video_uri, const char *region, const char *repeat)
{
    std::string reply;
    std::string mediaId;

    std::string playXml = play_into_conf_xml (conf_id, audio_uri, audio_type, base_audio_uri, video_uri,
                                              video_type, base_video_uri, region, repeat);

    reply = dispatchPut ("/default/conferences/", playXml, conf_id);
    if (!reply.empty ())
    {
        xmsReplyParser *parser = new xmsReplyParser (reply, playIntoConference);
        mediaId = parser->getMediaId ();
        delete parser;
        return mediaId;
    }
    else
    {
        LOGWARN ("No media ID from conference play");
        return mediaId;
    }
}


std::string
record_conference (std::string conf_id,
                   const char *audio_uri,
                   const char *audio_type,
                   const char *audio_codec,
                   const char *audio_rate,
                   const char *video_uri,
                   const char *video_type,
                   const char *video_codec,
                   const char *video_level,
                   const char *video_height, const char *video_width, const char *video_maxbitrate, const char * video_framerate, const char *record_time)
{
    std::string reply;
    std::string mediaId;

    std::string recordXml = record_conf_xml (conf_id, audio_uri, audio_type, audio_codec, audio_rate,
					     video_uri, video_type, video_codec, video_level,
					     video_height, video_width, video_maxbitrate, video_framerate,
						record_time);

    reply = dispatchPut ("/default/conferences/", recordXml, conf_id);
    if (!reply.empty ())
    {
        xmsReplyParser *parser = new xmsReplyParser (reply, recordConference);
        mediaId = parser->getMediaId ();
        delete parser;
        return mediaId;
    }
    else
    {
        LOGWARN ("No media ID from conference record");
        return mediaId;
    }
}

int
stop (std::string confId, std::string transactionId)
{
    std::string stopXml = stop_xml (transactionId);
    dispatchPut ("/default/conferences/", stopXml, confId);
    return 0;
}

/*
 *  Wrapper for xms_create_call()
 */
char *
create_call (int signalling, const char *sdp, const char *dtmf_mode)
{
/***********************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_SIGNALLING, (signalling ? XMS_VALUE_YES : XMS_VALUE_NO));
    if (sdp)
    {
        xms_param_append (request, XMS_KEY_SDP, sdp);
    }
    xms_param_append (request, XMS_KEY_CPA, XMS_VALUE_NO);
    xms_param_append (request, XMS_KEY_MEDIA, XMS_VALUE_AUDIOVIDEO);
    if (strcmp (dtmf_mode, "sipinfo") == 0)
    {
        // DTMF sent in SIP INFO message
        xms_param_append (request, XMS_KEY_DTMF_MODE, XMS_VALUE_OUTOFBAND);
    }
    else
    {
        // Default to RFC 2833
        xms_param_append (request, XMS_KEY_DTMF_MODE, XMS_VALUE_RFC2833);
    }
    print_xms_param ("xms_create_call", request);

    char *call_id = NULL;
    struct xms_param *response = xms_param_new ();
    int res = xms_create_call (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_create_call() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    else
    {
        call_id = strdup (xms_param_find (response, XMS_KEY_CALL_ID));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return call_id;
*******************************/
    return 0;
}


std::string
create_conference (const char *reserve, const char *max_parties, const char *layout,
                   const char *layout_size)
{

    std::string confId;
    std::string createConfXml = create_conference_xml (reserve, max_parties, layout, layout_size);
    std::string reply = dispatchPost ("/default/conferences?appid=app", createConfXml);
    if (!reply.empty ())
    {
        xmsReplyParser *parser = new xmsReplyParser (reply, createConference);
        std::string confId = parser->getConfId ();
        delete parser;
        return confId;
    }
    else
    {
        LOGCRIT ("No conference ID from create conference POST");
        return confId;
    }
}


int
destroy_conference (std::string conf_id)
{

    dispatchDelete ("/default/conferences/", conf_id);
    return 0;
}

int
destroy_eventhandler (std::string evhandler_id)
{

    dispatchDelete ("/default/eventhandlers/", evhandler_id);
    return 0;
}

int
add_party (std::string call_id, std::string conf_id, const char *region)
{
    std::string addPartyXml = add_party_xml (conf_id, region);
    //LOGDEBUG("Wrapper - add_party xml - " << addPartyXml);
    dispatchPut ("/default/calls/", addPartyXml, call_id);
    // JH - error handling!!
    return 0;
}

int
remove_party (std::string call_id)
{
/****************************** rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);

    print_xms_param ("xms_remove_party", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_remove_party (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_remove_party() failed: " << xms_param_find (response, XMS_KEY_STATUS) << "\n");
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
**********************************/
    return 0;
}

int
update_party (std::string call_id, const char *audio, const char *video, const char *region)
{
    std::string updatePartyXml = update_party_xml (audio, video, region);
    dispatchPut ("/default/calls/", updatePartyXml, call_id);
    // JH - error handling!!
    return 0;



/**************11***************************** rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    if (audio)
    {
        xms_param_append (request, XMS_KEY_AUDIO, audio);
    }
    if (video)
    {
        xms_param_append (request, XMS_KEY_VIDEO, video);
    }
    if (caption)
    {
        xms_param_append (request, XMS_KEY_CAPTION, caption);
    }
    if (duration)
    {
        xms_param_append (request, XMS_KEY_DURATION, duration);
    }
    if (region)
    {
        xms_param_append (request, XMS_KEY_REGION, region);
    }
    //xms_param_append(request, XMS_KEY_AGC, XMS_VALUE_NO);
    //xms_param_append(request, XMS_KEY_EC, XMS_VALUE_NO);
    //xms_param_append(request, XMS_KEY_CLAMP_DTMF, XMS_VALUE_NO);
    //if ( region && *region == '1' )
    //{
    //    xms_param_append(request, XMS_KEY_TX_MUTE, XMS_VALUE_YES);
    // }
    //else
    //{
    //    xms_param_append(request, XMS_KEY_TX_MUTE, XMS_VALUE_NO);
    //}

    xms_param_append (request, XMS_KEY_PRIVILEGE, XMS_VALUE_NO);

    print_xms_param ("xms_update_party", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_update_party (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_update_party() failed: " << xms_param_find (response, XMS_KEY_STATUS) << "\n");
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
**********************************/
    return 0;
}


int
update_call (std::string call_id, const char *tx_volume, const char *rx_volume, int async_dtmf, int async_tone)
{
/***********************************rest
    struwct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    if (tx_volume)
    {
        xms_param_append (request, XMS_KEY_TX_VOLUME, tx_volume);
    }
    if (rx_volume)
    {
        xms_param_append (request, XMS_KEY_RX_VOLUME, rx_volume);
    }

    xms_param_append (request, XMS_KEY_ASYNC_DTMF, (async_dtmf ? XMS_VALUE_YES : XMS_VALUE_NO));
    xms_param_append (request, XMS_KEY_ASYNC_TONE, (async_tone ? XMS_VALUE_YES : XMS_VALUE_NO));

    print_xms_param ("xms_update_call", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_update_call (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_update_call() failed: " << xms_param_find (response, XMS_KEY_STATUS) << "\n");
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
******************************/
    return 0;
}


int
join (const char *call_id, const char *call_id2)
{
/***************************************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    xms_param_append (request, XMS_KEY_CALL_ID2, call_id2);

    print_xms_param ("xms_join", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_join (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_join() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
******************************/
    return 0;
}


int
unjoin (const char *call_id, const char *call_id2)
{
/*****************************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    xms_param_append (request, XMS_KEY_CALL_ID2, call_id2);

    print_xms_param ("xms_unjoin", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_unjoin (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_unjoin() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
***************************************/
    return 0;
}

/*
 *  * Test xms_dial()
 *   */
int
dial (std::string call_id, const char *dest_uri, const char *called_uri, const char *caller_uri, int cpa)
{
/*****************************************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    xms_param_append (request, XMS_KEY_URI, dest_uri);
    xms_param_append (request, XMS_KEY_CALLED_URI, called_uri);

    // JH
    // Got sytax error on From header at Crocodile.  Don't use it
    // xms_param_append(request, XMS_KEY_CALLER_URI, caller_uri);
    // xms_param_append(request, XMS_KEY_CALLER_DISPLAY_NAME, "ÐовеÑºа");  // "check" in russian, tes-8UTFF
    //
    xms_param_append (request, XMS_KEY_CALLER_URI, caller_uri);
    xms_param_append (request, XMS_KEY_CALLER_DISPLAY_NAME, "XMS Verification Demo");
    xms_param_append (request, XMS_KEY_CPA, (cpa ? XMS_VALUE_YES : XMS_VALUE_NO));
    xms_param_append (request, XMS_KEY_TIMEOUT, "20s");

    print_xms_param ("xms_dial", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_dial (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_dial() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
*******************************/
    return 0;
}


char *
overlay (std::string call_id, const char *uri, const char *duration, const char *direction)
{
/***************************************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    xms_param_append (request, XMS_KEY_URI, uri);
    if (duration)
    {
        xms_param_append (request, XMS_KEY_DURATION, duration);
    }
    if (strcmp (direction, "send") == 0)
    {
        xms_param_append (request, XMS_KEY_DIRECTION, XMS_VALUE_SEND);
    }
    else if (strcmp (direction, "recv") == 0)
    {
        xms_param_append (request, XMS_KEY_DIRECTION, XMS_VALUE_RECV);
    }

    print_xms_param ("xms_overlay", request);

    char *media_id = NULL;
    struct xms_param *response = xms_param_new ();
    int res = xms_overlay (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_overlay() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    else
    {
        media_id = strdup (xms_param_find (response, XMS_KEY_MEDIA_ID));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return media_id;
**********************************/
    return (char *) "something";
}

int
update_conference (std::string conf_id, const char *layout_size, const char *layout_regions, const char *region_overlays)
{
    std::string updateConfXml = update_conference_xml (layout_regions, layout_size, region_overlays);
    dispatchPut ("/default/conferences/", updateConfXml, conf_id);
    return 0;
/****************************************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CONF_ID, conf_id);

    if (layout)
    {
        xms_param_append (request, XMS_KEY_LAYOUT, layout);
    }
    
    if (layout_regions)
	{
       xms_param_append(request, XMS_KEY_LAYOUT_REGIONS, layout_regions);
	}

    if (region_overlays)
    {
        xms_param_append (request, XMS_KEY_REGION_OVERLAYS, region_overlays);
    }

    print_xms_param ("xms_update_conference", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_update_conference (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_update_conference() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
**********************************/
    return 0;
}

/*
 *  * Test xms_update_play()
 *   */
int
update_play (const char *media_id, const char *action, const char *region)
{
/*****************************************rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_MEDIA_ID, media_id);
    if (action)
    {
        xms_param_append (request, XMS_KEY_ACTION, action);
    }
    if (region)
    {
        xms_param_append (request, XMS_KEY_REGION, region);
    }

    print_xms_param ("xms_update_play", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_update_play (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_update_play() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
*********************************/
    return 0;
}

/*
 *  *  * Test xms_send_info()
 *   *   */
int
send_info (std::string call_id, const char *content_type, const char *content)
{
/**********************************88rest
    struct xms_param *request = xms_param_new ();
    xms_param_append (request, XMS_KEY_CALL_ID, call_id);
    xms_param_append (request, XMS_KEY_CONTENT_TYPE, content_type);
    xms_param_append (request, XMS_KEY_CONTENT, content);

    print_xms_param ("xms_send_info", request);

    struct xms_param *response = xms_param_new ();
    int res = xms_send_info (request, response);
    if (res != XMS_SUCCESS)
    {
        LOGWARN ("xms_send_info() failed: " << xms_param_find (response, XMS_KEY_STATUS));
    }
    xms_param_delete (request);
    xms_param_delete (response);
    return res;
*********************************/
    return 0;
}


/* vim:ts=4:set nu:
 * EOF
 */
