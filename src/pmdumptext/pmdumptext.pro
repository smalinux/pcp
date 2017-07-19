TEMPLATE	= app
LANGUAGE	= C++
SOURCES		= pmdumptext.cpp
CONFIG		+= qt console warn_on
INCLUDEPATH	+= ../include ../libpcp_qmc/src
release:DESTDIR	= build/debug
debug:DESTDIR	= build/release
LIBS		+= -L../libpcp/src
LIBS		+= -L../libpcp_qmc/src -L../libpcp_qmc/src/$$DESTDIR
LIBS		+= -lpcp_qmc -lpcp
win32:LIBS	+= -lwsock32 -liphlpapi
QT		-= gui
QMAKE_CXXFLAGS	+= $$(PCP_CFLAGS)
