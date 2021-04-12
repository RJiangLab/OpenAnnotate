#ifndef _TANNO_H_
#define _TANNO_H_

#include "iopts.h"
#include "iargs.h"
#include "ianno.h"
#include "eutil.h"

using namespace std;

class TAnno : public IAnno
{
public:
	TAnno() : threadid(0), args(NULL), ipipe(NULL), opipe(NULL) {
		nid = nthread++;
	}
	~TAnno() {
		fprintf(stdout, "[%02d][Print] Destructed.\n", nid);
	}
public:
	virtual int Run(void * opts, void * args) {
        this->opts = (IOpts*)opts;
		this->args = (IArgs*)args;
        return pthread_create(&this->threadid, NULL, &ThreadProc, (void*)this);
	}
	virtual int Wait(void ** retp) {
		static void * rett = NULL;
		return pthread_join(threadid, retp?retp:&rett);
	}
    virtual void Attach(IPipe * pipe, bool in = true) {
        (in?ipipe=pipe:opipe=pipe)->Attach((IAnno*)this,in);
    }
    virtual void Detach(bool in = true) {
        (in?ipipe:opipe)->Detach((IAnno*)this,in);
        (in?ipipe=NULL:opipe=NULL);
    }
protected:
	virtual int Init(){
//		fprintf(stdout, "[%02d][Print] Started.\n", nid);
		return EXIT_SUCCESS;
	}
	virtual int Main(){
//		fprintf(stdout, "[%02d][Print] Running.\n", nid);
		return EXIT_SUCCESS;
	}
	virtual int Exit(){
		if(ipipe) Detach(PIPE_IN);
		if(opipe) Detach(PIPE_OUT);
//		fprintf(stdout, "[%02d][Print] Exiting.\n", nid);
		return EXIT_SUCCESS;
	}
protected:
    pthread_t threadid;
	IPipe *	  ipipe;
	IPipe *	  opipe;
	IOpts *	  opts;
	IArgs *   args;
	int		  nid;
private:
	static int nthread;
private:
	static void* ThreadProc(void * args) {
		TAnno * that = (TAnno*) args;
		int		nret = that ? EXIT_SUCCESS : EXIT_FAILURE;
		nret = (nret==EXIT_SUCCESS ? that->Init() : EXIT_FAILURE);
		nret = (nret==EXIT_SUCCESS ? that->Main() : EXIT_FAILURE);
		nret = (nret==EXIT_SUCCESS ? that->Exit() : EXIT_FAILURE);
		return (void*)(long)nret;
	}
};

#endif
