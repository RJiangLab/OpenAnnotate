#include <map>
#include <vector>
#include <stdio.h>
#include "eopts.h"
#include "ecomp.h"
#include "eutil.h"
#include "iopts.h"
#include "topts.h"
#include "tannoopts.h"


BEGIN_OPTION_TABLE(TAnnoOpts)
	OPTION_ENTRY(thread,		t,	REQARG,	16)
	OPTION_ENTRY(block-lpbk,	 , 	REQARG,	100)
	OPTION_ENTRY(block-epbk,	 ,	REQARG,	0)
	OPTION_ENTRY(block-rpbk,	 , 	REQARG,	100)
	OPTION_ENTRY(block-cpbk,	 ,	REQARG,	0)
END_OPTION_TABLE(TOpts)


void TAnnoOpts::LoadOptions()
{
//	fprintf(stdout, "void TAnnoOpts::LoadOptions()\n");
//	fprintf(stdout, "system=%s\n", (*this)["system"]);
//	fprintf(stdout, "organ=%s\n", (*this)["organ"]);
//	fprintf(stdout, "celltype=%s\n", (*this)["celltype"]);
//	fprintf(stdout, "target=%s\n", (*this)["target"]);
//	fprintf(stdout, "experiment=%s\n", (*this)["experiment"]);
//	fprintf(stdout, "biosample=%s\n", (*this)["biosample"]);
//	fprintf(stdout, "replicate=%s\n", (*this)["replicate"]);
	
	int    epbk = 0;
	int    cpbk = 0;
	char * scol[8] = {"replicate", "biosample", "experiment", "target", "celltype", "celltype", "organ", "system"};

	gzFile ghead = NULL;
	{
		string shead = string(GetDataPath()) + "/" + (*this)["species"] + "/" + (*this)["assembly"] + "/" + (*this)["assay"] + "/" + "file/head";
		if( shead != "") {
			ghead = gzopen(shead.c_str(), "rb");
		} else {
			fprintf(stderr, "[%02d][Error] Unable to read head [%s].\n", 99, shead.c_str());
		}
//		fprintf(stdout, "headpath=%s\n", shead.c_str());
	}

	map<string,string> mhead;
	for(char sbuf[2048] = {0}; ghead && !gzeof(ghead) && gzgets(ghead, sbuf, sizeof(sbuf)/sizeof(sbuf[0])) > 0; epbk++) {
		char tcol[32]; sprintf(tcol, "%d", epbk);
		char name[8][256] = {0};
		for(int i = 0; i < sizeof(sbuf)/sizeof(sbuf[0]); i++) {
			if(sbuf[i] == ' ') sbuf[i] = '%';
		}
		sscanf(sbuf, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s", name[0], name[1], name[2], name[3], name[4], name[5], name[6], name[7]);
		for(int i = 0; i < sizeof(name)/sizeof(name[0]); i++) {
			for(int j = 0; j < sizeof(name[i])/sizeof(name[i][0]) && name[i][j]; j++) {
				if((name[i][j]=toupper(name[i][j])) == '%') {
					for(int k = j; k < sizeof(name[i])/sizeof(name[i][0]); k++) {
						name[i][k] = name[i][k+1];
					}
					j--;
				}
			}
			string skey = string(scol[i]) + ":" + name[i];
			string sval = tcol;
			map<string,string>::iterator it = mhead.find(skey);
			if(it == mhead.end()) {
				mhead[skey] = sval;
			} else {
				it->second = it->second + "," + sval;
			}
//			fprintf(stdout, "name[%d][%d]=%s\n", epbk, i, name[i]);
//			fprintf(stdout, "%s --> %s\n", skey.c_str(), mhead[skey].c_str());
		}
	}
//	fprintf(stdout, "epbk=%d\n", epbk);

	vector<int> vin(epbk);
	for(int i = 0; i < epbk; i++) {
		vin[i] = 1;
	}
	for(int i = 0; i < sizeof(scol)/sizeof(scol[0]); i++) {
//		string tcol = string((*this)[scol[i]]) + ",";
		string tcol = (*this)[scol[i]];
		if(tcol == "NULL") continue;
		tcol += ",";
//		fprintf(stdout, "%s\n", tcol.c_str());
		vector<int> vthis(epbk);
		for(int i = 0; i < epbk; i++) {
			vthis[i] = 0;
		}
		for(int p = 0; (p=tcol.find(",")) != string::npos; tcol=tcol.substr(p+1)) {
			string item = tcol.substr(0,p);
			for(int i = 0; i < item.size(); i++) {
				item[i] = toupper(item[i]);
			}
			string skey = string(scol[i]) + ":" + item;
//			fprintf(stdout, "%s\n", skey.c_str());
			map<string,string>::iterator it = mhead.find(skey);
			if(it != mhead.end()) {
				string sval = it->second;
//				fprintf(stdout, "%s --> %s\n", skey.c_str(), sval.c_str());
				sval = sval + ",";
				for(int q = 0; (q=sval.find(",")) != string::npos; sval=sval.substr(q+1)) {
					string tval = sval.substr(0,q);
					int    ival = atoi(tval.c_str());
//					fprintf(stdout, "%s\n", tval.c_str());
					if(ival >= 0 && ival < epbk) {
//						fprintf(stdout, "%d\n", ival);
						vthis[ival] = 1;
					}
				}
			}
		}
		for(int i = 0; i < epbk; i++) {
			vin[i] &= vthis[i];
		}
	}

	string ctxt;	
	for(int i = 0; i < epbk; i++) {
		if(vin[i]) {
			char cbuf[32]; sprintf(cbuf, "%d", i);
			if( ctxt.size() > 0) {
				ctxt = ctxt + ",";
			}
			ctxt = ctxt + cbuf;
			cpbk++;
		}
	}
//	fprintf(stdout, "%s\n", ctxt.c_str());

	if( cpbk == 0) {
		cpbk = epbk;
		ctxt = "NULL";
	}
	SetOpt("column", 	 ctxt);
	SetOpt("block-epbk", epbk);
	SetOpt("block-cpbk", cpbk);

	if(ghead) {
		gzclose(ghead);
	}
}
