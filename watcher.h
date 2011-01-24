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
    pthread_mutex_t mutex;
    bool deactivateOnTaskFinish;

private:
    void runTask(const char*);
    bool active;
    bool lfjActive;
};

#endif
