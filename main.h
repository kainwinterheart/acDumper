/*
Copyright (C) 2011  Kain Winterheart <http://facebook.com/kain.winterheart>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ACDUMPER_H
#define ACDUMPER_H

#define CONFIG "acDumper.conf"
#define TASKLIST "acDumperTasks.conf"

#define USE_MUTEX true
#define JOB_STATUS_ACTIVE "active"

#ifdef _WIN32
	#define CMD_KILL "DIEPLZ"
#endif

#include "multidimensional.h"

#include <fstream>
#include <sstream>

#ifdef _WIN32
	#include <winsock.h>
#endif
#include <mysql.h>
#include <rude/config.h>

/* Googled snippets */

template< typename C >
inline std::string ToString( const C& obj ) {
	std::ostringstream out;
	out << obj;
	return out.str();
}

inline bool IsNull( const char* obj ) {
	if (obj == NULL) return true;
	if (strlen(obj) > 0) {
		return false;
	} else {
		return true;
	}
}

inline std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

inline int indexOf(const char* where, const char* what) {
	const char* substr;
	int pos = -1;

	substr = strstr(where, what);
	if (!IsNull(substr)) pos = substr - where;

	return pos;
}

inline bool fileExists(const char* fileName) {
    std::ifstream ifile( fileName );
    if ( ifile.is_open() ) {
        ifile.close();
        return true;
    } else return false;
}

inline bool fileExists(std::string _fileName) {
	const char* fileName = _fileName.c_str();
    std::ifstream ifile( fileName );
    if ( ifile.is_open() ) {
        ifile.close();
        return true;
    } else return false;
}

// Won't use it, but let it be here
/*inline unsigned int daysInMonth(unsigned int month, unsigned int year) {
    return (30 + (((month & 9) == 8) || ((month & 9) == 1)) - (month == 2) - (!(((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0))) && (month == 2)));
}*/

/* **************** */

#ifdef _WIN32
// Yes, 'cuz it's windows...
inline void sleep(int seconds) {
	Sleep(seconds*1000);
}
#endif

class acDumper {
public:
    acDumper(const char*);
    acDumper();
    ~acDumper();

    acMultiDim* getTables ();
    acMultiDim* getStructure (const char*);
    int saveData (std::string, std::string, std::string);

    std::string getSaveDir();
    acMultiDim* lookForJob();
    int getStartTime();

    bool isConnected;
    bool mustBreak;

private:
    MYSQL* conn;
    rude::Config* acConfig;
    std::string saveDir;
    std::string taskName;
    std::string initialTaskName;
    int startTime;

    MYSQL_RES* query (std::string);

    bool connect();
    void disconnect();
    void loadConfig(const char*);

    bool isItNow(std::string, unsigned int);

    const char* 	task_user;
    const char* 	task_pass;
    const char* 	task_host;
    const char* 	task_db;
    unsigned int 	task_port;
    const char* 	task_encoding;
    const char* 	task_alias;
    const char* 	task_outdir;
};

#endif
