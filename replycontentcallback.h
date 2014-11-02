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

#ifndef _REPLYCONTENTCALLBACK_H
#define _REPLYCONTENTCALLBACK_H

// A single definition of the callback function used by cURL to return a
// reply to an HTTP REST message

struct MemoryStruct
{
    char *
        memory;
    size_t
        size;
};

static
    size_t
replyContentCallback (void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *
        mem = (struct MemoryStruct *) userp;
    mem->memory = (char *) realloc (mem->memory, mem->size + realsize + 1);

    if (mem->memory == NULL)
    {
        // out of memory!
        LOGCRIT ("ReplyContentCallback - not enough memory (realloc returned NULL)");
        return 0;
    }

    memcpy (&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

#endif // _REPLYCONTENTCALLBACK_H

/* vim:ts=4:set nu:
 * EOF
 */

