/* Copyright (c) 2009 Richard Dobson

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
/* anvx.c - extract amplitude from mono soundfile */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portsf.h>

/* set size of multi-channel frame-buffer */
#define NFRAMES (1024)
#define DEFAULT_WINDOW_MSECS (15)

/* TODO define program argument list, excluding flags */
enum {ARG_PROGNAME,ARG_INFILE,ARG_OUTFILE,ARG_NARGS};

/* structures */
typedef struct panpos {
	double left;
	double right;
} PANPOS;

/* signatures */
PANPOS simplepan(double position);
double max_samp(float* buf, unsigned long block_size);

int main(int argc, char* argv[]) {

/* STAGE 1 */
	PSF_PROPS inprops; //,outprops;
	long framesread;
	/* init all dynamic resources to default states */
	int infile = -1; //,outfile = -1;
	int error = 0;
//	unsigned long nframes = NFRAMES;
	float* inframe = NULL;
	FILE *fp = NULL;
	double windur = DEFAULT_WINDOW_MSECS; // window duration
	double brktime;
	unsigned long winsize;
	unsigned long npoints;
	/* TODO: define an output frame buffer if channel width different 	*/

	printf("char size:  %lu\n", sizeof(char));
	printf("short size: %lu\n", sizeof(short));
	printf("int size:   %lu\n", sizeof(int));
	printf("long size:  %lu\n", sizeof(long));

/* STAGE 2 */
	printf("anvx.c - extract amplitude from mono soundfile\n");

	/* process any optional flags: remove this block if none used! */
	if(argc > 1){
		char flag;
		while(argv[1][0] == '-'){
			flag = argv[1][1];
			switch(flag){
			/*TODO: handle any flag arguments here */
			case('\0'):
				printf("Error: missing flag name\n");
				return 1;
			case('w'):
				windur = atof(&argv[1][2]); // use pointer instead of one char, otherwise we can't read numbers with more than one char
				if(windur<=0.0) {
					printf("bad value for window duration."
					       "must be positive.\n");
					      return 1;
				}
				break;
			default:
				break;
			}
			argc--;
			argv++;
		}
	}

	/* check rest of commandline */
	if(argc < ARG_NARGS){
		printf("insufficient arguments.\n"
			/* TODO: add required usage message */
			"usage: envx infile.wav outfile.brk\n"
			"    -wN: set extraction window size to N msecs\n"
			"    (default: 15)\n"
			);
		return 1;
	}

	/*  always startup portsf */
	if(psf_init()){
		printf("unable to start portsf\n");
		return 1;
	}

/* STAGE 3 */
	infile = psf_sndOpen(argv[ARG_INFILE],&inprops,0);
	if(infile < 0){
		printf("Error: unable to open infile %s\n",argv[ARG_INFILE]);
		error++;
		goto exit;
	}
	/* TODO: verify infile format for this application */

	/* verify infile format is acceptable */
	if(inprops.chans > 1) {
		printf("Soundfile contains %d channels:"
			"must be mono.\n", inprops.chans);
			error++;
			goto exit;
	}

	/* create output breakpoint file */
	fp = fopen(argv[ARG_OUTFILE],"w");
	if(fp == NULL) {
		printf("envx: unable to create breakpoint file %s\n",
			argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}


	windur /= 1000.0; // convert milliseconds to seconds
	winsize = (unsigned long)(windur * inprops.srate); // set buffer size to duration in seconds * samples per second


	/* allocate space for sample frame buffer ...*/
	inframe = (float*) malloc(winsize * sizeof(float));
	if(inframe==NULL){
		puts("No memory!\n");
		error++;
		goto exit;
	}




/* STAGE 5 */
	printf("processing....\n");
	/* TODO: init any loop-related variables */

	brktime = 0.0;
	npoints = 0;
	while ((framesread = psf_sndReadFloatFrames(infile,inframe,winsize)) > 0){
		double amp;
		amp = max_samp(inframe, framesread);
		if(fprintf(fp,"%f\t%f\n",brktime, amp) < 2) {
			printf("Failed to write breakpoint to file %s\n", argv[ARG_OUTFILE]);
			error++;
			break;
		}
		brktime += windur;
		npoints++;
	}

	if(framesread < 0)	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	} else {
		printf("Done: %d errors\n",error);
		printf("%lu breakpoints written to %s\n", npoints, argv[ARG_OUTFILE]);
	}

/* STAGE 7 */
	/* do all cleanup  */
exit:
	if(infile >= 0)
		if(psf_sndClose(infile))
			printf("%s: Warning: error closing infile %s\n",argv[ARG_PROGNAME],argv[ARG_INFILE]);

	if(inframe)
		free(inframe);

	/*TODO: cleanup any other resources */
	if(fp)
		if(fclose(fp))
			printf("envx: failed to close output file %s\n", argv[ARG_OUTFILE]);

	psf_finish();
	return error;
}

/* function implementations */
PANPOS simplepan(double position){
	PANPOS pos;

	position *= 0.5;
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
