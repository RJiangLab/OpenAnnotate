#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <pthread.h>
#include <memory.h>
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

#define BLEN 		1000000
#define PPBK 		100
#define PPBN 		10000000
#define BPBN 		(PPBN/PPBK)
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

typedef struct _HEAD_
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
} HEAD;

//	int LZ4_compress_HC(const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel);
//	int LZ4_compress_default(const char* source, char* dest, int sourceSize, int maxDestSize);
//	
int lz4_compress(const BYTE * ubuf, BYTE * cbuf, int ulen, int clen)
{
#if 1
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
#if 1
	return (LZ4_decompress_fast(cbuf, ubuf, ulen) == clen) ? ulen : -1;
#else
	return LZ4_decompress_safe(cbuf, ubuf, clen, ulen);
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
#if 1
	return (compress2((Bytef*)cbuf, &dlen, (const Bytef*)ubuf, (uLong)ulen, 9) == Z_OK) ? dlen : -1;
#else
	return (compress ((Bytef*)cbuf, &dlen, (const Bytef*)ubuf, (uLong)ulen)    == Z_OK) ? dlen : -1;
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

HEAD * create_head()
{
	HEAD * head = (HEAD*) malloc(sizeof(HEAD));
	if(head){
		memset(head, 0, sizeof(HEAD));

		strcpy(head->title, "OPENANNO");
		head->cmethod = CM_ZLIB;
		head->tmstamp = time(NULL);
		head->version = 0X00000100;
		head->bingrps;
		head->grprows;
		head->grpcols;
		head->grpbcks;
		head->bckrows;
		head->bckcols;
		head->cellsize;
	}
	return head;
}

typedef vector<string> SVEC;
typedef vector<void *> PVEC;

int create_dump(int argc, char * argv[])
{
	FILE * 	ftxt = NULL;
	FILE * 	fbin = NULL;
	FILE * 	flog = NULL;
	HEAD *	head = NULL;
	UI32 *	bubk = NULL;
	UI32 *	bcbk = NULL;
	UI32 *	btbk = NULL;
	UI64 *	bidx = NULL;
	UI64 *  gidx = NULL;
	UI32    ppbn = PPBN;
	UI32    ppbk = PPBK;
	UI32   	bpbn = BPBN;
	BYTE	cmtd = CM_ZLIB;
	UI32	epbn = 0;
	UI32	epbk = 0;
	SVEC	epvc;
	PVEC	egvc;
	UI32	gpbn = 0;
	SVEC	gpvc;

	if(argc == 4 && argv[1]){
		if(strcmp(argv[1], "lz4") == 0){
			cmtd = CM_LZ4;	
		} else if(strcmp(argv[1], "bz2") == 0){
			cmtd = CM_BZ2;
		} else if(strcmp(argv[1], "lz4z") == 0){
			cmtd = CM_LZ4;
		} else if(strcmp(argv[1], "bz2z") == 0){
			cmtd = CM_BZ2;
		} else if(strcmp(argv[1], "zlib") == 0){
			cmtd = CM_ZLIB;
		} else if(strcmp(argv[1], "lzma") == 0){
			cmtd = CM_LZMA;
		}	
		argc--;
		argv++;	
	}

	if(argc == 3 && argv[1] && argv[2]){
		char sbuf[1024] = {0};

		sprintf(sbuf, "%s", argv[1]);
		ftxt = fopen(sbuf, "rb");

		sprintf(sbuf, "%s.bin", argv[2]);
		fbin = fopen(sbuf, "wb");

		sprintf(sbuf, "%s.log", argv[2]);
		flog = fopen(sbuf, "wb");
	} else {
		fprintf(stderr, "[Error]    Incorrect command line argument.\n");
	}

	if(ftxt && fbin && flog){
		char stxt[1024] = {0};
		while(fscanf(ftxt, "%s", stxt)==1){
			fprintf( flog, "Read txt path [%s].\n", stxt);
			gpvc.push_back(stxt);
			gpbn++;
		}
		fclose(ftxt);
	} else {
		fprintf(stderr, "[Error]    Unable to open input file.\n");
	}
	if(gpbn){
		bidx = (UI64*) malloc(gpbn*(bpbn+1)*sizeof(UI64));
	}

	fprintf(stdout, "[Report]    gpbn=%d.\n", gpbn);
	for(int g = 0; g < gpbn; g++){
		epvc.clear();
		egvc.clear();
		epbk = 0;
		if((ftxt=fopen(gpvc[g].c_str(), "rb")) != NULL){
			char sbin[1024] = {0};
			while(fscanf(ftxt, "%s", sbin)==1){
				fprintf(flog, "Read bin path [%s].\n", sbin);
				epvc.push_back(sbin);
				egvc.push_back(gzopen(sbin, "rb"));
				epbk++;
			}
			fclose(ftxt);
		}
		fprintf(stdout, "[Report]    ppbk=%d.\n", ppbk);
		fprintf(stdout, "[Report]    epbk=%d.\n", epbk);
		if( head == NULL && epbk){
			head = (HEAD*) create_head();
			bubk = (UI32*) malloc(ppbk*epbk*sizeof(UI32)+1024);
			bcbk = (UI32*) malloc(ppbk*epbk*sizeof(UI32)+1024);
			btbk = (UI32*) malloc(ppbk*epbk*sizeof(UI32)+1024);
		}
		if( bidx != NULL){
			gidx = (UI64*)(bidx+g*(bpbn+1));
		}
		if(epbk && bubk && bcbk && btbk && bidx){
			memset(gidx, 0, (bpbn+1)*sizeof(UI64));
			for(int b  = 0; b < bpbn; b++){
				memset(bubk, 0, ppbk*epbk*sizeof(UI32)+1024);
				for(int e =  0; e < epbk; e++){
					if(egvc[e] && gzread((gzFile)egvc[e], btbk, ppbk*sizeof(UI32))==ppbk*sizeof(UI32)){
						for(int  p = 0; p < ppbk; p++){
							bubk[p*epbk+e] = btbk[p]; 
						}
					} else {
//						fprintf(stdout, "[Report]    Unable to read count from [%s].\n", epvc[e].c_str());
					}
				}
				gidx[b] = ftell(fbin);
				int ulen = ppbk*epbk*sizeof(UI32);
				int clen = ulen;
				    clen = compress((BYTE*)bubk, (BYTE*)bcbk, ulen, clen, cmtd);
				if( clen > 0){
					fwrite(bcbk, sizeof(BYTE), clen, fbin);
				} else {					
					fprintf(stderr, "[Error]    Unable to compress block [%g, %d].\n", g, b);
				}
				gidx[b+1] = ftell(fbin);
//				gidx[b+1] = gidx[b] + (clen>0?clen:0);
#ifdef CHECK_COMPRESS
				if(decompress((BYTE*)bcbk, (BYTE*)btbk, clen, ulen, cmtd) != ulen){
					fprintf(stderr, "[Error]    Unable to decompress block [%d, %d].\n", g, b);
				}
				if(memcmp(bubk, btbk, ulen)){
					fprintf(stderr, "[Error]    Uncompressed and compressed block does not equal [%d, %d].\n", g, b);
				}
#endif 
				fprintf(flog, "Block done, [%2d, %6d, %6d] bytes.\n", g, b, clen);
			}
		} else {
			fprintf(stderr, "[Error]    Empty task file.\n");
		}
		for(int i = 0; i < egvc.size(); i++){
			if(egvc[i]){
				gzclose((gzFile)egvc[i]);
			}
		}
		if( epbn != epbk){
			epbn  = epbk;
		}
	}
	{
		BYTE * cidx = (BYTE*)malloc((bpbn+1)*sizeof(UI64)+1024);
		if(cidx) for(int g = 0; g < gpbn; g++){
			int ulen = (bpbn+1)*sizeof(UI64);
			int clen = ulen;
		    	clen = compress((BYTE*)(bidx+g*(bpbn+1)), (BYTE*)cidx, ulen, clen, cmtd);
			if( clen > 0){
				head->indexpos[g] = ftell(fbin);
				fwrite(cidx, sizeof(BYTE), clen, fbin);
				head->indexpos[g+1] = ftell(fbin);
			} else {
				fprintf(stderr, "[Error]    Unable to compress indices.\n");
			}
		}
		if(cidx) free(cidx);
	}
	if(gpvc.size()){
		head->cellsize = sizeof(UI32);
		head->bingrps  = gpvc.size();
		head->grprows  = ppbn;
		head->grpcols  = epbn;
		head->grpbcks  = bpbn;
		head->bckrows  = ppbk;
		head->bckcols  = epbn;
		head->cmethod  = cmtd;
		head->tmstamp  = time(NULL);
		fwrite(head, sizeof(HEAD), 1, fbin);
	}

	if(fbin){
		fclose(fbin);
	}
	if(flog){
		fclose(flog);
	}
	if(head){
		free(head);
	}
	if(bubk){
		free(bubk);
	}
	if(bcbk){
		free(bcbk);
	}
	if(btbk){
		free(btbk);
	}
	if(bidx){
		free(bidx);
	}

	return EXIT_SUCCESS;
}

#if 0

#define BLEN 1000000
#define EPBK 531
#define PPBN 10000000
#define PPBK 1000
#define BPBN (PPBN/PPBK)
#define DATA "/home/open/human/hg19/data/anno/code/dseq/read/lz4z"
#define fseek fseeko64
#define ftell ftello64

int create_scan(int argc, char * argv[])
{
	gzFile gbed = NULL;
	FILE * fbin = NULL;
	UI32 * bubk = NULL;
	UI32 * bcbk = NULL;
	UI64 * bidx = NULL;
	UI32 * fcnt = NULL;
	UI32 * bcnt = NULL;
	HEAD   head = {0};

	if( argc == 2 && argv[1]){
		gbed = gzopen(argv[1], "rb");
	} else {
		fprintf(stderr, "[Error]    Incorrect input arguments.\n");
	}
	if( gbed){
		bubk = (UI32*) malloc( PPBK*EPBK*sizeof(UI32)*2);
		bcbk = (UI32*) malloc( PPBK*EPBK*sizeof(UI32)*2);
		bidx = (UI64*) malloc((BPBN+1)  *sizeof(UI64));
		fcnt = (UI32*) malloc( EPBK*sizeof(UI32));
		bcnt = (UI32*) malloc( EPBK*sizeof(UI32));
	} else {
		fprintf(stderr, "[Error]    Empty task file.\n");
	}
	if( bubk && bcbk && bidx && fcnt && bcnt){
		char sbin[1024];
		char sbuf[1024];
		char schr[16];
		int  nchr = 0;
		int  nbeg = 0;
		int  nend = 0;
		int	 line = 0; 
		int  cbin = 0;
		int  cbck = -1;
		while(gzgets(gbed, sbuf, sizeof(sbuf)/sizeof(sbuf[0]))){
			line++;
			if(sscanf(sbuf, "chr%s%d%d", schr, &nbeg, &nend)==3){
				nchr=(*schr=='X'?23:(*schr=='Y'?24:(*schr=='M'?25:atoi(schr))));
				memset(fcnt, 0, EPBK*sizeof(UI32));
				memset(bcnt, 0, EPBK*sizeof(UI32));
				for(int p = nbeg; p < nend; p++){
					int tbin  = nchr*100 + p/PPBN;
					int tbck  = p%PPBN/PPBK;
					int trow  = p%PPBK;
					if( tbin != cbin){
						sprintf(sbin, "%s/%04d", DATA, tbin);
						if( fbin ) fclose(fbin);
						if((fbin = fopen (sbin, "rb")) != NULL){
							fseek(fbin, -sizeof(HEAD), SEEK_END);
							if(fread(&head, sizeof(HEAD), 1, fbin) == 1){
								fseek(fbin, head.nidxpos, SEEK_SET);
								if(fread(bcbk, head.nidxlen, 1, fbin) == 1){
									if(decompress((BYTE*)bcbk, (BYTE*)bidx, head.nidxlen, (BPBN+1)*sizeof(UI64), head.cmethod) == (BPBN+1)*sizeof(UI64)){
										cbin = tbin;
										cbck = -1;
									} else {
										fprintf(stderr, "[Error]    Unable to decompress head block for bin file [%s].\n", sbin);
									}
								} else {
									fprintf(stderr, "[Error]    Unable to read index for bin file [%s].\n", sbin);
								}
							} else {
								fprintf(stderr, "[Error]    Unable to read head for bin file [%s].\n", sbin);
							}
						} else {
							fprintf(stderr, "[Error]    Unable to open bin file [%s].\n", sbin);
						}
					}
					if(tbck != cbck){
						if(fbin != NULL){
							int clen = bidx[tbck+1]-bidx[tbck];
							int ulen = PPBK*EPBK*sizeof(UI32)*2;
							int cmtd = head.cmethod;
							fseek(fbin, bidx[tbck], SEEK_SET);
							if(fread(bcbk, clen, 1, fbin) == 1){
								if(decompress((BYTE*)bcbk, (BYTE*)bubk, clen, ulen, cmtd) == ulen){
									cbck = tbck;
								} else {
									fprintf(stderr, "[Error]    Unable to decompress data block for bin file [%s, %d].\n", sbin, tbck);
								}
							} else {
								fprintf(stderr, "[Error]    Unable to read data block from bin file [%s, %d].\n", sbin, tbck);
							}
						} else {
							fprintf(stderr, "[Error]    Unable to open bin file [%s].\n", sbin);
						}
					}
					for(int e = 0; e < EPBK; e++){
						fcnt[e] += bubk[trow*EPBK + e];
						bcnt[e] += bubk[trow*EPBK + e + PPBK*EPBK];
					}
				}
				int flen = nend-nbeg;
				if( flen < 1) flen = 1;
				for(int e = 0; e < EPBK; e++){
					bcnt[e] /= flen;
					bcnt[e] -= fcnt[e];
				}
				fprintf(stdout, "chr%s\t%d\t%d", schr, nbeg, nend);
				for(int e = 0; e < EPBK; e++){
					if(bcnt[e] == 0) bcnt[e] = 1;
					float open = 1.0 * BLEN / flen * fcnt[e] / bcnt[e];
					fprintf(stdout, "\t%.4f", open);
				}
				fprintf(stdout, "\n");
			} else {
				fprintf(stderr, "[Error]    Incorrect input format [%d, %s].\n", line, sbuf);
			}
		}
	}
	if(gbed){
		gzclose(gbed);
	}
	if(fbin){
		fclose(fbin);
	}
	if(bubk){
		free(bubk);
	}
	if(bcbk){
		free(bcbk);
	}
	if(bidx){
		free(bidx);
	}
	if(fcnt){
		free(fcnt);
	}
	if(bcnt){
		free(bcnt);
	}
}
#endif

int main(int argc, char * argv[])
{
	if(0) for(int i = 0; i < argc; i++){
		fprintf(stdout, "%d\t%s\n", i, argv[i]);
	}

	if(argc < 3){
		fprintf(stderr, "[Error]    Incorrect input arguments.\n");
	} else if(strcmp(argv[1], "scan") == 0){
//		return create_scan(--argc, ++argv);
	} else if(strcmp(argv[1], "dump") == 0){
		return create_dump(--argc, ++argv);
	} else {
		return create_dump(argc, argv);
	}	
	
	return EXIT_FAILURE;
}
