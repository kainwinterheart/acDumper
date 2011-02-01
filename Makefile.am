AUTOMAKE_OPTIONS = foreign
CXXFLAGS = -Wall -O3
LDFLAGS = -lrudeconfig -lz -lpcrecpp -lpthread

if CYGWIN
CXXFLAGS += -g -pipe -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4 -m32 -fasynchronous-unwind-tables -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -fno-strict-aliasing -fwrapv
LDFLAGS += -lmysql
else
CXXFLAGS += `mysql_config --cflags`
LDFLAGS += `mysql_config --libs`
endif

bin_PROGRAMS = acDumper
acDumper_SOURCES = main.cpp multidimensional.cpp watcher.cpp zlibber.cpp
