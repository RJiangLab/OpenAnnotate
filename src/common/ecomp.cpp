#include "icomp.h"
#include "tcomp.h"
#include "ecomp.h"

int Compress(const void * ubuf, void * cbuf, int ulen, int clen, int method, int level)
{
	TComp  comp;
	return comp.Compress(ubuf, cbuf, ulen, clen, method, level);
}

int Decompress(const void * cbuf, void * ubuf, int clen, int ulen, int method)
{
	TComp  comp;
	return comp.Decompress(cbuf, ubuf, clen, ulen, method);
}
