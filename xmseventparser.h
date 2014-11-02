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

#ifndef _XMSEVENTPARSER_H
#define _XMSEVENTPARSER_H

/*------------------------------ Dependencies --------------------------------*/

#include <string>
#include <map>

#include "XmlDomDocument.h"
/*----------------------------------------------------------------------------*/

// class xmsEventParse - parse an XMS REST event, producing an STL map containing 
// all name/value pairs for a single event received from XMS

class xmsEventParser
{
  public:
    // ctor
    xmsEventParser (std::string eventXml)
    {
        std::string evtype;
        //std::cout << "Before doc creation " << evString << std::endl;
        doc_ = new XmlDomDocument (eventXml);
        if (doc_)
        {
            eventType_ = doc_->getAttribute ("event", 0, "type");
            LOGDEBUG ("Event type - " << eventType_);
            std::string key;
            std::string value;
            for (int i = 0; i < doc_->getChildCount ("event", 0, "event_data"); i++)
            {
                key = doc_->getChildAttribute ("event", 0, "event_data", i, "name");
                value = doc_->getChildAttribute ("event", 0, "event_data", i, "value");
                //LOGDEBUG ("event data " << i + 1 << " name -  " << key.c_str () << " value - " << value.c_str ());

                xmsEvent_[key] = value;
            }

            //LOGDEBUG ("Map size: " << xmsEvent_.size ());
            //for (map < std::string, std::string >::iterator ii = xmsEvent_.begin (); ii != xmsEvent_.end (); ++ii)
            //{
            //    LOGDEBUG ((*ii).first << ": " << (*ii).second);
            //}
        }
        else
        {
            LOGERROR ("Invalid event XML. Cannot parse");
        }

    }

     //dtor.
    ~xmsEventParser ()
    {
        delete doc_;
    };

    std::string & findValByKey (std::string key)
    {
        return xmsEvent_[key];
    }

    std::string & getEventType (void)
    {
        return eventType_;
    }

  private:
    std::string eventType_;
    std::map < std::string, std::string > xmsEvent_;
    XmlDomDocument *doc_;

};
#endif // _XMSEVENTPARSER_H
/* vim:ts=4:set nu:
 * EOF
 */
