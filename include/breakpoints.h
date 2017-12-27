#include <stdio.h>

typedef struct breakpoint {
	double time;
	double value;
} BREAKPOINT;

BREAKPOINT* get_breakpoints(FILE* fp, unsigned long* psize);
BREAKPOINT maxpoint(const BREAKPOINT* points, unsigned long npoints);
int inrange(const BREAKPOINT *points, double minval, double maxval, unsigned long npoints);
