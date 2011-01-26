#ifndef ACDUMPER_CPP
#define ACDUMPER_CPP

#include "main.h"
#include "threads.h"
#include <pcrecpp.h>
#include <time.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <signal.h>
	#include <dirent.h>
	#include <sys/stat.h>
#endif

using namespace std;
using namespace pcrecpp;
using namespace rude;

acWatcher* watcher;

void sigHandle(int sig) {
	watcher->Deactivate();
	cout << "\r" << "Caught signal " << ToString(sig) << ", stopping " << watcher->currentTasks << " job(s) and exiting, please wait." << endl;
}

acMultiDim* acDumper::lookForJob() {
	acMultiDim* jobList = new acMultiDim;
	for (int i = 0; i < acConfig->getNumSections(); i++) {
		const char* section = acConfig->getSectionNameAt(i);
		if (!IsNull(section)) {
			acConfig->setSection( section );
			const char* jobTime = acConfig->getStringValue("jobtime");
			const char* status = acConfig->getStringValue("status");
			if ( ToString( status ) != ToString( JOB_STATUS_ACTIVE ) )
				if ( isItNow( ToString( jobTime ), atoi( status ) ) ) jobList->push_dim1( ToString(section) );
			//delete jobTime;
			//delete status;
			//delete section;
		}
	}
	return jobList;
}

bool acDumper::isItNow(string jobTime, unsigned int lastTime) {
	RE_Options *reopt = new RE_Options;
	reopt->set_caseless(false);
	reopt->set_utf8(true);

	string tempJobTime = jobTime;
	RE *re1 = new RE (
			"(\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) \
			(\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) \
			(\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) \
			(\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) \
			(\\*|[A-Z][a-z][a-z])", *reopt);

	re1->GlobalReplace("", &tempJobTime);
	if (tempJobTime.empty()) jobTime = tempJobTime;

	vector<string> dateBits = split(jobTime, ' ');
	if (dateBits.size() != 5) return false;

	// Switching to false will force function to always return false
	bool result = true;

	/* Well, actually I don't remember what I've tried to do in these lines of code...*/
	time_t uepoch;
	struct tm tmepoch;
	memset(&tmepoch,0,sizeof tmepoch);
	tmepoch.tm_year = 1970 - 1900;
	tmepoch.tm_mon = 1 - 1;
	tmepoch.tm_mday = 1;
	tmepoch.tm_hour = 0;
	tmepoch.tm_min = 0;
	tmepoch.tm_sec = 0;
	uepoch = mktime(&tmepoch);
	uepoch = (time_t)startTime - uepoch;
	/* ********************************* */

	tm * hrTime = gmtime ( &uepoch );

	const char* dows[7] = {"Sun\0", "Mon\0", "Tue\0", "Wed\0", "Thu\0", "Fri\0", "Sat\0"};

	bool approved[5] = {false, false, false, false, false};
	bool isInTestTime[4] = {false, false, false, false};

	unsigned int timePassed = startTime - lastTime;
	unsigned int testTime = 0;

	// Minute
	string min = dateBits[0];
	if (min == "*") approved[0] = true;
	else {
		if ( indexOf(min.c_str(), "/") > -1 ) {
			min = split(min, '/')[1];
			testTime += (atoi( min.c_str() ) * 60);
			isInTestTime[0] = true;
		} else if (atoi(min.c_str()) == hrTime->tm_min) approved[0] = true;
	}

	// Hour
	string hour = dateBits[1];
	if (hour == "*") approved[1] = true;
	else {
		if ( indexOf(hour.c_str(), "/") > -1 ) {
			hour = split(hour, '/')[1];
			testTime += (atoi( hour.c_str() ) * 60 * 60);
			isInTestTime[1] = true;
		} else if (atoi(hour.c_str()) == (hrTime->tm_hour%24)) approved[1] = true;
	}

	// Day of month
	string dom = dateBits[2];
	if (dom == "*") approved[2] = true;
	else {
		if ( indexOf(dom.c_str(), "/") > -1 ) {
			dom = split(dom, '/')[1];
			testTime += (atoi( dom.c_str() ) * 24 * 60 * 60);
			isInTestTime[2] = true;
		} else if (atoi(dom.c_str()) == hrTime->tm_mday) approved[2] = true;
	}

	// Month
	string mon = dateBits[3];
	if (mon == "*") approved[3] = true;
	else {
		if ( indexOf(mon.c_str(), "/") > -1 ) {
			mon = split(mon, '/')[1];
			testTime += (atoi( mon.c_str() ) * 30 * 24 * 60 * 60);
			isInTestTime[3] = true;
		} else if (atoi(mon.c_str()) == (hrTime->tm_mon+1)) approved[3] = true;

	}

	// Day of week
	string dow = dateBits[4];
	if (dow == "*") approved[4] = true;
	else if (dow == dows[hrTime->tm_wday]) approved[4] = true;

	// If passed enough time - approving bits that has been tested
	if (timePassed > testTime)
		for (int i = 0; i < 4; i++)
			if (isInTestTime[i]) approved[i] = true;

	// Getting final approval
	for (int i = 0; i < 5; i++) {
		if (result) result = approved[i];
		if (!result) break;
	}

	return result;
}

string acDumper::getSaveDir() {
	if (!saveDir.empty()) return saveDir;

	const char* separator = "/";
	string _saveDir = ToString( task_outdir );

	#ifdef _WIN32
		separator = "\\\\";
	#endif

	RE_Options *reopt = new RE_Options;
	reopt->set_caseless(true);
	reopt->set_utf8(true);

	RE *re1 = new RE ("\\\\$", *reopt);
	RE *re2 = new RE ("/$",  *reopt);

	re1->GlobalReplace("", &_saveDir);
	re2->GlobalReplace("", &_saveDir);

	delete re2;
	delete re1;
	delete reopt;

	_saveDir = _saveDir + separator + taskName + "_" + ToString( startTime ) + separator;

	#ifdef _WIN32
	/*int len = strlen(_saveDir.c_str())+1;
	wchar_t *wText = new wchar_t[len];
	memset(wText,0,len);
	::MultiByteToWideChar(  CP_ACP, NULL,_saveDir.c_str(), -1, wText,len );

	if (CreateDirectory(wText, NULL) == 0) {*/
	if (CreateDirectory(_saveDir.c_str(), NULL) == 0) {
		if (GetLastError() == ERROR_PATH_NOT_FOUND) {
			mustBreak = true;
			_saveDir = "";
		}
	}
	//delete[] wText;
	#else
	if (closedir(opendir(_saveDir.c_str())) == -1) {
		if (mkdir(_saveDir.c_str(), 0755) == -1) {
			mustBreak = true;
			_saveDir = "";
		}
	}
	#endif

	saveDir = _saveDir;
	return saveDir;
}

acDumper::acDumper(const char* _taskName) {
	isConnected = false;
	mustBreak = false;
	taskName = ToString( _taskName );
	initialTaskName = taskName;

    task_user 		= NULL;
    task_pass 		= NULL;
    task_host 		= NULL;
    task_db 		= NULL;
    task_port 		= 0;
    task_encoding 	= NULL;
    task_alias 		= NULL;
    task_outdir		= NULL;

    startTime = time( NULL );
    acConfig = new Config();

    if ( fileExists( CONFIG ) ) {
        acConfig->load( CONFIG );
        if (!taskName.empty()) {
        	conn = new MYSQL;
        	isConnected = connect();
        }
    }
}

acDumper::acDumper() {
	isConnected = false;
	mustBreak = false;
    acConfig = new Config();
    if ( fileExists( CONFIG ) ) acConfig->load( CONFIG );
}

acDumper::~acDumper() {
	if (!taskName.empty()) {
		acConfig->load();
		acConfig->setSection( initialTaskName.c_str() );
		acConfig->setStringValue( "status", ToString( time(NULL) ).c_str() );
		acConfig->save();
	}
	mustBreak = true;
    disconnect();
    delete acConfig;
}

acMultiDim* acDumper::getTables() {
	string filename = getSaveDir() + taskName + ".sql";
	ofstream datafile( filename.c_str(), ios::trunc );
	datafile << "SET NAMES '" + ToString(task_encoding) + "';" << endl;
	datafile.close();

    MYSQL_RES* res = query( "show tables;" );
    if (res == NULL) return 0;
    MYSQL_ROW* row = new MYSQL_ROW;
    acMultiDim* tableList = new acMultiDim;

	if( mysql_num_rows( res ) > 0 ) {
		while( ( *row = mysql_fetch_row( res ) ) != NULL ) {
			if (mustBreak) break;
			tableList->push_dim1( ToString( (*row)[0] ) );
			if (mustBreak) break;
		}
	}

	delete row;
	mysql_free_result(res);
    return tableList;
}

acMultiDim* acDumper::getStructure(const char* _tableName) {
	string tableName = ToString(_tableName);
	//delete[] _tableName;

	MYSQL_RES* res = query( "show create table " + tableName + ";" );
	if (res == NULL) return 0;
	MYSQL_ROW* row = new MYSQL_ROW;
    acMultiDim* tableStructure = new acMultiDim;

    if( ( mysql_num_rows( res ) > 0 ) && ( mysql_num_fields( res ) == 2 ) ) {
    	while( ( *row = mysql_fetch_row( res ) ) != NULL ) {
    		if (mustBreak) break;
    		tableStructure->push_dim1( ToString( (*row)[1] ) + ";" );
    		break;
    	}
    }

    mysql_free_result(res);

    res = query( "describe " + tableName + ";" );
    if (res == NULL) return 0;

    if ( mysql_num_rows( res ) > 0 ) {
    	while( ( *row = mysql_fetch_row( res ) ) != NULL ) {
    		if (mustBreak) break;
    		if ( tableStructure->getSize_dim2() == -1 ) {
    			tableStructure->push_dim2( ToString( (*row)[0] ) );
    		} else {
    			tableStructure->set_dim2( 0, tableStructure->get_dim2(0) + ", " + ToString( (*row)[0] ) );
    		}
    		if (mustBreak) break;
    	}
    }

    delete row;
    mysql_free_result(res);
    return tableStructure;
}

int acDumper::saveData(string tableName, string fieldNames, string tableStructure) {
	MYSQL_RES* res = query("select " + fieldNames + " from " + tableName + ";");
	if (res == NULL) return 0;
	MYSQL_ROW* row = new MYSQL_ROW;
	string fieldValue = "";

	int rowCount = mysql_num_rows( res );
	int caret = 0;
	string status = "";

	RE_Options *reopt = new RE_Options;
	reopt->set_caseless(true);
	reopt->set_utf8(true);

	RE *re1 = new RE ("\\\\",  *reopt);
	RE *re2 = new RE ("\\\'",   *reopt);
	RE *re3 = new RE ("\\\"",  *reopt);
	RE *re4 = new RE ("\\cM",  *reopt);

	acMultiDim* tableData = new acMultiDim;

	string filename = getSaveDir() + taskName + ".sql";
	ofstream datafile( filename.c_str(), ios::app );

	datafile << "DROP TABLE IF EXISTS " << tableName << ";" << endl;
	datafile << tableStructure << endl;

	if( rowCount > 0 ) {
		while( ( *row = mysql_fetch_row( res ) ) != NULL ) {
			if (mustBreak) break;
			//status = ToString( caret+1 ) + "/" + ToString( rowCount );
			//cout << status << "\r";

			for (unsigned int i = 0; i < mysql_num_fields( res ); i++) {
				if (mustBreak) break;
    			fieldValue = ToString( (*row)[i] );

    			re1->GlobalReplace("\\\\\\\\", &fieldValue);
    			re2->GlobalReplace("\\\\\'", &fieldValue);
    			re3->GlobalReplace("\\\\\"", &fieldValue);
    			re4->GlobalReplace("", &fieldValue);

    			fieldValue = "\"" + fieldValue + "\"";

	    		if (( i == 0 ) && (caret == 0)) {
	    			tableData->push_dim1( fieldValue );
	    		} else if (( i == 0 ) && (caret > 0)) {
	    			tableData->set_dim1( 0, fieldValue );
	    		} else {
	    			tableData->set_dim1( 0, tableData->get_dim1(0) + ", " + fieldValue );
	    		}
	    		(*row)[i] = NULL;
	    		if (mustBreak) break;
			}

			datafile << "INSERT INTO " << tableName <<
					" (" << fieldNames << ") VALUES (" <<
					tableData->get_dim1(0) << ");" << endl;

			caret++;
			if (mustBreak) break;
		}
		//cout << endl;
	}

	datafile.close();
	delete tableData;
	delete row;
	mysql_free_result(res);

	delete re4;
	delete re3;
	delete re2;
	delete re1;
	delete reopt;

    return caret;
}

MYSQL_RES* acDumper::query(string sql) {
	//cout << "[" << taskName << "] " << sql << endl;
    mysql_real_query( conn, sql.c_str(), strlen(sql.c_str()) );
    return mysql_store_result( conn );
}

bool acDumper::connect() {
	if(!mysql_init( conn )) return false;

	loadConfig( taskName.c_str() );
	acConfig->setStringValue( "status", JOB_STATUS_ACTIVE );
	acConfig->save();

    if (!IsNull(task_alias)) loadConfig( task_alias );
    if (IsNull(task_encoding)) task_encoding = "utf-8";

	if (!mysql_real_connect(conn, task_host, task_user, task_pass, task_db, task_port, NULL, 0)) {
		return false;
	} else {
		mysql_free_result(query("set names '" + ToString(task_encoding) + "';"));
	}

	return true;
}

void acDumper::loadConfig(const char* _taskName) {
	acConfig->setSection( _taskName );

	if (IsNull(task_user)) 	 	task_user	  	= acConfig->getStringValue("username");
	if (IsNull(task_pass)) 	 	task_pass	  	= acConfig->getStringValue("password");
	if (IsNull(task_host)) 	 	task_host	  	= acConfig->getStringValue("server");
	if (IsNull(task_db)) 		task_db	  		= acConfig->getStringValue("database");
	if (task_port == 0) 	 	task_port	  	= acConfig->getIntValue("port");
	if (IsNull(task_encoding))	task_encoding	= acConfig->getStringValue("encoding");
	if (IsNull(task_alias)) 	task_alias	  	= acConfig->getStringValue("alias");
	if (IsNull(task_outdir)) 	task_outdir	  	= acConfig->getStringValue("outdir");

	//delete _taskName;
}

void acDumper::disconnect() {
	if (isConnected) mysql_close( conn );
	if (!taskName.empty()) delete conn;
}

int acDumper::getStartTime() {
	return startTime;
}

void* scannerThread(void* pointer) {
	while (watcher->isActive()) {

		//try {
		#if USE_MUTEX
			pthread_mutex_lock(&(watcher->mutex));
		#endif

		acMultiDim* jobList = watcher->lookForJob();

		#if USE_MUTEX
			pthread_mutex_unlock(&(watcher->mutex));
		#endif

		if (jobList != NULL)
			if (jobList->getSize_dim1() > -1)
				for (int i = 0; i <= jobList->getSize_dim1(); i++)
					if (watcher->isActive()) {
						//pthread_join( threadSetup( jobList->get_dim1(i).c_str(), watcher ), NULL );
						threadSetup( jobList->get_dim1(i).c_str(), watcher );
						sleep(watcher->currentTasks);
					}

		//jobList->cleanup();
		//
		//} catch (char * err) {}

		for (int i = 0; i < 40; i++) {
			sleep(1);
			if (!watcher->isActive()) break;
		}

		delete jobList;
	}
	pthread_exit(NULL);

	#ifdef _WIN32
		return NULL;
	#endif
}

int main(int argc, char *argv[]) {
	#ifndef _WIN32
		signal(SIGTERM, &sigHandle);
		signal(SIGINT,  &sigHandle);
		signal(SIGABRT, &sigHandle);
	#endif

	watcher = new acWatcher;

	#ifndef _WIN32
	if (argc > 1) {
		watcher->deactivateOnTaskFinish = true;
		watcher->forceDisableMutex = true;
		threadSetup( argv[1], watcher );
	} else {
	#endif

	pthread_t watcherThread;
	pthread_create( &watcherThread, NULL, scannerThread, NULL );

	#ifndef _WIN32
	}
	#endif

	while ( watcher->isActive() ) sleep(1);
	while ( watcher->isTaskActive() ) sleep(watcher->currentTasks);

	delete watcher;
    return 0;
}

#endif
