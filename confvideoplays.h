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

#ifndef _CONFVIDEOPLAYS_H
#define _CONFVIDEOPLAYS_H

/*------------------------------ Dependencies --------------------------------*/

#include <string>
#include <vector>
#include "confvideoplay.h"
#include "dispatchxmscmd.h"
/*----------------------------------------------------------------------------*/

/*!
 * \class onfVideoPPLays - an STL vector of all video plays into a conference
 * 	The class is a singleton.
 * 
 */
class ConfVideoPlays
{
  public:
    /*!
     * dtor.
     */
    ~ConfVideoPlays ();

    static ConfVideoPlays *Instance ()
    {
        if (!pInstance_)
            pInstance_ = new ConfVideoPlays;

        return pInstance_;
    }

    void addNewPlay (ConfVideoPlay newPlay)
    {
        conf_video_plays_.push_back (newPlay);
    }

    void setConfRegionById (const char *play_id, const int region)
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;
        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            if (strcmp (conf_play_iterator->getConfVideoPlayId (), play_id) == 0)
            {
                LOGDEBUG ("Setting conference region for video play id " << play_id << " to " << region);
                conf_play_iterator->setConfVideoPlayRegion (region);
            }
        }
    }

    void clearConfRegionByPlayId (const char *play_id)
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;
        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            if (strcmp (conf_play_iterator->getConfVideoPlayId (), play_id) == 0)
            {
                LOGDEBUG ("Clearing conference region for play id " << play_id);
                conf_play_iterator->clearConfVideoPlayRegion ();
            }
        }
    }

    bool areAnyConfPlaysActive ()
	{
		return !conf_video_plays_.empty();
	}

    bool isConfPlayActive (const char *play_id)
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;
        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            if (strcmp (conf_play_iterator->getConfVideoPlayId (), play_id) == 0)
            {
                LOGDEBUG ("Conference play for ID " << play_id << " is active");
                return true;
            }
        }
        LOGDEBUG ("Conference play for ID " << play_id << " is not active");
        return false;
    }

    int getConfRegionByPlayId (const char *play_id)
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;
        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            if (strcmp (conf_play_iterator->getConfVideoPlayId (), play_id) == 0)
            {
                LOGDEBUG ("Conference region for play id " << play_id << " is " << conf_play_iterator->getConfVideoPlayRegion ());
                return conf_play_iterator->getConfVideoPlayRegion ();
            }
        }
        LOGWARN ("Video play being fetched from active play list does not exist");
        return NULL;
    }

    const char *getVideoPlayIdByConfRegion (const int region)
    {
        std::vector < ConfVideoPlay >::iterator play_iterator;
        for (play_iterator = conf_video_plays_.begin (); play_iterator != conf_video_plays_.end (); play_iterator++)
        {
            if (play_iterator->getConfVideoPlayRegion () == region)
            {
                LOGDEBUG ("Call ID for region " << region << " is " << play_iterator->getConfVideoPlayId ());
                return play_iterator->getConfVideoPlayId ();
            }
        }
        return NULL;
    }


    void delConfVideoPlay (const char *play_id)
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;

        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            if (strcmp (conf_play_iterator->getConfVideoPlayId (), play_id) == 0)
            {
                LOGDEBUG ("Deleting play with play ID " << conf_play_iterator->getConfVideoPlayId () << " from active play list");
                conf_video_plays_.erase (conf_play_iterator);
                return;
            }
        }
        LOGDEBUG ("Nothing in active play list for deletion");
    }

	void setConfVideoPlayRegionByPlayId (const char * playId, int region)
	{
	    std::vector < ConfVideoPlay >::iterator conf_play_iterator;

        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            if  (strcmp(conf_play_iterator->getConfVideoPlayId (), playId) == 0)
			{
            	LOGDEBUG ("Setting new region " << region << " for video Play ID: " << conf_play_iterator->getConfVideoPlayId ());
				conf_play_iterator->setConfVideoPlayRegion (region);
			}
		}
	}


    void stopAllConfVideoPlays ()
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;

        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            LOGDEBUG ("Stopping Video Play ID: " << conf_play_iterator->getConfVideoPlayId ());
	    // Issue stop command; List cleanup done when END_PLAY event is received
            stop (conf_play_iterator->getConfId(), conf_play_iterator->getConfVideoPlayId ());
        }
    }

    void clearConfVideoPlayList ()
	{
		 conf_video_plays_.clear();
    }

    void printConfVideoPlayList ()
    {
        std::vector < ConfVideoPlay >::iterator conf_play_iterator;

        for (conf_play_iterator = conf_video_plays_.begin (); conf_play_iterator != conf_video_plays_.end ();
             conf_play_iterator++)
        {
            LOGDEBUG ("COnference Video Play list item: " << conf_play_iterator->getConfVideoPlayId ());
        }
    }

    // Needs to be public; can't get to it thru accessor
    std::vector < ConfVideoPlay > conf_video_plays_;

  private:
    /*!
     * ctor. Hide here as class is a singleton
     */
    ConfVideoPlays ()
    {
    };
    static ConfVideoPlays *pInstance_;



};


#endif // _CONFVIDEOPLAYS_H

/* vim:ts=4:set nu:
 * EOF
 */
