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

#ifndef _CALL_H
#define _CALL_H

/*------------------------------ Dependencies --------------------------------*/
#include "logger.h"
#include "conference720p.h"
/*----------------------------------------------------------------------------*/


/*!
 * \class Call
 * All info pertaining to a single call leg
 */
class Call
{
  public:

    Call ()
    {
        ;
    }

    Call (std::string appname, std::string callid, Conference720p * conference720pApp)
    {
        callid_ = callid;
        appname_ = appname;
        conference720pApp_ = conference720pApp;
        confregion_ = 0;
        audioMuted_ = false;
        videoHidden_ = false;
    }

    Call (const char *appname, const char *callid, Conference720p * conference720pApp)
    {
        callid_ = callid;
        appname_ = appname;
        conference720pApp_ = conference720pApp;
        confregion_ = 0;
        audioMuted_ = false;
        videoHidden_ = false;
    }

    virtual ~ Call ()
    {;
    }

    bool operator== (Call & call)
    {
        return (call.callid_ == callid_);
    }

    const std::string & getAppName () const
    {
        return appname_;
    }

    const std::string & getCallId () const
    {
        return callid_;
    }

    const char *getCallId ()
    {
        return callid_.c_str ();
    }

    Conference720p *getApp ()
    {
        return conference720pApp_;
    }

    void setConfRegion (int region)
    {
        confregion_ = region;
    }

    void clearConfRegion ()
    {
        confregion_ = 0;
    }

    int getConfRegion ()
    {
        return confregion_;
    }

    bool isAudioMuted ()
    {
        return audioMuted_;
    }

    void setAudioMuteOn ()
    {
        audioMuted_ = true;
    }

    void setAudioMuteOff ()
    {
        audioMuted_ = false;
    }

    bool isVideoHidden ()
    {
        return videoHidden_;
    }

    void setVideoHidden ()
    {
        videoHidden_ = true;
    }

    void setVideoVisible ()
    {
        videoHidden_ = false;
    }

  private:

    std::string appname_;
    std::string callid_;
    Conference720p *conference720pApp_;
    int confregion_;
	bool audioMuted_;
	bool videoHidden_;
};

///////////////////////////////////////////////////////////////////////////////


#endif // _CALL_H

/* vim:ts=4:set nu:
 * EOF
 */
