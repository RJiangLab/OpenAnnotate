#ifndef _TANNOWORK_H_
#define _TANNOWORK_H_

#include "ecomp.h"
#include "tcomp.h"
#include "etype.h"
#include "eutil.h"
#include "tanno.h"

using namespace std;

class TAnnoWork : public TAnno
{
public:
	TAnnoWork() {
		InitVars();
	}
	~TAnnoWork() {
		FreeVars();
	}
protected:
	virtual int Init() {
		int nret = TAnno::Init();
		    nret = (nret==EXIT_SUCCESS ? SetVars() : nret);
		return nret;
	}
protected:
	virtual int Main() {
		for(BEDBLOCK * pbed; (pbed=(BEDBLOCK*)ipipe->Pop()) != NULL; ) {
//			fprintf(stdout, "[%02d][Print] Received bed block [%d].\n", nid, pbed->index);
			InitBlock(pbed);
			CalcBlock();
			FormatBlock();
			ipipe->Free(pbed);
		}
		opipe->Push(NULL);
		return EXIT_SUCCESS;
	}
private:
	void InitBlock(BEDBLOCK * pbed) {
		nidx = pbed->index;
		nrow = pbed->nrows;
		ncol = cpbk;
		memcpy(info=(int*)(info?info:malloc(rpbk*ilen)), pbed+1,   nrow*ilen);
		for(int g = 0; g < 5; g++) {
//			if(!rgrp[g]) continue;
			memset(data[g]=(data[g]?data[g]:malloc(rpbk*dlen)), 0, rpbk*dlen);
		}
		if(braw == NULL) braw = malloc(rpbk*(ilen+dlen)*2);
		if(bzip == NULL) bzip = malloc(rpbk*(ilen+dlen)*2);
	}
	void CalcBlock() {
		for(int tgrp = 0; tgrp < 4; tgrp++){
			if(!rgrp[tgrp]) continue;
			for(int trow = 0; trow < nrow; trow++){
				CalcRow(tgrp, trow);
			}
		}
	}
	void CalcRow(int tgrp, int trow) {
		int nchr = info[trow*4+0];
		int nbeg = info[trow*4+1];
		int nend = info[trow*4+2];
		int nstr = info[trow*4+3];
			nbeg = tgrp == 1 ? (nbeg+nend)/2 : nbeg;
			nend = tgrp == 1 ? (nbeg+1)      : nend;
		int nlen = nend - nbeg;
//		fprintf(stdout, "[%02d][Print] CalcRow [%d\t%4d\t%02d\t%9d\t%9d].\n", nid, tgrp, trow, nchr, nbeg, nend);
		for(int npos = nbeg; npos < nend; npos++){
			void  * pvec = LoadVector(nchr, npos, tgrp);
			int   * ivec = (int*)  pvec;
			float * fvec = (float*)pvec;
			int   *	fgrc = this->fgrc + trow*cpbk;
			int   *	bgrc = this->bgrc + trow*cpbk;
			float *	peak = this->peak + trow*cpbk;
			float *	spot = this->spot + trow*cpbk;
			float *	open = this->open + trow*cpbk;
			for(int e = 0; e < cpbk; e++){
				if(tgrp == 0) {
					fgrc[e] += ivec[e];
					if(ropen){
						open[e]  = 1.0 * fgrc[e] / nlen;
					}
				} else if(tgrp == 1) {
					bgrc[e]  = ivec[e];
					if(ropen){
						open[e] /= 1.0 * (bgrc[e]>0?bgrc[e]:1) / blen;
						open[e]  = open[e] > 1E-4 ? open[e] : 0.0;
					}
				} else if(tgrp == 2) {
					peak[e]  = max(peak[e], fvec[e]);
					peak[e]  = peak[e] > 1E-4 ? peak[e] : 0.0;
				} else if(tgrp == 3) {
					spot[e]  = max(spot[e], fvec[e]);
					spot[e]  = spot[e] > 1E-4 ? spot[e] : 0.0;
				} else {
				}
			}
		}
		this->nchr = nchr;
	}
	void FormatBlock() {
		if(obin) FormatBin();
		if(otxt) FormatTxt();
	}
	void FormatBin() {
		for(int tgrp = 0; tgrp < 5; tgrp++){
			if(!ogrp[tgrp]) continue;
			BINBLOCK * pbin = NULL;
			if((pbin=(BINBLOCK*)opipe->Alloc(sizeof(BINBLOCK)+rpbk*(ilen+dlen))) != NULL) {
				FormatBin(pbin, tgrp);
				opipe->Push(pbin);
			} else {
				fprintf(stderr, "[%02d][Error] Unable to allocate bin block.\n", nid);
			}
		}
	}
	void FormatBin(BINBLOCK * pbin, int tgrp) {
		pbin->index = nidx;
		pbin->group = tgrp;
		pbin->chrom = nchr;
		pbin->nrows = nrow;
		pbin->ncols = ncol;
		pbin->ntype = 0;
//		pbin->nsize = nrow*(ilen+dlen);
		pbin->nsize = sizeof(int)*nrow*(4+cpbk);
		for(int trow = 0; trow < nrow; trow++) {
			int * thisinfo = ((int*)(pbin+1))     + trow*(4+cpbk) + 0;
			int * thisdata = ((int*)(pbin+1))     + trow*(4+cpbk) + 4;
			int * thatinfo = ((int*)(info  ))     + trow*(4);
			int * thatdata = ((int*)(data[tgrp])) + trow*(cpbk);
			memcpy(thisinfo, thatinfo, sizeof(int)*4);
			memcpy(thisdata, thatdata, sizeof(int)*cpbk);
		}
//		fprintf(stdout, "[%02d][Print] Formated bin group [%d, %d, %d, %d] bytes.\n", nid, tgrp, nrow, 4, cpbk);
//		fprintf(stdout, "[%02d][Print] Formated bin group [%d].\n", nid, tgrp);
	}
	void FormatTxt() {
		for(int tgrp = 0; tgrp < 5; tgrp++){
			if(!ogrp[tgrp]) continue;
			BINBLOCK * pbin = NULL;
			int size = FormatTxt(tgrp);
			if((pbin =(BINBLOCK*)opipe->Alloc(sizeof(BINBLOCK)+size)) != NULL) {
				memcpy(pbin, bzip, sizeof(BINBLOCK)+size);
				opipe->Push(pbin);
			} else {
				fprintf(stderr, "[%02d][Error] Unable to allocate txt block.\n", nid);
			}
		}
	}
	int FormatTxt(int tgrp) {
		BINBLOCK * ptxt = (BINBLOCK*) braw;
		char * btxt = (char*)(ptxt+1);
		ptxt->index = nidx;
		ptxt->group = tgrp;
		ptxt->chrom = nchr;
		ptxt->ntype = 1;
		ptxt->nsize = 0;
		int & p = ptxt->nsize;
		for(int trow = 0; trow < nrow; trow++) {
			int * info = ((int*)(this->info      )) + trow*4;
			int * data = ((int*)(this->data[tgrp])) + trow*cpbk;
			int   nchr = info[0];
			if(nchr < 23) p += sprintf(btxt+p, "chr%d", nchr);
			else		  p += sprintf(btxt+p, "chr%s", nchr==23?"X":(nchr==24?"Y":"M"));
			p += sprintf(btxt+p, "\t%d", (info[1]));
			p += sprintf(btxt+p, "\t%d", (info[2]));
			p += sprintf(btxt+p, "\t%c", (info[3]==0?'-':(info[3]==1?'+':'.')));
			for(int e = 0; e < cpbk; e++){
				if(tgrp < 2) p += sprintf(btxt+p, "\t%d",   ((int*)  data)[e]);
				else 	     p += sprintf(btxt+p, "\t%.4g", ((float*)data)[e]);
			}
			p += sprintf(btxt+p, "\n");
		}
//		fprintf(stdout, "[%02d][Print] Formated txt group [%d, %X].\n", nid, tgrp, ptxt);
		return CompressTxt();
	}
	int CompressTxt() {
		BINBLOCK *  praw = (BINBLOCK*)braw;
    	z_stream 	gzsm = {0};
    	z_stream * 	gzip = &gzsm;
		gzip->zalloc   	 = Z_NULL;
    	gzip->zfree 	 = Z_NULL;
    	gzip->opaque 	 = Z_NULL;
		gzip->next_in  	 = (Bytef*)(praw+1);
		gzip->avail_in 	 = praw->nsize;
        deflateInit2(gzip, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 9, Z_DEFAULT_STRATEGY);
		int clen = (int)deflateBound(gzip, praw->nsize);
		if( bzip == NULL) {
			bzip = malloc(clen+sizeof(BINBLOCK));
		} else if( malloc_usable_size(bzip) < clen) {
			bzip = realloc(bzip, clen+sizeof(BINBLOCK));
		}
		BINBLOCK * pzip = (BINBLOCK*)bzip;
        gzip->next_out  = (Bytef*)(pzip+1);
        gzip->avail_out = clen;
        if( deflate(gzip, Z_FINISH) == Z_STREAM_END) {
			memcpy(pzip, praw, sizeof(BINBLOCK));
			pzip->nsize = clen - gzip->avail_out;
		} else {
			fprintf(stderr, "[%02d][Error] Deflate faild.\n", nid);
		}
        deflateEnd(gzip);
		return pzip->nsize;
	}
	void * LoadVector(int nchr, int npos, int ngrp) {
		int tgrp  = ngrp;
		int tbin  = nchr*100 + npos/ppbn;
		int tbck  = npos%ppbn/ppbk;
		int trow  = npos%ppbk;
		if( tbin != cbin){
			LoadHead(tbin);
		}
		if( tgrp != cgrp){
			LoadGroup(tgrp);
		}
		if( tbck != cbck){
			LoadBlock(tbck);
		}
		return bcbk+trow*cpbk*bpun;
	}
	void LoadHead(int tbin) {
		string sbin = DataPath(tbin);
		if( fbin ) fclose(fbin);
		if((fbin=fopen(sbin.c_str(), "rb")) != NULL) {
			if(head == NULL) {
				head = (HEAD*)malloc(sizeof(HEAD)); 
			}
			if(head == NULL) {
				fprintf(stderr, "[%02d][Error] Unable to allocate memory for head [%s].\n", nid, sbin.c_str());
			}
			if(fseek(fbin,-sizeof(HEAD),SEEK_END)==0 && fread(head,sizeof(HEAD),1,fbin)==1) {
				cbin = tbin;
				cgrp = -1;
			} else {
				fprintf(stderr, "[%02d][Error] Unable to read head for bin file [%s].\n", nid, sbin.c_str());
			}
		} else {
			fprintf(stderr, "[%02d][Error] Unable to open bin file [%s].\n", nid, sbin.c_str());
		}
	}
	void LoadGroup(int tgrp) {
		if( bidx == NULL) {
			bidx =  (UI64*)malloc((bpbn+1)*sizeof(UI64));
		}
		if( bcbk == NULL) {
			bcbk =  (BYTE*)malloc(max((int)(bpbn*sizeof(UI64)),ppbk*epbk*bpun));
		}
		if( fbin && head && bidx && bcbk){
			BYTE cmtd = head->cmethod;
			UI64 ipos = head->indexpos[tgrp];
			UI32 clen = head->indexpos[tgrp+1] - ipos;
			UI32 ulen =(bpbn+1)*sizeof(UI64);
			if(fseek(fbin,ipos,SEEK_SET)==0 && fread(bcbk,clen,1,fbin)==1){
				if(Decompress((BYTE*)bcbk,(BYTE*)bidx,clen,ulen,cmtd) == ulen){
					cgrp = tgrp;
					cbck = -1;
				} else {
					fprintf(stderr, "[%02d][Error] Unable to decompress index [%d].\n", nid, tgrp);
				}
			} else {
				fprintf(stderr, "[%02d][Error] Unable to read index [%d].\n", nid, tgrp);
			}
		} else {
			fprintf(stderr, "[%02d][Error] Empty fbin or head [%d].\n", nid, tgrp);
		}
	}
	void LoadBlock(int tbck) {
		if( bubk == NULL) {
			bubk =  (BYTE*)malloc(ppbk*epbk*bpun+1024);
		}
		if( bcbk == NULL) {
			bcbk =  (BYTE*)malloc(max((int)(bpbn*sizeof(UI64)),ppbk*epbk*bpun));
		}
		if( fbin && bidx && bubk && bcbk){
			BYTE cmtd = head->cmethod;
			UI64 ipos = bidx[tbck];
			UI32 clen = bidx[tbck+1] - ipos;
			UI32 ulen = ppbk*epbk*bpun;
			if(fseek(fbin,ipos,SEEK_SET)==0 && fread(bcbk,clen,1,fbin)==1){
				if(Decompress((BYTE*)bcbk,(BYTE*)bubk,clen,ulen,cmtd) == ulen){
					cbck = tbck;
				} else {
					fprintf(stderr, "[%02d][Error] Unable to decompress data block [%d].\n", nid, tbck);
				}
			} else {
				fprintf(stderr, "[%02d][Error] Unable to read block [%d].\n", nid, tbck);
			}
		} else {
			fprintf(stderr, "[%02d][Error] Unable to read bin file.\n", nid);
		}
		for(int i = 0; i < ppbk; i++) {
			for(int j = 0; j < cpbk; j++) {
				for(int k = 0; k < bpun; k++) {
					bcbk[(i*cpbk+j)*bpun+k] = bubk[(i*epbk+cidx[j])*bpun+k];
				}
			}
		}
	}
	void LoadColumn() {
		cidx = (int*)malloc(cpbk*sizeof(int));
		if(cpbk == epbk) {
			for(int i = 0; i < cpbk; i++) {
				cidx[i] = i;
			}
		} else { 
			string  scol = string((*opts)["column"]) + ",";
			for(int i = 0, p = 0; (p=scol.find(",")) != string::npos; scol=scol.substr(p+1), i++) {
				string item = scol.substr(0,p);
				cidx[i] = atoi(item.c_str());
			}
		}
	}
	string DataPath(int tbin) {
		char   sbin[1024] = {0};
		string dhome = GetDataPath();
		string sname = (*opts)["species"];
		string gname = (*opts)["assembly"];
		string aname = (*opts)["assay"];
		string tname = "file";
		sprintf(sbin, "%s/%s/%s/%s/%s/%04d", dhome.c_str(), sname.c_str(), gname.c_str(), aname.c_str(), tname.c_str(), tbin);
		return  sbin;
	}
private:
	int SetVars() {
		lpbk = atoi((*opts)["block-lpbk"]);
		epbk = atoi((*opts)["block-epbk"]);
		rpbk = atoi((*opts)["block-rpbk"]);
		cpbk = atoi((*opts)["block-cpbk"]);
		ppbn = PPBN;
		bpbn = BPBN;
		ppbk = PPBK;
		bpun = sizeof(int);
		blen = BLEN;
		ilen = sizeof(BEDLINE);
		dlen = sizeof(int)*epbk;
		cgrp = -1;
		cbin = -1;
		cbck = -1;
		nidx = -1;

		rfgrc = ofgrc = (strcasecmp((*opts)["foreground"], "NULL") ? 1 : 0);
		rbgrc = obgrc = (strcasecmp((*opts)["background"], "NULL") ? 1 : 0);
		rpeak = opeak = (strcasecmp((*opts)["narrowpeak"], "NULL") ? 1 : 0);
		rspot = ospot = (strcasecmp((*opts)["broadpeak"],  "NULL") ? 1 : 0);
		ropen = oopen = (strcasecmp((*opts)["readopen"],   "NULL") ? 1 : 0);
		rfgrc = rfgrc | ropen;
		rbgrc = rbgrc | ropen;
		obin  = strcasecmp((*opts)["binary-path"],"NULL") ? 1 : 0;
		otxt  = ofgrc | obgrc | opeak | ospot | oopen;

		LoadColumn();
#if 0
		DumpVars();
		exit(1);
#endif

		return EXIT_SUCCESS;
	}
	void InitVars() {
		memset(vars, 0, sizeof(vars));
	}
	void FreeVars() {
		for(int i = 0; i < sizeof(ptrf)/sizeof(ptrf[0]); i++) {
			FCLOSE(ptrf[i]);
		}
		for(int i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
			FREE(ptrs[i]);
		}
	}
	void DumpVars() {
		fprintf(stdout, "lpbk=%d\n", lpbk);
		fprintf(stdout, "epbk=%d\n", epbk);
		fprintf(stdout, "rpbk=%d\n", rpbk);
		fprintf(stdout, "cpbk=%d\n", cpbk);
		fprintf(stdout, "ilen=%d\n", ilen);
		fprintf(stdout, "dlen=%d\n", dlen);
		for(int i = 0; i < 5; i++) {
			fprintf(stdout, "rgrp[%d]=%d\n", i, rgrp[i]);
		}
		for(int i = 0; i < 5; i++) {
			fprintf(stdout, "ogrp[%d]=%d\n", i, ogrp[i]);
		}
		for(int i = 0; i < 2; i++) {
			fprintf(stdout, "ofmt[%d]=%d\n", i, ofmt[i]);
		}
	}
private:
	union {
		void  *	vars[128];
	struct{
	union {
		FILE  *	ptrf[10];
		struct{
		FILE  *	fbin;
		};
	};
	union  {
		void  * ptrs[50];
		struct{
		struct{
		HEAD  *	head;
		UI64  *	bidx;
		BYTE  *	bcbk;
		BYTE  *	bubk;
		};
		struct{
		int   *	info;
		};
		struct{
		int   *	cidx;
		};
		union {
		void  * data[5];
		struct{
		int   *	fgrc;
		int   *	bgrc;
		float *	peak;
		float *	spot;
		float *	open;
		};
		};
		struct{
		void  * braw;
		void  * bzip;
		};
		};
	};
	union {
		int 	ints[50];
		struct  {
		struct  {
		int		lpbk;
		int		epbk;
		int		rpbk;
		int		cpbk;
		int 	ppbn;
		int 	bpbn;
		int 	ppbk;
		int 	bpun;
		int		blen;
		int		ilen;
		int		dlen;
		};
		struct  {
		int 	cgrp;
		int 	cbin;
		int 	cbck;
		};
		struct  {
		int	 	nidx;
		int		nchr;	
		int		nrow;
		int 	ncol;
		};
		union   {
		char	rgrp[5];
		struct  {
		char 	rfgrc;
		char	rbgrc;	
		char	rpeak;
		char	rspot;
		char	ropen;
		};
		};
		union   {
		char	ogrp[5];
		struct  {
		char 	ofgrc;
		char	obgrc;	
		char	opeak;
		char	ospot;
		char	oopen;
		};
		};
		union   {
		char	ofmt[2];
		struct  {
		char	obin;
		char	otxt;
		};
		};
		};
	};
	};
	};
};

#endif
