#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h> 
#include <memory.h>
#include <malloc.h>
#include <lz4.h>
#include <lz4hc.h>
#include <bzlib.h>
#include <zlib.h>
#include <lzma.h>

using namespace std;

#define CHECK_COMPRESS
 
#define CM_LZ4		1
#define CM_BZ2		2
#define CM_LZ4Z		1
#define CM_BZ2Z		2
#define CM_ZLIB		3
#define CM_LZMA		4

#define LPBK 		1000
#define BLEN 		1000000
#define EPBK 		871
#define PPBK 		100
#define PPBN 		10000000
#define BPBN 		(PPBN/PPBK)
#define NTHREAD 	16
#define fseek 		fseeko64
#define ftell 		ftello64


typedef char		BYTE;
typedef char 		CHAR;
typedef size_t 		SIZE;
typedef int8_t 		SI08;
typedef int16_t		SI16;
typedef int32_t		SI32;
typedef int64_t		SI64;
typedef u_int8_t	UI08;
typedef u_int16_t 	UI16;
typedef u_int32_t	UI32;
typedef u_int64_t	UI64;

//	int LZ4_compress_HC(const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel);
//	int LZ4_compress_default(const char* source, char* dest, int sourceSize, int maxDestSize);
//	
int lz4_compress(const BYTE * ubuf, BYTE * cbuf, int ulen, int clen)
{
#if 0
	return LZ4_compress_HC(ubuf, cbuf, ulen, clen, 9);
#else
	return LZ4_compress_default(ubuf, cbuf, ulen, clen);
#endif
}

//	int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize);
//	int LZ4_decompress_fast(const char* source, char* dest, int originalSize);
//
int lz4_decompress(const BYTE * cbuf, BYTE * ubuf, int clen, int ulen)
{
#if 0
	return LZ4_decompress_safe(cbuf, ubuf, clen, ulen);
#else
	return (LZ4_decompress_fast(cbuf, ubuf, ulen) == clen) ? ulen : -1;
#endif
}

//	int BZ2_bzBuffToBuffCompress( 
//      char*         dest, 
//      unsigned int* destLen,
//      char*         source, 
//      unsigned int  sourceLen,
//      int           blockSize100k, 
//      int           verbosity, 
//      int           workFactor 
//   );
//
int bz2_compress(const BYTE * ubuf, BYTE * cbuf, int ulen, int clen)
{
	unsigned int dlen = clen;
	return (BZ2_bzBuffToBuffCompress((char*)cbuf, &dlen, (char*)(*(&ubuf)), ulen, 9, 0, 0) == BZ_OK) ? dlen : -1;
}

//	int BZ2_bzBuffToBuffDecompress( 
//      char*         dest, 
//      unsigned int* destLen,
//      char*         source, 
//      unsigned int  sourceLen,
//      int           small, 
//      int           verbosity 
//   );
//
int bz2_decompress(const BYTE * cbuf, BYTE * ubuf, int clen, int ulen)
{
	unsigned int dlen = ulen;
	return (BZ2_bzBuffToBuffDecompress((char*)ubuf, &dlen, (char*)(*(&cbuf)), clen, 0, 0) == BZ_OK) ? dlen : -1;
}

//	int compress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
//	int compress2(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level);
//
int zlib_compress(const BYTE * ubuf, BYTE * cbuf, int ulen, int clen)
{
	uLongf dlen = clen;
#if 0
	return (compress ((Bytef*)cbuf, &dlen, (const Bytef*)ubuf, (uLong)ulen)    == Z_OK) ? dlen : -1;
#else
	return (compress2((Bytef*)cbuf, &dlen, (const Bytef*)ubuf, (uLong)ulen, 9) == Z_OK) ? dlen : -1;
#endif
}

//	int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
//
int zlib_decompress(const BYTE * cbuf, BYTE * ubuf, int clen, int ulen)
{
	uLongf dlen = ulen;
	return (uncompress((Bytef*)ubuf, &dlen, (const Bytef*)cbuf, (uLong)clen) == Z_OK) ? dlen : -1;
}

//	int lzma_easy_buffer_encode(
//		uint32_t preset, lzma_check check,
//		const lzma_allocator *allocator,
//		const uint8_t *in, size_t in_size,
//		uint8_t *out, size_t *out_pos, size_t out_size);
//
int lzma_compress(const BYTE * ubuf, BYTE * cbuf, int ulen, int clen)
{
	lzma_check 	check 	= LZMA_CHECK_CRC64;
	uint32_t 	preset 	= 6;
	size_t		out_pos	= 0;
	return (lzma_easy_buffer_encode(preset, check, NULL, (const uint8_t*)ubuf, (size_t)ulen, (uint8_t*)cbuf, &out_pos, (size_t)clen) == LZMA_OK) ? out_pos : -1;
}

// 	int lzma_stream_buffer_decode(
//		uint64_t *memlimit, uint32_t flags,
//		const lzma_allocator *allocator,
//		const uint8_t *in, size_t *in_pos, size_t in_size,
//		uint8_t *out, size_t *out_pos, size_t out_size);
//
int lzma_decompress(const BYTE * cbuf, BYTE * ubuf, int clen, int ulen)
{
	uint64_t 	memlimit = UINT64_MAX;
	uint32_t   	flags    = 0;
	size_t		in_pos	 = 0;
	size_t		out_pos	 = 0;
	return (lzma_stream_buffer_decode(&memlimit, flags, NULL, (const uint8_t*)cbuf, &in_pos, (size_t)clen, (uint8_t*)ubuf, &out_pos, (size_t)ulen) == LZMA_OK) ? out_pos : -1;
}


int compress(const BYTE * ubuf, BYTE * cbuf, int ulen, int clen, int method)
{
	if(method == CM_LZ4){
		return lz4_compress (ubuf, cbuf, ulen, clen);
	} else if(method == CM_BZ2){
		return bz2_compress(ubuf, cbuf, ulen, clen);
	} else if(method == CM_ZLIB){
		return zlib_compress(ubuf, cbuf, ulen, clen);
	} else if(method == CM_LZMA){
		return lzma_compress(ubuf, cbuf, ulen, clen);
	}
	return (-1);
}

int decompress(const BYTE * cbuf, BYTE * ubuf, int clen, int ulen, int method)
{
	if(method == CM_LZ4){
		return lz4_decompress (cbuf, ubuf, clen, ulen);
	} else if(method == CM_BZ2){
		return bz2_decompress(cbuf, ubuf, clen, ulen);
	} else if(method == CM_ZLIB){
		return zlib_decompress(cbuf, ubuf, clen, ulen);
	} else if(method == CM_LZMA){
		return lzma_decompress(cbuf, ubuf, clen, ulen);
	}
	return (-1);
}


struct HEAD
{
	union  {
	struct {
	CHAR 	title[15];
	BYTE	cmethod;
	UI32	tmstamp;
	UI32 	version;
	UI32	bingrps;
	UI32	grprows;
	UI32	grpcols;
	UI32	grpbcks;
	UI32	bckrows;
	UI32	bckcols;
	UI32	cellsize;
	UI64	indexpos[8];
	};
	BYTE	dummy[256];
	};	
};

struct BEDLINE
{
	int		nchr;
	int  	nbeg;
	int  	nend;
};

struct BEDBLOCK
{
	int		index;
	int		nrows;
	int		ncols;
};

struct BINBLOCK
{
	int		index;
	int		group;
	int		ntype;
	int		chrom;
	int		nsize;
};


class IOpts;
class IPipe;
class IAnno;

class IOpts
{
public:
	virtual const char * operator [] (const char *) = 0;
	virtual const char * operator [] (int)          = 0;
public:
	virtual void	ParseCmd(int argc, char * argv[]) = 0;
	virtual void	ParseIni(const char *) = 0;
};

class IPipe
{
public:
	virtual void* 	Alloc(int ) = 0;
	virtual void  	Free(void*) = 0;
	virtual void  	Push(void*) = 0;
	virtual void*	Pop() = 0;
	virtual void  	Attach(IAnno*, bool = true) = 0;
	virtual void  	Detach(IAnno*, bool = true) = 0;
	virtual int   	Size(bool = true) = 0;
};

class IAnno
{
public:
	virtual int   	Run(void*, void*) = 0;
	virtual int 	Wait(void**) = 0;
	virtual void 	Attach(IPipe*, bool = true) = 0;
	virtual void	Detach(bool = true) = 0;
};

class TOpts : public IOpts
{
public:
	TOpts() {
	}
	~TOpts() {
	}
public:
	virtual const char * operator [] (const char * key) {
		map<string, string>::iterator it = sopt.find(string(key));
		return it != sopt.end() ? it->second.c_str() : NULL;
	}
	virtual const char * operator [] (int key) {
		map<int, string>::iterator it = iopt.find(key);
		return it != iopt.end() ? it->second.c_str() : NULL;
	}
public:
	virtual void ParseCmd(int argc, char * argv[]) {
	}
	virtual void ParseIni(const char *) {
	}
public:
	void SetOpt(const char * key, const char * val) {
		if(key) {
			sopt[string(key)] = string(val);
		}
	}
	void SetOpt(int key, const char * val) {
		iopt[key] = string(val);
	}
public:
	void SetOpt(const char * key, int val) {
		if(key) {
			char buf[16];
			sprintf(buf, "%d", val);
			sopt[string(key)] = string(buf);
		}
	}
	void SetOpt(int key, int val) {
		char buf[16];
		sprintf(buf, "%d", val);
		iopt[key] = string(buf);
	}
public:
	const char * GetOpt(const char * key) {
		map<string, string>::iterator it = sopt.find(string(key));
		return it != sopt.end() ? it->second.c_str() : NULL;
	}
	const char * GetOpt(int key) {
		map<int, string>::iterator it = iopt.find(key);
		return it != iopt.end() ? it->second.c_str() : NULL;
	}
private:
	map<string, string> sopt;
	map<int,    string> iopt;
};


class TAnnoOpts : public TOpts
{
public:
	TAnnoOpts() {
		LoadDefault();
	}
	~TAnnoOpts() {
	}
public:
	void LoadDefault() {
		SetOpt("data-path",   	"data");
		SetOpt("thread-number",	16);
		SetOpt("block-rows", 	LPBK);
		SetOpt("block-cols", 	EPBK);
	}
};

class TPipe : public IPipe
{
public:
	TPipe() {
		pthread_mutex_init(&pipelock,  NULL);
	}
	~TPipe() {
	}
public:
	int Size(bool head = true) {
        pthread_mutex_lock(&pipelock);
		int size = (int)(head?headpool:tailpool).size();
        pthread_mutex_unlock(&pipelock);
		return size;
	}
	void Attach(IAnno * anno, bool head = true){
        pthread_mutex_lock  (&pipelock);
		vector<IAnno*>& pool = head ? headpool : tailpool;
		vector<IAnno*>::iterator it = pool.begin();
		while(it != pool.end() && *it != anno) it++;
		if(it == pool.end()) pool.push_back(anno);
        pthread_mutex_unlock(&pipelock);
	}
	void Detach(IAnno * anno, bool head = true){
        pthread_mutex_lock(&pipelock);
		vector<IAnno*>& pool = head ? headpool : tailpool;
		for(int i = 0; i < pool.size(); i++){
			if(pool[i] == anno){
				pool.erase(pool.begin()+i--);
			}
		}
        pthread_mutex_unlock(&pipelock);
	}
private:
	vector<IAnno*> 	tailpool;
	vector<IAnno*> 	headpool;
	pthread_mutex_t pipelock;
};

class TPoolPipe : public TPipe
{
public:
	TPoolPipe(int pipelong) : poolsize(pipelong) {
		pthread_mutex_init(&poollock,  NULL);
		pthread_cond_init (&poolaval,  NULL);
	}
public:
	~TPoolPipe() {
		for(set<void*>::iterator it = memblock.begin(); it != memblock.end(); it++){
			if(*it) free(*it);
		}
	}
public:
	virtual void * Alloc(int size) {
		void * data = NULL;
        pthread_mutex_lock(&poollock);
		if(pooldata.size() == 0 && memblock.size() < poolsize){
			memblock.insert(data=malloc(size));
			pooldata.push_back(data);
		}
		while(pooldata.size() == 0){
			pthread_cond_wait(&poolaval, &poollock);
		}
		data = pooldata.back();
		       pooldata.pop_back();
		if(malloc_usable_size(data) < size) {
			memblock.erase(data);
			memblock.insert(data=realloc(data, size));
		}
		memset(data, 0, size);
        pthread_mutex_unlock(&poollock);
		return data;
	}
	virtual void Free(void * data) {
        pthread_mutex_lock(&poollock);
		if(memblock.find(data) != memblock.end()) {
			pooldata.push_back(data);
		}
     	pthread_cond_signal(&poolaval);
        pthread_mutex_unlock(&poollock);
	}
private:
	int			  	poolsize;
	set<void*> 		memblock;
	vector<void*> 	pooldata;
	pthread_mutex_t	poollock;
	pthread_cond_t 	poolaval;
};

class TQueuePipe : public TPoolPipe
{
public:
	TQueuePipe(int poollong) : TPoolPipe(poollong) {
		pthread_mutex_init(&queuelock, NULL);
		pthread_cond_init (&queueaval, NULL);
	}
	~TQueuePipe() {
	}
public:
	virtual void Push(void * data) {
        pthread_mutex_lock(&queuelock);
		queuedata.push(data);
     	pthread_cond_signal (&queueaval);
        pthread_mutex_unlock(&queuelock);
	}
	virtual void * Pop() {
		void * data = NULL;
        pthread_mutex_lock(&queuelock);
       	while(queuedata.empty()){
			pthread_cond_wait(&queueaval, &queuelock);
		}
		data = queuedata.front();
			   queuedata.pop();
        pthread_mutex_unlock(&queuelock);
		return data;
	}
private:
	queue<void*> 	 queuedata;
	pthread_mutex_t  queuelock;
	pthread_cond_t   queueaval;
};


class TAnno : public IAnno
{
public:
	TAnno() : threadid(0), args(NULL), ipipe(NULL), opipe(NULL) {
		nid = nthread++;
	}
	~TAnno() {
	}
public:
	virtual int Run(void * opts, void * args) {
        this->opts = (IOpts*)opts;
		this->args = args;
        return pthread_create(&this->threadid, NULL, &ThreadProc, (void*)this);
	}
	virtual int Wait(void ** rets) {
		void * rett;
		return pthread_join(threadid, rets?rets:&rett);
	}
	virtual void Attach(IPipe * pipe, bool in = true) {
		(in?ipipe=pipe:opipe=pipe)->Attach((IAnno*)this,in);
	}
	virtual void Detach(bool in = true) {
		(in?ipipe:opipe)->Detach((IAnno*)this,in);
		(in?ipipe=NULL:opipe=NULL);
	}
protected:
	virtual void Init(){
//		fprintf(stdout, "[%02d][Print] Started.\n", nid);
	}
	virtual void * Main(void * args){
		return (void*) NULL;
	}
	virtual void Exit(){
		if(ipipe) Detach(true);
		if(opipe) Detach(false);
//		fprintf(stdout, "[%02d][Print] Exiting.\n", nid);
	}
protected:
    pthread_t threadid;
	IPipe *	  ipipe;
	IPipe *	  opipe;
	IOpts *	  opts;
	void  *   args;
	int		  nid;
private:
	static int nthread;
private:
	static void* ThreadProc(void * args) {
		void  * rets = NULL;
		TAnno * that = (TAnno*) args;
				that->Init();
		rets  = that->Main(that->args);
				that->Exit();
		return  rets;
	}
};
int TAnno::nthread = 0;

class TAnnoLoad : public TAnno
{
public:
	TAnnoLoad() {
	}
	~TAnnoLoad() {
	}
protected:
	virtual void * Main(void * args){
		const char * sbed = (*opts)["input-file"];
		gzFile 		 gbed = NULL;
		if( sbed){
			gbed = gzopen(sbed, "rb");
		} else {
			fprintf(stderr, "[%02d][Error] Null input file name.\n", nid);
		}
		char sbuf[1024];
		char schr[16];
		int  lpbk = LPBK;
		int  size = sizeof(BEDBLOCK)+lpbk*sizeof(int)*3;
		int  rpbk = 10;
		int  cidx = 0; 
		int  crow = 0; 
		int  cchr = 0;
		int  nchr = 0;
		int  nbeg = 0;
		int  nend = 0;
		BEDLINE  * lbed = NULL;
		BEDBLOCK * pbed = NULL;
		for(int nrow = 0; gbed && !gzeof(gbed); nrow = 0) {
			if((pbed=(BEDBLOCK*)opipe->Alloc(size)) == NULL) {
				fprintf(stderr, "[%02d][Error] Unable to allocate bed block.\n", nid);
				break;
			}
			lbed=(BEDLINE *)(pbed+1);
			if(cchr != nchr) {
//				fprintf(stdout, "[%02d][Print] New row [%4d\t%02d\t%9d\t%9d].\n", nid, nrow, nchr, nbeg, nend);
				lbed[nrow  ].nchr = cchr = nchr;
				lbed[nrow  ].nbeg = nbeg;
				lbed[nrow++].nend = nend;
			}
			while(nrow < rpbk && cchr==nchr && gzgets(gbed, sbuf, sizeof(sbuf)/sizeof(sbuf[0])) > 0){
				crow++;
				if(sscanf(sbuf, "chr%s%d%d", schr, &nbeg, &nend) ==3){
					nchr = (*schr=='X'?23:(*schr=='Y'?24:(*schr=='M'?25:atoi(schr))));
				}
				if(cchr == nchr) {
//					fprintf(stdout, "[%02d][Print] New row [%4d\t%02d\t%9d\t%9d].\n", nid, nrow, nchr, nbeg, nend);
					lbed[nrow  ].nchr = nchr;
					lbed[nrow  ].nbeg = nbeg;
					lbed[nrow++].nend = nend;
				}
			}
			if( nrow) {
				pbed->index = cidx++;
				pbed->nrows = nrow;
				pbed->ncols = 3;
				opipe->Push(pbed);
			} else {
				opipe->Free(pbed);
			}
			if(rpbk < lpbk) rpbk *= 2;
			if(rpbk > lpbk) rpbk  = lpbk;
		}
		for(int i = 0, I = atoi((*opts)["thread-number"]); i < I; i++){
			opipe->Push(NULL);
		}
		if(gbed){
			gzclose(gbed);
		}
		return (void*) NULL;
	}
};


class TAnnoSave : public TAnno
{
public:
	TAnnoSave() {
	}
	~TAnnoSave() {
	}
private:
	struct BININFO {
		int					cidx;
		vector<BINBLOCK*>	vbin;
		BININFO()  {
			cidx = 0;
		}
	};
	typedef	map<int,BININFO*> IMAP;
	typedef map<int,FILE*>    FMAP;
	IMAP	imap;
	FMAP	fmap;
protected:
	virtual void * Main(void * args) {
		int tnum = atoi((*opts)["thread-number"]);
		for(int t = tnum, b = 0, r = 0; t > 0 || r > 0; b++){ 
//			fprintf(stdout, "[%02d][Print] Received bin block [%d].\n", nid, b);
			BINBLOCK * pbin = (BINBLOCK*)ipipe->Pop();
			if(pbin != NULL) {
				BININFO * pinf = RegisterBlock(pbin);
				if(pinf != NULL){
					SaveBlock(pinf);
					r = RemainBlock();
				}
			} else {
				t--;
			}
		}
		FileClose();
	}
private:
	BININFO * RegisterBlock(BINBLOCK * pbin) {
		int group = pbin->group;
		int ntype = pbin->ntype;
		int binid = group*10 + ntype;
		if( imap.find(binid) == imap.end()) {
			imap[binid] = new BININFO;
		}
		BININFO * pinfo = imap[binid];
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
	void SaveBlock(BININFO * pinf) {
		vector<BINBLOCK*>  & vbin = pinf->vbin;
		int & cidx = pinf->cidx;
		for(int i = 0; i < vbin.size(); i++){
			BINBLOCK * pbin = vbin[i];
			if(pbin && pbin->index == cidx) {
//				fprintf(stdout, "[%02d][Print] Saving   bin block [%d].\n", nid, cidx);

				SaveBlock(pbin);

				cidx++;
				vbin.erase(vbin.begin()+i); i = -1;
			}
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
		}
		ipipe->Free(pbin);
	}
	FILE * FileOpen(BINBLOCK * pbin) {
		FILE * file = NULL;
		int    nfid = FileID(pbin);
		FMAP::iterator it = fmap.find(nfid);
		if(it == fmap.end()) {
			string name = FileName(pbin);
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
};

class TAnnoWork : public TAnno
{
public:
	TAnnoWork() : 
		head(NULL), bidx(NULL), bubk(NULL), bcbk(NULL), 
		fbin(NULL), cbin(  -1), cgrp(  -1), cbck(  -1), info(NULL), 
		fgrc(NULL), bgrc(NULL), peak(NULL), spot(NULL), open(NULL)
 	{
		lpbk = LPBK;
		epbk = EPBK;
		blen = BLEN;
		ilen = sizeof(BEDLINE);
		dlen = sizeof(int)*epbk;
		ppbn = PPBN;
		bpbn = BPBN;
		ppbk = PPBK;
		bpun = sizeof(int);
	}
	~TAnnoWork() {
		if(fbin) fclose(fbin);
		if(head) free(head);
		if(bidx) free(bidx);
		if(bubk) free(bubk);
		if(bcbk) free(bcbk);
		if(braw) free(braw);
		if(bzip) free(bzip);
		if(fgrc) free(fgrc);
		if(bgrc) free(bgrc);
		if(peak) free(peak);
		if(spot) free(spot);
		if(open) free(open);
	}
protected:
	virtual void * Main(void * args){
		for(BEDBLOCK * pbed; (pbed=(BEDBLOCK*)ipipe->Pop()) != NULL; ) {
//			fprintf(stdout, "[%02d][Print] Received bed block [%d].\n", nid, pbed->index);
			InitBlock(pbed);
			CalcBlock();
			FormatBlock();
			ipipe->Free(pbed);
		}
		opipe->Push(NULL);
	}
private:
	void InitBlock(BEDBLOCK * pbed) {
		nidx = pbed->index;
		nrow = pbed->nrows;
		ncol = epbk;
		memcpy(info=(int*)(info?info:malloc(lpbk*ilen)), pbed+1,   nrow*ilen);
		for(int g = 0; g < 5; g++) {
			memset(data[g]=(data[g]?data[g]:malloc(lpbk*dlen)), 0, lpbk*dlen);
		}
		if(braw == NULL) braw = malloc(lpbk*(ilen+dlen)*2);
		if(bzip == NULL) bzip = malloc(lpbk*(ilen+dlen)*2);
	}
	void CalcBlock() {
		for(int tgrp = 0; tgrp < 4; tgrp++){
			for(int trow = 0; trow < nrow; trow++){
				CalcRow(tgrp, trow);
			}
		}
	}
	void CalcRow(int tgrp, int trow) {
		int nchr = info[trow*3+0];
		int nbeg = info[trow*3+1];
		int nend = info[trow*3+2];
			nbeg = tgrp == 1 ? (nbeg+nend)/2 : nbeg;
			nend = tgrp == 1 ? (nbeg+1)      : nend;
		int nlen = nend - nbeg;
//		fprintf(stdout, "[%02d][Print] Calc row [%d\t%4d\t%02d\t%9d\t%9d].\n", nid, tgrp, trow, nchr, nbeg, nend);
		for(int npos = nbeg; npos < nend; npos++){
			void  * pvec = LoadVector(nchr, npos, tgrp);
			int   * ivec = (int*)  pvec;
			float * fvec = (float*)pvec;
			int   *	fgrc = this->fgrc + trow*epbk;
			int   *	bgrc = this->bgrc + trow*epbk;
			float *	peak = this->peak + trow*epbk;
			float *	spot = this->spot + trow*epbk;
			float *	open = this->open + trow*epbk;
			for(int e = 0; e < epbk; e++){
				if(tgrp == 0) {
					fgrc[e] += ivec[e];
					open[e]  = 1.0 * fgrc[e] / nlen;
				} else if(tgrp == 1) {
					bgrc[e]  = ivec[e];
					open[e] /= 1.0 * (bgrc[e]>0?bgrc[e]:1) / blen;
					open[e]  = open[e] > 1E-4 ? open[e] : 0.0;
				} else if(tgrp == 2) {
					peak[e]  = max(peak[e], fvec[e]);
					peak[e]  = peak[e] > 1E-4 ? peak[e] : 0.0;
				} else if(tgrp == 3) {
					spot[e]  = max(spot[e], fvec[e]);
					spot[e]  = spot[e] > 1E-4 ? spot[e] : 0.0;
				}
			}
		}
		this->nchr = nchr;
	}
	void FormatBlock() {
		for(int tgrp = 0; tgrp < 5; tgrp++){
			BINBLOCK * pbin = NULL;
			if((pbin=(BINBLOCK*)opipe->Alloc(sizeof(BINBLOCK)+lpbk*(ilen+dlen))) != NULL) {
				FormatBin(pbin, tgrp);
				opipe->Push(pbin);
			} else {
				fprintf(stderr, "[%02d][Error] Unable to allocate bin block.\n", nid);
			}
			int size = FormatTxt(tgrp);
			if((pbin =(BINBLOCK*)opipe->Alloc(sizeof(BINBLOCK)+size)) != NULL) {
				memcpy(pbin, bzip, sizeof(BINBLOCK)+size);
				opipe->Push(pbin);
			} else {
				fprintf(stderr, "[%02d][Error] Unable to allocate txt block.\n", nid);
			}
		}
	}
	void FormatBin(BINBLOCK * pbin, int tgrp) {
		pbin->index = nidx;
		pbin->group = tgrp;
		pbin->chrom = nchr;
		pbin->ntype = 0;
		pbin->nsize = nrow*(ilen+dlen);
		for(int trow = 0; trow < nrow; trow++) {
			int * thisinfo = ((int*)(pbin+1))     + trow*(3+epbk) + 0;
			int * thisdata = ((int*)(pbin+1))     + trow*(0+epbk) + 3;
			int * thatinfo = ((int*)(info  ))     + trow*(3);
			int * thatdata = ((int*)(data[tgrp])) + trow*epbk;
			memcpy(thisinfo, thatinfo, sizeof(int)*3);
			memcpy(thisdata, thatdata, sizeof(int)*epbk);
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
			int * info = ((int*)(this->info   ))    + trow*3;
			int * data = ((int*)(this->data[tgrp])) + trow*epbk;
			int   nchr = info[0];
			if(nchr < 23) p += sprintf(btxt+p, "chr%d", nchr);
			else		  p += sprintf(btxt+p, "chr%s", nchr==23?"X":(nchr==24?"Y":"M"));
			p += sprintf(btxt+p, "\t%d", info[1]);
			p += sprintf(btxt+p, "\t%d", info[2]);
			for(int e = 0; e < epbk; e++){
				if(tgrp < 2) p += sprintf(btxt+p, "\t%d",   ((int*)  data)[e]);
				else 	     p += sprintf(btxt+p, "\t%.4g", ((float*)data)[e]);
			}
			p += sprintf(btxt+p, "\n");
		}
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
		return bubk+trow*epbk*bpun;
	}
	void LoadHead(int tbin) {
		char sbin[1024] = {0};
		sprintf(sbin, "%s/%04d", (*opts)["data-path"], tbin);
		if( fbin ) fclose(fbin);
		if((fbin = fopen (sbin, "rb")) != NULL) {
			if(head == NULL) {
				head = (HEAD*)malloc(sizeof(HEAD)); 
			}
			if(head == NULL) {
				fprintf(stderr, "[%02d][Error] Unable to allocate memory for head [%s].\n", nid, sbin);
			}
			if(fseek(fbin,-sizeof(HEAD),SEEK_END)==0 && fread(head,sizeof(HEAD),1,fbin)==1) {
				cbin = tbin;
				cgrp = -1;
			} else {
				fprintf(stderr, "[%02d][Error] Unable to read head for bin file [%s].\n", nid, sbin);
			}
		} else {
			fprintf(stderr, "[%02d][Error] Unable to open bin file [%s].\n", nid, sbin);
		}
	}
	void LoadGroup(int tgrp) {
		int ppbk = PPBK;
		int epbk = EPBK;
		int bpun = sizeof(int);
		int bpbn = BPBN;
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
				if(decompress((BYTE*)bcbk,(BYTE*)bidx,clen,ulen,cmtd) == ulen){
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
		int ppbk = PPBK;
		int epbk = EPBK;
		int bpun = sizeof(int);
		int bpbn = BPBN;
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
				if(decompress((BYTE*)bcbk,(BYTE*)bubk,clen,ulen,cmtd) == ulen){
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
	}
private:
	struct {
		FILE * 	fbin;
		HEAD * 	head;
		UI64 * 	bidx;
		BYTE * 	bubk;
		BYTE * 	bcbk;
		int 	cgrp;
		int 	cbin;
		int 	cbck;
	};
	struct {
		int		lpbk;
		int		epbk;
		int		blen;
		int		ilen;
		int		dlen;
		int 	ppbn;
		int 	bpbn;
		int 	ppbk;
		int 	bpun;
	};
	struct {
		int	 	nidx;
		int		nchr;	
		int		nrow;
		int 	ncol;
		int   *	info;
		union {
		struct{
		int   *	fgrc;
		int   *	bgrc;
		float *	peak;
		float *	spot;
		float *	open;
		};
		void  * data[5];
		};
		void  * braw;
		void  * bzip;
	};
};


int create_anno(int argc, char * argv[])
{
	IOpts * iopts = new TAnnoOpts;
	TOpts * topts = (TOpts*)iopts;
//	topts->SetOpt("input-file", 	"/data/encode/human/dseq/peak/hg19/ENCFF001SOF/ENCFF001SOF.bed.gz");
//	topts->SetOpt("output-path", 	"/tmp/test");
	topts->SetOpt("input-file", 	argv[1]);
	topts->SetOpt("output-path", 	argv[2]);

	int     nwork = atoi((*iopts)["thread-number"])+2;
	IPipe * ipipe = new TQueuePipe( 50);
	IPipe * opipe = new TQueuePipe(512);
	IAnno** panno = new IAnno*[nwork];

    for(int i = 0; i < 1; i++){
		panno[i] = new TAnnoLoad;
		panno[i]->Attach(ipipe, false);
    }
    for(int i = 1; i < 2; i++){
        panno[i] = new TAnnoSave;
		panno[i]->Attach(opipe, true);
    }
    for(int i = 2; i < nwork; i++){
        panno[i] = new TAnnoWork;
		panno[i]->Attach(ipipe, true);
		panno[i]->Attach(opipe, false);
    }
    for(int i = 0; i < nwork; i++){
        panno[i]->Run(iopts, NULL);
    }
    for(int i = 0; i < nwork; i++){
        panno[i]->Wait(NULL);
    }
    for(int i = 0; i < nwork; i++){
        delete panno[i];
    }
	delete iopts;
	delete ipipe;
	delete opipe;

	return EXIT_SUCCESS;
}

int main(int argc, char * argv[])
{
	if(0) for(int i = 0; i < argc; i++){
		fprintf(stdout, "%d\t%s\n", i, argv[i]);
	}

	if(argc < 3){
		fprintf(stderr, "[Error]    Incorrect input arguments.\n");
	} else {
		return create_anno(argc, argv);
	}

	return EXIT_FAILURE;
}
