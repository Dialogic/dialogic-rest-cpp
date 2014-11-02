/*
 * Copyright (C) 2009 Dialogic Corp.
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

/*!
 * \file    logger.cpp
 * \brief   Simple logger for C++ applications.
 * \author  John Tarlton
 * \version 0.1
 */


/*----------------------------- Dependencies -------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "logger.h"

/*--------------------------------------------------------------------------*/

/*
 * Set the default logging level.
 */
Logger::Logger()
	: level_ (LOGLEVEL_NOTICE)
{
	pthread_mutex_init(&write_mutex_, NULL);
}


/*
 * Delete all appenders.
 */
Logger::~Logger()
{
	std::vector<Appender*>::iterator i;
	for ( i = appenders_.begin(); i != appenders_.end(); ++i )
	{
		delete *i;
	}
}


/*
 * Attach an Appender to the logger.
 */
void Logger::attachAppender( Logger::Appender* appender )
{
	appenders_.push_back(appender);
}


/*
 * Validate and set the active logging level.
 */
void Logger::setLevel( LogLevel level )
{
	if ( (level == LOGLEVEL_EMERG) ||
	     (level == LOGLEVEL_ALERT) ||
	     (level == LOGLEVEL_CRIT) ||
	     (level == LOGLEVEL_ERR) ||
	     (level == LOGLEVEL_WARN) ||
	     (level == LOGLEVEL_NOTICE) ||
	     (level == LOGLEVEL_INFO) ||
	     (level == LOGLEVEL_DEBUG) )
	{
			level_ = level;
	}
}


/*
 * Build a logging record and write it to the backend appenders.
 * Format: <TIMESTAMP><SPACE><LOGLEVEL><SPACE><MESSAGE><NEWLINE>
 */
void Logger::write( LogLevel level, const std::string& s )
{
	if ( (level > level_) || (level < LOGLEVEL_EMERG) )
	{
		return;
	}

	struct timeval tvbuf;
	gettimeofday(&tvbuf, 0);

	struct tm tmbuf;
	localtime_r(&tvbuf.tv_sec, &tmbuf);

	char timestamp[26 + 1];   /* YYYY-MM-DD HH:MM:SS.uuuuuu<NUL> */

	const char* fmt = "%Y-%m-%d %H:%M:%S";
	size_t len = strftime(timestamp, sizeof(timestamp)-7, fmt, &tmbuf);
	sprintf(&timestamp[len], ".%06ld", tvbuf.tv_usec);

	const char* loglevels[] = { "EMERG ", "ALERT ", "CRIT  ", "ERROR ", "WARN  ", "NOTICE", "INFO  ", "DEBUG " };

	std::stringstream ss;
	ss << timestamp << " "
	   << loglevels[level] << " "
	   << s << "\n";

	/* write the formatted message out to all appenders.
	 */
	pthread_mutex_lock(&write_mutex_);

	std::vector<Appender*>::const_iterator i;
	for ( i = appenders_.begin(); i != appenders_.end(); ++i )
	{
		(*i)->write(level, ss.str());
	}

	pthread_mutex_unlock(&write_mutex_);
}


/*
 * Tell each appender to restart.
 */
void Logger::restart()
{
	pthread_mutex_lock(&write_mutex_);

	std::vector<Appender*>::const_iterator i;
	for ( i = appenders_.begin(); i != appenders_.end(); ++i )
	{
		(*i)->restart();
	}

	pthread_mutex_unlock(&write_mutex_);
}


///////////////////////////////////////////////////////////////////////////////

/*!
 * ctor, if colour has been requested and the tty supports it, enable its use.
 */
ConsoleAppender::ConsoleAppender( bool colour )
	: colour_term_ (false)
{
	if ( colour )
	{
		const char* term = getenv("TERM");
		if ( term )
		{
			if ( !strcmp(term, "xterm") || !strcmp(term, "linux") )
			{
				colour_term_ = true;
			}
		}
	}
}


/*!
 * dtor, reset terminal colours.
 */
ConsoleAppender::~ConsoleAppender()
{
	if ( colour_term_ )
	{
		std::cerr << "\033[0m";
	}
}


/*
 * If enabled set the text attribute and colour.
 */
void ConsoleAppender::textcolour( ConsoleAttr attr,
                                  ConsoleColour fg,
                                  ConsoleColour bg )
{
	if ( colour_term_ )
	{
		std::cerr << "\033[" << attr << ';' << fg + 30 << ';' << bg + 40 << 'm';
	}
}


/*
 * Copy the message to stderr.
 */
void ConsoleAppender::write( Logger::LogLevel level, const std::string& msg )
{
	switch ( level )
	{
		case Logger::LOGLEVEL_EMERG:
			textcolour(BRIGHT, RED, BLACK);
			break;
		case Logger::LOGLEVEL_ALERT:
			textcolour(BRIGHT, RED, BLACK);
			break;
		case Logger::LOGLEVEL_CRIT:
			textcolour(BRIGHT, RED, BLACK);
			break;
		case Logger::LOGLEVEL_ERR:
			textcolour(DIM, RED, BLACK);
			break;
		case Logger::LOGLEVEL_WARN:
			textcolour(BRIGHT, YELLOW, BLACK);
			break;
		case Logger::LOGLEVEL_NOTICE:
			textcolour(BRIGHT, BLUE, BLACK);
			break;
		case Logger::LOGLEVEL_INFO:
			textcolour(BRIGHT, WHITE, BLACK);
			break;
		case Logger::LOGLEVEL_DEBUG:
			textcolour(DIM, WHITE, BLACK);
			break;
		default:
			break; /* unreachable */
	}

	std::cerr << msg;

	textcolour(DIM, WHITE, BLACK);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * ctor.
 */
FileAppender::FileAppender( const std::string& path,
                            const std::string& basename,
                            ssize_t max_size )
	: path_ (path),
	  basename_ (basename),
	  max_size_ (max_size)
{
	;
}


/*
 * dtor. close the file
 */
FileAppender::~FileAppender()
{
	fout_.close();
}


/*
 * Open a new file. The file is named by appending the current time stamp
 * to the base filename.
 *
 *     basename-YYYYMMDD-HHMMSS.log
 */
void FileAppender::open()
{
	time_t now = time(NULL);
	struct tm tmbuf;
	localtime_r(&now, &tmbuf);

	std::stringstream filename;
	filename << path_ << '/' << basename_ << '-';

	filename << tmbuf.tm_year + 1900;
	filename << std::setw(2) << std::setfill('0') << tmbuf.tm_mon + 1;
	filename << std::setw(2) << std::setfill('0') << tmbuf.tm_mday;
	filename << '-';
	filename << std::setw(2) << std::setfill('0') << tmbuf.tm_hour;
	filename << std::setw(2) << std::setfill('0') << tmbuf.tm_min;
	filename << std::setw(2) << std::setfill('0') << tmbuf.tm_sec;
	filename << ".log";

	fout_.open(filename.str().c_str(), std::ios::app);
}


/*
 * Close the current file and open a new one.
 */
void FileAppender::rollover()
{
	fout_.close();
	fout_.clear();

	open();
}


/*
 * Copy the message to a file.
 */
void FileAppender::write( Logger::LogLevel level, const std::string& msg )
{
	if ( !fout_.is_open() )
	{
		open();
	}

	if ( fout_.good() )
	{
		fout_ << msg;
		if ( level <= Logger::LOGLEVEL_NOTICE )
		{
			fout_.flush();
		}
		if ( (max_size_) && (fout_.tellp() > max_size_) )
		{
			rollover();
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

/*
 * ctor, open the syslog.
 */
SyslogAppender::SyslogAppender( const std::string& name )
	: name_ (name)
{
	openlog(name_.c_str(), 0, LOG_USER);
}


/*!
 * dtor, close the syslog.
 */
SyslogAppender::~SyslogAppender()
{
	closelog();
}


/*
 * Write to the syslog daemon. Ignore low severity messages.
 */
void SyslogAppender::write( Logger::LogLevel level, const std::string& msg )
{
	if ( level <= Logger::LOGLEVEL_NOTICE )
	{
		syslog(level, "%s", msg.c_str());
	}
}


/*
 * vim:ts=4:set nu:
 */

