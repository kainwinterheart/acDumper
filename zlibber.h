#ifndef ZLIBBER_H
#define ZLIBBER_H

#include <stdio.h>

#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
	#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
	#define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

class acZlibber {
public:
	acZlibber();
	~acZlibber();

	int pack(FILE*, FILE*, int);
	int unpack(FILE*, FILE*);

	const char* getLastError(int);
};

#endif
