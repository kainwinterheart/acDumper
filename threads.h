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

	if (watcher->isTaskActive()) sleep(watcher->currentTasks);

	#if USE_MUTEX
		if (!watcher->forceDisableMutex) pthread_mutex_lock(&(watcher->mutex));
	#endif

	watcher->currentTasks++;

	#if USE_MUTEX
		if (!watcher->forceDisableMutex) pthread_mutex_unlock(&(watcher->mutex));
	#endif

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
	//delete info;

	pthread_exit(NULL);

	#ifdef _WIN32
		return NULL;
	#endif
}

inline pthread_t threadSetup(const char* _taskName, acWatcher* watcher) {
	const size_t len = strlen(_taskName);
	char * taskName = new char[len + 1];
	strncpy(taskName, _taskName, len);
	taskName[len] = '\0';

	threadArgs* INFO = new threadArgs;
	INFO->pWatcher = watcher;
	INFO->taskName = taskName;

	if (watcher->isTaskActive()) sleep(watcher->currentTasks);

	pthread_t newThread;
	pthread_create( &newThread, NULL, threadRun, (void*)INFO );
	return newThread;
}

#endif
