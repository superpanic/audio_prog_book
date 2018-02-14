#ifndef __BREAKPOINT__
#define __BREAKPOINT__

#include <stdio.h>
#include <stdint.h>

typedef struct breakpoint {
	double time;
	double value;
} BREAKPOINT;

typedef struct breakpoint_stream {
	BREAKPOINT *points;
	BREAKPOINT leftpoint;
	BREAKPOINT rightpoint;
	uint32_t npoints;
	double curpos;
	double incr;
	double width;
	double height;
	uint32_t ileft;
	uint32_t iright;
	uint16_t more_points;
} BRKSTREAM;

BREAKPOINT* get_breakpoints(FILE* fp, uint32_t* psize);
BREAKPOINT maxpoint(const BREAKPOINT* points, unsigned long npoints);
BREAKPOINT minpoint(const BREAKPOINT* points, unsigned long npoints);

void bps_minmax(const BREAKPOINT* points, uint32_t npoints, double *min, double *max);

int inrange(const BREAKPOINT *points, double minval, double maxval, unsigned long npoints);
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time);

BRKSTREAM *bps_newstream(FILE *fp, uint32_t srate, uint32_t *size);
void bps_freepoints(BRKSTREAM* stream);

#endif