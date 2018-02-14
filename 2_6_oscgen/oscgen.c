/* oscgen.c */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <portsf.h>
#include "wave.h"
#include "breakpoints.h"

#define NFRAMES (1024)

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_AMP, ARG_FREQ, ARG_WAVE, ARG_NOSC, ARG_NARGS };
enum {WAVE_SQUARE, WAVE_SAWUP, WAVE_SAWDOWN, WAVE_TRIANGLE, WAVE_NWAVES};
const char *wave_names[] = {"square", "saw up", "saw down", "triangle"};

int main(int argc, char *argv[]) {
	int err = 0;

	// audio format stuff
	double duration;  // in seconds
	uint32_t srate;   // sample rate (typically 48000)
	double amp;       // amplification (0.0-1.0)
	double freq;      // frequency (concert A = 440)
	uint8_t wavetype;

	// oscilators
	uint32_t noscs = 0;       // number of oscilators
	OSCIL **oscs = NULL; // array of OSCIL pointers
	double *oscamps = NULL;     // array of amps
	double *oscfreqs = NULL;    // array of frequencies
	
	// outfile stuff
	float *outframe = NULL;
	uint32_t nframes = NFRAMES;
	PSF_PROPS props;
	int ofd = -1;

	uint32_t nbufs; // total number of frame buffers
	uint32_t outframes; // frames written to file
	uint32_t remainder; // remainder in last frame buffer

	// amp breakpoint stuff
	BRKSTREAM *ampstream = NULL;
	FILE *fp_amp = NULL;
	uint32_t brkamp_size = 0;

	// freq breakpoint stuff
	BRKSTREAM *freqstream = NULL;
	FILE *fp_freq = NULL;
	uint32_t brkfreq_size = 0;

	// main loop
	uint32_t i,j,k;
	double val;

	// check number of arguments
	if(argc < ARG_NARGS) {
		printf("\tproduces a sound file based on user arguments:\n"
			"\t\t\033[7m%s filename duration samplerate amplification frequency wavetype noscilators\033[0m\n"
			"aborting ...",
			argv[ARG_PROGNAME]);
		err++;
		return err;
	}

	// duration
	duration = atof(argv[ARG_DUR]);
	if(duration <= 0) {
		printf("\terror: got duration '%s', duration needs to be a float larger than 0.0\n"
			"\taborting ...", argv[ARG_DUR]);
		err++;
		return err;
	}

	// samplerate
	srate = atoi(argv[ARG_SRATE]);
	if(srate <= 0) {
		printf("\terror: got sample rate '%s', sample rate needs to be an integer larger than 0\n", argv[ARG_SRATE]);
		err++;
		return err;
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
	props.format = PSF_WAVE_EX; // TODO: also check for filename ending with .wav
	props.chformat = STDWAVE;

	ofd = psf_sndCreate(argv[ARG_OUTFILE], &props, 0, 0, PSF_CREATE_RDWR);
	if(ofd < 0) {
		printf("\terror %i: unable to create outfile '%s'\n\taborting...\n", ofd, argv[ARG_OUTFILE]);
		printf("\tsrate: %i\n", props.srate);
		printf("\tpath: %s\n", argv[ARG_OUTFILE]);
		printf("\tsamptype: %i\n", props.samptype);
		printf("\tformat: %i\n", props.format);
		printf("\tchannel format: %i\n", props.chformat);
		err++;
		goto exit;
	}	

	// amplification
	fp_amp = fopen(argv[ARG_AMP], "r");
	if(fp_amp == NULL) {
		char *ptr;
		amp = strtod(argv[ARG_AMP], &ptr);
		if(ptr == argv[ARG_AMP]) {
			printf("error: did not find a file name or number in %s", ptr);
		}
		err++;
		goto exit;
	} else {
		printf("\tusing breakpoint file %s to modulate the amplification\n", argv[ARG_AMP]);
		ampstream = bps_newstream(fp_amp, props.srate, &brkamp_size);
		double min;
		double max;
		bps_minmax(ampstream->points, ampstream->npoints, &min, &max);
		if( min < 0.0 ) {
			printf("error: breakpoint values are out of range: %lf (must be larger than 0.0)\n", min);
			err++;
			goto exit;
		}
		printf("\tmin: %lf, max: %lf, breakpoint file size: %i\n", min, max, brkamp_size);
	}

	// frequency
	fp_freq = fopen(argv[ARG_FREQ], "r");
	if(fp_freq == NULL) {
		char *ptr;
		freq = strtod(argv[ARG_FREQ], &ptr);
		if(ptr == argv[ARG_FREQ]) {
			printf("error: did not find a filename of number in %s\n", ptr);
			err++;
			goto exit;
		}
		if(freq <= 0) {
			printf("\terror: got freq value at %lf. needs to be higher than 0.\n"
				"\taborting...\n", freq);
			err++;
			goto exit;
		} else {
			printf("freq is set to %lf\n", freq);
		}
	} else {
		printf("\tusing breakpoint file %s to modulate the frequency\n", argv[ARG_FREQ]);
		freqstream = bps_newstream(fp_freq, props.srate, &brkfreq_size);
		double min;
		double max;
		bps_minmax(freqstream->points, freqstream->npoints, &min, &max);
		if(min <= 0.0) {
			printf("error: breakpoint values are out of range: %lf (must be larger than 0.0)\n", min);
			err++;
			goto exit;
		}
		printf("\tmin: %lf, max: %lf, breakpoint file size: %i\n", min, max, brkfreq_size);
	}

	// get wave type
	wavetype = atoi(argv[ARG_WAVE]);
	if(wavetype >= WAVE_NWAVES || wavetype < 0) {
		printf("error: '%s' is unknown wavetype, value must be %i or less\n", argv[ARG_WAVE], WAVE_NWAVES-1);
		err++;
		goto exit;
	}
	printf("\twavetype: %i, %s\n", wavetype, wave_names[wavetype]);

	// get number of oscilators
	noscs = atoi(argv[ARG_NOSC]);
	if(noscs <= 0) {
		printf("error: noscs needs to be 1 or more, got %s\n", argv[ARG_NOSC]);
		err++;
		goto exit;
	}
	printf("\tnumber of oscilators: %i\n", noscs);

	// allocate oscilators
	oscamps = (double*) malloc(noscs * sizeof(double));
	if(oscamps == NULL) {
		printf("error: out of memory trying to create array %i of oscamps\n", noscs);
		err++;
		goto exit;
	}

	oscfreqs = (double*) malloc(noscs * sizeof(double));
	if(oscfreqs == NULL) {
		printf("error: out of memory trying to create array %i of oscfreqs\n", noscs);
		err++;
		goto exit;
	}

	oscs = (OSCIL **) malloc(noscs * sizeof(OSCIL *));
	if(oscs == NULL) {
		printf("error: out of memory trying to allocate array of %i (OSCIL **)\n", noscs);
		err++;
		goto exit;
	}

	// initialize arrays ...
	double ampfac, freqfac, ampadjust;
	ampfac = 1.0;
	freqfac = 1.0;
	ampadjust = 0.0;
	switch(wavetype) {
		case(WAVE_SQUARE):
			for(i=0;i<noscs;i++) {
				ampfac = 1.0 / freqfac;
				oscamps[i] = ampfac;
				oscfreqs[i] = freqfac;
				freqfac += 2.0;
				ampadjust += ampfac;
			}
			break;
		case(WAVE_TRIANGLE):
			for(i=0;i<noscs;i++) {
				ampfac = 1.0 / (freqfac*freqfac);
				oscamps[i] = ampfac;
				oscfreqs[i] = freqfac;
				freqfac += 2.0;
				ampadjust += ampfac;
			}
			break;
		case(WAVE_SAWUP):
		case(WAVE_SAWDOWN):
			for(i=0;i<noscs;i++) {
				ampfac = 1.0 / freqfac;
				oscamps[i] = ampfac;
				oscfreqs[i] = freqfac;
				freqfac += 1.0;
				ampadjust += ampfac;
			}
			if(wavetype == WAVE_SAWUP) ampadjust = -ampadjust; // inverts waveform
			break;
	}
	
	for(i=0; i<noscs; i++) {
		printf("ampadjust: %lf\n", ampadjust);
		if(ampadjust != 0) oscamps[i] /= ampadjust;
		else oscamps[i] = 0.0;
	}

	// setup for processing loop
	outframes = (uint32_t) (duration * props.srate + 0.5);
	nbufs = outframes / nframes;
	remainder = outframes - nbufs * nframes;
	if(remainder > 0) nbufs++;

	// allocate space for outfile buffer
	outframe = (float *) malloc(nframes * props.chans * sizeof(float));
	if(outframe == NULL) {
		printf("error: out of memory when trying to create the outfile buffer outframe, %i of floats\n", nframes);
		err++;
		goto exit;
	}

	// and then create each OSCIL
	for(i=0; i<noscs; i++) {
		oscs[i] = new_oscil(props.srate);
		if(oscs[i] == NULL) {
			printf("error: out of memory");
			err++;
			goto exit;
		}
	}

	// processing loop
	for(i=0; i<nbufs; i++) {
		if(i==nbufs-1) nframes = remainder;

		for(j=0;j<nframes;j++) {
			if(freqstream) freq = bps_tick(freqstream);
			if(ampstream) amp = bps_tick(ampstream);
			val = 0.0;
			for(k=0; k<noscs; k++) {
				val += oscamps[k] * sinetick(oscs[k], freq * oscfreqs[k]);
			}
			outframe[j] = (float)(val * amp);
			/*
			if(j%500==0) printf("ouframe[%i] = %.2lf"
					       "\tval = %.2lf"
					       "\tamp = %.2lf"
					      "\tfreq = %.2lf"
					"\toscamps[0] = %.2lf"
					"\toscfreq[0] = %.2lf\n", j, outframe[j], val, amp, freq, oscamps[0], oscfreqs[0]);
			*/
		}

		if(psf_sndWriteFloatFrames(ofd,outframe,nframes)!=nframes) {
			printf("error: unable to write to outfile.\n");
			err++;
			goto exit;
		}
	}
	printf("\n");

	printf("\n\tsuccessfully rendering %.2lf seconds of audio\n" 
		"\tto file: %s\n" 
		"\tusing: %i oscilators\n"
		"\twith wavetype: %s\n\n", duration, argv[ARG_OUTFILE], noscs, wave_names[wavetype]);


exit:
	if(ampstream) {
		bps_freepoints(ampstream);
		free(ampstream);
	}
	if(fp_amp) {
		if(fclose(fp_amp)) {
			printf("error: failed to close the breakpoint file %s\n", argv[ARG_AMP]);
		}
	}

	if(freqstream) {
		bps_freepoints(freqstream);
		free(freqstream);
	}
	if(fp_freq) {
		if(fclose(fp_freq)) {
			printf("error: failed to close the breakpoints file %s\n", argv[ARG_FREQ]);
		}
	}

	if(oscamps) free(oscamps);
	if(oscfreqs) free(oscfreqs);
	if(oscs) free(oscs);


	if(ofd >= 0) psf_sndClose(ofd);
	psf_finish();

	return err;	
}