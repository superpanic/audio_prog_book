// player2.c
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "tinyAudioLib.h"
#include "portsf.h"

void SYNTAX() 
{
	printf("syntax is:\n\tplayer2 [tTIME] [-dDUR] filename\n");
}

#define FRAME_BLOCK_LEN 512

int main(int argc, char **argv) 
{
	int err=0;
	float buf[FRAME_BLOCK_LEN * 2]; // multiply with 2 for stereo
	int sfd; // sound file descriptor
	// int opened = 0; // is filed opened or not?
	PSF_PROPS props; // struct
	uint32_t counter;
	uint32_t length;
	uint32_t endpoint;
	extern int arg_index;
	extern char *arg_option;
	extern int crack(int argc, char **argv, char *flags, int ign);
	int flag, timflag=0, durflag=0;
	uint32_t nread; // number of frames read
	double starttime, dur;
	while ( ( flag = crack(argc, argv, "t|d|T|D|", 0) ) ) {
		switch (flag) {
		case 't':
		case 'T':
			if(*arg_option) {
				timflag=1;
				starttime = atof(arg_option);
			} else {
				printf("error: -t flag set without specifying a start time in seconds.\n");
				SYNTAX();
				err++;
				return err;
			}
			break;
		case 'd':
		case 'D':
			if(*arg_option) {
				durflag=1;
				dur = atof(arg_option);
			} else {
				printf("error: -d flag set without specifying a duration in seconds\n");
				SYNTAX();
				err++;
				return err;
			}
			break;
		case EOF:
			err++;
			return err;
		}
	}

	if(argc < 2) {
		printf("error: bad command line arguments\n");
		SYNTAX();
		err++;
		return err;
	}

	psf_init();
	sfd = psf_sndOpen(argv[arg_index], &props, 0);

	if(sfd < 0) {
		printf("an error occured opening audio file\n");
		err++;
		goto end;
	}
	
	printf("file \'%s\' opened. . .\n", argv[arg_index]);
	printf("sampling rate: %d\n", props.srate);
	printf("number of chanels: %d\n", props.chans);
	length = psf_sndSize(sfd);
	printf("duration: %f\n", (float)length / (float)props.srate);
	if(timflag)
		counter = (uint32_t) (starttime * props.srate / (float) props.srate);
	else
		counter = 0; // beginning of file
	
	if (durflag) {
		endpoint = (u_int32_t) (dur * props.srate + counter);
		endpoint = (endpoint < length) ? endpoint : length;
	} else {
		endpoint = length;
		dur = (double)(endpoint-counter) / (double)props.srate;
	}

	if (props.chans > 2) {
		printf("invalid number of channels in audio file, "
		"max 2 chanels allowed\n");
		goto end;
	}

	psf_sndSeek(sfd, counter, PSF_SEEK_SET); // begin position at the right point
	printf("playing the file from time position %0.3lf for %0.3lf seconds. . .\n", starttime, dur);

	// engine start
	do {
		nread = psf_sndReadFloatFrames(sfd, buf, FRAME_BLOCK_LEN);
		if(props.chans==2) // stereo
			outBlockInterleaved(buf, FRAME_BLOCK_LEN);
		else // mono
			outBlockMono(buf, FRAME_BLOCK_LEN);
		counter += FRAME_BLOCK_LEN;
	} while (counter < endpoint);

	// engine end

end:
	printf("finished!\n");
	psf_sndClose(sfd);
	psf_finish();
	return err;

}

