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

#ifndef _DISPATCHXMSCMD_H
#define __DISPATCHXMSCMD_H

/*------------------------------ Dependencies --------------------------------*/

//#include <xms.h>

/*----------------------------------------------------------------------------*/

/* TODO create resource classes to wrap the raw API */

//int app_register (const char *name, const char *version, const char *desc);

//int app_unregister (const char *name);

//struct xms_param *get_event ();

int answer (std::string call_id, const char *dtmf_mode);

int hangup (std::string call_id);


char *play (const char *id,
            const char *audio_uri,
            const char *audio_type,
            const char *audio_base_uri, const char *video_uri, const char *video_type, const char *video_base_uri);

std::string play_into_conf (std::string conf_id,
                      const char *audio_uri,
                      const char *audio_type,
                      const char *audio_base_uri,
                      const char *video_uri, const char *video_type, const char *video_base_uri, const char *region,
                      const char *repeat);

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
                   const char *video_height, const char *video_width, const char *video_maxbitrate, const char *video_framerate, const char *record_time);

int stop (std::string confId, std::string transactionId);

char *create_call (int signaling, const char *sdp, const char *dtmf_mode);

std::string create_conference (const char *reserve, const char *max_parties, const char *layout,  const char *layout_size);

int destroy_conference (std::string conf_id);

int destroy_eventhandler (std::string evhandler_id);

int add_party (std::string call_id, std::string conf_id, const char *region);

int remove_party (std::string call_id);

int update_party (std::string call_id, const char *audio, const char *video, const char *region);

int update_call (std::string call_id, const char *tx_volume, const char *rx_volume, int async_dtmf, int async_tone);

//int unjoin (const char *call_id, const char *call_id2);
//int join (const char *call_id, const char *call_id2);
int dial (std:: string call_id, const char *dest_uri, const char *called_uri, const char *caller_uri, int cpa);
char *overlay (std::string call_id, const char *uri, const char *duration, const char *direction);

int update_conference (std::string conf_id, const char *layout_regions, const char *layout_size, const char *region_overlays);
int update_play (const char *media_id, const char *action, const char *region);
int send_info (std::string call_id, const char *content_type, const char *content);

#endif // _DISPATCHXMSCMD_H

/* vim:ts=4:set nu:
 * EOF
 */
