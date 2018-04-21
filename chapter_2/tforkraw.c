// tforkraw.c gen raw sfile with native endianness
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>

#ifndef M_PI
#define M_PI (3.141592654)
#endif

enum {ARG_NAME, ARG_OUTFILE, ARG_DUR, ARG_HZ, ARG_SR, ARG_AMP, ARG_TYPE, ARG_NARGS};

enum samptype {RAWSAMP_SHORT, RAWSAMP_FLOAT};

int byte_order() {
	int one = 1;
	char* endptr = (char *) &one; // casting the int to the shorter pointer type char trims away the 1 on little-endian machines
	return (*endptr); // return 0 for big-endian machines, 1 for little-endian machines
}

const char* endianness[2] = {"big_endian", "little-endian"};

int main(int argc, char* argv[]) {
	unsigned int i, nsamps;
	unsigned int maxframe = 0;
	unsigned int samptype, endian, bitreverse;
	double samp, dur, freq, srate, amp, step;
	double start, end, fac, maxsamp;
	double twopi = 2.0 * M_PI;
	double angleincr;
	FILE* fp = NULL;
	float fsamp;
	int16_t ssamp;

        char  buffer [128] ;
        sf_command (NULL, SFC_GET_LIB_VERSION, buffer, sizeof (buffer)) ;
	printf("%s\n", buffer);

	if(argc != ARG_NARGS) {
		printf("usage: tforkraw outfile. raw dur freq srate amp isfloat\n");
		return 1;
	}
	dur = atof(argv[ARG_DUR]);
	if(dur <= 0.0) {
		printf("error: duration needs to be larger than 0.0\n");
		return 1;
	}
	freq = atof(argv[ARG_HZ]);
	if(freq <= 0.0) {
		printf("error: frequency needs to be larger than 0.0\n");
		return 1;
	}
	srate = atof(argv[ARG_SR]);
	if(srate <= 0.0) {
		printf("error: srate needs to be larger than 0.0\n");
		return 1;
	}
	amp = atof(argv[ARG_AMP]);
	if(amp < 0.0) {
		printf("error: amp needs to be a positive number\n");
		return 1;
	}
	samptype = (unsigned int) atoi(argv[ARG_TYPE]);
	if(samptype > 1) {
		printf("error: sampletype can be only 0 or 1\n");
		return 1;
	}
	// create binary file
	fp = fopen(argv[ARG_OUTFILE], "wb");
	if(fp==NULL) {
		fprintf(stderr, "error: failed to create output file %s\n", argv[ARG_OUTFILE]);
		return 1;
	}

	nsamps = (int)(dur * srate);
	angleincr = twopi * freq / nsamps;
	step = dur / nsamps;
	// normalized range, just scale by amp
	start = 1.0;
	end = 1.0e-4;

	maxsamp = 0.0;
	fac = pow(end/start, 1.0/nsamps);
	endian = byte_order();

	printf("Writing %d %s samples\n", nsamps, endianness[endian]);

	// run the loop for this samptype
	if( samptype == RAWSAMP_SHORT ) {
		for(i=0; i<nsamps; i++) {
			samp = amp * sin(angleincr * i);
			samp *= start;
			start *= fac;
			// use 32767 to avoid overflow problem with 16-bit sample
			// and avoid clicking
			ssamp = (int16_t) (samp * 32767.0);
			if(fwrite(&ssamp, sizeof(int16_t), 1, fp) != 1) {
				printf("error: failed to write data to file\n");
				return 1;
			}
			printf("%i\n", ssamp);
			if(fabs(samp) > maxsamp) {
				maxsamp = fabs(samp);
				maxframe = 1;
			}
		}
	} else {
		for(i=0; i<nsamps; i++) {
			samp = amp * sin(angleincr*i);
			samp *= start;
			start *= fac;
			fsamp = (float)samp;
			if(fwrite(&fsamp, sizeof(float), 1, fp) != 1) {
				printf("error: failed to write to file\n");
				return 1;
			}
			if(fabs(samp) > maxsamp) {
				maxsamp = fabs(samp);
				maxframe = i;
			}
		}
	}

	fclose(fp);
	printf("Done! Maximum sample value = %.8lf at frame %d\n", maxsamp, maxframe);
	return 0;
}











//
