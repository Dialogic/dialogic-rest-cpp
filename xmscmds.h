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

#ifndef _XMSCMDS_H
#define _XMSCMDS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <iostream>

std::string&
createEvhandlerXml ( );

std::string
create_conference_xml (const char *reserve, const char *max_parties, const char *layout,
                       const char *layout_size);

std::string
answer_call_xml (const char *dtmf_mode);

std::string
update_conference_xml (const char *layout_regions, const char *layout, const char *region_overlays);

std::string
play_into_conf_xml (std::string conf_id, const char *audio_uri, const char *audio_type, const char *base_audio_uri,
                    const char *video_uri, const char *video_type, const char *base_video_uri, const char *region,
                    const char *repeat);

std::string
record_conf_xml (std::string conf_id, const char *audio_uri, const char *audio_type,
                    const char *audio_codec, const char *audio_rate, const char *video_uri,
                    const char *video_type, const char *video_codec, const char *video_level,
                    const char *video_height, const char *video_width,
                    const char * video_maxbitrate, const char *video_framerate, const char *record_time);

std::string
stop_xml (std::string transaction_id);

//std::string
//modify_call_xml (const char *tx_volume, const char *rx_volume, const char *async_dtmf, const char *async_tone);

std::string
add_party_xml (std::string conf_id, const char *region);

std::string
update_party_xml (const char *audio, const char *video, const char *region);

std::string
hangup_xml (void);
#endif // _XMSCMDS_H

