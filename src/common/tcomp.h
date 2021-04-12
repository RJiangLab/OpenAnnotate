#ifndef _TCOMP_H_
#define _TCOMP_H_

#include <lz4.h>
#include <lz4hc.h>
#include <bzlib.h>
#include <zlib.h>
#include <lzma.h>
#include "icomp.h"

class TComp : public IComp {
public:
	virtual int Compress(const void * ubuf, void * cbuf, int ulen, int clen, int method = CM_ZLIB, int level = 6) {
		if(method == CM_LZ4){
			return lz4_compress (ubuf, cbuf, ulen, clen, level);
		} else if(method == CM_BZ2){
			return bz2_compress(ubuf, cbuf, ulen, clen, level);
		} else if(method == CM_ZLIB){
			return zlib_compress(ubuf, cbuf, ulen, clen, level);
		} else if(method == CM_LZMA){
			return lzma_compress(ubuf, cbuf, ulen, clen, level);
		}
		return (-1);
	}
	virtual int Decompress(const void * cbuf, void * ubuf, int clen, int ulen, int method = CM_ZLIB) {
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
private:
	int lz4_compress(const void * ubuf, void * cbuf, int ulen, int clen, int level) {
#if 1
		return LZ4_compress_HC((const char*)ubuf, (char*)cbuf, ulen, clen, level);
#else
		return LZ4_compress_default((const char*)ubuf, (char*)cbuf, ulen, clen);
#endif
	}
	int lz4_decompress(const void * cbuf, void * ubuf, int clen, int ulen) {
#if 1
		return (LZ4_decompress_fast((const char*)cbuf, (char*)ubuf, ulen) == clen) ? ulen : -1;
#else
		return LZ4_decompress_safe((const char*)cbuf, (char*)ubuf, clen, ulen);
#endif
	}
private:
	int bz2_compress(const void * ubuf, void * cbuf, int ulen, int clen, int level) {
		unsigned int dlen = clen;
		return (BZ2_bzBuffToBuffCompress((char*)cbuf, &dlen, (char*)(*(&ubuf)), ulen, level, 0, 0) == BZ_OK) ? dlen : -1;
	}
	int bz2_decompress(const void * cbuf, void * ubuf, int clen, int ulen) {
		unsigned int dlen = ulen;
		return (BZ2_bzBuffToBuffDecompress((char*)ubuf, &dlen, (char*)(*(&cbuf)), clen, 0, 0) == BZ_OK) ? dlen : -1;
	}
private:
	int zlib_compress(const void * ubuf, void * cbuf, int ulen, int clen, int level) {
		uLongf dlen = clen;
#if 1
		return (compress2((Bytef*)cbuf, &dlen, (const Bytef*)ubuf, (uLong)ulen, level) == Z_OK) ? dlen : -1;
#else
		return (compress ((Bytef*)cbuf, &dlen, (const Bytef*)ubuf, (uLong)ulen)    == Z_OK) ? dlen : -1;
#endif
	}
	int zlib_decompress(const void * cbuf, void * ubuf, int clen, int ulen) {
		uLongf dlen = ulen;
		return (uncompress((Bytef*)ubuf, &dlen, (const Bytef*)cbuf, (uLong)clen) == Z_OK) ? dlen : -1;
	}
private:
	int lzma_compress(const void * ubuf, void * cbuf, int ulen, int clen, int level) {
		lzma_check 	check 	= LZMA_CHECK_CRC64;
		uint32_t 	preset 	= level;
		size_t		out_pos	= 0;
		return (lzma_easy_buffer_encode(preset, check, NULL, (const uint8_t*)ubuf, (size_t)ulen, (uint8_t*)cbuf, &out_pos, (size_t)clen) == LZMA_OK) ? out_pos : -1;
	}
	int lzma_decompress(const void * cbuf, void * ubuf, int clen, int ulen) {
		uint64_t 	memlimit = UINT64_MAX;
		uint32_t   	flags    = 0;
		size_t		in_pos	 = 0;
		size_t		out_pos	 = 0;
		return (lzma_stream_buffer_decode(&memlimit, flags, NULL, (const uint8_t*)cbuf, &in_pos, (size_t)clen, (uint8_t*)ubuf, &out_pos, (size_t)ulen) == LZMA_OK) ? out_pos : -1;
	}
};

#endif
