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

class acWatcher
{
	public:
		acWatcher();
		~acWatcher();

		void startTask( const char* );
		void log( std::string );
		void Activate();
		void Deactivate();

		const char* getTaskListFile();
		const char* conf_LogFile;

		bool isActive();
		bool isTaskActive();
		bool isLFJActive();
		bool isPoolFull();
		bool deactivateOnTaskFinish;
		bool forceDisableMutex;
		bool conf_BeDaemon;

		acMultiDim* lookForJob();

		int currentTasks;
		unsigned int conf_MaxThreads;

		#if USE_MUTEX
			pthread_mutex_t mutex;
		#endif

		acZlibber* zlibber;

	private:
		void runTask( const char* );

		bool active;
		bool lfjActive;

		std::string taskListFile;
		rude::Config* acConfig;
};

#endif
