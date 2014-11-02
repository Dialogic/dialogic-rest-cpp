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

#ifndef _CALLS_H
#define _CALLS_H

/*------------------------------ Dependencies --------------------------------*/

#include <string>
#include <vector>
#include "call.h"
/*----------------------------------------------------------------------------*/

/*!
 * \class Calls - an STL vector of all calls active in the demo
 * 	The class is a singleton.
 * 
 */
class Calls
{
  public:
    /*!
     * dtor.
     */
    ~Calls ();

    static Calls *Instance ()
    {
        if (!pInstance_)
            pInstance_ = new Calls;

        return pInstance_;
    }

    void clearAllCalls ()
    {
        LOGDEBUG ("Clearing all calls from call list");
        verification_calls_.clear ();
    }

    void addNewCall (Call newCall)
    {
        verification_calls_.push_back (newCall);
    }

    Conference720p *getAppById (const char *call_id)
    {
        std::vector < Call >::iterator call_iterator;

        if (call_id == NULL)
            return NULL;
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), call_id) == 0)
            {
                LOGDEBUG ("Event match for call id " << call_id);
                return call_iterator->getApp ();
            }
        }
        return NULL;
    }

    void setConfRegionById (const char *call_id, const int region)
    {
        std::vector < Call >::iterator call_iterator;

        if (call_id == NULL)
        {
            LOGWARN ("Bad call ID. Conf region not set");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), call_id) == 0)
            {
                LOGDEBUG ("Setting conference region for call id " << call_id << " to " << region);
                call_iterator->setConfRegion (region);
            }
        }
    }

    void clearConfRegionById (const char *call_id)
    {
        std::vector < Call >::iterator call_iterator;
        if (call_id == NULL)
        {
            LOGWARN ("Bad call ID. Conf region not cleared");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), call_id) == 0)
            {
                LOGDEBUG ("Clearing conference region for call id " << call_id);
                call_iterator->clearConfRegion ();
            }
        }
    }

    int getConfRegionByCallId (const char *call_id)
    {
        std::vector < Call >::iterator call_iterator;
        if (call_id == NULL)
        {
            LOGWARN ("Bad call ID. Conf region not returned");
            return NULL;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), call_id) == 0)
            {
                LOGDEBUG ("Conference region for call id " << call_id << " is " << call_iterator->getConfRegion ());
                return call_iterator->getConfRegion ();
            }
        }
        LOGWARN ("No match. Conf region not returned");
        return NULL;
    }

    const char *getCallIdByConfRegion (const int region)
    {
        std::vector < Call >::iterator call_iterator;
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (call_iterator->getConfRegion () == region)
            {
                LOGDEBUG ("Call ID for region " << region << " is " << call_iterator->getCallId ());
                return call_iterator->getCallId ();
            }
        }
        return NULL;
    }

    void setCallRegionByCallId (const char *callId, int region)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Call region not set");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                LOGDEBUG ("Setting new region " << region << " for Call ID: " << call_iterator->getCallId ());
                call_iterator->setConfRegion (region);
            }
        }
    }

    bool isAudioMuteOnForCallId (const char *callId)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Unreliable flag returned");
            return false;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                return call_iterator->isAudioMuted ();
            }
        }
        LOGWARN ("No match. Unreliable flag returned");
        return false;
    }

    bool isVideoHiddenForCallId (const char *callId)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Unreliable flag returned");
            return false;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                return call_iterator->isVideoHidden ();
            }
        }
        LOGWARN ("No match. Unreliable flag returned");
        return false;
    }

    void setAudioMutedByCallId (const char *callId)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Audio mute not set");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                call_iterator->setAudioMuteOn ();
                LOGDEBUG ("Set audio mute on for " << call_iterator->getCallId ());
				return;
            }
        }
        LOGWARN ("No match. Audio mute not set");
    }

    void setVideoHiddenByCallId (const char *callId)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Video hidden not set");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                call_iterator->setVideoHidden ();
                LOGDEBUG ("Set video hidden for " << call_iterator->getCallId ());
				return;
            }
        }
        LOGWARN ("No match. Video hidden not set");
    }

    void setAudioUnmutedByCallId (const char *callId)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Audio unmute not set");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                call_iterator->setAudioMuteOff ();
                LOGDEBUG ("Set audio mute off for " << call_iterator->getCallId ());
				return;
            }
        }
        LOGWARN ("No match. Audio unmute not set");
    }

    void setVideoVisibleByCallId (const char *callId)
    {
        std::vector < Call >::iterator call_iterator;

        if (callId == NULL)
        {
            LOGWARN ("Bad call ID. Video visble not set");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), callId) == 0)
            {
                call_iterator->setVideoVisible ();
                LOGDEBUG ("Set video visible for " << call_iterator->getCallId ());
				return;
            }
        }
        LOGWARN ("No match. Video visible not set");
    }

    void delCall (const char *call_id)
    {
        std::vector < Call >::iterator call_iterator;

        if (call_id == NULL)
        {
            LOGWARN ("Bad call ID. Call cannot be deleted");
            return;
        }
        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            if (strcmp (call_iterator->getCallId (), call_id) == 0)
            {
                LOGDEBUG ("Deleting call with call ID " << call_iterator->getCallId () << " from active call list");
                verification_calls_.erase (call_iterator);
                return;
            }
        }
        LOGWARN ("No match. Call being deleted from active call list does not exist");
    }

    std::vector < Call > getCallList ()
    {
        return verification_calls_;
    }

    void printCallList ()
    {
        std::vector < Call >::iterator call_iterator;

        for (call_iterator = verification_calls_.begin (); call_iterator != verification_calls_.end (); call_iterator++)
        {
            LOGDEBUG ("Print Call list. Call ID: " << call_iterator->getCallId () << " Region: " << call_iterator->
                      getConfRegion ());
        }
    }

    std::vector < Call > verification_calls_;

  private:
    /*!
     * ctor. Hide here as class is a singleton
     */
    Calls ()
    {
    };
    static Calls *pInstance_;
};


#endif // _CALLS_H

/* vim:ts=4:set nu:
 * EOF
 */
