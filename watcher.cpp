#ifndef ACWATCHER_CPP
#define ACWATCHER_CPP

#include "main.h"
#include "threads.h"

using namespace std;

acWatcher::acWatcher() {
	Activate();
	currentTasks = 0;
	lfjActive = false;
	deactivateOnTaskFinish = false;
	mutex = PTHREAD_MUTEX_INITIALIZER;
}

acWatcher::~acWatcher() {
	currentTasks = 0;
	Deactivate();
}

acMultiDim* acWatcher::lookForJob() {
	if ((isActive()) && (!isLFJActive())) {
		lfjActive = true;
		acDumper* dumper = new acDumper( "" );
		//acMultiDim* jobList = dumper->lookForJob();
		//delete dumper;
		lfjActive = false;
		return NULL;// jobList;
	} else return NULL;
}

bool acWatcher::isActive() {
	return active;
}

bool acWatcher::isLFJActive() {
	return lfjActive;
}

bool acWatcher::isTaskActive() {
	return (currentTasks > 0);
}

void acWatcher::Activate() {
	active = true;
}

void acWatcher::Deactivate() {
	active = false;
}

void acWatcher::startTask(const char* taskName) {
	if ( ( !IsNull( taskName ) ) && (!isLFJActive()) ) {
		runTask( taskName );
	}
}

void acWatcher::runTask(const char* taskName) {
	// sleep(currentTasks);

	pthread_mutex_lock(&mutex);
	acDumper* dumper = new acDumper( taskName );
	pthread_mutex_unlock(&mutex);

	string outName = dumper->getSaveDir() + ToString( taskName ) + ".log";
	ofstream cout ( outName.c_str(), ios::trunc );

	if ((dumper -> isConnected) && (isActive()) && (!dumper->mustBreak) ) {
	   	int startTime = dumper -> getStartTime();
	    cout << "[" << taskName << "] Initialized in thread " << currentTasks << "." << endl;
	    acMultiDim* tableList = dumper -> getTables();

	    if (tableList != 0)
	    for (int tableNum = 0; tableNum <= tableList->getSize_dim1(); ++tableNum) {
	      	cout << "[" << taskName << "] Receiving data from " << tableList->get_dim1( tableNum ) << "..." << endl;

	      	if (!isActive()) dumper->mustBreak = true;
	      	acMultiDim* tableStructure = dumper -> getStructure( tableList->get_dim1( tableNum ) );
	      	if (tableStructure == 0) {
	      		cout << "\tError reading structure." << endl;
	      		continue;
	      	}

	      	if (!isActive()) dumper->mustBreak = true;
	        dumper -> saveData( tableList->get_dim1( tableNum ), tableStructure->get_dim2(0) , tableStructure->get_dim1(0) );
	        delete tableStructure;
		    if (!isActive()) break;
	    }

	    delete tableList;
	    cout << "[" << taskName << "] Finished in " << ( time( NULL ) - startTime ) << " seconds." << endl;
	} else {
	   	cout << "[" << taskName << "] Initialization failed." << endl;
	}

	cout.close();

	pthread_mutex_lock(&mutex);
	delete dumper;
	pthread_mutex_unlock(&mutex);
}

#endif