#ifndef _IPIPE_H_
#define _IPIPE_H_

#include "iobjs.h"

#define PIPE_HEAD true
#define PIPE_TAIL false

struct IPipe : public Interface
{
	virtual void* 	Alloc(int ) = 0;
	virtual void  	Free(void*) = 0;
	virtual void  	Push(void*) = 0;
	virtual void*	Pop() = 0;
    virtual void    Attach(Object*, bool = true) = 0;
    virtual void    Detach(Object*, bool = true) = 0;
    virtual int     Size(bool = true) = 0;
};

#endif
