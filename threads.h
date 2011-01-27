#ifndef THREADS_H
#define THREADS_H

#include "watcher.h"

struct threadArgs {
	acWatcher* pWatcher;
	const char* taskName;
};

inline void* threadRun(void* pointer) {
	threadArgs* info = (threadArgs*)pointer;

	acWatcher* watcher = info->pWatcher;
	const char* taskName = info->taskName;

	delete info;
	if (IsNull(taskName)) pthread_exit(NULL);

	// Also just in case
	if (watcher->isTaskActive()) sleep(watcher->currentTasks);

	#if USE_MUTEX
		if (!watcher->forceDisableMutex) pthread_mutex_lock(&(watcher->mutex));
	#endif

	watcher->currentTasks++;

	#if USE_MUTEX
		if (!watcher->forceDisableMutex) pthread_mutex_unlock(&(watcher->mutex));
	#endif

	// And finally we're doing a job!
	watcher->startTask( taskName );

	#if USE_MUTEX
		if (!watcher->forceDisableMutex) pthread_mutex_lock(&(watcher->mutex));
	#endif

	watcher->currentTasks--;

	#if USE_MUTEX
		if (!watcher->forceDisableMutex) pthread_mutex_unlock(&(watcher->mutex));
	#endif

	if (watcher->deactivateOnTaskFinish) watcher->Deactivate();

	delete[] taskName;

	pthread_exit(NULL);

	#ifdef _WIN32
		return NULL;
	#endif
}

inline pthread_t threadSetup(const char* _taskName, acWatcher* watcher) {
	/* Because of these lines I CAN delete jobList in LFJ thread' main */
	const size_t len = strlen(_taskName);
	char * taskName = new char[len + 1];
	strncpy(taskName, _taskName, len);
	taskName[len] = '\0';
	/* *************************************************************** */

	threadArgs* INFO = new threadArgs;
	INFO->pWatcher = watcher;
	INFO->taskName = taskName;

	// Just in case, leave it alone
	if (watcher->isTaskActive()) sleep(watcher->currentTasks);

	pthread_t newThread;
	pthread_create( &newThread, NULL, threadRun, (void*)INFO );
	return newThread;
}

#endif
