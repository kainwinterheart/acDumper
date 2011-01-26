AUTOMAKE_OPTIONS = foreign
CXXFLAGS = -Wall -O3 `mysql_config --cflags`
LDFLAGS = `mysql_config --libs` -lrudeconfig -lz -lpcrecpp -lpthread

bin_PROGRAMS = acDumper
acDumper_SOURCES = main.cpp multidimensional.cpp watcher.cpp zlibber.cpp
