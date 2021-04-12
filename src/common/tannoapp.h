#ifndef _TANNOAPP_H_
#define _TANNOAPP_H_

#include "iopts.h"
#include "iargs.h"
#include "iannoapp.h"

class TAnnoApp : public IAnnoApp
{
public:
	TAnnoApp() : opts(NULL), args(NULL) {
	}
	~TAnnoApp() {
		if(opts) delete opts;
		if(args) delete args;
	}
public:
	virtual int Run(int argc, char * argv[]) {
		int nret = LoadOptions(argc, argv);
			nret = nret==EXIT_SUCCESS ? Init() : nret;
			nret = nret==EXIT_SUCCESS ? Main() : nret;
			nret = nret==EXIT_SUCCESS ? Exit() : nret;
		return nret;
	}
protected:
	virtual int Init() { 
		return EXIT_SUCCESS;
	}
	virtual int Main() { 
		return EXIT_SUCCESS;
	}
	virtual int Exit() { 
		return EXIT_SUCCESS;
	}
protected:
	virtual int LoadOptions(int argc, char * argv[]) {
		return EXIT_SUCCESS;
	}
	virtual int LoadArguments(int argc, char * argv[]) {
		return EXIT_SUCCESS;
	}
protected:
	IOpts * opts;
	IArgs * args;
};

#endif
