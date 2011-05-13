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

#ifndef ACWATCHER_CPP
#define ACWATCHER_CPP

#include "main.h"
#include "threads.h"

using namespace std;
using namespace rude;

acWatcher::acWatcher()
{
	Activate();

	conf_LogFile           = "";
	conf_MaxThreads        = 0;
	conf_BeDaemon          = 0;
	taskListFile           = "";
	currentTasks           = 0;
	lfjActive              = false;
	deactivateOnTaskFinish = false;
	forceDisableMutex      = false;
	zlibber                = new acZlibber;
	acConfig               = new Config();

	bool usingDefaults     = false;

	if( fileExists( CONFIG ) )
	{
		acConfig -> load( CONFIG );

		acConfig -> setSection( "main" );

		conf_MaxThreads = acConfig -> getIntValue( "maxThreads" );
		conf_LogFile 	= acConfig -> getStringValue( "logFile" );

		acConfig -> setSection( "linux" );

		conf_BeDaemon = acConfig -> getBoolValue( "daemonize" );
	} else {
		usingDefaults   = true;
		conf_MaxThreads = 0;
		conf_BeDaemon   = 1;
	}

	if( !IsNull( conf_LogFile ) )
	{
		FILE* newFile = fopen( conf_LogFile, "w" );
		if( newFile != NULL ) fclose( newFile );
	}

	#if USE_MUTEX
		pthread_mutex_init( &mutex, NULL );
	#endif

	if( usingDefaults ) log( "Can't load config file, using default settings." );

	log( "Watcher alive." );
}

acWatcher::~acWatcher()
{
	#if USE_MUTEX
		pthread_mutex_destroy( &mutex );
	#endif

	currentTasks = 0;

	Deactivate();

	delete acConfig;
	delete zlibber;

	log( "Watcher dead." );
}

acMultiDim* acWatcher::lookForJob()
{
	if( ( isActive() ) && ( !isLFJActive() ) )
	{
		lfjActive = true;

		log('lfj start');
		acDumper* dumper = new acDumper();
		acMultiDim* jobList = dumper -> lookForJob();
		log('lfj end');

		delete dumper;

		lfjActive = false;
		return jobList;

	} else return ( new acMultiDim );
}

void acWatcher::log( string data )
{
	if( conf_BeDaemon == 1 )
	{
		if( IsNull( conf_LogFile ) ) return;

		time_t logTime = time( NULL );
		char timeStr[ 100 ];

		strftime( timeStr, 100, "%d-%m-%Y %H:%M:%S", localtime( &logTime ) );

		ofstream logfile ( conf_LogFile, ios::app );
		logfile << timeStr << ": " << data << endl;
		logfile.close();
	} else puts( data.c_str() );
}

const char* acWatcher::getTaskListFile()
{
	if( !taskListFile.empty() ) return taskListFile.c_str();

	string tempString = "";

	string rcFile = "~/." + ToString( TASKLIST );
	string etcFile = "/etc/" + ToString( TASKLIST );

	if( fileExists( rcFile ) ) tempString = rcFile;
	else if( fileExists( etcFile ) ) tempString = etcFile;
	else tempString = ToString( TASKLIST );

	taskListFile = tempString;

	log( "Task list file is " + tempString );

	return taskListFile.c_str();
}

/* Because it's cool xD */
bool acWatcher::isActive()
{
	return active;
}

bool acWatcher::isLFJActive()
{
	return lfjActive;
}

bool acWatcher::isTaskActive()
{
	return ( currentTasks > 0 );
}

bool acWatcher::isPoolFull()
{
	return ( ( conf_MaxThreads > 0 ) ? ( currentTasks >= ( conf_MaxThreads - 1 ) ) : false );
}

void acWatcher::Activate()
{
	active = true;
}

void acWatcher::Deactivate()
{
	active = false;
}
/* ******************** */

void acWatcher::startTask(const char* taskName)
{
	// Well, it's already mutexed...
	if( !IsNull( taskName ) ) runTask( taskName );
}

void acWatcher::runTask( const char* taskName )
{
	#if USE_MUTEX
		if( !forceDisableMutex ) pthread_mutex_lock( &mutex );
	#endif

	// Task list file manipulations...
	acDumper* dumper = new acDumper( taskName );
	log( "Task \"" + ToString( taskName ) + "\" initialized." );

	#if USE_MUTEX
		if( !forceDisableMutex ) pthread_mutex_unlock( &mutex );
	#endif

	ostringstream taskLog;

	taskLog << "[" << taskName << "]" << ( ( dumper -> isConnected ) ? " Connected" : " Connection failed" ) << endl;

	taskLog << "[" << taskName << "]" << ( ( isActive() ) ? " Active" : " Inactive" ) << endl;

	taskLog << "[" << taskName << "]" << ( ( !dumper -> mustBreak ) ? " Continuing" : " Must break" ) << endl;

	taskLog << "[" << taskName << "]" << " SaveDir is " << dumper -> getSaveDir() << endl;

	if( ( dumper -> isConnected ) && ( isActive() ) && ( !dumper -> mustBreak ) )
	{
	   	int startTime = dumper -> getStartTime();

	   	taskLog << "[" << taskName << "] Initialized in thread " << currentTasks << "." << endl;

	    acMultiDim* tableList = dumper -> getTables();

	    if( tableList != 0 )
	    for( int tableNum = 0; tableNum <= tableList -> getSize_dim1(); ++tableNum )
	    {
	    	taskLog << "[" << taskName << "] Receiving data from " << tableList -> get_dim1( tableNum ) << "..." << endl;

	      	if( !isActive() ) dumper -> mustBreak = true;

	      	acMultiDim* tableStructure = dumper -> getStructure( tableList -> get_dim1( tableNum ).c_str() );

	      	if( tableStructure == 0 )
	      	{
	      		taskLog << "\tError reading structure." << endl;
	      		continue;
	      	}

	      	if( !isActive() ) dumper -> mustBreak = true;

	        dumper -> saveData( tableList -> get_dim1( tableNum ), tableStructure -> get_dim2( 0 ) , tableStructure -> get_dim1( 0 ) );

	        delete tableStructure;

		    if( !isActive() ) break;
	    }

	    delete tableList;

	    taskLog << "[" << taskName << "] Finished in " << ( time( NULL ) - startTime ) << " seconds." << endl;

	} else taskLog << "[" << taskName << "] Initialization failed." << endl;

	#if USE_MUTEX
		if( !forceDisableMutex ) pthread_mutex_lock( &mutex );
	#endif

	// Also task list file manipulations.
	delete dumper;
	log( "Task \"" + ToString( taskName ) + "\" finished." );

	#if USE_MUTEX
		if( !forceDisableMutex ) pthread_mutex_unlock( &mutex );
	#endif

	string outName = dumper -> getSaveDir() + ToString( taskName );

	// Yeah, it's scary, but I had errors with pointers here and was sleepy...again
	// Also compression begins AFTER job is marked as completed
	if( ( fileExists( ToString( outName + ".sql" ).c_str() ) ) && ( isActive() ) )
	{
		FILE* sqlFile = fopen( ToString( outName + ".sql" ).c_str(), "r" );
		FILE* zlbFile = fopen( ToString( outName + ".zlb" ).c_str(), "w" );

		SET_BINARY_MODE( sqlFile );
		SET_BINARY_MODE( zlbFile );

		taskLog << "[" << taskName << "] Compressing..." << endl;

		int startTime = time( NULL );

		zlibber -> pack( sqlFile, zlbFile, 9 );

		taskLog << "[" << taskName << "] Compression finished in " << ( time( NULL ) - startTime ) << " seconds." << endl;

		fclose( zlbFile );
		fclose( sqlFile );

		sleep( 1 );

		remove( ToString( outName + ".sql" ).c_str() );
	} else taskLog << "[" << taskName << "] Compression skipped." << endl;

	if( isActive() )
	{
		ofstream taskLogFile ( ToString( outName + ".log" ).c_str(), ios::trunc );
		taskLogFile << taskLog.str();
		taskLogFile.close();
	}
}

#endif
