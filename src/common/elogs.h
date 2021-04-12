#ifndef _DEBUG_H_
#define _DEBUG_H_

#define LOG(...)											\
{															\
	fprintf(stderr, "%-20s [%d]:\t", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);							\
}

#ifdef _DEBUG_
#define DPRINTF(...)										\
{															\
	fprintf(stderr, "%-20s [%d]:\t", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);							\
}
#else
#define DPRINTF(...)
#endif

#define LPRINTF0(...)										\
	lprintf(0, __VA_ARGS__)

#define LPRINTF1(...)										\
	lprintf(1, __VA_ARGS__)

#define LPRINTF2(...)										\
	lprintf(2, __VA_ARGS__)

#define LPRINTF3(...)										\
	lprintf(3, __VA_ARGS__)

#define EPRINTF(...)										\
	eprintf(__VA_ARGS__)

#define OPRINTF(...)										\
	oprintf(__VA_ARGS__)

#ifdef _SERVER_
#define RPRINTF(...)										\
	rprintf(__VA_ARGS__)
#else
#define RPRINTF(...)
#endif

#endif
