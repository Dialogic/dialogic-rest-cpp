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

/*------------------------------- Dependencies -------------------------------*/

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>

#include <iostream>

#include "logger.h"
#include "getoption.h"

#include "appframework.h"

/*----------------------------------------------------------------------------*/

/* PACKAGE_VERSION is defined by the autotools.
 */
#ifdef PACKAGE_VERSION
static const char *APP_VERSION = PACKAGE_VERSION;
#else
static const char *APP_VERSION = __DATE__ " " __TIME__;
#endif

static const char *APP_NAME = "restconfdemo";
static const char *APP_DESCRIPTION = "PowerMedia XMS - REST Conferencing C++ Demo Application";

static const char *PID_FILE = "/var/run/restconfdemo.pid";

// Event queue and locks
std::queue<std::string> eventQueue;
pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_t evHandlerThread;

// REST service address/port
std::string xmsAddr;

static std::string eventHandlerId;

/*!
 * \var exit_pipe
 * Pipe used by signal handlers to tell the application to exit cleanly.
 */
static int exit_pipe[2];
int term = false;
int log_restart = false;

/*!
 * Function prototype for signal handlers.
 */
typedef void (signal_handler_t) (int);

/*!
 * Helper to install signal handler functions.
 */
signal_handler_t *
setSignalHandler (int signum, signal_handler_t * handler)
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;

    struct sigaction old_sa;
    sigaction (signum, &sa, &old_sa);

    return old_sa.sa_handler;
}


/*!
 * Signal handler for termination signal.
 * Write the signal's id to the exit_pipe.
 */
void
sig_terminate (int signo)
{
    LOGDEBUG("SIGTERM received.  Shutting down application");
    // Send a signal to main thread to break it loose from waiting on
    // the event queue
    pthread_cond_signal(&queueNotEmpty);
    char sig = (char) signo;
    write (exit_pipe[1], &sig, 1);
    term = true;
}


/*!
 * Signal handler for SIGHUP.
 */
void
sig_reload (int signo)
{
    (void) signo;
	log_restart = true;
}


/*!
 * Signal handler for SIGCHLD. Call wait() to remove zombie.
 */
void
sig_child_exit (int signo)
{
    (void) signo;
    wait (NULL);
}

/*!
 * Entry point. Do the standard initialisation and pass control to the
 * application manager.
 */
int
main (int argc, char *argv[])
{
    /* Parse command line options.
     */
    GetOptions opts;
    opts.addOptionNoArg ('h', "help", "Display this information.");
    opts.addOptionNoArg ('\0', "version", "Display this applications's version.");
    opts.addOptionNoArg ('v', "verbose", "Run with maximum logging.");
    opts.addOptionNoArg ('c', "console", "Run as a console application.");
    opts.addOptionRequiredArg ('\0', "log-dir", "Directory used to store log files.");
    opts.addOptionRequiredArg ('d', "dtmf-mode", "DTMF type - rfc2833 or sipinfo");
    opts.addOptionRequiredArg ('a', "ip-address", "XMS server IP address");
    opts.addOptionRequiredArg ('p', "port", "XMS server REST messaging port");
    opts.parseOptions (argc, argv);

    if (opts.isFound ("help"))
    {
        std::cout << "Command line options:" << std::endl << opts << std::endl;
        exit (0);
    }

    if (opts.isFound ("version"))
    {
        std::cout << APP_DESCRIPTION << ". Version: " << APP_VERSION << std::endl << std::endl;

        std::cout << "Copyright (C) 2014 Dialogic Corp. " << std::endl << std::endl;
        exit (0);
    }

    /* Setup logging
     */
    if (opts.isFound ("verbose"))
    {
        Logger::instance ().setLevel (Logger::LOGLEVEL_DEBUG);
    }
    else
    {
        Logger::instance ().setLevel (Logger::LOGLEVEL_NOTICE);
    }

    std::string opt_log_dir = opts.getValue ("log-dir");
    if (opt_log_dir.empty ())
    {
        opt_log_dir = ".";
    }

    Logger::instance ().attachAppender (new FileAppender (opt_log_dir, APP_NAME, 1024 * 1024 * 4));
    Logger::instance ().attachAppender (new SyslogAppender (APP_NAME));

    if (opts.isFound ("console"))
    {
        Logger::instance ().attachAppender (new ConsoleAppender);
    }
    else
    {
        if (daemon (1, 0) == -1)        /* don't chdir, redirect stdio to /dev/null */
        {
            LOGCRIT ("main() daemon() failed " << errno);
            exit (1);
        }
        /* Create a file containing the pid of this process. This is used
         * by the system when managing this process. It also locks the file
         * ensure that only one instance of this program can be run.
         */
        int fd = open (PID_FILE, O_RDWR | O_CREAT, 0640);
        if (fd < 0)
        {
            LOGCRIT ("main() failed to open lock file: " << PID_FILE);
            exit (EXIT_FAILURE);
        }
        if (lockf (fd, F_TLOCK, 0) < 0)
        {
            /* already running */
            exit (0);
        }
        std::stringstream pid;
        pid << getpid () << std::endl;
        write (fd, pid.str ().c_str (), pid.str ().length ());
    }

    /* Create 'selfpipe' for sig handlers
     */
    pipe (exit_pipe);
    fcntl (exit_pipe[0], F_SETFL, O_NONBLOCK | fcntl (exit_pipe[0], F_GETFL));
    fcntl (exit_pipe[1], F_SETFL, O_NONBLOCK | fcntl (exit_pipe[1], F_GETFL));

    /* Signal handlers
     */
    setSignalHandler (SIGPIPE, SIG_IGN);
    setSignalHandler (SIGINT, sig_terminate);
    setSignalHandler (SIGQUIT, sig_terminate);
    setSignalHandler (SIGTERM, sig_terminate);
    setSignalHandler (SIGHUP, sig_reload);
    setSignalHandler (SIGCHLD, sig_child_exit);

    /* Log version and the command line options.
     */
    LOGNOTICE ("Starting " << APP_NAME << " " << APP_VERSION << " Built: " << __DATE__ << " " << __TIME__);

    std::map < std::string, std::string > opt_values;
    opts.getAllValues (opt_values);
    std::map < std::string, std::string >::const_iterator i;
    for (i = opt_values.begin (); i != opt_values.end (); ++i)
    {
        LOGNOTICE ("Option: " << (*i).first << ((*i).second.empty ()? "" : "=" + (*i).second));
    }

    /* Pass control to the application
     */
    AppFramework *app = new AppFramework;
    bool exit_status = app->run (opts, exit_pipe[0]);
    delete app;

    close (exit_pipe[0]);
    close (exit_pipe[1]);

    if (!exit_status)
    {
        LOGWARN ("Abnormal exit");
        return 1;
    }

    LOGNOTICE ("Normal exit");
    return 0;
}


/* vim:ts=4:set nu:
 * EOF
 */
