#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <zlib.h>

using namespace std;


#define BLOCK_LENGTH		1000000
#define POSITION_PER_BIN	10000000
#define POSITION_PER_BLOCK	1000

int main(int argc, char * argv[])
{
//	for(int i = 0; i < argc; i++){
//		fprintf(stdout, "%d\t%s\n", i, argv[i]);
//	}

	gzFile gtxt = NULL;
	gzFile gbin = NULL;
	gzFile gcnt = NULL;
	int  * fcnt = NULL;
	int  * bcnt = NULL;
	int  * tcnt = NULL;
	int    clen = 0;
	int    blen = BLOCK_LENGTH;
	int    ppbn = POSITION_PER_BIN;
	int    ppbk = POSITION_PER_BLOCK;

	if(argc == 3 && argv[1] && argv[2]){
		gtxt = gzopen(argv[1], "rb");
	} else {
		fprintf(stderr, "[Error]       Incorrect inputs.\n");
	}

	if(gtxt){
		for(char   tbuf[1024] = {0}; gzgets(gtxt,tbuf,sizeof(tbuf)/sizeof(tbuf[0])) != NULL; ){
			int    tlen = 0;
			int    npos = 0;
			int    ncnt = 0;
			sscanf(tbuf, "%s", tbuf);
//			fprintf(stdout, "Loading       %s\n", tbuf);
			if( gcnt ) gzclose(gcnt);
			if((gcnt = gzopen (tbuf, "rb")) == NULL){
				fprintf(stderr, "[Error]       Unable to open cnt file.\n");
				break;
			}
			if(gzgets(gcnt,tbuf,sizeof(tbuf)/sizeof(tbuf[0])) == NULL){
				fprintf(stderr, "[Error]       Unable to read cnt file.\n");
				break;
			} else if(sscanf(tbuf, "%*s%d", &tlen) != 1){
				fprintf(stderr, "[Error]       Incorrect number of items read.\n");
				break;
			}
			if( fcnt == NULL){
				clen  = tlen;
				tlen  = (tlen+ppbn)*sizeof(int);
				fcnt  = (int *) malloc(tlen);
				bcnt  = (int *) malloc(tlen);
				tcnt  = (int *) malloc(ppbk*sizeof(int));
				memset(fcnt, 0, tlen);
				memset(bcnt, 0, tlen);
			} else if (clen  != tlen){
				fprintf(stderr, "[Error]       Chromosome length does not match.\n");
				break;
			}
			if(fcnt == NULL || bcnt == NULL){
				fprintf(stderr, "[Error]       Empty buffer.\n");
				break;
			}
			while( gzgets(gcnt,tbuf,sizeof(tbuf)/sizeof(tbuf[0])) != NULL){
				if(sscanf(tbuf, "%*d:%d%d", &npos, &ncnt) != 2){
					fprintf(stderr, "[Error]       Incorrect number of items read %s", tbuf);
					continue;
				}
				if(npos < 0 || npos >= clen){
					fprintf(stderr, "[Error]       Incorrect position read.\n");
					continue;
				}
				if(ncnt <= 0){
					fprintf(stderr, "[Error]       Incorrect count read.\n");
					continue;
				}
				fcnt[npos] += ncnt;
			}
		}
	}
	if(fcnt && bcnt){
    	int ncnt = 0;
		if( blen >= clen){
			for(int i = 0; i < clen; i++){
				ncnt += fcnt[i];
			}
			for(int i = 0; i < clen; i++){
				bcnt[i] = ncnt;
			}
		} else {
			for(int i = 0; i < blen; i++){
				ncnt += fcnt[i];
			}
			for(int i = 0; i < blen/2; i++){
				bcnt[i] = ncnt;
			}
			for(int i = blen/2,j = 0,k = blen; k < clen; i++,j++,k++){
				ncnt -= fcnt[j];
				ncnt += fcnt[k];
				bcnt[i] = ncnt;
			}
			for(int i = clen-blen/2; i < clen; i++){
				bcnt[i] = ncnt;
			}
		}
	}
	if(fcnt && bcnt){
//		fprintf(stdout, "Writing       %s\n", argv[2]);
		for(int i = 0; i <= clen/ppbn; i++){
			char    fnbn[1024];
			sprintf(fnbn, "%s%02d.fgrc.bin", argv[2], i);
			if((gbin = gzopen(fnbn, "wb")) != NULL){
				for(int j = 0; j < ppbn; j += ppbk){
					gzwrite(gbin, fcnt+i*ppbn+j, ppbk*sizeof(int));
				}
				gzclose(gbin);
			} else {
				fprintf(stderr, "[Error]       Unable to open %s for write.\n", fnbn);
			}
			if((gbin = gzopen(fnbn, "rb")) != NULL){
				for(int j = 0; j < ppbn; j += ppbk){
					gzread(gbin, tcnt, ppbk*sizeof(int));
					if(memcmp(tcnt, fcnt+i*ppbn+j, ppbk*sizeof(int))){
						fprintf(stderr, "[Error]       tcnt != fcnt at position [%d].\n", i*ppbn+j);
					}
				}
				gzclose(gbin);
			} else {
				fprintf(stderr, "[Error]       Unable to open %s for read.\n", fnbn);
			}

			sprintf(fnbn, "%s%02d.bgrc.bin", argv[2], i);
			if((gbin = gzopen(fnbn, "wb")) != NULL){
				for(int j = 0; j < ppbn; j += ppbk){
					gzwrite(gbin, bcnt+i*ppbn+j, ppbk*sizeof(int));
				}
				gzclose(gbin);
			} else {
				fprintf(stderr, "[Error]       Unable to open %s for write.\n", fnbn);
			}
			if((gbin = gzopen(fnbn, "rb")) != NULL){
				for(int j = 0; j < ppbn; j += ppbk){
					gzread(gbin, tcnt, ppbk*sizeof(int));
					if(memcmp(tcnt, bcnt+i*ppbn+j, ppbk*sizeof(int))){
						fprintf(stderr, "[Error]       tcnt != bcnt at position [%d].\n", i*ppbn+j);
					}
				}
				gzclose(gbin);
			} else {
				fprintf(stderr, "[Error]       Unable to open %s for read.\n", fnbn);
			}
		}
	}
	if(fcnt){
		free(fcnt);
	}
	if(bcnt){
		free(bcnt);
	}
	if(tcnt){
		free(tcnt);
	}
	if(gcnt){
		gzclose(gcnt);
	}
	if(gtxt){
		gzclose(gtxt);
	}

	return 0;
}
