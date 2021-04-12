#ifndef _IARGS_H_
#define _IARGS_H_

#include "iobjs.h"

struct IArgs : public Interface
{
	virtual const char* operator [] (const char *) = 0;
};

#endif
