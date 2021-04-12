#ifndef _IANNOAPP_H_
#define _IANNOAPP_H_

#include "iobjs.h"

struct IAnnoApp : public Interface
{
	virtual int Run(int argc, char * argv []) = 0;
};

template<class ANNOTYPE>
int AnnoMain(int argc, char * argv[])
{
    int nret = EXIT_FAILURE;
	IAnnoApp * panno = new ANNOTYPE;
    if(panno) {
		nret = panno->Run(argc, argv);
		delete panno;
    }
	return nret;
}

#endif
