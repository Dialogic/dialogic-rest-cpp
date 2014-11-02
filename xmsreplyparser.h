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

#ifndef _XMSREPLYPARSER_H
#define _XMSREPLYPARSER_H

/*------------------------------ Dependencies --------------------------------*/

#include <string>
#include <map>

#include "XmlDomDocument.h"
/*----------------------------------------------------------------------------*/

// class xmsReplyParse - parse an XMS REST reply.
// Each reply is different and needs unique parsing

enum ReplyType
{ createEventhandler, createConference, playIntoConference, recordConference };


class xmsReplyParser
{
  public:
    // ctor
    xmsReplyParser (std::string replyXml, enum ReplyType replyType)
    {
        if (replyType == createEventhandler)
        {
            doc_ = new XmlDomDocument (replyXml);
            if (doc_)
            {
                eventhandlerHref_ = doc_->getAttribute ("eventhandler_response", 0, "href");
                eventhandlerId_ = doc_->getAttribute ("eventhandler_response", 0, "identifier");
            }
            else
            {
                LOGERROR ("Invalid XML on reply to create event handler. Cannot parse");
            }
        }
        else if (replyType == createConference)
        {
            doc_ = new XmlDomDocument (replyXml);
            if (doc_)
            {
                confId_ = doc_->getAttribute ("conference_response", 0, "identifier");
            }
            else
            {
                LOGERROR ("Invalid XML on reply to create conference. Cannot parse");
            }
        }
        else if (replyType == playIntoConference)
        {
            doc_ = new XmlDomDocument (replyXml);
            if (doc_)
            {
                mediaId_ = doc_->getChildAttribute ("conference_response", 0, "play", 0, "transaction_id");
            }
            else
            {
                LOGERROR ("Invalid XML on reply to conference play. Cannot parse");
            }
        }
        else if (replyType == recordConference)
        {
            doc_ = new XmlDomDocument (replyXml);
            if (doc_)
            {
                mediaId_ = doc_->getChildAttribute ("conference_response", 0, "record", 0, "transaction_id");
            }
            else
            {
                LOGERROR ("Invalid XML on reply to conference record. Cannot parse");
            }
        }
        else
        {
            LOGERROR ("Invlaid reply type - " << replyType);
        }
    }                           // end ctor

    //dtor.
    ~xmsReplyParser ()
    {
        delete doc_;
    };

    std::string getEventhandlerHref (void)
    {
        return eventhandlerHref_;
    }

    std::string getEventhandlerId (void)
    {
        return eventhandlerId_;
    }

    std::string getConfId (void)
    {
        return confId_;
    }

    std::string getMediaId (void)
    {
        return mediaId_;
    }

  private:
    std::string eventhandlerHref_;
    std::string eventhandlerId_;
    XmlDomDocument *doc_;
    std::string confId_;
    std::string mediaId_;

};
#endif // _XMSREPLYPARSER_H
/* vim:ts=4:set nu:
 * EOF
 */
