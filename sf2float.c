// sf2float.c
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>

enum  {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_NARGS};

int main(int argc, char* argv[]) {
	PSF_PROPS props; // defined in portsf.H
	long framesread, totalread;
	int ifd = -1, ofd = -1; // in file descriptor, out file descriptor
	int error = 0;
	psf_format outformat = PSF_FMT_UNKNOWN;
	PSF_CHPEAK* peaks = NULL; // array of peaks
	float* frame = NULL;

	printf("SF2FLOAT: convert soundfile to floats format\n");

	if(argc < ARG_NARGS) {
		printf("error: insufficient arguments.\n"
		       "usage:\n\ttsf2float infile outfile\n");
		return 1;
	}

	// start up portsf
	if(psf_init()) {
		printf("unable to start portsf\n");
		return 1;
	}

	ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0); // filling ni props

	if(ifd < 0) {
		printf("error: unable to open infile %s\n", argv[ARG_INFILE]);
		return 1;
	}

	/* we now have a resource, so we use goto hereafter on hitting an error */

	// tell user if source file is already floats
	if(props.samptype == PSF_SAMP_IEEE_FLOAT) {
		printf("info: infile is already in floats format\n");
	}

	props.samptype = PSF_SAMP_IEEE_FLOAT;
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
	frame = (float*) malloc(props.chans * sizeof(float));
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

	printf("copying ...\n");

	int s = psf_sndSize(ifd);
	// printf("size: %i\n", s);


	// single frame loop to do copy, report any errors
	framesread = psf_sndReadFloatFrames(ifd, frame, 1);
	totalread = 0;

	while (framesread == 1) {
		totalread++;
		if(psf_sndWriteFloatFrames(ofd, frame, 1) != 1) {
			printf("error: failed to write outfile\n");
			error++;
			break;
		}

		if(totalread%1000 == 0) {
			printf("\r%i%%", (int)(((float)totalread/(float)s)*100.0)+1);
		}

		// do any processing here!
		framesread = psf_sndReadFloatFrames(ifd,frame,1);
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
		printf("PEAK information:\n");
		for(i=0;i<props.chans;i++) {
			peaktime = (double) peaks[i].pos / props.srate;
			printf("CH %ld:\t%.4f at %.4f secs\n", i+1, peaks[i].val, peaktime);
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












//
