#ifndef _ECOMP_H_
#define _ECOMP_H_

#include "tcomp.h"

extern int Compress(const void * ubuf, void * cbuf, int ulen, int clen, int method = CM_ZLIB, int level = 6);
extern int Decompress(const void * cbuf, void * ubuf, int clen, int ulen, int method = CM_ZLIB);

#endif
