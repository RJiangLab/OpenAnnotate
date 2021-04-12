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


int main(int argc, char * argv[])
{
//	for(int i = 0; i < argc; i++){
//		fprintf(stdout, "%d\t%s\n", i, argv[i]);
//	}

	gzFile gtxt = NULL;
	gzFile gbin = NULL;
	gzFile gbed = NULL;
	gzFile gcnt = NULL;
	float* oval = NULL;
	float* tval = NULL;
	int  * fcnt = NULL;
	int  * bcnt = NULL;
	int    clen = 0;
	int    blen = 1000000;
	int    ppbn = 10000000;
	int    ppbk = 1000;

	if(argc == 3 && argv[1] && argv[2]){
		gtxt = gzopen(argv[1], "rb");
	} else {
		fprintf(stderr, "[Error]       Incorrect inputs.\n");
	}

	if(gtxt){
		for(char tbuf[1024] = {0}; gzgets(gtxt,tbuf,sizeof(tbuf)/sizeof(tbuf[0])) != NULL; memset(tbuf,0,sizeof(tbuf))){
			int    tlen = 0;
			int    npos = 0;
			int    ncnt = 0;
			sscanf(tbuf, "%s", tbuf);
//			fprintf(stdout, "Loading       %s\n", tbuf);
			if(strncmp(tbuf+strlen(tbuf)-6,"bed.gz",6) == 0){
				if((gbed = gzopen (tbuf, "rb")) == NULL){
					fprintf(stderr, "[Warng]       Unable to open bed file.\n");
				}
				continue;
			}
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
				fcnt  = (int  *) malloc(tlen);
				bcnt  = (int  *) malloc(tlen);
				tval  = (float*) malloc(ppbk*sizeof(float));
				oval  = (float*) malloc(tlen);
				memset(fcnt, 0, tlen);
				memset(bcnt, 0, tlen);
				memset(oval, 0, tlen);
			} else if (clen  != tlen){
				fprintf(stderr, "[Error]       Chromosome length does not match.\n");
				break;
			}
			if(fcnt == NULL || bcnt == NULL || tval == NULL || oval == NULL){
				fprintf(stderr, "[Error]       Empty buffer.\n");
				break;
			}
			while( gzgets(gcnt,tbuf,sizeof(tbuf)/sizeof(tbuf[0])) != NULL){
				if(sscanf(tbuf, "%*d:%d%d", &npos, &ncnt) != 2){
					fprintf(stderr, "[Error]       Incorrect number of cnt items read %s", tbuf);
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
	if(fcnt && bcnt && oval && gbed){
		int    nbeg = 0;
		int    nend = 0;
		float  open = 0.0;
		for(char tbuf[1024] = {0}; gzgets(gbed,tbuf,sizeof(tbuf)/sizeof(tbuf[0])) != NULL; ){
			if(sscanf(tbuf, "chr%*s\t%d%d", &nbeg, &nend) != 2){
				fprintf(stderr, "[Error]       Incorrect number of bed items read %s", tbuf);
				continue;
			}
			if(nbeg  > nend){
				nend = nbeg + 1;
			}
			if(nbeg < 0 || nbeg >= clen){
				fprintf(stderr, "[Error]       Incorrect begin read.\n");
				continue;
			}
			if(nbeg < 1 || nbeg > clen){
				fprintf(stderr, "[Error]       Incorrect end read.\n");
				continue;
			}
			int fsum  = 0;
			int bsum  = bcnt[(nbeg+nend)/2];
			int flen  = nend-nbeg;
			for(int i = nbeg; i < nend; i++){
				fsum += fcnt[i];
			}
			bsum -=  fsum;
			bsum  = (bsum>0) ? bsum : 1;
			open  = 1.0 * fsum * blen / flen / bsum;
			for(int  i = nbeg; i < nend; i++){
				if(  oval[i] < open){
					 oval[i] = open;
				}
			}
		}
	}
	if(oval){
//		fprintf(stdout, "Writing       %s\n", argv[2]);
		for(int i = 0; i <= clen/ppbn; i++){
			char    fnbn[1024];
			sprintf(fnbn, "%s%02d.bin", argv[2], i);
			if((gbin = gzopen(fnbn, "wb")) != NULL){
				for(int j = 0; j < ppbn; j += ppbk){
					gzwrite(gbin, oval+i*ppbn+j, ppbk*sizeof(float));
				}
				gzclose(gbin);
			} else {
				fprintf(stderr, "[Error]       Unable to open %s for write.\n", fnbn);
			}
			if((gbin = gzopen(fnbn, "rb")) != NULL){
				for(int j = 0; j < ppbn; j += ppbk){
					gzread(gbin, tval, ppbk*sizeof(float));
					if(memcmp(tval, oval+i*ppbn+j, ppbk*sizeof(float))){
						fprintf(stderr, "[Error]       tval != oval at position [%d].\n", i*ppbn+j);
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
	if(oval){
		free(oval);
	}
	if(tval){
		free(tval);
	}
	if(gcnt){
		gzclose(gcnt);
	}
	if(gbed){
		gzclose(gbed);
	}
	if(gtxt){
		gzclose(gtxt);
	}

	return 0;
}
