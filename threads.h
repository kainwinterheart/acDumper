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

#ifndef THREADS_H
#define THREADS_H

#include "watcher.h"

struct threadArgs
{
	acWatcher* pWatcher;
	const char* taskName;
};

inline void* threadRun( void* pointer )
{
	threadArgs* info = ( threadArgs* )pointer;

	acWatcher* watcher = info -> pWatcher;
	const char* taskName = info -> taskName;

	delete info;

	if( IsNull( taskName ) ) pthread_exit( NULL );
	else watcher -> log( "Launched thread for task \"" + ToString( taskName ) + "\"." );

	// Also just in case
	if( watcher -> isTaskActive() ) sleep( watcher -> currentTasks );

	#if USE_MUTEX
		if( !watcher -> forceDisableMutex ) pthread_mutex_lock( &( watcher -> mutex ) );
	#endif

	watcher -> currentTasks++;

	#if USE_MUTEX
		if( !watcher -> forceDisableMutex ) pthread_mutex_unlock( &( watcher -> mutex ) );
	#endif

	// And finally we're doing a job!
	watcher -> startTask( taskName );

	#if USE_MUTEX
		if ( !watcher -> forceDisableMutex ) pthread_mutex_lock( &( watcher -> mutex ) );
	#endif

	watcher -> currentTasks--;

	#if USE_MUTEX
		if( !watcher -> forceDisableMutex ) pthread_mutex_unlock( &( watcher -> mutex ) );
	#endif

	if( watcher -> deactivateOnTaskFinish ) watcher -> Deactivate();

	watcher -> log( "Thread for task \"" + ToString( taskName ) + "\" exited." );
	delete[] taskName;

	pthread_exit( NULL );
}

inline pthread_t threadSetup( const char* _taskName, acWatcher* watcher )
{
	/* Because of these lines I CAN delete jobList in LFJ thread' main */
	const size_t len = strlen( _taskName );
	char * taskName = new char[ len + 1 ];
	strncpy( taskName, _taskName, len );
	taskName[ len ] = '\0';
	/* *************************************************************** */

	threadArgs* INFO = new threadArgs;
	INFO -> pWatcher = watcher;
	INFO -> taskName = taskName;

	// Just in case, leave it alone
	if( watcher -> isTaskActive() ) sleep( watcher -> currentTasks );

	pthread_t newThread;
	pthread_create( &newThread, NULL, threadRun, ( void* )INFO );

	return newThread;
}

#endif
