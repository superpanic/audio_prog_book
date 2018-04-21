// player.c

#include <stdio.h>
#include <stdint.h>
#include "tinyAudioLib.h"
#include "portsf.h"

#define FRAME_BLOCK_LEN 512

int main(int argc, char **argv) 
{
	float buf[FRAME_BLOCK_LEN * 2]; // multiply by 2 for stereo
	int sfd;
	PSF_PROPS props;
	uint32_t nread;
	int err = 0;

	if(argc != 2) {
		printf("error: bad command line. syntax is:\n");
		printf("\t%s filename\n", argv[0]);
		err++;
		return err;
	}

	// portsf setup
	psf_init();
	sfd = psf_sndOpen(argv[1], &props, 0);

	if(sfd < 0) {
		printf("an error occured opening audio file");
		err++;
		goto end;
	}

	printf("\tplaying audio file %s\n", argv[1]);
	printf("\tsamplerate:     %i\n", props.srate);
	printf("\tchans:          %i\n", props.chans);
	printf("\tsamptype:       %i\n", props.samptype);
	printf("\tformat:         %i\n", props.format);
	printf("\tchannel format: %i\n", props.chformat);

	if(props.chans > 2) {
		printf("invalid number (%i) of chanels in audio file\n", props.chans);
		err++;
		goto end;
	}

	// engine
	do {
		nread = psf_sndReadFloatFrames(sfd, buf, FRAME_BLOCK_LEN);
		if(props.chans == 2)
			outBlockInterleaved(buf, FRAME_BLOCK_LEN);
		else
			outBlockMono(buf, FRAME_BLOCK_LEN);
	} while (nread == FRAME_BLOCK_LEN);

	// finish
end:
	printf("finished\n");
	psf_sndClose(sfd);
	psf_finish();

	return err;
}
