#ifndef _TANNOSAVE_H_
#define _TANNOSAVE_H_

#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include "etype.h"
#include "eutil.h"
#include "tanno.h"
#include "tannoload.h"

using namespace std;

class TAnnoSave : public TAnno
{
public:
	TAnnoSave() {
		InitVars();
	}
	~TAnnoSave() {
		FreeVars();
	}
private:
	struct BININFO {
		int					cidx;
		vector<BINBLOCK*>	vbin;
		BININFO() : cidx(0) {
		}
	};
	typedef	map<int,BININFO*> IMAP;
	typedef	map<int,string>   NMAP;
	typedef map<int,FILE*>    FMAP;
	typedef	map<int,int> 	  DMAP;
	typedef	map<int,int> 	  CMAP;
	IMAP	imap;
	NMAP	nmap;
	FMAP	fmap;
	DMAP	dmap;
	CMAP	cmap;
	struct  {
		int	percent;
	};
	int  chrcou[25]={0};//total count of each chrom
	int  chrsta[25]={0};//already done count of each chrom
protected:
	virtual int Init() {
		int nret = TAnno::Init();
		nret = (nret==EXIT_SUCCESS ? SetVars() : nret);//这里直接运行SetVars
		return nret;
	}
	virtual int Exit() {
		int nret = TAnno::Exit();
		if( nret == EXIT_SUCCESS) {
#if 0
			for(CMAP::iterator it = cmap.begin(); it != cmap.end(); it++) {
				printf("%d=%d\n", it->first, it->second);
			}
#endif
			for(int c = 1; c <= 25; c++) {
				UpdateProgress(c);
			}
		}
		return nret;
	}
protected:
	virtual int Main() {
		SaveHead();
		ChrTotalCount();
		for(int b = 0; ipipe->Size(PIPE_TAIL) > 0 || RemainBlock() > 0; b++){ 
			for(BINBLOCK * pbin = NULL; (pbin = (BINBLOCK*)ipipe->Pop()) != NULL; ) {
//				fprintf(stdout, "[%02d][Print] Received bin block [%d].\n", nid, b);
				BININFO * pinf = RegisterBlock(pbin);
				if(pinf != NULL){
					SaveBlock(pinf);
				}
			}
		}
		FileClose();
		return EXIT_SUCCESS;
	}
private:
	void SaveHead() {
		gzFile ghout = NULL;
		{
			string shout = (*opts)["head-file"];
			if( shout != "" && shout != "NULL") {
				MakePath(shout.c_str());
				ghout = gzopen(shout.c_str(), "wb");
			} else {
				fprintf(stderr, "[%02d][Error] Unable to open head [%s].\n", nid, shout.c_str());
			}
		}
		if(ghout == NULL) {
			return;
		}

		int epbk = atoi((*opts)["block-epbk"]);
		int cpbk = atoi((*opts)["block-cpbk"]);
		
		vector<int> cout(epbk);
		for(int i = 0; i < cpbk; i++) {
			cout[i] = 0;
		}
		if(cpbk == epbk) {
			for(int i = 0; i < cpbk; i++) {
				cout[i] = 1;
			}
		} else { 
			string  scol = string((*opts)["column"]) + ",";
			for(int i = 0, p = 0; (p=scol.find(",")) != string::npos; scol=scol.substr(p+1), i++) {
				string item = scol.substr(0,p);
				cout[atoi(item.c_str())] = 1;
			}
		}

		gzFile ghead = NULL;
		{
			string shead = string(GetDataPath()) + "/" + (*opts)["species"] + "/" + (*opts)["assembly"] + "/" + (*opts)["assay"] + "/" + "file/head";
			if( shead != "") {
				ghead = gzopen(shead.c_str(), "rb");
			} else {
				fprintf(stderr, "[%02d][Error] Unable to read head [%s].\n", 99, shead.c_str());
			}
		}
		char sbuf[2048] = {0};
		for( int i = 0; ghead && !gzeof(ghead) && gzgets(ghead, sbuf, sizeof(sbuf)/sizeof(sbuf[0])) > 0; i++) {
			if(cout[i]) {
//				fprintf(stdout, "%s", sbuf);
				gzputs(ghout, sbuf);
			}
		}
		if(ghead) {
			gzclose(ghead);
		}

		if(ghout) {
			gzclose(ghout);
		}
	}
private:
	BININFO * RegisterBlock(BINBLOCK * pbin) {
		int group = pbin->group;
		int ntype = pbin->ntype;
		int binid = group*10 + ntype;				//??????????//
		if( imap.find(binid) == imap.end()) {
			imap[binid] = new BININFO;
		}
		BININFO * pinfo = imap[binid];				//??????????//
		pinfo->vbin.push_back(pbin);
		return pinfo;
	}
	int RemainBlock() {
		int remain = 0;
		for(IMAP::iterator it = imap.begin(); it != imap.end(); it++){
			BININFO * pinfo =  it->second;
			remain += pinfo ?  pinfo->vbin.size() : 0;
		}
		return remain;
	}
	void ChrTotalCount(){
		const char * sbed = (*opts)["input-file"];
		gzFile 		 gbed = NULL;
		if( sbed){
			gbed = gzopen(sbed, "rb");
		} else {
			fprintf(stderr, "[%02d][Error] Null input file name.\n", nid);
		}
		char sbuf[1024];
		char schr[16];
		int  nchr = 0;
		int  nbeg = 0;
		int  nend = 0;
			
		while(gzgets(gbed, sbuf, sizeof(sbuf)/sizeof(sbuf[0])) > 0){
			if(sscanf(sbuf, "chr%s%d%d", schr, &nbeg, &nend) ==3){
				nchr = (*schr=='X'?23:(*schr=='Y'?24:(*schr=='M'?25:atoi(schr))));
			}
			if(strcmp((*opts)["perbasepair"], "FALSE")){
				chrcou[nchr-1] += nend-nbeg;
			}else{
				chrcou[nchr-1]++;
			}
			
		}
		//for(int i=0;i<25;i++){
		//	printf("chr%d: %d\n", i+1,chrcou[i]);
		//}	
		if(gbed){
			gzclose(gbed);
		}
	}
	void SaveBlock(BININFO * pinf) {
		vector<BINBLOCK*>  & vbin = pinf->vbin;
		int & cidx = pinf->cidx;
		for(int i = 0; i < vbin.size(); i++){
			BINBLOCK * pbin = vbin[i];
			if(!pbin || pbin->index != cidx) continue;
//			fprintf(stdout, "[%02d][Print] Saving   bin block [%d].\n", nid, cidx);
			SaveBlock(pbin);
			UpdateProgress(pbin);
			UpdateProgress(pbin->chrom);

			cidx++;
			vbin.erase(vbin.begin()+i); i = -1;			//??????????//
		}
	}
	void SaveBlock(BINBLOCK * pbin) {
		FILE * fbin = FileOpen(pbin);
		if(fbin != NULL) {
			int nsize  = pbin->nsize;
			int nsave  = fwrite(pbin+1, 1, nsize, fbin);
			if( nsave != nsize) {
				fprintf(stderr, "[%02d][Error] Unable to write bin block.\n", nid);
			}
#if 0
			{
				static int i = 0;
				char   sdump[1024] = {0};
				FILE * fdump = NULL;
				sprintf(sdump, "/tmp/%04d.bin", i++);
				if((fdump=fopen(sdump, "wb")) != NULL) {
					fwrite(pbin+1, 1, nsize, fdump);
				}
				fclose(fdump);
				fprintf(stdout, "[%02d][Print] Saved bin block [%s][%d] bytes.\n", nid, sdump, nsize);
			}
#endif
		}
		ipipe->Free(pbin);
		int index = pbin->index;
		if( dmap.find(index) == dmap.end()) {
			dmap[index] = 0;
		}
		if( ++dmap[index] == 1 && opipe) {
			opipe->Push(opipe->Alloc(1));
		}
	}
	void UpdateProgress(BINBLOCK * pbin){
		int index = pbin->index;
		if( dmap[index] == 1) {
			chrsta[pbin->chrom-1] += pbin->nrows;
			const char * sbed = (*opts)["status-file"];
			FILE * file = NULL;
			if( strcmp(sbed, "NULL") && (file=fopen(sbed, "w")) != NULL) {			
				for(int i=1;i<=25;i++){
					if(chrcou[i-1] > 0 && 1.0*chrsta[i-1]/chrcou[i-1] > 0.01)
					fprintf(file,"%d\t%d\t%.1f%%\n",i,chrcou[i-1],100.0*chrsta[i-1]/chrcou[i-1]);
				}
				fclose(file);
			} else {
//				fprintf(stderr, "[Error] Unable to open file [%s].\n", sbed);
			}

		}
		
	}
	FILE * FileOpen(BINBLOCK * pbin) {
		FILE * file = NULL;
		int    nfid = FileID(pbin);
		FMAP::iterator it = fmap.find(nfid);
		if(it == fmap.end()) {
//			string name = FileName(pbin);
			string name;
			map<int,string>::iterator it = nmap.find(nfid);
			if(it != nmap.end()) {
				name = it->second;
			} else {
				fprintf(stderr, "[%02d][Error] Unable to open file [%X, %s].\n", nid, nfid, name.c_str());
			}
//			fprintf(stdout, "[%02d][Pirnt] Open file [%d, %s].\n", nid, nfid, name.c_str());
			MakePath(name.c_str());
			if((file=fopen(name.c_str(), "wb")) != NULL) {
				fmap[nfid] = file;
			} else {
				fprintf(stderr, "[%02d][Error] Unable to open file [%X\t%s].\n", nid, nfid, name.c_str());
			}
		} else {
			file = fmap[nfid];
		}
		return file;
	}
	string FileName(BINBLOCK * pbin) {
		char  stxt[1024]    = {0};
		const char * home   = (*opts)["output-path"];
		const char * ndir[] = {"fgrc", "bgrc", "peak", "spot", "open"};
		const char * name[] = {"foreread", "backread", "peakopen", "spotopen", "readopen"};
		const char * nsuf[] = {"bin", "txt.gz"};
		int ntype = pbin->ntype;
		int group = pbin->group;
		int chrom = pbin->chrom;
		if( ntype == 0) {
			sprintf(stxt, "%s/%s/%02d.%s", home, ndir[group], chrom,  nsuf[ntype]);
		} else if (ntype == 1) {
			sprintf(stxt, "%s/%s/%s.%s",   home, "anno", name[group], nsuf[ntype]);
		}
		return string(stxt);
	}
	int FileID(BINBLOCK * pbin) {
		int group = pbin->group;
		int ntype = pbin->ntype;
		int chrom = ntype==0 ? pbin->chrom : 0;
		return (group<<16) | (ntype<<8) | chrom;
	}
	void FileClose() {
		for(FMAP::iterator it = fmap.begin(); it != fmap.end(); it++){
			FILE * file =  it->second;
			if(file) {
				fclose(file);
				it->second = NULL;
			}
		}
	}
	int MakePath(const char * file) {
		int    nret = 1;
		string path = file;
		for(int p=1; (p=path.find_first_of('/',p)) != string::npos; p++) {
			string dirname = path.substr(0, p);
			DIR  * diropen = opendir(dirname.c_str());
			if(diropen == NULL) {
				nret = nret && (mkdir(dirname.c_str(),0755)==0);
			} else {
				closedir(diropen);
			}
		}
		return nret;
	}
	int UpdateProgress(int chrom) {
		if( chrom < 1) return 0;
		int updated  = 0;
		if( cmap.find(chrom) == cmap.end()) {
			cmap[chrom] = 0;
		}
		cmap[chrom]++;
		int ratio    = 100 * cmap.size() / 25;

#if 1
		if( percent == 0) {
			printf("\033[?25l");
		}
		if( percent != ratio) {
			printf("\r\033[37m[%3d%%]", ratio);
			printf("[");
			for(int i = 0; i < ratio/2; i++) {
				printf("\033[33m\xDB");
			}
			for(int i = ratio/2; i < 50; i++) {
				printf("\033[34m\xB0");
			}
			printf("\033[37m]");
			if( ratio == 100) {
				printf("\n\033[?25h");
			}
			fflush(stdout);
		}
#endif

		if( percent != ratio) {
			percent  = ratio;
			updated  = 1;
		}

		return updated;
	}
private:
	int SetVars() {
		string dbin = (*opts)["binary-path"];
		if(strcasecmp(dbin.c_str(), "NULL")) {
			char   stxt[1024] = {0};
			const char * sbin[] = {"fgrc", "bgrc", "peak", "spot", "open"};
			for(int g = 0; g < 5; g++) {
				for(int c = 1; c <= 25; c++) {
					int k = (g<<16) | (0<<8) | c;
					sprintf(stxt, "%s/%s/%02d.bin", dbin.c_str(), sbin[g], c);
					nmap[k] = stxt;
				}
			}
		}
	
		const char * ftxt[] = {"foreground", "background", "narrowpeak", "broadpeak", "readopen"};
		for(int i = 0; i < sizeof(ftxt)/sizeof(ftxt[0]); i++) {
			string name = (*opts)[ftxt[i]];
			if(strcasecmp(name.c_str(), "NULL")) { 
				nmap[(i<<16) | (1<<8)] = name;
			}
		}

		return EXIT_SUCCESS;
	}
	int InitVars() {
		percent = 0;
		return EXIT_SUCCESS;	
	}
	int FreeVars() {
		for(IMAP::iterator it = imap.begin(); it != imap.end(); it++) {
			BININFO * pinfo = it->second;
			if(pinfo) {
				delete pinfo;
				it->second = NULL;
			}
		}
		return EXIT_SUCCESS;	
	}
};

#endif
