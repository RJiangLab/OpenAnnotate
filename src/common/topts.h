#ifndef _TOPTS_H_
#define _TOPTS_H_

#include <map>
#include "iopts.h"

using namespace std;

#define	NONARG	no_argument
#define REQARG	required_argument
#define OPTARG	optional_argument

#define DECLARE_OPTION_TABLE()												\
public:																		\
	static  OPTENTRY   OptTable[];											\
	virtual OPTENTRY * GetOptTable() {		   								\
		return OptTable;													\
	}																		\

#define BEGIN_OPTION_TABLE(CLASS)											\
	OPTENTRY CLASS::OptTable[] = {

#define END_OPTION_TABLE(CLASS)												\
	{NULL,	0,	0,	(const char*)CLASS::OptTable},							\
};

#define OPTION_ENTRY(optlong, optshort, opttype, optdefault)				\
	{#optlong, *#optshort, opttype, #optdefault},


struct OPTENTRY {
	const char* optlong;
	int			optshort;
	int			opttype;	
	const char* optdefault;
};


class TOpts : public IOpts
{
public:
	TOpts() {
        pthread_mutex_init(&soptlock,  NULL);
		LoadDefault();	
	}
	~TOpts() {
	}
public:
	virtual const char * operator [] (const char * key) {
		return GetOpt(key);
	}
public:
	const char * GetOpt(const char * key) {
		const char * val = "NULL";
        pthread_mutex_lock(&soptlock);
		map<string,string>::iterator it = sopt.find(string(key));
		val = (it != sopt.end() ? it->second.c_str() : val);
        pthread_mutex_unlock(&soptlock);
		return val;
	}
public:
	void SetOpt(const string & key, const string & val) {
        pthread_mutex_lock(&soptlock);
		sopt[string(key)] = string(val);
        pthread_mutex_unlock(&soptlock);
	}
	void SetOpt(const char * key, const char * val) {
		if(key && val) {
			SetOpt(string(key), string(val));
		}
	}
	void SetOpt(const char * key, int ival) {
		if(key) {
			char sval[16];
			sprintf(sval, "%d", ival);
			SetOpt(key, sval);
		}
	}
public:
	virtual void LoadOptions(int argc, char * argv[]);
	virtual void LoadOptions(FILE *);
public:
	virtual void SaveOptions(FILE *);
public:
	virtual void LoadDefault();
	virtual void LoadOptions();
public:
	virtual void DumpOptions();
private:
	map<string, string> sopt;
    pthread_mutex_t  	soptlock;
private:
	DECLARE_OPTION_TABLE();
};

#endif
