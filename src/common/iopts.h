#ifndef _IOPTS_H_
#define _IOPTS_H_

#include "iobjs.h"

struct IOpts : public Interface
{
	virtual const char* operator [] (const char *) = 0;
	virtual void  		LoadOptions (int argc, char * argv[]) = 0;
	virtual void  		LoadOptions (FILE *) = 0;
	virtual void  		SaveOptions (FILE *) = 0;
};

#endif
