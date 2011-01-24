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

	pthread_mutex_lock(&(watcher->mutex));
	watcher->currentTasks++;
	pthread_mutex_unlock(&(watcher->mutex));

	watcher->startTask( taskName );

	pthread_mutex_lock(&(watcher->mutex));
	watcher->currentTasks--;
	pthread_mutex_unlock(&(watcher->mutex));

	if (watcher->deactivateOnTaskFinish) watcher->Deactivate();

	pthread_exit(NULL);
}

inline pthread_t threadSetup(const char* taskName, acWatcher* watcher) {
	threadArgs* INFO = new threadArgs;
	INFO->pWatcher = watcher;
	INFO->taskName = taskName;

	if (watcher->isTaskActive()) sleep(watcher->currentTasks);

	pthread_t newThread;
	pthread_create( &newThread, NULL, threadRun, (void*)INFO );
	return newThread;
}

#endif
