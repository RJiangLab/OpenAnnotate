#ifndef _TANNOOPTS_H_
#define _TANNOOPTS_H_

#include "topts.h"

class TAnnoOpts : public TOpts
{
public:
	TAnnoOpts() {
		LoadDefault();
	}
	~TAnnoOpts() {
	}
public:
	virtual void LoadOptions();
private:
	DECLARE_OPTION_TABLE();
};

#endif
