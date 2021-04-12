#ifndef _TYPE_H_
#define _TYPE_H_


#define BF_NONE		0X00000000
#define BF_EXIT		0X80000000
#define BF_COMP		0X0000000F
#define BF_ZLIB		0X00000001
#define BF_LZ4Z		0X00000002
#define BF_BZ2Z		0X00000004
#define BF_LZMA		0X00000008


#define LPBK 		100
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
	int		nstr;
};

struct BEDBLOCK
{
	int		nsize;
	int		nflag;
	int		index;
	int		nrows;
	int		ncols;
};

struct BINBLOCK
{
	int		nsize;
	int		nflag;
	int		index;
	int		nrows;
	int		ncols;
	int		group;
	int		ntype;
	int		chrom;
};

#endif
