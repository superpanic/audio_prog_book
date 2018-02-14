/* wave.h */
#ifndef __WAVE__
#define __WAVE__

#include <stdint.h>
#include "breakpoints.h"

#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif
#define TWOPI (2.0 * M_PI)

typedef struct t_oscil {
	double twopiovrsr; /* to hold a constant value 2PI/samplerate */
	double curfreq;
	double curphase;
	double incr;
} OSCIL;

void oscil_init(OSCIL *osc, uint32_t srate);
OSCIL* oscil(void);
OSCIL* new_oscil(uint32_t srate);

// define pointer type to tickfunctions
typedef double (*tickfunc)(OSCIL *osc, double);
// tickfunctions
double sinetick(OSCIL *p_osc, double freq);
double sqtick(OSCIL *p_osc, double freq);
double risetick(OSCIL *p_osc, double freq);
double falltick(OSCIL *p_osc, double freq);
double tritick(OSCIL *p_osc, double freq);
double bps_tick(BRKSTREAM* stream);

#endif
