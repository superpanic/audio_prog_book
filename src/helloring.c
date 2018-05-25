//helloring.c

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "portaudio.h"

#define FRAME_BLOCK_LEN 256
#define SAMPLING_RATE 44100
#define TWO_PI (3.14159265 * 2.0)

PaStream *audioStream;
double si = 0;

int audio_callback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) 
{
	
	float *in = (float*) inputBuffer;
	float *out = (float*) outputBuffer;
	static double phase = 0;
	uint32_t i;
	for(i=0; i<framesPerBuffer;i++) {
		double sine = sin(phase);
		*out++ = *in++ * sine; // left channel
		*out++ = *in++ * sine; // right channel
		phase += si;
	}
	
	return paContinue;
}

int init_stuff() 
{
	float frequency;
	int err;
	int i, id;
	const PaDeviceInfo *info;
	const PaHostApiInfo *hostapi;
	PaStreamParameters outputParameters, inputParameters;

	printf("Type the modulator frequency in Herz: ");
	scanf("%f", &frequency); // get the modulator frequency

	si = TWO_PI * frequency / SAMPLING_RATE; // calculate sampling increment
	printf("sampling increment [si] = %lf\n", si);

	printf("Initializing Portaudio. Please wait ...\n");
	err = Pa_Initialize(); // initialize portaudio

	if(err) {
		printf("Pa_Initialize error: %s\n", Pa_GetErrorText(err));
		return 0; // failure!
	}

	for(i=0; i<Pa_GetDeviceCount(); i++) {
		info = Pa_GetDeviceInfo(i); // get info from current device
		hostapi = Pa_GetHostApiInfo(info->hostApi); // get info from current host api
		if (info->maxOutputChannels > 0) // if current device supports output
			printf("%d: [%s] %s (output)\n", i, hostapi->name, info->name);
	}

	printf("\ttype AUDIO output device number: ");
	scanf("%d", &id); // get the output device number
	info = Pa_GetDeviceInfo(id);
	hostapi = Pa_GetHostApiInfo(info->hostApi); // get host api struct
	printf("opening AUDIO output device [%s] %s\n", hostapi->name, info->name);
	
	outputParameters.device = id;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = info->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL; // no specific info

	for(i=0; i<Pa_GetDeviceCount(); i++) {
		info = Pa_GetDeviceInfo(i);
		hostapi = Pa_GetHostApiInfo(info->hostApi);
		if(info->maxInputChannels>0) printf("%d [%s] %s (input)\n", i, hostapi->name, info->name);
	}

	printf("\ttype AUDIO input device number: ");
	scanf("%d", &id);
	info = Pa_GetDeviceInfo(id);
	hostapi = Pa_GetHostApiInfo(info->hostApi);

	printf("opening AUDIO input device [%s] %s\n", hostapi->name, info->name);

	inputParameters.device = id;
	inputParameters.channelCount = 2;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = info->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&audioStream,
		&inputParameters,
		&outputParameters,
		SAMPLING_RATE,
		FRAME_BLOCK_LEN,
		paClipOff,
		&audio_callback,
		NULL
	);

	if(err) {
		printf("Pa_OpenStream error: %s\n", Pa_GetErrorText(err));
		return 0; // failure!
	}
	
	err = Pa_StartStream(audioStream);
	if(err) {
		printf("Pa_StartStream error: %s\n", Pa_GetErrorText(err));
		return 0; // failure!
	}
	
	printf("running ... press space bar [enter] to exit\n");
	return 1; // success!
}

void terminate_stuff() 
{
	Pa_StopStream( audioStream );
	Pa_CloseStream( audioStream );
	Pa_Terminate();
}

int main() 
{
	if(init_stuff()) {
		while(getchar() != ' ') Pa_Sleep(100); // wait for spacebar and enter
	}
	terminate_stuff();
	printf("terminating\n");
	return 0;
}
