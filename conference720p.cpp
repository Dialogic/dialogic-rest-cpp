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

#include "logger.h"
#include "conference720p.h"
#include "calls.h"
#include "confvideoplays.h"
#include "xmscmds.h"

/*----------------------------------------------------------------------------*/


Conference720p::Conference720p (std::string dtmf_mode)
{
        nullExclusiveMediaOp ();
        num_callers_ = 0;
        strcpy (dtmf_mode_, dtmf_mode.c_str ());

    resetConference ();
}

Conference720p::~Conference720p ()
{
    if (conf_id_.empty ())
    {
        LOGDEBUG ("Destroying conference " << conf_id_);
        destroy_conference (conf_id_);
    }
}

void
Conference720p::resetConference ()
{
    // initialize 
    is_very_first_call_ = true;
    nullExclusiveMediaOp ();
    conf_id_ = '\0';
    layout_ = '4';
    init_region_use ();
    recordInProgress_ = false;
    //overlay_id_ = '\0';
    captionsOn_ = false;
    scrolling_overlay_ = false;
    slide_show_ = false;
}

int
Conference720p::file_exists (const char *filename)
{
    struct stat buffer;
    LOGDEBUG ("For file: " << filename << " stat returns " << stat (filename, &buffer));
    return (stat (filename, &buffer) == 0);
}


void
Conference720p::notify_all_callers (const char *message)
{
    // Notify all callers with message
    std::vector < Call >::iterator call_iterator;
    for (call_iterator = Calls::Instance ()->verification_calls_.begin ();
         call_iterator != Calls::Instance ()->verification_calls_.end (); call_iterator++)
    {
        send_info (call_iterator->getCallId (), "text/plain", message);
    }
}

const char *
Conference720p::get_next_layout ()
{
    switch (layout_)
    {
    // Layouts rotate between 4 -> 6 -> 9 -> 4   
/****** JH
    case '1':
        layout_ = '2';
        break;
    case '2':
        layout_ = '4';
        //return custom4PartyLayout();
        return "4";
        break;
*************/
    case '4':
        layout_ = '6';
        return "6";
        break;
    case '6':
        layout_ = '9';
        return "9";
        break;
    case '9':
        layout_ = '4';
        return "4";
        break;
    }
    return &layout_;
}

void
Conference720p::turnOnCaption (int region)
{
    LOGDEBUG ("Turning conferee caption on for region " << region);
    std::string showCallerId = showCallerIdOverlay (region, "XXXX for now");
    update_conference (conf_id_, NULL, NULL, showCallerId.c_str ());
}

void
Conference720p::turnOffCaption (int region)
{
    LOGDEBUG ("Turning conferee caption off for region " << region);
    std::string delCallerId = deleteCallerIdOverlay (region);
    update_conference (conf_id_, NULL, NULL, delCallerId.c_str ());
}

void
Conference720p::turnOnAllCaptions ()
{
    LOGDEBUG ("Turning conferee captions on");
    std::vector < Call >::iterator call_iterator;
    // Need to make list public - can't get it with accessor?
    int confereeNum = 1;

    // Loop over all calls first
    for (call_iterator = Calls::Instance ()->verification_calls_.begin ();
         call_iterator != Calls::Instance ()->verification_calls_.end (); call_iterator++)
    {
        LOGDEBUG ("Turning conferee caption on for region " << call_iterator->getConfRegion ());
        char caption[32];
        sprintf (caption, "Conferee #%d", confereeNum);
        std::string showCallerId = showCallerIdOverlay (call_iterator->getConfRegion (), caption);
        update_conference (conf_id_, NULL, NULL, showCallerId.c_str ());

        confereeNum++;
    }

    // Now loop over any videos playing
    std::vector < ConfVideoPlay >::iterator conf_play_iterator;

    for (conf_play_iterator = ConfVideoPlays::Instance ()->conf_video_plays_.begin ();
         conf_play_iterator != ConfVideoPlays::Instance ()->conf_video_plays_.end (); conf_play_iterator++)
    {
        std::string showVidLabel = showVideoLabelOverlay (conf_play_iterator->getConfVideoPlayRegion ());
        update_conference (conf_id_, NULL, NULL, showVidLabel.c_str ());
    }
}

void
Conference720p::turnOffAllCaptions ()
{
    LOGDEBUG ("Turning conferee captions off");
    std::vector < Call >::iterator call_iterator;
    int confereeNum = 1;
    std::string delCallerId;

    // Loop over calls first
    for (call_iterator = Calls::Instance ()->verification_calls_.begin ();
         call_iterator != Calls::Instance ()->verification_calls_.end (); call_iterator++)
    {
        delCallerId = deleteCallerIdOverlay (call_iterator->getConfRegion ());
        update_conference (conf_id_, NULL, NULL, delCallerId.c_str ());
        confereeNum++;
    }

    // And video labels next
    turnOffVideoLabels ();
}

void
Conference720p::turnOffVideoLabels ()
{
    //char xmlString[128];
    std::vector < ConfVideoPlay >::iterator conf_play_iterator;

    // Loop over any videos playing
    for (conf_play_iterator = ConfVideoPlays::Instance ()->conf_video_plays_.begin ();
         conf_play_iterator != ConfVideoPlays::Instance ()->conf_video_plays_.end (); conf_play_iterator++)
    {
        std::string deleteVidLabel = deleteVideoLabelOverlay (conf_play_iterator->getConfVideoPlayRegion ());
        update_conference (conf_id_, NULL, NULL, deleteVidLabel.c_str ());
    }
}

void
Conference720p::moveRegionOverlays (const int fromRegion, const int toRegion)
{
    // Delete overlay in fromRegion and activate in toRegion
    if (Calls::Instance ()->isAudioMuteOnForCallId (Calls::Instance ()->getCallIdByConfRegion (fromRegion)))
    {
        // Move the mute overlay to the new region
        LOGDEBUG ("Moving mic mute overlay from region " << fromRegion << " to region " << toRegion);
        std::string deleteMicMute = deleteMicMuteOverlay (fromRegion);
        update_conference (conf_id_, NULL, NULL, deleteMicMute.c_str ());
        std::string showMicMute = showMicMuteOverlay (toRegion);
        update_conference (conf_id_, NULL, NULL, showMicMute.c_str ());
    }
    if (Calls::Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (fromRegion)))
    {
        // Move the video hiding overlay to the new region
        LOGDEBUG ("Moving video hiding overlay from region " << fromRegion << " to region " << toRegion);

        std::string delBaghead = deleteBagheadOverlay (fromRegion);
        update_conference (conf_id_, NULL, NULL, delBaghead.c_str ());

        std::string showBaghead = showBagheadOverlay (toRegion);
        update_conference (conf_id_, NULL, NULL, showBaghead.c_str ());
    }
}

void
Conference720p::copyRegionOverlays (const int fromRegion, const int toRegion)
{
    // Same as moveRegionOverlays, but do not remove fromRegion overlay
    if (Calls::Instance ()->isAudioMuteOnForCallId (Calls::Instance ()->getCallIdByConfRegion (fromRegion)))
    {
        // Move the mute overlay to the new region
        LOGDEBUG ("Copying mic mute overlay from region " << fromRegion << " to region " << toRegion);
        std::string showMicMute = showMicMuteOverlay (toRegion);
        update_conference (conf_id_, NULL, NULL, showMicMute.c_str ());
    }
    if (Calls::Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (fromRegion)))
    {
        // Move the video hiding overlay to the new region
        LOGDEBUG ("Copying video hiding overlay from region " << fromRegion << " to region " << toRegion);
        std::string showBaghead = showBagheadOverlay (toRegion);
        update_conference (conf_id_, NULL, NULL, showBaghead.c_str ());
    }
}

int
Conference720p::get_next_region (const int region)
{
    // Next region to rotate into depends on current layout
    switch (layout_)
    {
    case '1':
        return 0;
        break;
    case '2':
        if (region == 2)
            return 1;
        else
            return region + 1;
        break;
    case '4':
        if (region == 4)
            return 1;
        else
            return region + 1;
        break;
    case '6':
        if (region == 6)
            return 1;
        else
            return region + 1;
        break;
    case '9':
        if (region == 9)
            return 1;
        else
            return region + 1;
        break;
    }
    return 0;
}

bool
Conference720p::is_region_max_for_layout (const int region)
{
    switch (layout_)
    {
    case '1':
        if (region == 1)
            return true;
        break;
    case '2':
        if (region == 2)
            return true;
        break;
    case '4':
        if (region == 4)
            return true;
        break;
    case '6':
        if (region == 6)
            return true;
        break;
    case '9':
        if (region == 9)
            return true;
        break;
    }
    return false;
}

int
Conference720p::find_clicked_region (const char *resolution, int layout, int posX, int posY)
{
    int maxX;
    int maxY;

    if (strcmp (resolution, "cif") == 0)
    {
        maxX = 352;
        maxY = 240;
    }
    else if (strcmp (resolution, "vga") == 0)
    {
        maxX = 640;
        maxY = 480;
    }
    else if (strcmp (resolution, "720p") == 0)
    {
        maxX = 1280;
        maxY = 720;
    }
    else
    {
        LOGERROR ("Unsupported resolution - " << resolution);
        return NULL;
    }

    // 1,2,4,6,9 determine the region number returned
    if (layout == 1)
    {
        if ((posX >= 0 && posX <= maxX) && (posY >= 0 && posY <= maxY))
        {
            return 1;
        }
        else
        {
            return NULL;
        }
    }
    else if (layout == 2)
    {
        // Get a positive result when clicking on black bars above and below
        // conferee, but maybe that's not bad.  This is the only layout
        // with unused real estate
        if ((posX >= 0 && posX <= maxX * .50) && (posY >= 0 && posY <= maxY))
        {
            return 1;
        }
        else if ((posX >= .50 * maxX && posX <= maxX) && (posY >= 0 && posY <= maxY))
        {
            return 2;
        }
        else
        {
            return NULL;
        }
    }
    else if (layout == 4)
    {
        if ((posX >= 0 && posX <= maxX * .50) && (posY >= 0 && posY <= maxY * .50))
        {
            return 1;
        }
        else if ((posX >= 0 && posX <= maxX * .50) && (posY >= maxY * .50 && posY <= maxY))
        {
            return 2;
        }
        else if ((posX >= .50 * maxX && posX <= maxX) && (posY >= 0 && posY <= maxY * .50))
        {
            return 3;
        }
        else if ((posX >= .50 * maxX && posX <= maxX) && (posY >= maxY * .50 && posY <= maxY))
        {
            return 4;
        }
        else
        {
            return NULL;
        }
    }
    else if (layout == 6)
    {
        if ((posX >= 0 && posX <= maxX * .66) && (posY >= 0 && posY <= maxY * .66))
        {
            return 1;
        }
        else if ((posX >= .66 * maxX && posX <= maxX) && (posY >= 0 && posY <= maxY * .33))
        {
            return 2;
        }
        else if ((posX >= .66 * maxX && posX <= maxX) && (posY >= maxY * .33 && posY <= maxY * .66))
        {
            return 3;
        }
        else if ((posX >= .66 * maxX && posX <= maxX) && (posY >= maxY * .66 && posY <= maxY))
        {
            return 4;
        }
        else if ((posX >= .33 * maxX && posX <= .66 * maxX) && (posY >= maxY * .66 && posY <= maxY))
        {
            return 5;
        }
        else if ((posX >= 0 && posX <= .33 * maxX) && (posY >= maxY * .66 && posY <= maxY))
        {
            return 6;
        }
        else
        {
            return NULL;
        }
    }
    else if (layout == 9)
    {
        if ((posX >= 0 && posX <= maxX * .33) && (posY >= 0 && posY <= maxY * .33))
        {
            return 1;
        }
        else if ((posX >= 0 && posX <= maxX * .33) && (posY >= .33 * maxY && posY <= maxY * .66))
        {
            return 2;
        }
        else if ((posX >= 0 && posX <= maxX * .33) && (posY >= .66 * maxY && posY <= maxY))
        {
            return 3;
        }
        else if ((posX >= .33 * maxX && posX <= .66 * maxX) && (posY >= 0 && posY <= .33 * maxY))
        {
            return 4;
        }
        else if ((posX >= .33 * maxX && posX <= .66 * maxX) && (posY >= maxY * .33 && posY <= .66 * maxY))
        {
            return 5;
        }
        else if ((posX >= .33 * maxX && posX <= .66 * maxX) && (posY >= maxY * .66 && posY <= maxY))
        {
            return 6;
        }
        else if ((posX >= .66 * maxX && posX <= maxX) && (posY >= 0 && posY <= .33 * maxY))
        {
            return 7;
        }
        else if ((posX >= .66 * maxX && posX <= maxX) && (posY >= maxY * .33 && posY <= .66 * maxY))
        {
            return 8;
        }
        else if ((posX >= .66 * maxX && posX <= maxX) && (posY >= maxY * .66 && posY <= maxY))
        {
            return 9;
        }
        else
        {
            return NULL;
        }
    }
    return 0;
}

void
Conference720p::resetDemo ()
{
    // The Grand Reset
    // Stop plays, destroy conference, throw everybody out
    // Next call after this will start a new conference
    LOGDEBUG ("720p Conference reset");
    //Loop over all videos, stopping them
    ConfVideoPlays::Instance ()->stopAllConfVideoPlays ();

    // Destroy the conference.  This will automatically remove conference parties first
    destroy_conference (conf_id_);
    conf_id_ = "";

    //  Loop over all open calls and hang them up.
    std::vector < Call >::iterator call_iterator;
    for (call_iterator = Calls::Instance ()->verification_calls_.begin ();
         call_iterator != Calls::Instance ()->verification_calls_.end (); call_iterator++)
    {
        LOGDEBUG ("Hanging up call " << call_iterator->getCallId ());
        hangup (call_iterator->getCallId ());
        decNumCallers ();
        int region = call_iterator->getConfRegion ();
        LOGDEBUG ("Relinquishing conference region " << region);
        clear_region (region);
    }
    // Nobody left; clear the list of all calls
    Calls::Instance ()->clearAllCalls ();
    // Resets for a  new conference on next call
    // Note that destroying a conference will internally kill
    // all overlays associated with the conference.  Saves
    // the app the trouble.
    resetConference ();
}

void
Conference720p::rotate_to_next_region ()
{
    // Loop over all possible regions
    set_all_regions_not_processed ();
    const char *displacedPlayFromRegionOne = NULL;
    const char *displacedCallFromRegionOne = NULL;
    bool wrappedFromMaxToOne = false;
    for (int curRegion = 9; curRegion >= 1; curRegion--)
    {
        LOGDEBUG ("Looking at region " << curRegion << " Used: " << is_region_used (curRegion) <<
                  "  Not Processed: " << is_region_not_processed (curRegion) <<
                  " Wrapped from max to one: " << wrappedFromMaxToOne);
        //if (is_region_used (curRegion) && is_region_not_processed (curRegion))
        if (is_region_used (curRegion))
        {
            if (is_region_not_processed (curRegion))
            {
                const char *callId = Calls::Instance ()->getCallIdByConfRegion (curRegion);
                LOGDEBUG ("Examine region " << curRegion << " for Call Id. Have: " << callId);
                if (callId != NULL)
                {
                    LOGDEBUG ("Call in region " << curRegion);
                    int nextRegion;
                    char nextRegionString[10];

                    // If this is region one and we wrapped something in here from the
                    // max region for the layout, take what we saved and put it in region 2
                    // instead of what was wrapped into here
                    if (curRegion == 1 && wrappedFromMaxToOne)
                    {
                        LOGDEBUG ("Displaced region one call = " << displacedCallFromRegionOne);
                        if (displacedCallFromRegionOne)
                        {
                            LOGDEBUG ("Restoring displaced call from region one into region two");
                            if (Calls::Instance ()->isAudioMuteOnForCallId (callId))
                            {
                                update_party (displacedCallFromRegionOne, "recvonly", "sendrecv", "2");
                            }
                            else
                            {
                                update_party (displacedCallFromRegionOne, "sendrecv", "sendrecv", "2");
                            }
                            set_region (2);
                            Calls::Instance ()->setCallRegionByCallId (displacedCallFromRegionOne, 2);
                            if (Calls::Instance ()->getCallIdByConfRegion (2) != NULL)
                            {
                                // Adjust overlays for region2 
                                LOGDEBUG ("AUDIOON region 2 = " << Calls::
                                          Instance ()->isAudioMuteOnForCallId ((Calls::Instance ()->getCallIdByConfRegion (2))));
                                LOGDEBUG ("VIDEOHIDDEN region 2 = " << Calls::
                                          Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (2)));
                                if (Calls::Instance ()->isAudioMuteOnForCallId (Calls::Instance ()->getCallIdByConfRegion (2)))
                                {
                                    std::string showMicMute = showMicMuteOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, showMicMute.c_str ());
                                }
                                else
                                {
                                    std::string deleteMicMute = deleteMicMuteOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, deleteMicMute.c_str ());
                                }
                                if (Calls::Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (2)))
                                {
                                    std::string showBaghead = showBagheadOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, showBaghead.c_str ());
                                }
                                else
                                {
                                    std::string delBaghead = deleteBagheadOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, delBaghead.c_str ());
                                }
                            }

                            if (Calls::Instance ()->getCallIdByConfRegion (1) != NULL)
                            {
                                // Adjust overlays for region 1
                                LOGDEBUG ("AUDIOON region 1 = " << Calls::
                                          Instance ()->isAudioMuteOnForCallId ((Calls::Instance ()->getCallIdByConfRegion (1))));
                                LOGDEBUG ("VIDEOHIDDEN region 1 = " << Calls::
                                          Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (1)));
                                if (Calls::Instance ()->isAudioMuteOnForCallId (Calls::Instance ()->getCallIdByConfRegion (1)))
                                {
                                    std::string showMicMute = showMicMuteOverlay (1);
                                    update_conference (conf_id_, NULL, NULL, showMicMute.c_str ());
                                }
                                else
                                {
                                    std::string deleteMicMute = deleteMicMuteOverlay (1);
                                    update_conference (conf_id_, NULL, NULL, deleteMicMute.c_str ());
                                }
                                if (Calls::Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (1)))
                                {
                                    std::string showBaghead = showBagheadOverlay (1);
                                    update_conference (conf_id_, NULL, NULL, showBaghead.c_str ());
                                }
                                else
                                {
                                    std::string delBaghead = deleteBagheadOverlay (1);
                                    update_conference (conf_id_, NULL, NULL, delBaghead.c_str ());
                                }
                            }
                        }
                        LOGDEBUG ("Displaced region one play = " << displacedPlayFromRegionOne);
                        if (displacedPlayFromRegionOne)
                        {
                            LOGDEBUG ("Restoring displaced play from region one into region two");
                            update_play (displacedPlayFromRegionOne, NULL, "2");
                            set_region (2);
                            ConfVideoPlays::Instance ()->setConfVideoPlayRegionByPlayId (displacedPlayFromRegionOne, 2);
                        }
                        // Done with region one; exit loop
                        break;
                    }

                    // Special case if wrapping around from last to first region
                    if (is_region_max_for_layout (curRegion))
                    {
                        wrappedFromMaxToOne = true;
                        // If region one is occupied, save its contents
                        LOGDEBUG ("Saving whatever is in region one (if anyithing) as displaced");
                        displacedCallFromRegionOne = Calls::Instance ()->getCallIdByConfRegion (1);
                        displacedPlayFromRegionOne = ConfVideoPlays::Instance ()->getVideoPlayIdByConfRegion (1);
                    }
                    // Normal region processing
                    nextRegion = get_next_region (curRegion);
                    LOGDEBUG ("Video call with call ID " << callId << " in region " << curRegion << " moving to region " <<
                              nextRegion);
                    moveRegionOverlays (curRegion, nextRegion);
                    Calls::Instance ()->setCallRegionByCallId (callId, nextRegion);
                    clear_region (curRegion);
                    set_region (nextRegion);
                    set_region_processed (curRegion);
                    sprintf (nextRegionString, "%d", nextRegion);

                    // Know when we wrapped 
                    if (is_region_max_for_layout (curRegion))
                        wrappedFromMaxToOne = true;

                    update_party (callId, NULL, NULL, nextRegionString);
                }
                else
                {
                    LOGDEBUG ("Look for play in region" << curRegion);
                    const char *playId = ConfVideoPlays::Instance ()->getVideoPlayIdByConfRegion (curRegion);
                    LOGDEBUG ("Examine region " << curRegion << " for Play Id. Have: " << playId);
                    if (playId != NULL)
                    {
                        LOGDEBUG ("Play found in region " << curRegion);
                        int nextRegion;
                        char nextRegionString[10];
                        // If this is region one and we wrapped something in here from the
                        // max region for the layout, take what we saved and put it in region 2
                        // instead of what was wrapped into here
                        if (curRegion == 1 && wrappedFromMaxToOne)
                        {
                            LOGDEBUG ("Displaced region one call = " << displacedCallFromRegionOne);
                            if (displacedCallFromRegionOne)
                            {
                                LOGDEBUG ("Duct Tape 2");
                                LOGDEBUG ("Restoring displaced call from region one into region two");
                                // JH duct tape for disappearing party on restore to region 2
                                if (Calls::Instance ()->isAudioMuteOnForCallId (callId))
                                    update_party (displacedCallFromRegionOne, "recvonly", "sendrecv", "2");
                                else
                                    update_party (displacedCallFromRegionOne, "sendrecv", "sendrecv", "2");
                                set_region (2);
                                Calls::Instance ()->setCallRegionByCallId (displacedCallFromRegionOne, 2);
                                if (Calls::Instance ()->isAudioMuteOnForCallId (Calls::Instance ()->getCallIdByConfRegion (2)))
                                {
                                    LOGDEBUG ("Turn on mic mute overlay in region 2");
                                    std::string showMicMute = showMicMuteOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, showMicMute.c_str ());
                                }
                                else
                                {
                                    std::string deleteMicMute = deleteMicMuteOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, deleteMicMute.c_str ());
                                }
                                if (Calls::Instance ()->isVideoHiddenForCallId (Calls::Instance ()->getCallIdByConfRegion (2)))
                                {
                                    // Move the video hiding overlay to the new region
                                    LOGDEBUG ("Turn on video hide overlay in region 2");
                                    std::string showBaghead = showBagheadOverlay (2);
                                    update_conference (conf_id_, NULL, NULL, showBaghead.c_str ());
                                }
                            }
                            LOGDEBUG ("Displaced region one play = " << displacedPlayFromRegionOne);
                            if (displacedPlayFromRegionOne)
                            {
                                LOGDEBUG ("Restoring displaced play from region one into region two");
                                update_play (displacedPlayFromRegionOne, NULL, "2");
                                set_region (2);
                                ConfVideoPlays::Instance ()->setConfVideoPlayRegionByPlayId (displacedPlayFromRegionOne, 2);
                            }
                            // Done with region one; exit loop
                            break;
                        }
                        // Special case if wrapping around from last to first region
                        if (is_region_max_for_layout (curRegion))
                        {
                            wrappedFromMaxToOne = true;
                            LOGDEBUG ("Saving whatever is in region one (if anyithing) as displaced");

                            // If region one is occupied, save its contents
                            displacedCallFromRegionOne = Calls::Instance ()->getCallIdByConfRegion (1);
                            displacedPlayFromRegionOne = ConfVideoPlays::Instance ()->getVideoPlayIdByConfRegion (1);
                        }
                        // Normal processing
                        nextRegion = get_next_region (curRegion);
                        LOGDEBUG ("Video play with play ID " << playId << " in region " << curRegion << " moving to region "
                                  << nextRegion);
                        ConfVideoPlays::Instance ()->setConfVideoPlayRegionByPlayId (playId, nextRegion);
                        clear_region (curRegion);
                        set_region (nextRegion);
                        set_region_processed (curRegion);
                        sprintf (nextRegionString, "%d", nextRegion);
                        //  int update_play( const char * media_id, const char * action, const char * region )

                        update_play (playId, NULL, nextRegionString);
                    }
                }
            }
        }
    }
}

void
Conference720p::onEvent (xmsEventParser * eventParser)
{
    std::string eventType = eventParser->getEventType ();
    LOGDEBUG ("Conference720p app handling event");
    if (eventType == "incoming")

    {
        std::string call_id = eventParser->findValByKey ("call_id");

        if (isVeryFirstCall ())
        {
            // On first call, create a conference, reserving no resources,
            // with a max of 9 parties, 4 tiles, 720p resolution
            conf_id_ = create_conference ("0", "9", "4", "720p");
            LOGDEBUG ("First call - created new conference " << conf_id_);
            didVeryFirstCall ();
        }

        incNumCallers ();
        if (getNumCallers () > 6)
        {
            LOGDEBUG ("Exceeding six callers allowed in 720p Conference demo. Not accepting call.");
            hangup (call_id);
            decNumCallers ();
            return;
        }
        // All calls go into conference
        LOGDEBUG ("Answering call " << call_id);
        answer (call_id, getDtmfMode ());
    }
    else if (eventType == "answered")
    {
        LOGDEBUG ("Answered event received");

        // Call goes into next open conference tile/region
        std::string call_id = eventParser->findValByKey ("call_id");
        int region = get_next_open_region ();
        char region_string[10];
        sprintf (region_string, "%d", region);
        // Who's where bookkeeping
        Calls::Instance ()->setConfRegionById (call_id.c_str (), region);

        LOGDEBUG ("Adding party to conference " << conf_id_ << " in region " << region);
        add_party (call_id, conf_id_, region_string);
        // JH - add error handling
        if (0)
        {
            decNumCallers ();
            hangup (call_id);
        }
        // Turn on this guy's caption maybe?
        if (areCaptionsOn ())
            turnOnCaption (region);
    }
    else if (eventType == "accepted")
    {
        LOGDEBUG ("Accepted event received. No action taken");
    }
    else if (eventType == "hangup")
    {
        LOGDEBUG ("Hangup event received");
        decNumCallers ();
        std::string call_id = eventParser->findValByKey ("call_id");

        int region = Calls::Instance ()->getConfRegionByCallId (call_id.c_str ());
        if (Calls::Instance ()->isAudioMuteOnForCallId (call_id.c_str ()))
        {
            // Get rid of muted microphone overlay
            LOGDEBUG ("Removing mic mute overlay from region " << region);
            std::string deleteMicMute = deleteMicMuteOverlay (region);
            update_conference (conf_id_, NULL, NULL, deleteMicMute.c_str ());
        }
        if (Calls::Instance ()->isVideoHiddenForCallId (call_id.c_str ()))
        {
            // Get rid of video hiding overlay
            LOGDEBUG ("Removing video hiding overlay from region " << region);
            std::string delBaghead = deleteBagheadOverlay (region);
            update_conference (conf_id_, NULL, NULL, delBaghead.c_str ());
        }
        LOGDEBUG ("Relinquishing conference region " << region);
        clear_region (region);
        LOGDEBUG ("Removing " << call_id << " from active call list");
        Calls::Instance ()->delCall (call_id.c_str ());
        if (areCaptionsOn ())
            turnOffCaption (region);
        if (getNumCallers () == 0)
        {
            LOGDEBUG ("Last caller leaving conference");
            if (scrollingOverlayOn ())
            {
                LOGDEBUG ("Turning scrolling overlay off");
                std::string delTicker = deleteStockTickerOverlay (0);
                update_conference (conf_id_, NULL, NULL, delTicker.c_str ());
            }
            if (slideShowOn ())
            {
                LOGDEBUG ("Turning slide show off");
                std::string delSlide = deleteSlideOverlay (0);
                update_conference (conf_id_, NULL, NULL, delSlide.c_str ());
                setSlideShowOff ();
            }
            if (strlen (getExclusiveMediaOp ()) != 0)
            {
                LOGDEBUG ("Stopping exclusive media op");
                stop (conf_id_, getExclusiveMediaOp ());
            }
            // Destroy conference and reset so a new one is started for next caller
            LOGDEBUG ("Destroy conference " << conf_id_ << " and reset for a new one");
            destroy_conference (conf_id_);
            conf_id_ = "";
            resetConference ();
        }
    }
    else if (eventType == "dtmf")
    {
        LOGDEBUG ("DTMF event received");
        // JH - want to go over DTMF use, make saner. Maybe use INFO messages?
        std::string digit = eventParser->findValByKey ("digits");
        if (digit == "1")
        {
            if (strlen (getExclusiveMediaOp ()) == 0)
            {
                LOGDEBUG ("Recording conference for 60 seconds max");
                // Notify all callers of record in progress
                notify_all_callers ("720p Conference now being recorded...");
                // Put a recording icon on the screen
                std::string showMicOn = showMicOnOverlay (0);
                update_conference (conf_id_, NULL, NULL, showMicOn.c_str ());
                std::string media_id = record_conference (conf_id_,
                                                          "file://restconfdemo/conf_recording.wav",
                                                          "audio/x-wav",
                                                          "L16",
                                                          "16000",
                                                          "file://restconfdemo/conf_recording.vid",
                                                          "video/x-vid", "h264", "3.1", "720", "1280", "1536000", "30", "60s");
                if (!media_id.empty ())
                {
                    setExclusiveMediaOp (media_id.c_str ());
                    setRecordInProgress ();
                    LOGDEBUG ("Saved media ID " << getExclusiveMediaOp () << " for conference record");
                }
            }
            else
            {
                LOGDEBUG ("Full screen media operation in progress...");
            }
        }
        else if (digit == "C")
        {
            if (isRecordInProgress ())
            {
                LOGDEBUG ("Stopping conference record");
                stop (conf_id_, getExclusiveMediaOp ());
                setRecordNotInProgress ();
            }
        }
        else if (digit == "2")
        {
            if (strlen (getExclusiveMediaOp ()) == 0)
            {
                if (file_exists
                    ("/var/lib/xms/media/en-US/restconfdemo/conf_recording.wav")
                    && file_exists ("/var/lib/xms/media/en-US/restconfdemo/conf_recording.vid"))
                {
                    LOGDEBUG ("Playing conference recording into next open region");
                    int region = get_next_open_region ();
                    char region_string[10];
                    sprintf (region_string, "%d", region);
                    LOGDEBUG ("Adding video to region: " << region);
                    LOGDEBUG ("Replaying last conference recording");
                    std::string media_id = play_into_conf (conf_id_,
                                                           "conf_recording.wav",
                                                           "audio/x-wav",
                                                           "file://restconfdemo",
                                                           "conf_recording.vid",
                                                           "video/x-vid", "file://restconfdemo", region_string, "0");
                    ConfVideoPlay recordingPlay (conf_id_, media_id.c_str (), region);
                    ConfVideoPlays::Instance ()->addNewPlay (recordingPlay);
                    ConfVideoPlays::Instance ()->printConfVideoPlayList ();
                }
                else
                {
                    LOGDEBUG ("Full screen media operation in progress...");
                }
            }
            else
            {
                LOGWARN ("Please record a conference before trying to replay");
            }
        }
        else if (digit == "3")
        {
            if (file_exists
                ("/var/lib/xms/media/en-US/restconfdemo/conf_recording.wav")
                && file_exists ("/var/lib/xms/media/en-US/restconfdemo/conf_recording.vid"))
            {
                if (strlen (getExclusiveMediaOp ()) == 0)
                {
                    LOGDEBUG ("Replaying last conference recording into full screen");
                    std::string media_id = play_into_conf (conf_id_,
                                                           "conf_recording.wav",
                                                           "audio/x-wav",
                                                           "file://restconfdemo",
                                                           "conf_recording.vid", "video/x-vid", "file://restconfdemo", "0", "0");
                    setExclusiveMediaOp (media_id.c_str ());
                    if (areCaptionsOn ())
                    {
                        std::string showVidLabel = showVideoLabelOverlay (0);
                        update_conference (conf_id_, NULL, NULL, showVidLabel.c_str ());
                    }
                }
                else
                {
                    LOGDEBUG ("Full screen media operation in progress...");
                }
            }
            else
            {
                LOGWARN ("Please record a conference before trying to replay");
            }
        }
        else if (digit == "4")
        {
            if (file_exists
                ("/var/lib/xms/media/en-US/restconfdemo/Dialogic_NetworkFuel.wav")
                && file_exists ("/var/lib/xms/media/en-US/restconfdemo/Dialogic_NetworkFuel.vid"))
            {
                if (strlen (getExclusiveMediaOp ()) == 0)
                {
                    LOGDEBUG ("Playing Network Fuel video into next open region");
                    int region = get_next_open_region ();
                    char region_string[10];
                    sprintf (region_string, "%d", region);
                    LOGDEBUG ("Adding video to region: " << region);
                    std::string media_id = play_into_conf (conf_id_,
                                                           "Dialogic_NetworkFuel.wav",
                                                           "audio/x-wav",
                                                           "file://restconfdemo",
                                                           "Dialogic_NetworkFuel.vid", "video/x-vid",
                                                           "file://restconfdemo", region_string, "infinite");
                    ConfVideoPlay recordingPlay (conf_id_, media_id.c_str (), region);
                    ConfVideoPlays::Instance ()->addNewPlay (recordingPlay);
                    ConfVideoPlays::Instance ()->printConfVideoPlayList ();
                    if (areCaptionsOn ())
                    {
                        std::string showVidLabel = showVideoLabelOverlay (region);
                        update_conference (conf_id_, NULL, NULL, showVidLabel.c_str ());
                    }
                }
                else
                {
                    LOGDEBUG ("Full screen media operation in progress...");
                }
            }
            else
            {
                LOGWARN ("Dialogic_NetworkFuel media not found");
            }
        }
        else if (digit == "5")
        {
            if (file_exists
                ("/var/lib/xms/media/en-US/restconfdemo/sintel_short_clip.wav")
                && file_exists ("/var/lib/xms/media/en-US/restconfdemo/sintel_short_clip.vid"))
            {
                if (strlen (getExclusiveMediaOp ()) == 0)
                {
                    LOGDEBUG ("Playing Sintel video into next open region");
                    int region = get_next_open_region ();
                    char region_string[10];
                    sprintf (region_string, "%d", region);
                    LOGDEBUG ("Adding video to region: " << region);
                    std::string media_id = play_into_conf (conf_id_,
                                                           "sintel_short_clip.wav",
                                                           "audio/x-wav",
                                                           "file://restconfdemo",
                                                           "sintel_short_clip.vid",
                                                           "video/x-vid", "file://restconfdemo", region_string, "infinite");
                    ConfVideoPlay sintelPlay (conf_id_, media_id.c_str (), region);
                    ConfVideoPlays::Instance ()->addNewPlay (sintelPlay);
                    ConfVideoPlays::Instance ()->printConfVideoPlayList ();
                    if (areCaptionsOn ())
                    {
                        std::string showVidLabel = showVideoLabelOverlay (region);
                        update_conference (conf_id_, NULL, NULL, showVidLabel.c_str ());
                    }
                }
                else
                {
                    LOGDEBUG ("Full screen media operation in progress...");
                }
            }
            else
            {
                LOGWARN ("sintel_short_clip media not found");
            }
        }
        else if (digit == "6")
        {
            if (file_exists
                ("/var/lib/xms/media/en-US/restconfdemo/Dialogic_NetworkFuel.wav")
                && file_exists ("/var/lib/xms/media/en-US/restconfdemo/Dialogic_NetworkFuel.vid"))
            {
                if (strlen (getExclusiveMediaOp ()) == 0)
                {
                    LOGDEBUG ("Stopping any ongoing region videos");
                    ConfVideoPlays::Instance ()->stopAllConfVideoPlays ();
                    LOGDEBUG ("Playing Network Fuel into full conference screen");
                    std::string media_id = play_into_conf (conf_id_,
                                                           "Dialogic_NetworkFuel.wav",
                                                           "audio/x-wav",
                                                           "file://restconfdemo",
                                                           "Dialogic_NetworkFuel.vid",
                                                           "video/x-vid", "file://restconfdemo", "0", "infinite");
                    setExclusiveMediaOp (media_id.c_str ());
                }
                else
                {
                    LOGDEBUG ("Full screen media operation in progress...");
                }
            }
            else
            {
                LOGWARN ("Dialogic_NetworkFuel media not found");
            }
        }
        else if (digit == "7")
        {
            if (file_exists
                ("/var/lib/xms/media/en-US/restconfdemo/sintel_short_clip.wav")
                && file_exists ("/var/lib/xms/media/en-US/restconfdemo/sintel_short_clip.vid"))
            {
                if (strlen (getExclusiveMediaOp ()) == 0)
                {
                    LOGDEBUG ("Playing Sintel video into full conference screen");
                    std::string media_id = play_into_conf (conf_id_,
                                                           "sintel_short_clip.wav",
                                                           "audio/x-wav",
                                                           "file://restconfdemo",
                                                           "sintel_short_clip.vid",
                                                           "video/x-vid", "file://restconfdemo", "0", "infinite");
                    setExclusiveMediaOp (media_id.c_str ());
                }
                else
                {
                    LOGDEBUG ("Full screen media operation in progress");
                }
            }
            else
            {
                LOGWARN ("sintel_short_clip media not found");
            }
        }
        else if (digit == "8")
        {
            LOGDEBUG ("Stopping all media play operations");
            ConfVideoPlays::Instance ()->stopAllConfVideoPlays ();
            if (strlen (getExclusiveMediaOp ()) != 0)
            {
                LOGDEBUG ("Stopping exclusive media operation");
                stop (conf_id_, getExclusiveMediaOp ());
            }
            if (areCaptionsOn ())
            {
                turnOffVideoLabels ();
            }
        }
        else if (digit == "B")
        {
            if (!scrollingOverlayOn ())
            {
                //  Scrolling overlay is on full conference screen "0"
                LOGDEBUG ("Displaying scrolling overlay");
                setScrollingOverlayOn ();
                std::string showTicker = showStockTickerOverlay (0);
                update_conference (conf_id_, NULL, NULL, showTicker.c_str ());
            }
            else
                LOGDEBUG ("Scrolling overlay already on");
        }
        else if (digit == "E")
        {
            if (scrollingOverlayOn ())
            {
                LOGDEBUG ("Turning scrolling overlay off");
                setScrollingOverlayOff ();
                std::string delTicker = deleteStockTickerOverlay (0);
                update_conference (conf_id_, NULL, NULL, delTicker.c_str ());
            }
            else
                LOGDEBUG ("Scrolling overlay not on");
        }
        else if (digit == "F")
        {
            if (!slideShowOn ())
            {
                LOGDEBUG ("Displaying silde show");
                setSlideShowOn ();
                // Scrolling overlay is on full conference screen "0"
                std::string slide = showSlideOverlay (0, 1);
                update_conference (conf_id_, NULL, NULL, slide.c_str ());
            }
            else
                LOGDEBUG ("Slide show already on");
        }
        else if (digit == "G")
        {
            if (slideShowOn ())
            {
                LOGDEBUG ("Turning slide show off");
                std::string delShow = deleteSlideOverlay (0);
                update_conference (conf_id_, NULL, NULL, delShow.c_str ());
                setSlideShowOff ();
            }
            else
                LOGDEBUG ("Slide show not on");
        }
        else if (digit == "9")
        {
            LOGDEBUG ("Turning conferee captions on");
            setCaptionsOn ();
            turnOnAllCaptions ();
        }
        else if (digit == "0")
        {
            LOGDEBUG ("Turning conferee captions off");
            setCaptionsOff ();
            turnOffAllCaptions ();
        }
        else if (digit == "#")
        {
            LOGDEBUG ("Changing conference layout");
            // Rotating over all layouts - 4, 6, 9 
            //Standard regions
            update_conference (conf_id_, get_next_layout (), NULL, NULL);
        }
        else if (digit == "*")
        {
            LOGDEBUG ("Rotating conference layout");
            // Rotate all conferees through the "next" region  - 0->1, 1->2,
            // 2->4, 4->6, 6->9, 9->0, etc.
            if (areCaptionsOn ())
                turnOffAllCaptions ();
            rotate_to_next_region ();
            if (areCaptionsOn ())
                turnOnAllCaptions ();
        }
        else if (digit == "D")
        {
            // The Grand reset
            // Stop plays, destroy conference, throw everybody out
            // Next call after this will start a new conference
            resetDemo ();
        }
        else if (digit == "A")
        {
            // Up for grabs...
        }
        else
        {
            LOGWARN ("Unhandled DTMF entered");
        }
    }
    else if (eventType == "end_play")
    {
        LOGDEBUG ("End play event received");
        if (ConfVideoPlays::Instance ()->areAnyConfPlaysActive ())
        {
            // Mark region cleared and update play list
            std::string play_id = eventParser->findValByKey ("transaction_id");
            int region = ConfVideoPlays::Instance ()->getConfRegionByPlayId (play_id.c_str ());
            clear_region (region);
            ConfVideoPlays::Instance ()->clearConfRegionByPlayId (play_id.c_str ());
            ConfVideoPlays::Instance ()->delConfVideoPlay (play_id.c_str ());
            ConfVideoPlays::Instance ()->printConfVideoPlayList ();

            // If captions are on
            if (areCaptionsOn ())
            {
                std::string deleteVidLabel = deleteVideoLabelOverlay (region);
                update_conference (conf_id_, NULL, NULL, deleteVidLabel.c_str ());
            }
        }
        if (strlen (getExclusiveMediaOp ()) != 0)
        {
            // Mark the Exclusive media operation as complete
            nullExclusiveMediaOp ();
        }
    }
    //restelse if (strcmp (evtype, XMS_EVENT_END_RECORD) == 0)
    else if (eventType == "end_record")

    {
        LOGDEBUG ("End record event received");
        notify_all_callers ("720p Conference recording terminated");
        // Remove recording icon from screen
        std::string deleteMicOn = deleteMicOnOverlay (0);
        update_conference (conf_id_, NULL, NULL, deleteMicOn.c_str ());

        // Mark the Exclusive media operation as complete
        nullExclusiveMediaOp ();
    }
    else if (eventType == "conf_overlay_expired")

    {
        LOGDEBUG ("End overlay event received. Do the next slide in the rotating show");
        std::string contentId = eventParser->findValByKey ("content_id");
        if (contentId == "slide1")
        {
            std::string slide = showSlideOverlay (0, 2);
            update_conference (conf_id_, NULL, NULL, slide.c_str ());
        }
        else if (contentId == "slide2")
        {
            std::string slide = showSlideOverlay (0, 3);
            update_conference (conf_id_, NULL, NULL, slide.c_str ());
        }
        else
        {
            std::string delShow = deleteSlideOverlay (0);
            update_conference (conf_id_, NULL, NULL, delShow.c_str ());
            std::string slide = showSlideOverlay (0, 1);
            update_conference (conf_id_, NULL, NULL, slide.c_str ());
        }
    }
    else if (eventType == "info")

    {
        LOGDEBUG ("Info event received");
        std::string msg = eventParser->findValByKey ("content");
        std::string infoCallId = eventParser->findValByKey ("call_id");

        // JH - change use of msg and infoCallId to std::string functions below

        if (strstr (msg.c_str (), "CLICK") != NULL)
        {
            // Mouse Click event on conference region
            // Parse space-separated list with: CLICK,posX,posY,function, username
            // where function:
            //              mute - audio mute, put semi-transparent small muted mic icon over conferee
            //              hide - audio mute, put full size "shhhh, quiet!" overlay over conferee
            //              2nd hide/mute toggles audio mute or hide to full on
            int posX, posY;
            char type[16], function[16], userName[16];
            sscanf (msg.c_str (), "%s%d%d%s%s", type, &posX, &posY, function, userName);
            LOGDEBUG ("Mouse click at " << posX << "," << posY << " function: " << function << " User name: " << userName);

                /*** If the click is from an Android, simply interpret the click as "mute/unmute me"
                if (strcmp (userName, "android") == 0)
                {
                    if (Calls::Instance ()->isAudioMuteOnForCallId (infoCallId))
                    {
                        update_party (infoCallId, "sendrecv", "sendrecv", NULL, NULL, NULL);
                        Calls::Instance ()->setAudioUnmutedByCallId (infoCallId);
                        LOGDEBUG ("Setting call ID " << infoCallId << " to unmuted ");
                    }
                    else
                    {
                        update_party (infoCallId, "recvonly", "recvonly", NULL, NULL, NULL);
                        Calls::Instance ()->setAudioMutedByCallId (infoCallId);
                        LOGDEBUG ("Setting call ID " << infoCallId << " to muted ");
                    }
                    return;
                }  ***/

            // Otherwise, we need to be more clever - get the region clicked and see if the conference
            // controller is the clicker, and allow or not allow                            
            int region = find_clicked_region ("720p", get_cur_layout (),
                                              posX, posY);
            LOGDEBUG ("Layout is " << get_cur_layout () << " so region " << region << " was clicked");
            if (region != 0)
            {
                const char *regionCallId = Calls::Instance ()->getCallIdByConfRegion (region);
                if (regionCallId != NULL)
                {
                    if (strcmp (function, "mute") == 0)
                    {
                        // Toggle audio mute/unmute
                        // Allow conference controller to shut anybody up. Otherwise you can
                        // only shut yourself up
                        if ((strcmp (userName, "c") == 0) ||
                            (strcmp (userName, "controller") == 0) || (strcmp (userName, "ctrlr") == 0) ||
                            (strcmp (infoCallId.c_str (), regionCallId) == 0))
                        {
                            if (Calls::Instance ()->isAudioMuteOnForCallId (regionCallId))
                            {
                                std::string deleteMicMute = deleteMicMuteOverlay (region);
                                update_conference (conf_id_, NULL, NULL, deleteMicMute.c_str ());
                                update_party (regionCallId, "sendrecv", "sendrecv", NULL);
                                Calls::Instance ()->setAudioUnmutedByCallId (regionCallId);
                                LOGDEBUG ("Setting call ID " << regionCallId << " to unmuted ");
                            }
                            else
                            {
                                std::string showMicMute = showMicMuteOverlay (region);
                                update_conference (conf_id_, NULL, NULL, showMicMute.c_str ());
                                update_party (regionCallId, "recvonly", "sendrecv", NULL);
                                Calls::Instance ()->setAudioMutedByCallId (regionCallId);
                                LOGDEBUG ("Setting call ID " << regionCallId << " to muted ");
                            }
                        }
                        else
                        {
                            LOGDEBUG ("Caller with call ID " << infoCallId << " cannot mute call in region " << region);
                        }
                    }
                    else if (strcmp (function, "hide") == 0)
                    {
                        // Toggle video hide 
                        // Allow conference controller shut off anyone. Otherwise you can
                        // only shut yourself off
                        if ((strcmp (userName, "c") == 0) ||
                            (strcmp (userName, "controller") == 0) || (strcmp (userName, "ctrlr") == 0) ||
                            (strcmp (infoCallId.c_str (), regionCallId) == 0))
                        {
                            if (Calls::Instance ()->isVideoHiddenForCallId (regionCallId))
                            {
                                // Get rid of overlay hiding stream
                                std::string delBaghead = deleteBagheadOverlay (region);
                                update_conference (conf_id_, NULL, NULL, delBaghead.c_str ());
                                Calls::Instance ()->setVideoVisibleByCallId (regionCallId);
                                LOGDEBUG ("Turning video back on for call ID " << regionCallId);
                            }
                            else
                            {
                                // Display an overlay to blot out video stream
                                std::string showBaghead = showBagheadOverlay (region);
                                update_conference (conf_id_, NULL, NULL, showBaghead.c_str ());
                                Calls::Instance ()->setVideoHiddenByCallId (regionCallId);
                                LOGDEBUG ("Overlaying baghead for call ID " << regionCallId);
                            }
                        }
                        else
                        {
                            LOGDEBUG ("Caller with call ID " << infoCallId << " cannot hide video in region " << region);
                        }
                    }
                    else
                    {
                        LOGWARN ("Invalid mouse click function: " << function);
                    }
                }
                else
                {
                    LOGDEBUG ("No active call in region " << region);
                }
            }
            else
            {
                LOGDEBUG ("Mouse clicked outside of conference screen");
            }
        }
        else
        {
            LOGWARN ("Unknown INFO message received");
        }
    }                           // end if INFO event
    else if (eventType == "alarm")

    {
        //rest const char *alarm = xms_param_find (event, XMS_KEY_ALARM);
        //rest const char *alarmState = xms_param_find (event, XMS_KEY_STATE);
        std::string alarmType = eventParser->findValByKey ("alarm");
        std::string alarmState = eventParser->findValByKey ("state");

        LOGWARN ("Alarm event " << alarmType << " " << alarmState << " received");
        // Possible strategy:
        //
        // If we get an RTCP alarm On, can assume something bad
        // has happened and hang up the call. Perform the Grand Reset
        // Stop plays, destroy conference, throw everybody out
        // Next call after this will start a new conference.
        // RTP alarms can occur and mean nothing - may only indcate a 
        // muted call; ignore that
        //
        //if (strcmp (alarm, "rtcp-timeout") == 0 && strcmp (alarmState, "on") == 0)
        //    resetDemo ();
    }
    else if (eventType == "stream")

    {
        LOGDEBUG ("Stream event received");
    }
    else
    {
        LOGWARN ("Unknown event received");
    }
}                               // end OnEvent

//}

/*
 * vim:ts=4:set nu: EOF 
 */
