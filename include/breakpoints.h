#include <stdio.h>

typedef struct breakpoint {
	double time;
	double value;
} BREAKPOINT;

BREAKPOINT* get_breakpoints(FILE* fp, unsigned long* psize);
BREAKPOINT maxpoint(const BREAKPOINT* points, unsigned long npoints);
BREAKPOINT minpoint(const BREAKPOINT* points, unsigned long npoints);
void maxmin(const BREAKPOINT* points, unsigned long npoints, BREAKPOINT maxmin[2]);

int inrange(const BREAKPOINT *points, double minval, double maxval, unsigned long npoints);
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time);
