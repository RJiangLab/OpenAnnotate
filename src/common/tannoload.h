#ifndef _TANNOLOAD_H_
#define _TANNOLOAD_H_

#include <zlib.h>
#include <queue>
#include "etype.h"
#include "iopts.h"
#include "tanno.h"
using namespace std;

class TAnnoLoad : public TAnno
{
public:
	TAnnoLoad() {
	}
	~TAnnoLoad() {
	}
protected:
	class XBedFile {
	public: XBedFile(const char * sbed, int npbp) : gbed(NULL), bpbp(false) {
		if( sbed){
			gbed = gzopen(sbed, "rb");
		} else {
			fprintf(stderr, "[%02d][Error] Null input file name.\n", 0);
		}
		if( !gbed) {
			fprintf(stderr, "[%02d][Error] Unable to open bed file [%s].\n", 0, sbed);
		}
		bpbp = npbp ? true : false;
	}
	public: ~XBedFile() {
		if(gbed) gzclose(gbed);
	}
	public: int eof() {
		return qbed.size() ? 0 : (gbed ? gzeof(gbed) : 1); 
	}
	public: int getline(BEDLINE * pbed) {
		char sbuf[1024];
		int  nret = 0;
		if( !qbed.size() && gbed && !gzeof(gbed) && gzgets(gbed, sbuf, sizeof(sbuf)/sizeof(sbuf[0])) > 0){
			BEDLINE  cbed;
			char schr[16];
			char sstr[16];
			int  nchr = 0;
			int  nbeg = 0;
			int  nend = 0;
			int  nstr = 2;
			if(  sscanf(sbuf, "chr%s%d%d%*s%*s%s", schr, &nbeg, &nend, sstr) == 4){
				 nchr = (*schr=='X'?23:(*schr=='Y'?24:(*schr=='M'?25:atoi(schr))));
				 nstr = (*sstr=='-'? 0:(*sstr=='+'? 1:(2)));
			}
			cbed.nchr = nchr;
			cbed.nstr = nstr;
			if(bpbp) for(int cbeg = nbeg; cbeg < nend; cbeg++) {
				cbed.nbeg = cbeg;
				cbed.nend = cbeg + 1;
				qbed.push(cbed);
			} else {
				cbed.nbeg = nbeg;
				cbed.nend = nend;
				qbed.push(cbed);
			}
		}
		if(pbed && qbed.size()) {
			memcpy(pbed, &qbed.front(), sizeof(BEDLINE));
			nret = 1;
		}
		return nret;
	}
	public : int popline() {
		if( qbed.size()) {
			qbed.pop();
		}
	}
	private: bool           bpbp;
	private: gzFile 		gbed;
	private: queue<BEDLINE> qbed;
	};
protected:
	virtual int Init() {
		int nret = TAnno::Init();
		return nret;
	}
protected:
	virtual int Main() {
		int  rpbk = atoi((*opts)["block-rpbk"]);
		int  size = sizeof(BEDBLOCK)+rpbk*sizeof(BEDLINE);
		int  cidx = 0; 
		int  crow = 0; 
		int  cchr = 0;
		int  nrow = 0;
		BEDLINE    cbed;
		BEDLINE  * lbed = NULL;
		BEDBLOCK * pbed = NULL;
		for(XBedFile xbed((*opts)["input-file"], strcmp((*opts)["perbasepair"], "FALSE")); !xbed.eof(); ) {
			if((pbed=(BEDBLOCK*)opipe->Alloc(size)) == NULL) {
				fprintf(stderr, "[%02d][Error] Unable to allocate bed block.\n", nid);
				break;
			}
			lbed=(BEDLINE *)(pbed+1);
			for(nrow = 0; nrow < rpbk && !xbed.eof() && xbed.getline(&cbed); nrow++) {
				if( cchr != cbed.nchr) {
					cchr  = cbed.nchr;
					break;	
				} else {
					memcpy(lbed+nrow, &cbed, sizeof(BEDLINE));
					xbed.popline();
				}
			}
			if( nrow) {
				pbed->nsize = sizeof(BEDBLOCK) + nrow*sizeof(BEDLINE);
				pbed->nflag = 0;
				pbed->index = cidx++;
				pbed->nrows = nrow;
				pbed->ncols = 4;
				opipe->Push(pbed);
			} else {
				opipe->Free(pbed);
			}
//			if(ipipe && cidx > 32) {
//				ipipe->Free(ipipe->Pop());
//			}
		}
		for(int i = 0; i < 5*opipe->Size(true); i++){
			opipe->Push(NULL);
		}
		return EXIT_SUCCESS;
	}
};

#endif
