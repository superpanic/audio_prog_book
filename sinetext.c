// sinetext
// write sine wave as text

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.141592654)
#endif

/* define our program argument list */
enum {ARG_NAME, ARG_NSAMPS, ARG_FREQ, ARG_SR, ARG_NARGS};

int main(int argc, char* argv[]) {
	int i, nsamps;
	double sin_samp,cos_samp,freq,srate;
	double twopi = 2.0 * M_PI;
	double angle_delta;

	if(argc != ARG_NARGS) {
		fprintf(stderr, "Usage: sinetext nsamps freq srate\n");
		return 1;
	}

	nsamps = atoi(argv[ARG_NSAMPS]);
	freq = atof(argv[ARG_FREQ]);
	srate = atof(argv[ARG_SR]);
	angle_delta = twopi * freq/srate;

	for(i=0;i<=nsamps;i++) {
		sin_samp = sin(angle_delta * i);
		cos_samp = cos(angle_delta * i);
		fprintf(stdout, "%.8lf\t%.8lf\n",sin_samp,cos_samp);
	}
	fprintf(stderr, "done!\n");
	return 0;
}
