#ifndef ZLIBBER_H
#define ZLIBBER_H

#include <stdio.h>

#define SET_BINARY_MODE( file )

#define CHUNK 16384

class acZlibber
{
	public:
		acZlibber();
		~acZlibber();

		int pack( FILE*, FILE*, int );
		const char* getLastError( int );
};

#endif
