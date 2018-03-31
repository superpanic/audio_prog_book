/* hellotable.c */
#include <stdio.h>
#include <math.h>
#include "hellotable.h"

#define SAMPLING_RATE 44100
#define PI 3.14159265
#define TABLE_LEN 512

enum {SINE, SQUARE, SAW, TRIANGLE};

float table[TABLE_LEN];

void fill_sine() {
	int j;
	for(j = 0; j< TABLE_LEN; j++) {
		table[j] = (float) sin(2 * PI *j/TABLE_LEN);
	}
}

void fill_square() {
	int j;
	for(j = 0; j<TABLE_LEN/2; J++) table[j] = 1;
	for(j=TABLE_LEN/2; j<TABLE_LEN; j++) table[j] = -1;
}

void fill_saw() {
	int j;
	for(j=0; j<TABLE_LEN; j++) {
		table[j] = 1 - (2 * (float)j / (float)TABLE_LEN);
	}
}

void fill_triangle() {
	int j;
	// rising
	for(j=0; j<TABLE_LEN/2; j++) {
		table[j] = 2 * (float)j / (float)(TABLE_LEN/2) - 1;
	}
	// falling
	for(j=TABLE_LEN/2; j<TABLE_LEN; j++) {
		table[j] = 1 - (2 * (float)(j-TABLE_LEN/2)/(float)(TABLE_LEN/2));
	}
}



// TODO: add code for output...




int main(int argc, char *argv[]) {
	int err = 0;
	int waveform;
	const float frequency, duration;
	printf("Type the frequency of the wave to output in Hz, and press ENTER: ");
	scanf("%f", &frequency);

	printf("\nType the duration of tone in seconds, and press ENTER: ");
	scanf("%f", &duration);

wrong_waveform:
	printf("\nType a number from 0 to 3 corresponding to the waveform you intend to choose\n");
	printf("\t0 = sine, 1 = square, 2 = sawtooth, 3 = triangle\n");
	printf("and press ENTER: ");
	scanf("%d", &waveform);
	if(waveform < 0 || waveform > 3) {
		printf("\nwrong number for waveform, try again:\n");
		goto wrong_waveform;
	}

	// fill the table
	switch(waveform) {
		case SINE:
			printf("\nyou've choosen a SINE wave\n");
			fill_sine();
			break;
		case SQUARE:
			printf("\nyou've choosen a SQUARE wave\n");
			fill_square();
			break;
		case SAW:
			printf("\nyou've choosen a SAW wave\n");
			fill_saw();
			break;
		case TRIANGLE:
			printf("\nyou've choosen a TRIANGLE wave\n");
			fill_triangle();
			break;
		default:
			printf("wrong wave! ending program.\n");
			err++;
			return err;
	}

	init();

	// synthesis engine start
	{
		int j;
		double sample_increment = frequency * TABLE_LEN / SAMPLING_RATE;
		double phase = 0;
		float sample;
		for (j = 0; j < duration * SAMPLING_RATE; j++) {
			sample = table[(long) phase];
			outSample(sample);
			phase += sample_increment;
			if(phase > TABLE_LEN) phase -= TABLE_LEN;
		}
	}

	// synthesis engine end
	cleanup();
	printf("End of process");

	return 0;
}