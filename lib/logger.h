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
 * \file    lib/logger.h
 * \brief   Simple logger for C++ applications.
 * \author  John Tarlton
 * \version 0.1

 */

#ifndef _LOGGER_H
#define _LOGGER_H

/*----------------------------- Dependencies -------------------------------*/

#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <pthread.h>

/*--------------------------------------------------------------------------*/

/*!
 * \class Logger
 * Simple logging utility
 * Responsible for producing formatted logging messages and writing them out
 * through various backends.
 */
class Logger
{
public:

	/*!
	 * Logging levels. Equivalent to the priorities defined in syslog.h
	 * These are defined below in order of decreasing severity.
	 */
	enum LogLevel
	{
		LOGLEVEL_EMERG  = 0,  /*!< panic condition */
		LOGLEVEL_ALERT  = 1,  /*!< a condition that needs immediate attention */
		LOGLEVEL_CRIT   = 2,  /*!< critical conditions */
		LOGLEVEL_ERR    = 3,  /*!< error messages */
		LOGLEVEL_WARN   = 4,  /*!< potentially harmful situations */
		LOGLEVEL_NOTICE = 5,  /*!< normal but significant condition  */
		LOGLEVEL_INFO   = 6,  /*!< informational messages */
		LOGLEVEL_DEBUG  = 7   /*!< fine-grained info for debugging */
	};

	/*!
	 * \class Appender
	 * Appenders are used to write logging output to specific destinations such
	 * as files or the console.
	 */
	class Appender
	{
	public:

		/*!
		 * ctor
		 */
		Appender() {}

		/*!
		 * dtor
		 */
		virtual ~Appender() {}

		/*!
		 * Restart the appender.
		 */
		virtual void restart() { ; }

		/*!
		 * Output the message
		 */
		virtual void write( Logger::LogLevel level, const std::string& s ) = 0;
	};

	/*!
	 * Get a reference to the single instance of the Logger object.
	 */
	static Logger& instance()
	{
		static Logger singleton;
		return singleton;
	}

	/*!
	 * Attach and take ownership of an Appender object.
	 * This is not thread-safe, designed to be called from a single threaded
	 * environment at startup.
	 */
	void attachAppender( Logger::Appender* appender );

	/*!
	 * Set the logging level. This causes all write() operations for log levels
	 * that are less severe than 'level' to be discarded.
	 * This is not thread-safe, designed to be called from a single threaded
	 * environment at startup.
	 * \param level - one of the LogLevel constants.
	 */
	void setLevel( Logger::LogLevel level );

	/*!
	 * Write a log message.
	 * \param level - one of the LogLevel constants.
	 * \param s - the logging message.
	 */
	void write( Logger::LogLevel level, const std::string& s );

	/*!
	 * Restart the appenders.
	 */
	void restart();

private:

	/* Hide ctors/dtor.
	 */
	Logger();
	Logger( const Logger& );
	Logger& operator = ( const Logger& );

	~Logger();

private:

	LogLevel level_;
	std::vector<Appender*> appenders_;

	pthread_mutex_t write_mutex_;   /*!< ensures writes to appenders are atomic */
};

///////////////////////////////////////////////////////////////////////////////

/*!
 * \class ConsoleAppender
 * Write logging information to stderr.
 */
class ConsoleAppender : public Logger::Appender
{
public:

	/*!
	 * ctor
	 * \param colour - if true the output will use colours to highlight the
	 *                 logging level. This is only a hint and requires support
	 *                 from the terminal device.
	 */
	ConsoleAppender( bool colour = true );

	virtual ~ConsoleAppender();

	virtual void write( Logger::LogLevel level, const std::string& msg );

private:

	/*!
	 * Text attribute values for use with 'linux' and 'xterm' escape sequences.
	 */
	enum ConsoleAttr
	{
		RESET = 0,
		BRIGHT = 1,
		DIM = 2,
		UNDERLINE = 3,
		BLINK = 4,
		REVERSE = 7,
		HIDDEN = 8
	};

	/*!
	 * Base colour values for use with 'linux' and 'xterm' escape sequences.
	 * For foreground values add 30, for backgound values add 40.
	 */
	enum ConsoleColour
	{
		BLACK = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3,
		BLUE = 4,
		MAGENTA = 5,
		CYAN = 6,
		WHITE = 7
	};

	/*!
	 * Set the display attribute and colours.
	 * \param attr - attibute.
	 * \param fg - foreground (text) colour.
	 * \param bg - background colour.
	 */
	void textcolour( ConsoleAttr attr, ConsoleColour fg, ConsoleColour bg );

private:

	bool colour_term_; /*!< true if tty supports colour escape sequences */
};

///////////////////////////////////////////////////////////////////////////////

/*!
 * \class FileAppender
 */
class FileAppender : public Logger::Appender
{
public:

	/*!
	 * ctor,
	 * \param path - directory where the log file(s) are written.
	 * \param basename - the basename for each log file. The extension ".log"
	 *                   is appended automatically.
	 * \param max_size - maximum size for each file.
	 */
	FileAppender( const std::string& path,
	              const std::string& basename,
	              ssize_t max_size = 0);

	virtual ~FileAppender();

	virtual void write( Logger::LogLevel level, const std::string& msg );

	virtual void restart() { rollover(); }

private:

	FileAppender( const FileAppender& );
	FileAppender& operator = ( const FileAppender& );

	void open();
	void rollover();

private:

	std::string   path_;
	std::string   basename_;
	ssize_t       max_size_;
	std::ofstream fout_;
};

///////////////////////////////////////////////////////////////////////////////

/*!
 * \class SyslogAppender
 */
class SyslogAppender : public Logger::Appender
{
public:

	/*!
	 * ctor,
	 * \param name - identifier prepended to each message by the syslog daemon.
	 */
	SyslogAppender( const std::string& name );

	virtual ~SyslogAppender();

	virtual void write( Logger::LogLevel level, const std::string& msg );

private:

	SyslogAppender( const SyslogAppender& );
	SyslogAppender& operator = ( const SyslogAppender& );

private:

	std::string name_;
};

///////////////////////////////////////////////////////////////////////////////


/* Log macros providing a stream compatible interface. One macro per log level.
 *
 * Usage:
 *      	LOGNOTICE("Initialising...");
 *      	LOGDEBUG("Received " << count << " bytes.");
 */
#define LOGDEBUG(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_DEBUG, _ss.str()); \
} while(0)

#define LOGINFO(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_INFO, _ss.str()); \
} while(0)

#define LOGNOTICE(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_NOTICE, _ss.str()); \
} while(0)

#define LOGWARN(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_WARN, _ss.str()); \
} while(0)

#define LOGERROR(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_ERR, _ss.str()); \
} while(0)

#define LOGCRIT(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_CRIT, _ss.str()); \
} while(0)

#define LOGALERT(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_ALERT, _ss.str()); \
} while(0)

#define LOGEMERG(message) do { \
	std::stringstream _ss; \
	_ss << message; \
	Logger::instance().write(Logger::LOGLEVEL_EMERG, _ss.str()); \
} while(0)


#endif //_LOGGER_H


/*
 * vim:ts=4:set nu:
 */

