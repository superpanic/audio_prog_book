/* siggen.c */
#include <stdint.h>
#include <portsf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wave.h"

#define NFRAMES (1024)

enum { ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_AMP, ARG_FREQ, ARG_SHAPE, ARG_NARGS };
enum { SINE, SQUARE, RISING, FALLING, TRIANGLE };

int main(int argc, char *argv[]) {

	int err = 0;

	// audio format stuff
	double duration; // duration in seconds
	uint32_t srate; // sample rate, example: 44100 or 48000
	double amp; // amplitude, usually: 1.0
	double freq; // frequency, example: 440 (concert A)

	// outfile stuff
	float *outframe = NULL; // one frame of samples
	uint32_t nframes = NFRAMES;
	PSF_PROPS props; // audio file properties
	int ofd = -1; // out file error check

	// processing loop stuff
	uint32_t i,j; // loop counters
	uint32_t nbufs; // total number of frame buffers
	uint32_t outframes; // frames written to file
	uint32_t remainder; // remainder in last frame buffer

	// pointer to tickfunction
	tickfunc tick = sinetick;

	// oscilator
	OSCIL *p_osc; // sinus oscilator properties

	if(argc < ARG_NARGS-1) {
		printf("\terror:\n"
				"\t\twrong number of arguments, got %i arguments, expected %i\n"
			"\tusage:\n"
				"\t\t%s outfile duration samplerate amplification frequency [shape]\n"
			"\tshapes:\n"
				"\t\tsine (default)\n\t\tsquare\n\t\trising\n\t\tfalling\n\t\ttriangle\n"
			"\taborting...\n", 
			argc-1, ARG_NARGS-2, argv[ARG_PROGNAME]);
		err++;
		return err;
	}

	duration = atof(argv[ARG_DUR]);
	if(duration <= 0) {
		printf("\terror: got duration %lf, duration needs to be more than 0.\n\taborting...\n", duration);
		err++;
		return err;
	}

	srate = atoi(argv[ARG_SRATE]);
	if(srate <= 0) {
		printf("\terror: got sample rate %i, sample rate need to be more than 0.\n\taborting...\n", srate);
		err++;
		return err;
	}

	amp = atof(argv[ARG_AMP]);
	if(amp <= 0) {
		printf("\terror: got amp value at %lf. this would result in silent audio.\n\taborting...\n", amp);
		err++;
		return err;
	}

	freq = atof(argv[ARG_FREQ]);
	if(freq <= 0) {
		printf("\terror: got freq value at %lf. needs to be higher than 0.\n\taborting...\n", freq);
		err++;
		return err;
	}


	// check if we got optional arguments
	if(argc > ARG_NARGS-1) {
		if( strcmp(argv[ARG_SHAPE],"sine") == 0)          tick = sinetick;
		else if( strcmp(argv[ARG_SHAPE],"square") == 0)   tick = sqtick;
		else if( strcmp(argv[ARG_SHAPE],"rising") == 0)   tick = risetick;
		else if( strcmp(argv[ARG_SHAPE],"falling") == 0)  tick = falltick;
		else if( strcmp(argv[ARG_SHAPE],"triangle") == 0) tick = tritick;
		else printf("warning:\n\tunknown wave shape, setting shape to default sine wave\n");
	}

	// start up portsf
	if( psf_init() ) {
		printf("unable to start portsf\n");
		err++;
		return err;
	}

	// define outfile format - set to mono 16 bit format
	props.srate = srate;
	props.chans = 1;
	props.samptype = PSF_SAMP_16;
	props.chformat = STDWAVE;

	ofd = psf_sndCreate(argv[ARG_OUTFILE], &props, 0, 0, PSF_CREATE_RDWR);
	if(ofd < 0) {
		printf("\terror: unable to create outfile %s\n\taborting...\n", argv[ARG_OUTFILE]);
		err++;
		goto exit;
	}

	// create the oscilator struct
	p_osc = new_oscil(props.srate);

	// setup for processing loop
	outframes = (uint32_t) (duration * props.srate + 0.5);
	nbufs = outframes / nframes;
	remainder = outframes - nbufs * nframes;
	if(remainder > 0) nbufs++;

	// allocate space for outfile buffer
	outframe = (float *) malloc(nframes * props.chans * sizeof(float)); // should it really be float? not uint16_t?
	if(outframe == NULL) {
		printf("error: no memory!\n");
		err++;
		goto exit;
	}

	// processing loop
	for(i=0; i<nbufs; i++) {
		if(i==nbufs-1) nframes = remainder;
		for(j=0; j<nframes;j++) {
			//outframe[j] = (float)(amp * sqtick(p_osc, freq));
			outframe[j] = (float)(amp * tick(p_osc, freq));
		}
		if(psf_sndWriteFloatFrames(ofd,outframe,nframes)!=nframes) {
			printf("error: unable to write to outfile.\n");
			err++;
			goto exit;
		}
	}

printf("\tdone rendering %.2lf seconds of %s audio to file %s\n", duration, argv[ARG_SHAPE], argv[ARG_OUTFILE]);

exit:
	if(outframe) free(outframe);
	if(p_osc) free(p_osc);
	if(ofd >= 0) psf_sndClose(ofd);
	psf_finish();

	return err;

}
