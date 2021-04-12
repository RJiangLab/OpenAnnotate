#include <set>
#include <vector>
#include <stdio.h>
#include <memory.h>
#include "eopts.h"
#include "iopts.h"
#include "topts.h"

using namespace std;

OPTENTRY OptTable[] = {
	{NULL, 0, 0, NULL}
};

BEGIN_OPTION_TABLE(TOpts)
	OPTION_ENTRY(help,	h,	NONARG,	FALSE)
END_OPTION_TABLE()

void TOpts::LoadDefault()
{
	for(OPTENTRY * opts = GetOptTable(); opts && opts->optlong; opts++){
		int    arg = opts->opttype;
		string key = opts->optlong;
		string val = opts->optdefault;
//		printf("%d\t%s\t%s\n", arg, key.c_str(), val.c_str());
		SetOpt(key, val);	
	}
}

void TOpts::DumpOptions()
{
	for(map<string,string>::iterator it = sopt.begin(); it != sopt.end(); it++) {
		fprintf(stdout, "%s=%s\n", it->first.c_str(), it->second.c_str());
	}
}

void TOpts::SaveOptions(FILE * fopt)
{
	if(fopt) for(map<string,string>::iterator it = sopt.begin(); it != sopt.end(); it++) {
		fprintf(fopt, "%s=%s\n", it->first.c_str(), it->second.c_str());
	}
}

void TOpts::LoadOptions(FILE * fopt)
{
	char  bopt[1024] = {0};
	char  bkey[1024] = {0};
	char  bval[1024] = {0};
	while(fopt && !feof(fopt) && fgets(bopt, sizeof(bopt), fopt) != NULL) {
		if(sscanf(bopt, "%s=%s", bkey, bval) == 2) {
			SetOpt(bkey, bval);
		}
	}
	LoadOptions();
}

void TOpts::LoadOptions(int argc, char * argv[])
{
	vector<OPTENTRY *> opttable;
	{
		set<const char *> exist;
		for(OPTENTRY * ptable = GetOptTable(); ptable; ) {
			if( ptable->optlong != NULL && exist.find(ptable->optlong)==exist.end()) {
				exist.insert(ptable->optlong);
				opttable.push_back(ptable++);
			} else {
				ptable=(OPTENTRY *)ptable->optdefault;
			}
		}
	}
	if(0) for(int i = 0; i < opttable.size(); i++) {
		printf("%d\t%s\n", i, opttable[i]->optlong);
	}

	char * short_options = new char[3*opttable.size()+1];
	getoptex::option * long_options = new getoptex::option[opttable.size()+1];
	if( short_options && long_options){
		memset(short_options, 0, sizeof(char)*3*opttable.size()+1);
		memset(long_options,  0, sizeof(getoptex::option)*(opttable.size()+1));

		int sp = 0;
		for(int i = 0; i < opttable.size(); i++){
			long_options[i].name	= opttable[i]->optlong;
			long_options[i].has_arg = opttable[i]->opttype;
			long_options[i].flag	= NULL;
			long_options[i].val		= 0;

			if(opttable[i]->optshort == 0) continue;
			short_options[sp++] = opttable[i]->optshort;
			if(opttable[i]->opttype != NONARG) {
				short_options[sp++] = ':';
			}
			if(opttable[i]->opttype == OPTARG) {
				short_options[sp++] = ':';
			}
		}

#if 0
		printf("short_options: [%s]\n", short_options);
		printf("long_options:\n", short_options);
		for(int i = 0; i < opttable.size(); i++){
			printf("\t%d\t%s\t%d\n", i, long_options[i].name, long_options[i].has_arg);
		}
#endif

		getoptex getopt;
		int opt = -1;
		int idx = -1;
		while((opt=getopt.getopt_long(argc, (const char**)argv, short_options, long_options, &idx)) != -1){
			if(opt > 0){
				for(idx = 0; idx < opttable.size() && opt != opttable[idx]->optshort; idx++);
			}
			if(idx >= 0 && idx < opttable.size()){
				int arg = opttable[idx]->opttype;
				const char * key = opttable[idx]->optlong;
				const char * val = getopt.optarg;
//				printf("%d\t%s\t%s\n", arg, key, val);
				if(val == NULL) {
					if( arg == NONARG) {
						val =  "TRUE";
					} else if( arg == OPTARG) {
						val = opttable[idx]->optdefault;
					} else {
						fprintf(stderr, "Set null value for option %s\n", key);
					}	
				}
//				printf("%d\t%s\t%s\n", arg, key, val);
				SetOpt(key, val);
			}
		}

		char buf[1024];
		for(int i = 0; i < argc-getopt.optind; i++){
			sprintf(buf, "argv[%d]", i);
			SetOpt(buf, argv[getopt.optind+i]);
		}
	}
	if(short_options) delete [] short_options;
	if(long_options)  delete [] long_options;
	LoadOptions();
}

void TOpts::LoadOptions()
{
}
