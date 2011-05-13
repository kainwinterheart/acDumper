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

#ifndef ACDUMPER_CPP
#define ACDUMPER_CPP

#include "main.h"
#include "threads.h"

#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;
using namespace pcrecpp;
using namespace rude;

acWatcher* watcher;

void sigHandle( int sig )
{
	watcher -> Deactivate();
	watcher -> log( "Caught signal " + ToString( sig ) +
					", stopping " + ToString( watcher -> currentTasks ) +
					" job(s) and exiting, please wait." );
}

// Look in the task list for a new job
acMultiDim* acDumper::lookForJob()
{
	acMultiDim* jobList = new acMultiDim;

	for( int i = 0; i < acConfig -> getNumSections(); i++ )
	{
		const char* section = acConfig -> getSectionNameAt( i );

		if( !IsNull( section ) )
		{
			acConfig -> setSection( section );

			const char* jobTime = acConfig -> getStringValue( "jobtime" );
			const char* status  = acConfig -> getStringValue( "status" );

			if( ToString( status ) != ToString( JOB_STATUS_ACTIVE ) )
				if ( isItNow( ToString( jobTime ), atoi( status ) ) ) jobList -> push_dim1( ToString( section ) );
		}
	}

	return jobList;
}

// Parse jobTime and ensure it's time to run the job
bool acDumper::isItNow( string jobTime, unsigned int lastTime )
{
	RE_Options *reopt = new RE_Options;

	reopt -> set_caseless( false );
	reopt -> set_utf8( true );

	string tempJobTime = jobTime;

	// Don't touch this regexp!!!!111111111!111111!!11!!oneoneoneeleven
	RE *re1 = new RE( "(\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) (\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) (\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) (\\*|[0-9]|[0-9][0-9]|\\*/[0-9]|\\*/[0-9][0-9]) (\\*|[A-Z][a-z][a-z])", *reopt );

	re1 -> GlobalReplace( "", &tempJobTime );

	delete re1;
	delete reopt;

	if( !tempJobTime.empty() ) return false;

	vector<string> dateBits = split( jobTime, ' ' );

	// Switching to false will force function to always return false
	bool result = true;

	/* Well, actually I don't remember what I've tried to do in these lines of code...*/
	time_t uepoch;
	struct tm tmepoch;
	memset( &tmepoch, 0, sizeof tmepoch );

	tmepoch.tm_year = 1970 - 1900;
	tmepoch.tm_mon  = 1 - 1;
	tmepoch.tm_mday = 1;
	tmepoch.tm_hour = 0;
	tmepoch.tm_min  = 0;
	tmepoch.tm_sec  = 0;

	uepoch = mktime( &tmepoch );
	uepoch = ( time_t )startTime - uepoch;

	memset( &tmepoch, 0, sizeof tmepoch );
	/* ********************************* */

	// It's the time jobTime is compared with
	tm * hrTime = gmtime ( &uepoch );

	const char* dows[ 7 ] = { "Sun\0", "Mon\0", "Tue\0", "Wed\0", "Thu\0", "Fri\0", "Sat\0" };

	bool approved[ 5 ] = { false, false, false, false, false };
	bool isInTestTime[ 4 ] = { false, false, false, false };

	unsigned int timePassed = startTime - lastTime;
	unsigned int testTime = 0;

	// Minute
	string min = dateBits[ 0 ];
	if( min == "*" ) approved[ 0 ] = true;
	else
	{
		if( indexOf( min.c_str(), "/" ) > -1 )
		{
			min = split( min, '/' )[ 1 ];
			testTime += ( atoi( min.c_str() ) * 60 );
			isInTestTime[ 0 ] = true;
		} else if( atoi( min.c_str() ) == hrTime -> tm_min ) approved[ 0 ] = true;
	}

	// Hour
	string hour = dateBits[ 1 ];
	if( hour == "*" ) approved[ 1 ] = true;
	else {
		if( indexOf( hour.c_str(), "/" ) > -1 )
		{
			hour = split( hour, '/' )[ 1 ];
			testTime += ( atoi( hour.c_str() ) * 60 * 60 );
			isInTestTime[ 1 ] = true;
		} else if( atoi( hour.c_str() ) == ( hrTime -> tm_hour % 24 ) ) approved[ 1 ] = true;
	}

	// Day of month
	string dom = dateBits[ 2 ];
	if( dom == "*" ) approved[ 2 ] = true;
	else {
		if( indexOf( dom.c_str(), "/" ) > -1 )
		{
			dom = split( dom, '/' )[ 1 ];
			testTime += ( atoi( dom.c_str() ) * 24 * 60 * 60 );
			isInTestTime[ 2 ] = true;
		} else if( atoi( dom.c_str() ) == hrTime -> tm_mday ) approved[ 2 ] = true;
	}

	// Month
	string mon = dateBits[ 3 ];
	if( mon == "*" ) approved[ 3 ] = true;
	else {
		if( indexOf( mon.c_str(), "/" ) > -1 )
		{
			mon = split( mon, '/' )[ 1 ];
			testTime += ( atoi( mon.c_str() ) * 30 * 24 * 60 * 60 );
			isInTestTime[ 3 ] = true;
		} else if( atoi( mon.c_str() ) == ( hrTime -> tm_mon + 1 ) ) approved[ 3 ] = true;
	}

	// Day of week
	string dow = dateBits[ 4 ];
	if( dow == "*" ) approved[ 4 ] = true;
	else if( dow == dows[ hrTime -> tm_wday ] ) approved[ 4 ] = true;

	watcher -> log( ToString( timePassed ) + " > " + ToString( testTime ) );

	// If passed enough time - approving bits that has been tested
	if( timePassed > testTime )
		for( int i = 0; i < 4; i++ )
			if( isInTestTime[ i ] ) approved[ i ] = true;

	// Getting final approval
	for( int i = 0; i < 5; i++ )
	{
		if( result ) result = approved[ i ];
		if( !result ) break;
	}

	dateBits.erase( dateBits.begin() );
	vector<string>().swap( dateBits );

	return result;
}

// Returns path to files of current task, or zero-length string if can't access path
string acDumper::getSaveDir()
{
	if( !saveDir.empty() ) return saveDir;

	const char* separator = "/";
	string _saveDir = ToString( task_outdir );

	RE_Options *reopt = new RE_Options;
	reopt -> set_caseless( true );
	reopt -> set_utf8( true );

	RE *re1 = new RE( "\\\\$", *reopt );
	RE *re2 = new RE( "/$",  *reopt );

	re1 -> GlobalReplace( "", &_saveDir );
	re2 -> GlobalReplace( "", &_saveDir );

	delete re2;
	delete re1;
	delete reopt;

	_saveDir = _saveDir + separator + taskName + "_" + ToString( startTime ) + separator;

	if( closedir( opendir( _saveDir.c_str() ) ) == -1 )
	{
		if( mkdir( _saveDir.c_str(), 0755 ) == -1 )
		{
			mustBreak = true;
			_saveDir = "";
		}
	}

	saveDir = _saveDir;
	return saveDir;
}

// Used for processing tasks
acDumper::acDumper( const char* _taskName )
{
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

    if( fileExists( watcher -> getTaskListFile() ) )
    {
        acConfig -> load( watcher -> getTaskListFile() );
        if( !taskName.empty() )
        {
        	conn = new MYSQL;
        	isConnected = connect();
        }
    }

    watcher -> log( "Dumper for task \"" + initialTaskName + "\" initialized." );
}

// Used for LFJ
acDumper::acDumper()
{
	isConnected = false;
	taskName = "";
	initialTaskName = taskName;
	startTime = time( NULL );
    acConfig = new Config();
    if( fileExists( watcher -> getTaskListFile() ) ) acConfig -> load( watcher -> getTaskListFile() );
}

acDumper::~acDumper()
{
	if( !taskName.empty() )
	{
		acConfig -> load();
		acConfig -> setSection( initialTaskName.c_str() );
		acConfig -> setStringValue( "status", ToString( time( NULL ) ).c_str() );
		acConfig -> save();

	    watcher -> log( "Dumper for task \"" + initialTaskName + "\" uninitialized." );
	}

	mustBreak = true;
    disconnect();
    delete acConfig;
}

// Returns list of db tables and creates dump file
acMultiDim* acDumper::getTables()
{
	string filename = getSaveDir() + taskName + ".sql";
	ofstream datafile( filename.c_str(), ios::trunc );

	datafile << "SET NAMES '" + ToString( task_encoding ) + "';" << endl;
	datafile.close();

    MYSQL_RES* res = query( "show tables;" );
    if( res == NULL ) return 0;

    MYSQL_ROW* row = new MYSQL_ROW;
    acMultiDim* tableList = new acMultiDim;

	if( mysql_num_rows( res ) > 0 )
	{
		while( ( *row = mysql_fetch_row( res ) ) != NULL )
		{
			if( mustBreak ) break;
			tableList -> push_dim1( ToString( ( *row )[ 0 ] ) );
			if( mustBreak ) break;
		}
	}

	delete row;
	mysql_free_result( res );

    return tableList;
}

// Returns structure of the given table
acMultiDim* acDumper::getStructure( const char* _tableName )
{
	string tableName = ToString( _tableName );

	MYSQL_RES* res = query( "show create table " + tableName + ";" );
	if (res == NULL) return 0;

	MYSQL_ROW* row = new MYSQL_ROW;
    acMultiDim* tableStructure = new acMultiDim;

    if( ( mysql_num_rows( res ) > 0 ) && ( mysql_num_fields( res ) == 2 ) )
    {
    	while( ( *row = mysql_fetch_row( res ) ) != NULL )
    	{
    		if( mustBreak ) break;
    		tableStructure -> push_dim1( ToString( ( *row )[ 1 ] ) + ";" );
    		break;
    	}
    }

    mysql_free_result( res );

    res = query( "describe " + tableName + ";" );
    if( res == NULL ) return 0;

    if( mysql_num_rows( res ) > 0 )
    {
    	while( ( *row = mysql_fetch_row( res ) ) != NULL )
    	{
    		if( mustBreak ) break;

    		if( tableStructure -> getSize_dim2() == -1 )
    			tableStructure -> push_dim2( ToString( ( *row )[ 0 ] ) );
    		else
    			tableStructure -> set_dim2( 0, tableStructure -> get_dim2( 0 ) + ", " + ToString( ( *row )[ 0 ] ) );

    		if( mustBreak ) break;
    	}
    }

    delete row;
    mysql_free_result( res );

    return tableStructure;
}

// Returns number of saved rows, just in case
// Also saves a dump of table' data
int acDumper::saveData( string tableName, string fieldNames, string tableStructure )
{
	MYSQL_RES* res = query( "select " + fieldNames + " from " + tableName + ";" );
	if (res == NULL) return 0;

	string fieldValue = "";

	int rowCount = mysql_num_rows( res );
	int caret = 0;
	string status = "";

	RE_Options *reopt = new RE_Options;
	reopt -> set_caseless( true );
	reopt -> set_utf8( true );

	RE *re1 = new RE( "\\\\",  *reopt );
	RE *re2 = new RE( "\\\'",   *reopt );
	RE *re3 = new RE( "\\\"",  *reopt );
	RE *re4 = new RE( "\\cM",  *reopt );

	acMultiDim* tableData = new acMultiDim;
	std::ostringstream datafile;

	datafile << "DROP TABLE IF EXISTS " << tableName << ";" << endl;
	datafile << tableStructure << endl;
	watcher -> log( "Dumping table " + tableName + "..." );

	if( rowCount > 0 )
	{
		MYSQL_ROW* row = new MYSQL_ROW;

		while( ( *row = mysql_fetch_row( res ) ) != NULL )
		{
			if( mustBreak ) break;

			if( watcher -> conf_BeDaemon == 0 )
			{
				status = ToString( caret + 1 ) + "/" + ToString( rowCount );
				cout << status << "\r";
			}

			for( unsigned int i = 0; i < mysql_num_fields( res ); i++ )
			{
				if ( mustBreak ) break;

	    		fieldValue = ToString( ( *row )[ i ] );

	    		re1 -> GlobalReplace( "\\\\\\\\", &fieldValue );
	    		re2 -> GlobalReplace( "\\\\\'", &fieldValue );
	    		re3 -> GlobalReplace( "\\\\\"", &fieldValue );
	    		re4 -> GlobalReplace( "", &fieldValue );

	    		fieldValue = "\"" + fieldValue + "\"";

		    	if( ( i == 0 ) && ( caret == 0 ) )
		    		tableData -> push_dim1( fieldValue );
		    	else if( ( i == 0 ) && ( caret > 0 ) )
		    		tableData -> set_dim1( 0, fieldValue );
		    	else
		    		tableData -> set_dim1( 0, tableData -> get_dim1( 0 ) + ", " + fieldValue );

	    		( *row )[ i ] = NULL;

	    		if( mustBreak ) break;
			}

			datafile << "INSERT INTO " << tableName <<
					" (" << fieldNames << ") VALUES (" <<
					tableData -> get_dim1( 0 ) << ");" << endl;

			caret++;

			if( mustBreak ) break;
		}

		if( watcher -> conf_BeDaemon == 0 ) cout << endl;
		delete row;
	}

	delete tableData;
	mysql_free_result( res );

	delete re4;
	delete re3;
	delete re2;
	delete re1;
	delete reopt;

	if( !mustBreak )
	{
	        string filename = getSaveDir() + taskName + ".sql";
	        ofstream sqlfile( filename.c_str(), ios::app );
	        sqlfile << datafile.str();
	        sqlfile.close();
	}

    return caret;
}

MYSQL_RES* acDumper::query( string sql )
{
    mysql_real_query( conn, sql.c_str(), strlen( sql.c_str() ) );
    return mysql_store_result( conn );
}

bool acDumper::connect()
{
	if( taskName.empty() ) return false;
	if( !mysql_init( conn ) ) return false;

	loadConfig( taskName.c_str() );

	// Mark current task as active
	acConfig -> setStringValue( "status", JOB_STATUS_ACTIVE );
	acConfig -> save();

	// If current task is an alias - read missing info from original task
    if( !IsNull( task_alias ) ) loadConfig( task_alias );
    if( IsNull( task_encoding ) ) task_encoding = "utf-8";

    // If port is zero (undefined in task' config) - it'll became 3306
	if( !mysql_real_connect( conn, task_host, task_user, task_pass, task_db, task_port, NULL, 0 ) )
	{
		watcher -> log( "DB connection for task \"" + initialTaskName + "\" failed." );
		return false;
	} else
	{
		mysql_free_result( query( "set names '" + ToString( task_encoding ) + "';" ) );
		watcher -> log( "DB connection for task \"" + initialTaskName + "\" established." );
	}

	return true;
}

void acDumper::loadConfig( const char* _taskName )
{
	acConfig -> setSection( _taskName );

	if( IsNull( task_user ) ) 	 	task_user	  	= acConfig -> getStringValue( "username" );
	if( IsNull( task_pass ) ) 	 	task_pass	  	= acConfig -> getStringValue( "password" );
	if( IsNull( task_host ) ) 	 	task_host	  	= acConfig -> getStringValue( "server" );
	if( IsNull( task_db ) ) 		task_db	  		= acConfig -> getStringValue( "database" );
	if( task_port == 0 )	 	 	task_port	  	= acConfig -> getIntValue( "port" );
	if( IsNull( task_encoding ) )	task_encoding	= acConfig -> getStringValue( "encoding" );
	if( IsNull( task_alias ) ) 		task_alias	  	= acConfig -> getStringValue( "alias" );
	if( IsNull( task_outdir ) ) 	task_outdir	  	= acConfig -> getStringValue( "outdir" );
}

void acDumper::disconnect()
{
	if( isConnected ) mysql_close( conn );
	if( !taskName.empty() ) delete conn;
}

int acDumper::getStartTime()
{
	return startTime;
}

// LFJ thread' main
void* scannerThread( void* pointer )
{
	watcher -> log( "Watcher thread started." );

	while( watcher -> isActive() )
	{
		#if USE_MUTEX
			pthread_mutex_lock( &( watcher -> mutex ) );
		#endif

		// Mutex here 'cuz it reads data from task list
		if( !watcher -> isTaskActive() ) trimFile( watcher -> getTaskListFile() );
		acMultiDim* jobList = watcher -> lookForJob();

		#if USE_MUTEX
			pthread_mutex_unlock( &( watcher -> mutex ) );
		#endif

		if( jobList != NULL )
			if( jobList -> getSize_dim1() > -1 )
				for( int i = 0; i <= jobList -> getSize_dim1(); i++ )
					if( watcher -> isActive() )
					{
						while( watcher -> isPoolFull() ) sleep( watcher -> currentTasks );

						threadSetup( jobList -> get_dim1( i ).c_str(), watcher );

						if( watcher -> isTaskActive() ) sleep( watcher -> currentTasks );
						if( watcher -> isPoolFull() ) watcher -> log( "Thread pool is full, waiting for free slot..." );
					}
		for( int i = 0; i < 60; i++ )
		{
			sleep( 1 );
			if( !watcher -> isActive() ) break;
		}

		delete jobList;
	}

	watcher -> log( "Watcher thread is dead." );
	pthread_exit( NULL );
}

int main( int argc, char *argv[] )
{
	// Defined in the top of this file
	watcher = new acWatcher;

	if( argc > 1 ) watcher -> conf_BeDaemon = false;

	if( watcher -> conf_BeDaemon )
	{
		close( STDIN_FILENO );
		close( STDOUT_FILENO );
		close( STDERR_FILENO );
	}

	if( watcher -> conf_BeDaemon )
	{
		pid_t pid, sid;

		pid = fork();

		if( pid < 0 )
		{
			watcher -> log( "Can't fork." );
			delete watcher;
			exit( EXIT_FAILURE );
		} else if ( pid > 0 )
		{
			delete watcher;
			exit( EXIT_SUCCESS );
		}

        sid = setsid();

        if( sid < 0 )
        {
        	watcher -> log( "Can't get SID." );
        	delete watcher;
        	exit( EXIT_FAILURE );
        }
	}

	signal( SIGTERM, &sigHandle );
	signal( SIGINT,  &sigHandle );
	signal( SIGABRT, &sigHandle );

	watcher -> log( "Initialized." );

	if( argc > 1 )
	{
		watcher -> log( "Launched for single task." );

		watcher -> deactivateOnTaskFinish = true;
		watcher -> forceDisableMutex = true;

		threadSetup( argv[ 1 ], watcher );
	} else
	{
	// Running LFJ thread
		pthread_t watcherThread;
		pthread_create( &watcherThread, NULL, scannerThread, NULL );
	}

	// If process needs to be stopped - finish current operation first
	while( watcher -> isActive() ) sleep( 1 );
	while( watcher -> isTaskActive() ) sleep( watcher -> currentTasks );

	delete watcher;
    return EXIT_SUCCESS;
}

#endif
