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

#ifndef ACWATCHER_H
#define ACWATCHER_H

#include <pthread.h>
#include "zlibber.h"
#include <rude/config.h>

class acWatcher {
public:
	acWatcher();
    ~acWatcher();

    void startTask(const char*);
    void log(std::string);
    const char* getTaskListFile();

    bool isActive();
    bool isTaskActive();
    bool isLFJActive();
    bool isPoolFull();

    void Activate();
    void Deactivate();

    acMultiDim* lookForJob();

    int currentTasks;
    bool deactivateOnTaskFinish;
    bool forceDisableMutex;

    #if USE_MUTEX
    	pthread_mutex_t mutex;
	#endif

    acZlibber* zlibber;

    /* Configuration options */
    // main
    const char* conf_LogFile;
    unsigned int conf_MaxThreads;

	#ifdef _WIN32
		wchar_t* conf_ConnFile;
	#endif

	bool conf_BeDaemon;

    /* ********************* */

private:
    void runTask(const char*);
    bool active;
    bool lfjActive;
    std::string taskListFile;
    rude::Config* acConfig;
};

#endif
