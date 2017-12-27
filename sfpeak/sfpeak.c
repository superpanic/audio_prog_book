#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <math.h>

#define NFRAMES (1024)

enum {ARG_PROGNAME, ARG_INFILE, ARG_NARGS};

double max_samp(float* buf, unsigned long block_size);

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
	PSF_PROPS props;

	float framesread, totalread;
	double in_peak = 0.0;

	unsigned long nframes = NFRAMES;
	int ifd = -1;
	int error = 0;
	int snd_size;
	PSF_CHPEAK *peaks = NULL;
	float *frame = NULL;
	double peaktime;
	float dB;

	long i;

	printf("sfpeak: print the peak values of a soundfile\n");

	if(argc < ARG_NARGS) {
		printf("error: insufficient arguments.\n"
			"usage: %s infile\n", argv[ARG_PROGNAME]);
		return 1;
	}

	// start up portsf
	if(psf_init()) {
		printf("unable to start portsf\n");
		return 1;
	}

	ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0);

	if(ifd < 0) {
		printf("error: unable to open infile %s\n", argv[ARG_INFILE]);
		return 1;
	}

	printf("audio chanels:\t%i\n", props.chans);
	printf("sample rate:\t%i\n", props.srate);

	/* we now have a resource, so we use goto hereafter on hititng an error */

	// allocate space for one sample frames
	frame = (float*) malloc(nframes * props.chans * sizeof(float));
	if(frame == NULL) {
		printf("error: no memory!\n");
		error++;
		goto exit;
	}

	// and to allocate space for PEAK information
	peaks = (PSF_CHPEAK*) malloc(props.chans * sizeof(PSF_CHPEAK));
	if(peaks == NULL) {
		printf("error: no memory!\n");
		error++;
		goto exit;
	}

	snd_size = psf_sndSize(ifd);
	printf("sound size: %i\n", snd_size);

	if(psf_sndReadPeaks(ifd,peaks,NULL)>0) {
		printf("reading peak value ...\n");
		long i;
		for(i=0; i<props.chans; i++) {
			if(peaks[i].val > in_peak) {
				in_peak = peaks[i].val;
			}
		}
	} else {
		printf("peak value not in header,\nfinding peak value ...\n");
		framesread = psf_sndReadFloatFrames(ifd,frame,nframes);
		totalread = 0;

		while(framesread > 0) {
			totalread += framesread;
			double this_peak;
			unsigned long block_size = framesread * props.chans;
			this_peak = max_samp(frame,block_size);
			if(this_peak > in_peak) {
				in_peak = this_peak;
			}
			framesread = psf_sndReadFloatFrames(ifd, frame, nframes);
		}
	}

	if(in_peak == 0.0) {
		printf("infile %s is silent!\n", argv[ARG_PROGNAME]);
		goto exit;
	}

	// report PEAK values to user
	printf("peak information\n");
	printf("max peak: %f ", in_peak);
	printf("(%f dB)\n", log10(in_peak) * 20.0);

/*
	printf("peak information\n");
	for(i=0;i<props.chans;i++){
		peaktime = (double) peaks[i].pos / props.srate;
		dB = log10(peaks[i].val * 20.0);
		printf("CH %ld:\t%.4f (%.2f dB) at %.4f secs\n", i+1, peaks[i].val, dB, peaktime);
	}
*/

exit:	// do all cleanup
	printf("exit, cleaning up\n");
	if(ifd >= 0) psf_sndClose(ifd);
	if(frame) free(frame);
	if(peaks) free(peaks);
	psf_finish();
	return error;

}



















//
