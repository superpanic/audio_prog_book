// sf2float.c
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <math.h>

#define NFRAMES (1024)

enum  {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_DBVAL, ARG_NARGS};

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

int main(int argc, char* argv[]) {
	PSF_PROPS props; // defined in portsf.H
	
	float amp_fac, scale_fac;
	double db_val, in_peak = 0.0;

	long framesread, totalread;
	unsigned long nframes = NFRAMES;
	int ifd = -1, ofd = -1; // in file descriptor, out file descriptor
	int error = 0;
	int snd_size;
	psf_format outformat = PSF_FMT_UNKNOWN;
	PSF_CHPEAK* peaks = NULL; // array of peaks
	float* frame = NULL; // an array of one to n number of channels

	printf("sfnorm: adjust level to specified dB\n");

	if(argc < ARG_NARGS) {
		printf("error: insufficient arguments.\n"
		       "usage: %s infile outfile dbval\n", argv[ARG_PROGNAME]);
		return 1;
	}
	// start up portsf
	if(psf_init()) {
		printf("unable to start portsf\n");
		return 1;
	}

	db_val = (float) atof(argv[ARG_DBVAL]);
	if( db_val > 0.0 ) {
		printf("error: dbval %f cannot be positive\n", db_val);
		return 1;
	}
	amp_fac = (float) pow(10.0, db_val/20.0);

	ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0); // filling ni props

	if(ifd < 0) {
		printf("error: unable to open infile %s\n", argv[ARG_INFILE]);
		return 1;
	}

	/* we now have a resource, so we use goto hereafter on hitting an error */

	// check outfile extension is one we know about
	outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
	if(outformat == PSF_FMT_UNKNOWN) {
		printf("error: outfile name %s has unknown format.\n"
	 	       "use any of .wav .aiff. aif .afc .aifc\n",
	       	        argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	props.format = outformat;

	ofd = psf_sndCreate(argv[ARG_OUTFILE], &props, 0, 0, PSF_CREATE_RDWR);
	if(ofd < 0) {
		printf("error: unable to create outfile %s\n", argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	// allocate space for one sample frame
	frame = (float*) malloc(nframes * props.chans * sizeof(float));
	if(frame == NULL) {
		printf("error: no memory!\n");
		error++;
		goto exit;
	}

	// and to allocate space for PEAK info
	peaks = (PSF_CHPEAK*) malloc(props.chans * sizeof(PSF_CHPEAK));
	if( peaks == NULL ) {
		printf("error: no memory!\n");
		error++;
		goto exit;
	}

	snd_size = psf_sndSize(ifd);


	if(psf_sndReadPeaks(ifd,peaks,NULL)>0) {
		long i;
		for(i=0; i<props.chans; i++) {
			if(peaks[i].val > in_peak) {
				in_peak = peaks[i].val;
			}
		}
	} else {
		printf("finding peak value ...\n");
		framesread = psf_sndReadFloatFrames(ifd,frame,nframes); // will try to read nframes, will return number of frames successfully read
		totalread = 0;

		while (framesread > 0) {
			totalread += framesread;
			double this_peak;
			unsigned long block_size = framesread * props.chans;
			this_peak = max_samp(frame,block_size);
			if(this_peak > in_peak) {
				in_peak = this_peak;
			}
			// report progress in percent
			printf("\r%i%%", (int)(((float)totalread/(float)snd_size)*100.0));
			framesread = psf_sndReadFloatFrames(ifd, frame, nframes);
		}
		printf("\n");
		// rewind file
		if((psf_sndSeek(ifd, 0, PSF_SEEK_SET)) < 0) {
			printf("error: unable to rewind infile.\n");
			error++;
			goto exit;
		}
	}

	if(in_peak == 0.0) {
		printf("infile is silent! No outfile is created.\n");
		goto exit;
	}

	// now that we have the peak value we can calculate the amp factor
	scale_fac = (float)(amp_fac / in_peak);

	printf("copying ...\n");
	// single frame loop to do copy, report any errors
	framesread = psf_sndReadFloatFrames(ifd, frame, nframes);
	totalread = 0;
	

	while (framesread > 0) {
		long i;
		totalread += framesread;
		// do any processing here!
		for(i=0; i<framesread*props.chans; i++) {
			frame[i] *= scale_fac;
		}
		if(psf_sndWriteFloatFrames(ofd, frame, nframes) == 0) {
			printf("error: failed to write outfile\n");
			error++;
			break;
		}
		// report progress in percent
		printf("\r%i%%", (int)(((float)totalread/(float)snd_size)*100.0));
		framesread = psf_sndReadFloatFrames(ifd,frame,nframes);
	}

	printf("\n");

	if(framesread < 0) {
		printf("error: failed reading infile. outfile is incomplete.\n");
		error++;
	} else {
		printf("done! %ld sample frames copied to %s\n", totalread, argv[ARG_OUTFILE]);
	}

	// report PEAK values to user
	if(psf_sndReadPeaks(ofd, peaks, NULL) > 0) {
		long i;
		double peaktime;
		float dB;
		printf("PEAK information:\n");
		for(i=0;i<props.chans;i++) {
			peaktime = (double) peaks[i].pos / props.srate;
			dB = log10(peaks[i].val) * 20.0f;
			printf("CH %ld:\t%.4f (%.2f dB) at %.4f secs\n", i+1, peaks[i].val, dB, peaktime);
		}
	}


	/* do all cleanup */
exit:
	if(ifd >= 0) psf_sndClose(ifd);
	if(ofd >= 0) psf_sndClose(ofd);
	if(frame) free(frame);
	if(peaks) free(peaks);
	psf_finish();
	return error;

}

