#ifndef _IANNO_H_
#define _IANNO_H_

#include "iobjs.h"
#include "ipipe.h"

#define PIPE_IN		true
#define PIPE_OUT	false

struct IAnno : public Interface
{
	virtual int   	Run(void*, void*) = 0;
	virtual int 	Wait(void**) = 0;
	virtual void 	Attach(IPipe*, bool = true) = 0;
	virtual void	Detach(bool = true) = 0;
};

#endif
