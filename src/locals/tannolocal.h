#ifndef _TANNOLOCAL_H_
#define _TANNOLOCAL_H_

#include "eopts.h"
#include "tannoapp.h"
#include "tannoopts.h"

class TAnnoLocalOpts : public TAnnoOpts
{
public:
	TAnnoLocalOpts() {
		LoadDefault();
	}
	~TAnnoLocalOpts() {
	}
private:
	DECLARE_OPTION_TABLE();
};


class TAnnoLocal : public TAnnoApp
{
public:
	TAnnoLocal() {
	}
	~TAnnoLocal() {
	}
protected:
	virtual int LoadOptions(int argc, char * argv[]);
	virtual int Main();
protected:
	int OnHelp();
	int OnAnnotate();
	int OnLoadIni();
protected:
	int CheckArgs();
};

#endif
