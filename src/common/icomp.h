#ifndef _ICOMP_H_
#define _ICOMP_H_

#include "iobjs.h"

#define CM_LZ4		1
#define CM_BZ2		2
#define CM_LZ4Z		1
#define CM_BZ2Z		2
#define CM_ZLIB		3
#define CM_LZMA		4

struct IComp : public Interface
{
	virtual int Compress  (const void * ubuf, void * cbuf, int ulen, int clen, int method = CM_ZLIB, int level = 6) = 0;
	virtual int Decompress(const void * cbuf, void * ubuf, int clen, int ulen, int method = CM_ZLIB) = 0;
};

#endif
