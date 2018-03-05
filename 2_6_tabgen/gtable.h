// gtable.h
#ifndef __GTABLE__
#define __GTABLE__

#include <stdint.h>
#include "../include/wave.h"

typedef struct t_gtable {
	double *table; // ptr to array containing the waveform
	uint32_t length; // excluding guard point

} GTABLE;

typedef struct t_tab_oscil {
	OSCIL osc;
	const GTABLE* gtable;
	double dtablen;
	double sizeovrsr;
} OSCILT;

GTABLE *new_sine(uint32_t length);
GTABLE *new_triangle(uint32_t length, uint32_t nharms);
GTABLE *new_gtable(uint32_t length);
void gtable_free(GTABLE** gtable);

OSCILT *new_oscilt(double srate, const GTABLE* gtable, double phase);
double tabtick(OSCILT* p_osc, double freq);
double tabitick(OSCILT* p_osc, double freq);

#endif