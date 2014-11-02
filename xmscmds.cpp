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

#include "logger.h"
#include "xmscmds.h"

// A collection of REST commands for use with the 720p conferening demo.
// Parameters that are always the same are "hardwired", those that may
// change, depending on invocation, are taken as input paramters

std::string & createEvhandlerXml ()
{
//JH - why does this need to remain static and have reference passed back?
// JH - content gets trashed.
    static std::string xmlCmd =
        "<web_service version=\"1.0\"><eventhandler>  <eventsubscribe action=\"add\" type=\"any\" resource_id=\"any\" resource_type=\"any\"/> </eventhandler></web_service>";
    return xmlCmd;
}

std::string
create_conference_xml (const char *reserve, const char *max_parties, const char *layout, const char *layout_size)
{
    // Start with known parameters
    std::string xmlCmd =
        "<web_service version=\"1.0\"><conference type=\"audiovideo\" beep=\"yes\" clamp_dtmf=\"yes\" auto_gain_control=\"yes\" echo_cancellation=\"yes\" caption=\"no\" ";

    // Add possible params
    if (reserve)
    {
        xmlCmd += "reserve=\"";
        xmlCmd += reserve;
        xmlCmd += "\" ";
    }
    if (max_parties)
    {
        xmlCmd += "max_parties=\"";
        xmlCmd += max_parties;
        xmlCmd += "\" ";
    }
    if (layout)
    {
        xmlCmd += "layout=\"";
        xmlCmd += layout;
        xmlCmd += "\" ";
    }
    if (layout_size)
    {
        xmlCmd += "layout_size=\"";
        xmlCmd += layout_size;
        xmlCmd += "\" ";
    }

    xmlCmd += "/></web_service>";
    return xmlCmd;
}

std::string
answer_call_xml (const char *dtmf_mode)
{
    // Start with known parameters
    std::string xmlCmd =
        "<web_service version=\"1.0\"> <call answer=\"yes\" media=\"audiovideo\" info_ack_mode=\"manual\" async_dtmf=\"yes\" async_tone=\"yes\" ";

    // Add possible params
    if (strcmp (dtmf_mode, "sipinfo") == 0)
    {
        // DTMF sent in SIP INFO message
        xmlCmd += "dtmf_mode=\"outofband\" ";
    }
    else
    {
        // Default to RFC 2833
        xmlCmd += "dtmf_mode=\"rfc2833\" ";
    }
    xmlCmd += "/></web_service>";
    return xmlCmd;
}

std::string
update_conference_xml (const char *layout_regions, const char *layout, const char *region_overlays)
{
    // Start with known parameters
    std::string xmlCmd = "<web_service version=\"1.0\"> <conference ";

    // Add possible params
    if (layout_regions)
    {
        xmlCmd += "layout_regions=\"";
        xmlCmd += layout_regions;
        xmlCmd += "\" ";
    }

    if (layout)
    {
        xmlCmd += "layout=\"";
        xmlCmd += layout;
        xmlCmd += "\" ";
    }

    if (region_overlays)
    {
        xmlCmd += "region_overlays=\"";
        xmlCmd += region_overlays;
        xmlCmd += "\" ";
    }

    xmlCmd += "/></web_service>";
    return xmlCmd;
}

std::string
add_party_xml (std::string conf_id, const char *region)
{
    // Start with known parameters
    std::string xmlCmd;
    xmlCmd = "<web_service version=\"1.0\"><call> <call_action> <add_party conf_id=\"";
    xmlCmd += conf_id;
    xmlCmd += "\" ";;
    xmlCmd += "audio=\"sendrecv\" ";
    xmlCmd += "video=\"sendrecv\" ";
    xmlCmd += "auto_gain_control=\"yes\" ";
    xmlCmd += "echo_cancellation=\"yes\" ";
    xmlCmd += "mode=\"normal\" ";
    xmlCmd += "mute=\"no\" ";
    xmlCmd += "privilege=\"no\" ";
    xmlCmd += "clamp_dtmf=\"no\" ";

    // Add possible params
    if (region)
    {
        xmlCmd += " region=\"";
        xmlCmd += region;
        xmlCmd += "\" ";
    }

    xmlCmd += "/> </call_action> </call></web_service>";
    return xmlCmd;
}

std::string
update_party_xml (const char *audio, const char *video, const char *region)
{
    // Start with known parameters
    std::string xmlCmd;
    xmlCmd = "<web_service version=\"1.0\"><call> <call_action> <update_party ";

    // Add possible params
    if (audio)
    {
        xmlCmd += " audio=\"";
        xmlCmd += audio;
        xmlCmd += "\" ";
    }
    if (video)
    {
        xmlCmd += " video=\"";
        xmlCmd += video;
        xmlCmd += "\" ";
    }
    if (region)
    {
        xmlCmd += " region=\"";
        xmlCmd += region;
        xmlCmd += "\" ";
    }

    xmlCmd += "/> </call_action> </call></web_service>";
    return xmlCmd;
}

std::string
hangup_xml (void)
{
    // Just two known parameters
    std::string xmlCmd =
        "<web_service version=\"1.0\"><call> <call_action> content_type=\"text/plain\" content=\"Conference app terminating call\" </call_action> </call></web_service>";
    return xmlCmd;
}

std::string
record_conf_xml (std::string conf_id, const char *audio_uri, const char *audio_type,
                 const char *audio_codec, const char *audio_rate, const char *video_uri,
                 const char *video_type, const char *video_codec, const char *video_level,
                 const char *video_height, const char *video_width,
                 const char *video_maxbitrate, const char *video_framerate, const char *record_time)
{
    // Start with known parameters
    std::string xmlCmd = "<web_service version=\"1.0\"><conference> <conf_action> <record ";

    // Add possible params
    if (record_time)
    {
        xmlCmd += "max_time=\"";
        xmlCmd += record_time;
        xmlCmd += "\" ";
    }
    if (audio_uri)
    {
        xmlCmd += "recording_audio_uri=\"";
        xmlCmd += audio_uri;
        xmlCmd += "\" ";
    }
    if (audio_type)
    {
        xmlCmd += "recording_audio_type=\"";
        xmlCmd += audio_type;
        xmlCmd += "\" ";
    }
    if (video_uri)
    {
        xmlCmd += "recording_video_uri=\"";
        xmlCmd += video_uri;
        xmlCmd += "\" ";
    }
    if (video_type)
    {
        xmlCmd += "recording_video_type=\"";
        xmlCmd += video_type;
        xmlCmd += "\"> ";
    }
    if (audio_codec || audio_rate)
    {
        xmlCmd += "<recording_audio_mime_params ";
        if (audio_codec)
        {
            xmlCmd += "codec=\"";
            xmlCmd += audio_codec;
            xmlCmd += "\" ";
        }
        if (audio_rate)
        {
            xmlCmd += " rate=\"";
            xmlCmd += audio_rate;
            xmlCmd += "\" ";
        }
        xmlCmd += "/> ";
    }
    if (video_codec || video_level || video_height || video_width || video_framerate || video_maxbitrate)
    {
        xmlCmd += "<recording_video_mime_params ";
        if (video_codec)
        {
            xmlCmd += "codec=\"";
            xmlCmd += video_codec;
            xmlCmd += "\" ";
        }
        if (video_level)
        {
            xmlCmd += "level=\"";
            xmlCmd += video_level;
            xmlCmd += "\" ";
        }
        if (video_height)
        {
            xmlCmd += "height=\"";
            xmlCmd += video_height;
            xmlCmd += "\" ";
        }
        if (video_width)
        {
            xmlCmd += "width=\"";
            xmlCmd += video_width;
            xmlCmd += "\" ";
        }
        if (video_framerate)
        {
            xmlCmd += "framerate=\"";
            xmlCmd += video_framerate;
            xmlCmd += "\" ";
        }
        if (video_maxbitrate)
        {
            xmlCmd += "maxbitrate=\"";
            xmlCmd += video_maxbitrate;
            xmlCmd += "\"";
        }
        xmlCmd += "/> ";
    }

    xmlCmd += "</record> </conf_action> </conference></web_service>";
    return xmlCmd;
}

std::string
play_into_conf_xml (std::string conf_id, const char *audio_uri, const char *audio_type, const char *base_audio_uri,
                    const char *video_uri, const char *video_type, const char *base_video_uri, const char *region,
                    const char *repeat)
{
    // Start with known parameters
    std::string xmlCmd = "<web_service version=\"1.0\"><conference> <conf_action> <play ";
    // Add possible params
    if (region)
    {
        xmlCmd += "region=\"";
        xmlCmd += region;
        xmlCmd += "\" >";
    }

    xmlCmd += "<play_source ";
    if (audio_uri)
    {
        xmlCmd += "audio_uri=\"";
        xmlCmd += audio_uri;
        xmlCmd += "\" ";
    }
    if (base_audio_uri)
    {
        xmlCmd += "base_audio_uri=\"";
        xmlCmd += base_audio_uri;
        xmlCmd += "\" ";
    }
    if (audio_type)
    {
        xmlCmd += "audio_type=\"";
        xmlCmd += audio_type;
        xmlCmd += "\" ";
    }
    if (video_uri)
    {
        xmlCmd += "video_uri=\"";
        xmlCmd += video_uri;
        xmlCmd += "\" ";
    }
    if (base_video_uri)
    {
        xmlCmd += "base_video_uri=\"";
        xmlCmd += base_video_uri;
        xmlCmd += "\" ";
    }
    if (video_type)
    {
        xmlCmd += "video_type=\"";
        xmlCmd += video_type;
        xmlCmd += "\" ";
    }
    xmlCmd += " /> </play> </conf_action> </conference></web_service>";
    return xmlCmd;
}

std::string
stop_xml (std::string transaction_id)
{
    std::string xmlCmd = "<web_service version=\"1.0\"><conference> <conf_action> <stop transaction_id=\"";
    xmlCmd += transaction_id;
    xmlCmd += "\"/></conf_action> </conference></web_service>";
    return xmlCmd;
}
