dialogic-cplusplus-rest
=======================

Dialogic PowerMedia XMS WebRTC 720p Conferencing Demo - a C++ Restful demo showing many of XMS's video conferencing features

## Introduction ##

This is a demo that shows how to write a C++ program that will drive a PowerMedia XMS media server using XMS's RESTful application interface. 

The demo itself is a handles video conferencing at a 720p resolution and is intended to work with WebRTC clients. Many features available in video conferencing - multiple video layouts, moving conferees around on the screen, video overlays, etc. - are shown.

Perhaps more important is that is demonstrates how to write a C++ REST application. We have only published Java examples so far.

Two well-known open source libraries are used - cURL to send and receive HTTP RESTful messages and Xerces to parse the XML message payloads. The application is multi-threaded (using pthreads) with a separate event processing thread and a main worker thread containing application logic and messaging.  Incoming events from XMS are queued and then processed by the worker thread.

The demo also uses XMS's Javascript API for the web client.

**Conference Demo Features**

* 720p Video, with option for VGA for mobile devices
* Up to 6 conferees (with sufficient server horsepower)
* Multiple video plays into the conference
* Selectable conference regions – 4/6/9
* Rotating conferees through regions
* Conference recording/replay
* Captions for conferees, video plays 
* Slide Show Overlay
* Stock Ticker Overlay
* Mute/Hide for Audio/Video

## Installing and Building the Demo ##

The minimum XMS system version is 2.3, SU 1. This should be available the week of Novermber 15, 2014. Older versions will not work.

The demo can be built and run on the XMS server, or it can be build on any suitable Linux system. In either case, two additional libraries must be installed and ready for use before building the demo:

* Xercess-C++ 3.1.1 Follow the Installation/Build Instructions at http://xerces.apache.org/xerces-c/ Depending on you platform, there are different ways to install Xerces-C++, so no further instructions will be given here.
* cURL 7.39.0 Follow the instructions at http://curl.haxx.se/download.html As with Xerces, no further build install instructions will ge given here.

Autoconf and automake should be installed, as they needed to generate the makefile for the project. Below is the sequence of commands to build and install the restconfdemo executable.  Before doing so, modifications to Makefile.am will likely be necessary to find cURL and Xerces-C++ includes and libraries. restconfdemo_CPPFLAGS may need an entry like  -I/usr/src/xerces-c-3.1.1-x86_64-linux-gcc-3.4/include to locate Xerces' include files and restconfdemo_LDADD an entry like -L/usr/src/xerces-c-3.1.1-x86_64-linux-gcc-3.4/lib to locate the Xerces-C++ shared object libraries.

        > aclocal
        > autoconf
        > automake --add-missing
        > ./configure
        > make
        > make install

In addition to the C++ source code for the REST application itself, there are a set of media, html and javascript files that accompany the demo. These files must be installed on the XMS server. These are found in restconfdemo-media.tgz. Install them on the XMS server (as root) as follows:

    > cd /var
    > tar xvfz <file-location>/restconfdemo.tgz

* XMS Licensing – the 4 port verification license is adequate for up to 4 conferees, while the demo limits the number of 720p callers to 6. A 10 port license (or larger) license amy be installed if the system can handle it.  The license must include HD video.
* REST Application Registration -the application needs to be registered in the XMS Routing scrren before it will be available. Using the XMS Admin GUI, got to the Routing screen. Add the Pattern ^(sip|rtc):conf_demo_720p.* for Application app. The new entry will be at the bottom of the list. Select it and drag it up to a spot between the last MSML and first verification apps. Apply the change.


##Starting the Demo#

* Use latest Chrome browser. Firefox may work, but is has been far less tested/used.
* Start the demo executable. Options available are:
 
    ./cpprestdemo --help

    Command line options:
    -h, --help  Display this information.
    --version   Display this application's version.
    -v, --verbose   Run with maximum logging.
    -c, --console   Run as a console application.
    --log-dir   Directory used to store log files.
    -d, --dtmf-mode DTMF type - rfc2833 or sipinfo
    -a, --ip-address XMS server IP address
    -p, --port  XMS server REST messaging port

* HTTP URL for the demo is http://<xms_ip_addr>/rtcweb/restconfdemo.html  
* One user must log in as “controller”. The short form – “c” or “ctrlr” can aslo be used. Only the controller will have all conference functions available.  All other logins should just be a unique user ID and will only allow the caller to be put into the conference.  Designate only one caller as the controller.
* Allow camera and microphone use
* Using a browser from a PC should result in 720p being chosen from the system’s camera. (check small print in Status area. From a mobile device, VGA should be chosen. You may manually change requested resolution with the 720p/VGA button next to the Call button. If you change resolutions, you will have to Allow camera use again
* Name of person to call is prepopulated with “conf_demo_720p”. Hit call button.  You will be in conference.
* This is 720p – video is wide. So, expand browser window horizontally for a good fit. Also use CTRL – /CTRL+ to adjust the size of the image displayed in the browser.

##Demo Operation##
Functions available for the conference controller are listed below. Most functions are found on the pull-down menu (gear symbol) as well as a subset as icons under the video screen. Mousing over the icons will display their function.

**General Functions**

* Audio Mute – a single mouse click on a conferee’s image will bring up a disabled microphone symbol and mute the conferee.  A second single click will unmute. The conference controller can mute any conferee. A conferee can only mute/unmute only 
* Video Hide – a double mouse click on a conferee’s image will bring up a conferee-sized overlay and effectively hide the conferee.  A second double click will dismiss the hiding overlay  The conference controller can hide any conferee. A conferee can only mute/unmute himself.
* Full Screen Video – the video region can be blown up to full screen. Run the mouse over the screen and an audio/video playbar will appear. Hit the expand icon on the right side of the bar for full screen video. Press Escape to return to normal. Note that audio and video mute/hide will not work in full screen mode.
* Conference Record – start recording conference.  Recording will take place for a max of 60 seconds or until stopped. Recording in progress indicator comes up as a transparent red recording symbol and as text in status area.
* Stop Conference Record – stops conference recording.
* Replay Recording – play current conference recording  into the next open region.
* Play Recording Full Screen - play current conference recording full screen, overlaying any ongoing conference activity.
* Conference Video Playback– play videos into an ongoing conference as a conferee. Multiple concurrent plays are allowed.
* Play Network Fuel Video - play a Dialogic promotional video  into the next available region.
* Play Sintel Video - play open source computer animation  video  into the next available region.
* Play Network Fuel Full Screen - play a Dialogic promotional video in full screen, overlaying any ongoing conference activity.
* Play Sintel Video Full Screeen - play open source computer animation  video full screen, overlaying any ongoing conference activity.
* Stop Video Play – stop all ongoing video plays.

**Overlays**

A series of video overlay demos show XMS's overlay capabilities.

* Stock Ticker On – displays a simulated stock ticker using an overlay with scrolling text in the center of the bottom half of the screen.   If any conferees are in this area, they will be overwritten.
* Stock Ticker Off –simulated stock ticker disappears
* Slide Show On - – displays a 3-slide PPT presentation  in the upper left area of screen.  The image is sized to fit in the large region of a 6-region layout, so that layout should be selected and region 1 cleared by rotating any conferee out of it. If a conferee is in this area, he will be overwritten. The show will continuously show the 3 slides until it is stopped.
* Slide Show Off – stop the slide show.  
* Captions On – turn on captions for all conferees and video plays
* Captions Off – turn off captions for all conferees and video plays

**Region Control**

* Change Layout – go through three different conference layouts – 4 -> 6 -> 9 -> 4
* Rotate Conference – rotate current conferees through regions of current layout.  This looks best with 6 region layout, where there’s one large region
* Reset Conference – Stop all video plays into conference, hang all users up. The first subsequent call will go into a new 4 region conference. 

**View**

* Video Panel – show/hide the video screen
* Local Video Window – show/hide a small window showing local camera output


When the last conferee hangs up, the conference is destroyed. A new caller will start a new conference, which defaults to 4 regions, no video plays active, no ongoing record, no overlays active.

