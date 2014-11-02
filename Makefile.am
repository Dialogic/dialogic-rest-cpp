# PowerMedia Demonstration Application
#
bin_PROGRAMS = restconfdemo

restconfdemo_SOURCES = main.cpp \
	             appframework.cpp appframework.h \
	             conference720p.cpp conference720p.h \
	             dispatchxmscmd.cpp dispatchxmscmd.h \
		     call.h calls.h \
		     XmlDomDocument.cpp XmlDomDocument.h \
		     xmscmds.cpp xmscmds.h \
	             lib/logger.cpp lib/logger.h \
	             lib/inifile.cpp lib/inifile.h \
	             lib/getoption.h

restconfdemo_LDADD = -lpthread -lxerces-c -lcurl


restconfdemo_CPPFLAGS = -Werror -Wall -Wextra  -Wno-unused-parameter \
	                    -DNDEBUG -Wno-reorder -O2 \
	                    -I lib


restconfdemo_CFLAGS = -Werror -Wall \
	                  -I lib



