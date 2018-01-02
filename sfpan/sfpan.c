#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <math.h>
#include <breakpoints.h>
#include "sfpan.h"

#define NFRAMES (1024)
enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_BRKFILE, ARG_NARGS};

PANPOS constpowerpan(double position) {
	PANPOS pos;
	const double piovr2 = 4.0 * atan(1.0) * 0.5;
	const double root2ovr2 = sqrt(2.0) * 0.5;
	double thispos = position * piovr2;
	double cangle = cos(thispos * 0.5);
	double sangle = sin(thispos * 0.5);
	pos.left = root2ovr2 * (cangle - sangle);
	pos.right = root2ovr2 * (cangle + sangle);
	return pos;
}

PANPOS simplepan(double position) {
	PANPOS pos;
	position *= 0.5; // shift position from -1 to 1 -> -0.5-1.5
	pos.left = position - 0.5;
	pos.right = position + 0.5;
	return pos;
}

double max_samp(float* buf, unsigned long block_size) {
	double abs_val, peak = 0.0;
	unsigned long i;
	for(i=0; i<block_size; i++) {
		abs_val = fabs(buf[i]);
		if(abs_val > peak) {
			peak = abs_val;
		}
	}
	return peak;
}

int main(int argc, char *argv[]) {
	PSF_PROPS props, out_props;

	float framesread, totalread;
	unsigned long nframes = NFRAMES;

	int ifd = -1;
	int ofd = -1;
	int error = 0;
	int snd_size;

	float *inframe = NULL;
	float *outframe = NULL;

	double position;
	PANPOS thispos;

	long i, out_i;

	// additions for reading from breakpoint
	FILE *fp = NULL;
	unsigned long size;
	BREAKPOINT *points = NULL;
	double timeincr, sampletime, stereopos;
	//

	printf("sfpan: pan the sound\n");

	// arguments check
	if(argc < ARG_NARGS) {
		printf( "error: insufficient arguments.\n"
			"usage: %s infile outfile posfile.brk\n"
			"\tposfile.brk is breakpoint file,\n"
			"\twith values in range -1.0 <= pos <= 1.0\n"
			"\twhere -1.0 = full left, 0 = centre, 1.0 = full right\n",
			argv[ARG_PROGNAME]);
		return 1;
	}

/*
	// infile value range test
	position = atof( argv[ARG_PANPOS] );
	if(position < -1.0 || position > 1.0) {
		printf("error: panpos value %lf out of range\n"
			"should be between -1.0 and 1.0", position);
		error++;
		return 1;
	}
*/

	// open breakpoint file
	fp = fopen(argv[ARG_BRKFILE], "r");
	if(fp == NULL) {
		printf("Error: unable to open"
		"breakpoint file %s\n", argv[ARG_BRKFILE]);
		error++;
		goto exit;
	}

	// read breakpoints
	points = get_breakpoints(fp, &size);
	if(points==NULL) {
		printf("No breakpoints read.\n");
		error++;
		goto exit;
	}

	// error check breakpoints
	if(size < 2) {
		printf("error: at least two breakpoints required\n");
		error++;
		goto exit;
	}

	if(points[0].time != 0.0) {
		printf("error: breakpoint time must start from 0.0.\n");
		error++;
		goto exit;
	}

	if(!inrange(points, -1.0, 1.0, size)) {
		printf("error: values out of range -1 and 1 in %s\n", argv[ARG_BRKFILE]);
		printf("maxpoint: %lf\n", maxpoint(points, size).value);
		printf("minpoint: %lf\n", minpoint(points, size).value);
		// exercise 2.3.3, create a function that returns both max and min BREAKPOINTS
		BREAKPOINT mami[2];
		maxmin(points, size, mami);
		printf("maxpoint: %lf at: %lf, minpoint: %lf at: %lf\n", mami[0].value, mami[0].time, mami[1].value, mami[1].time);
		//
		error++;
		goto exit;
	}

	// start up portsf
	if( psf_init() ) {
		printf("unable to start portsf\n");
		return 1;
	}

	ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0);

	if(ifd < 0) {
		printf("error: unable to open infile %s\n", argv[ARG_INFILE]);
		return 1;
	}

	/* we now have a resource, so we use goto hereafter on hititng an error */

	// make sure infile is mono
	if(props.chans != 1) {
		printf("error: infile must be mono\n");
		error++;
		goto exit;
	}

	// allocate space for infile buffer
	inframe = (float*) malloc(nframes * sizeof(float));
	if(inframe == NULL) {
		printf("error: no memory!\n");
		error++;
		goto exit;
	}

	// copy the properties if the infile and change from mono to stereo
	out_props = props;
	out_props.chans = 2;


	ofd = psf_sndCreate(argv[ARG_OUTFILE], &out_props, 0, 0, PSF_CREATE_RDWR);
	if(ofd < 0) {
		printf("error: unable to create outfile %s\n", argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	// allocate space for outfile buffer
	outframe = (float *) malloc(nframes * out_props.chans * sizeof(float));
	if(outframe == NULL) {
		printf("error: no memory!\n");
		error++;
		goto exit;
	}

	// get size of infile
	snd_size = psf_sndSize(ifd);
	printf("sound size: %i\n", snd_size);


	timeincr = 1.0 / props.srate;
	sampletime = 0.0;

	// replace val_at_brktime
	unsigned long timestep = 1;
	BREAKPOINT leftbrk, rightbrk;
	double widthbrk, fracbrk;
	//

//	thispos = simplepan(position);
//	printf("left: %lf right: %lf\n", thispos.left, thispos.right );

	while ( (framesread = psf_sndReadFloatFrames(ifd, inframe, nframes)) > 0 ) {

		for(i=0, out_i=0; i<framesread; i++) {

			// replacing val_at_brktime
			if(points[timestep].time <= sampletime) timestep++;
			if(timestep >= size) {
				stereopos = points[size-1].value;
			} else {
				leftbrk = points[timestep-1];
				rightbrk = points[timestep];
				widthbrk = rightbrk.time - leftbrk.time;
				if(widthbrk == 0.0) {
					stereopos = rightbrk.time;
				} else {
					fracbrk = (sampletime - leftbrk.time) / widthbrk;
					stereopos = leftbrk.value + ((rightbrk.value-leftbrk.value) * fracbrk);
				}
			}
			//

			// stereopos = val_at_brktime(points, size, sampletime);

			thispos = constpowerpan(stereopos);
			outframe[out_i++] = (float)(inframe[i] * thispos.left);
			outframe[out_i++] = (float)(inframe[i] * thispos.right);
			sampletime += timeincr;
		}

		if( psf_sndWriteFloatFrames(ofd, outframe, framesread) != framesread ) {
			printf("error: failed to write to outfile\n");
			error++;
			goto exit;
		}

	}



exit:	// do all cleanup
	printf("exit, cleaning up\n");
	if(fp) fclose(fp);
	if(points) free(points);
	if(ifd >= 0) psf_sndClose(ifd);
	if(inframe) free(inframe);
	if(ofd >= 0) psf_sndClose(ofd);
	if(outframe) free(outframe);
	psf_finish();
	return error;

}



















//
