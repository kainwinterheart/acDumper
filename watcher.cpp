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
	forceDisableMutex = false;
	zlibber = new acZlibber;

	#if USE_MUTEX
		//mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&mutex, NULL);
	#endif
}

acWatcher::~acWatcher() {
	#if USE_MUTEX
		pthread_mutex_destroy(&mutex);
	#endif
	currentTasks = 0;
	Deactivate();
	delete zlibber;
}

acMultiDim* acWatcher::lookForJob() {
	if ((isActive()) && (!isLFJActive())) {
		lfjActive = true;
		acDumper* dumper = new acDumper( "" );
		acMultiDim* jobList = dumper->lookForJob();
		delete dumper;
		lfjActive = false;
		return jobList;
	} else return (new acMultiDim);
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

	#if USE_MUTEX
		if (!forceDisableMutex) pthread_mutex_lock(&mutex);
	#endif

	acDumper* dumper = new acDumper( taskName );

	#if USE_MUTEX
		if (!forceDisableMutex) pthread_mutex_unlock(&mutex);
	#endif

	string outName = dumper->getSaveDir() + ToString( taskName );
	ofstream cout ( ToString(outName + ".log").c_str(), ios::trunc );

	if ((dumper -> isConnected) && (isActive()) && (!dumper->mustBreak) ) {
	   	int startTime = dumper -> getStartTime();
	    cout << "[" << taskName << "] Initialized in thread " << currentTasks << "." << endl;
	    acMultiDim* tableList = dumper -> getTables();

	    if (tableList != 0)
	    for (int tableNum = 0; tableNum <= tableList->getSize_dim1(); ++tableNum) {
	      	cout << "[" << taskName << "] Receiving data from " << tableList->get_dim1( tableNum ) << "..." << endl;

	      	if (!isActive()) dumper->mustBreak = true;
	      	acMultiDim* tableStructure = dumper -> getStructure( tableList->get_dim1( tableNum ).c_str() );
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

	#if USE_MUTEX
		if (!forceDisableMutex) pthread_mutex_lock(&mutex);
	#endif

	delete dumper;

	#if USE_MUTEX
		if (!forceDisableMutex) pthread_mutex_unlock(&mutex);
	#endif

	if ((fileExists(ToString(outName + ".sql").c_str())) && (isActive())) {
		FILE* sqlFile = fopen(ToString(outName + ".sql").c_str(), "r");
		FILE* zlbFile = fopen(ToString(outName + ".zlb").c_str(), "w");

		SET_BINARY_MODE(sqlFile);
		SET_BINARY_MODE(zlbFile);

		cout << "[" << taskName << "] Compressing..." << endl;
		int startTime = time( NULL );
		zlibber->pack(sqlFile, zlbFile, 9);
		cout << "[" << taskName << "] Compression finished in " << ( time( NULL ) - startTime ) << " seconds." << endl;

		fclose (zlbFile);
		fclose (sqlFile);
		remove (ToString(outName + ".sql").c_str());
	} else cout << "[" << taskName << "] Compression skipped." << endl;

	cout.close();
	//delete taskName;
}

#endif
