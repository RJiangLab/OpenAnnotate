#ifndef _EUTIL_H_
#define _EUTIL_H_

#include <string>
#include "etype.h"

using namespace std;

#define PATH_INTERNAL			0X01
#define PATH_WINDOWS			0X02
#define PATH_LINUX				0X04
#define PATH_MACOS				0X08
#if defined(WINDOWS)
#define PATH_NATIVE				PATH_WINDOWS
#elif defined(LINUX)
#define PATH_NATIVE				PATH_LINUX
#elif defined(MACOS)
#define PATH_NATIVE				PATH_MACOS
#else
#error "Target operating system is not defined."
#endif


#define PATH_SEPARATOR_INTERNAL '/'
#define PATH_SEPARATOR_WINDOWS  '\\'
#define PATH_SEPARATOR_LINUX    '/'
#define PATH_SEPARATOR_MACOS    '/'
#if defined(WINDOWS)
#define PATH_SEPARATOR_NATIVE	PATH_SEPARATOR_WINDOWS
#elif defined(LINUX)
#define PATH_SEPARATOR_NATIVE	PATH_SEPARATOR_LINUX
#elif defined(MACOS)
#define PATH_SEPARATOR_NATIVE	PATH_SEPARATOR_MACOS
#else
#error "Target operating system is not defined."
#endif

#define FREE(p)					\
	if(p){						\
		free(p);				\
		p = NULL;				\
	}

#define DELONE(p)				\
	if(p){						\
		delete p;				\
		p = NULL;				\
	}

#define DELALL(p)				\
	if(p){						\
		delete [] p;			\
		p = NULL;				\
	}

#define FOPEN(f, m)				\
	sfopen(f, m)

#define FCLOSE(p)				\
	if(p){						\
		fclose(p);				\
		p = NULL;				\
	}


int		MakeDir(const char * dir);
int		MakePath(const char * path);
int		PathExists(const char * path);
char *	MakeFileName(char * name, int separator);
string  MakeFileName(const string & name, int separator);
string  GetExecPath();
string  GetDataPath();
string 	GetDirName(const string & path);
string 	GetBaseName(const string & path);
void    Sleep(unsigned const int millisecond);

#endif
