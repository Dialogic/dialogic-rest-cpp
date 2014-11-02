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

#ifndef _CONFVIDEOPLAY_H
#define _CONFVIDEOPLAY_H

/*------------------------------ Dependencies --------------------------------*/
#include "conference720p.h"
/*----------------------------------------------------------------------------*/


/*!
 * \class ConfVideoPlay
 * All info pertaining to a single video stream playing into a conference
 */
class ConfVideoPlay
{
  public:

    ConfVideoPlay ()
    {
        ;
    }

    ConfVideoPlay (std::string confId, std::string playid, int region)
    {
        confId_ = confId;
        playid_ = playid;
        confregion_ = region;
    }

    virtual ~ ConfVideoPlay ()
    {;
    }

    const std::string & getConfVideoPlayId () const
    {
        return playid_;
    }

    const char *getConfVideoPlayId ()
    {
        return playid_.c_str ();
    }

    const char *getConfId ()
    {
        return confId_.c_str ();
    }

    void setConfVideoPlayRegion (int region)
    {
        confregion_ = region;
    }

    void clearConfVideoPlayRegion ()
    {
        confregion_ = 0;
    }

    int getConfVideoPlayRegion ()
    {
        return confregion_;
    }

  private:

    std::string confId_;
    std::string playid_;
    int confregion_;
};

///////////////////////////////////////////////////////////////////////////////


#endif // _CONFVIDEOPLAY_H

/* vim:ts=4:set nu:
 * EOF
 */
