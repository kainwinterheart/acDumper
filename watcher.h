#ifndef ACWATCHER_H
#define ACWATCHER_H

#include <pthread.h>

class acWatcher {
public:
	acWatcher();
    ~acWatcher();

    void startTask(const char*);
    bool isActive();
    bool isTaskActive();
    bool isLFJActive();

    void Activate();
    void Deactivate();

    acMultiDim* lookForJob();

    int currentTasks;
    bool deactivateOnTaskFinish;
    bool forceDisableMutex;

    #if USE_MUTEX
    	pthread_mutex_t mutex;
	#endif

private:
    void runTask(const char*);
    bool active;
    bool lfjActive;
};

#endif
