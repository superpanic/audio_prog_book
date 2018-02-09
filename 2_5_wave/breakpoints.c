#include <breakpoints.h>
#include <stdlib.h>
#include <stdint.h>

BREAKPOINT* get_breakpoints(FILE* fp, uint32_t *psize) {
	int got;
	long npoints = 0;
	long size = 64;
	double lasttime = 0.0;
	BREAKPOINT* points = NULL;
	char line[80];

	if(fp == NULL) return NULL;
	points = (BREAKPOINT*) malloc(sizeof(BREAKPOINT) * size);
	if(points == NULL) return NULL;

	while(fgets(line,80,fp)) {
		got = sscanf(line, "%lf%lf", &points[npoints].time, &points[npoints].value);
		if(got<0) continue;
		if(got==0){
			printf("Line %ld has non-numeric data\n", npoints+1);
			break;
		}
		if(got==1) {
			printf("Incomplete breakpoint found at point %ld\n", npoints+1);
			break;
		}
		if(points[npoints].time < lasttime) {
			printf("data error at point %ld: time not increasing\n", npoints+1);
			break;
		}
		lasttime = points[npoints].time;
		if(++npoints == size) {
			// increase size of memory
			BREAKPOINT* tmp;
			size += npoints;
			tmp = (BREAKPOINT*)realloc(points, sizeof(BREAKPOINT) * size);
			if(tmp == NULL) {
				/* have to release the memory, and return NULL to caller */
				npoints = 0;
				free(points);
				points = NULL;
				break;
			}
			points = tmp;
		}

	}
	if(npoints) *psize = npoints;
	return points;
}

BREAKPOINT maxpoint(const BREAKPOINT* points, unsigned long npoints) {
	int i;
	BREAKPOINT point;
	point.time = points[0].time;
	point.value = points[0].value;
	for(i=1; i<npoints; i++) {
		if(point.value < points[i].value) {
			point.value = points[i].value;
			point.time = points[i].time;
		}
	}
	return point;
}

BREAKPOINT minpoint(const BREAKPOINT* points, unsigned long npoints) {
	int i;
	BREAKPOINT point;
	point.time = points[0].time;
	point.value = points[0].value;
	for(i=1; i<npoints; i++) {
		if(point.value > points[i].value) {
			point.value = points[i].value;
			point.time = points[i].time;
		}
	}
	return point;
}

void bps_minmax(const BREAKPOINT* points, uint32_t npoints, double *min, double *max) {
	uint32_t i;
	*min = points[0].value;
	*max = points[0].value;
	for(i=1; i<npoints; i++) {
		if(*min > points[i].value) *min = points[i].value;
		if(*max < points[i].value) *max = points[i].value;
	}
}

int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints) {
	unsigned long i;
	int range_OK = 1;
	for (i = 0; i < npoints; i++) {
		if(points[i].value < minval || points[i].value > maxval) {
			range_OK = 0;
			break;
		}
	}
	return range_OK;
}

double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time) {
	unsigned long i;
	BREAKPOINT left, right;
	double frac, val, width;
	// scan until we find a span containing our time
	for(i=1; i<npoints; i++) {
		if(time <= points[i].time) break;
	}
	// maintain final value if time beyond end of data
	if(i == npoints) {
		return points[i-1].value;
	}
	left = points[i-1]; // safe because we start loop at 1
	right = points[i];
	// check for instant jump (two points with same time)
	width = right.time - left.time;
	if(width == 0.0) return right.value;
	// get value from this span using linear interpolation
	frac = (time - left.time) / width;
	val = left.value + ((right.value-left.value) * frac);
	// done!
	return val;
}

BRKSTREAM* bps_newstream(FILE *fp, uint32_t srate, uint32_t *size) {
	BRKSTREAM *stream;
	BREAKPOINT *points; // list of breakpoints
	uint32_t npoints; // number of points
	if(srate == 0) {
		printf("error: cannot create stream, srate must be larger than zero.\n");
		return NULL;
	}
	/* load breakpoint file and setup stream info */
	points = get_breakpoints(fp, &npoints);

	if(points == NULL) {
		printf("error: failed to read breakpoint file.\n");
		return NULL;
	}
	if(npoints<2) {
		printf("error: breakpoint file is too small (%i) – at least two points required.\n", npoints);
		return NULL;
	}

	/* init the stream object */
	stream = (BRKSTREAM*) malloc(sizeof(BRKSTREAM));
	if(stream == NULL) return NULL;

	stream->points = points;
	stream->npoints = npoints;
	stream->curpos = 0.0;
	stream->ileft = 0;
	stream->iright = 1;
	stream->incr = 1.0 / srate; // srate is an argument

	// first span
	stream->leftpoint = stream->points[stream->ileft];
	stream->rightpoint = stream->points[stream->iright];
	stream->width = stream->rightpoint.time - stream->leftpoint.time;
	stream->height = stream->rightpoint.value - stream->leftpoint.value;
	stream->more_points = 1;

	// report size, if requested
	if(size) *size = stream->npoints;

	return stream;
}

void bps_freepoints(BRKSTREAM* stream) {
	if(stream && stream->points) {
		free(stream->points);
		stream->points = NULL;
	}
}