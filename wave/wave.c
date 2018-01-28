#include "wave.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

void oscil_init(OSCIL *osc, uint32_t srate) {
	if(osc == NULL) return;
	osc->twopiovrsr = TWOPI / (double) srate;
	osc->curfreq = 0.0;
	osc->curphase = 0.0;
	osc->incr = 0.0;
}

OSCIL* oscil(void) {
	OSCIL *osc = (OSCIL*) malloc(sizeof(OSCIL));
	if(osc == NULL) return NULL;
	return osc;
}

OSCIL* new_oscil(uint32_t srate) {
	/* use like this */
	// OSCIL *osc = new_oscil(41100);

	OSCIL* p_osc;
	p_osc = (OSCIL*) malloc(sizeof *p_osc);
	if(p_osc == NULL) return NULL;
	p_osc->twopiovrsr = TWOPI / (double) srate;
	p_osc->curfreq = 0.0;
	p_osc->curphase = 0.0;
	p_osc->incr = 0.0;
	return p_osc;
}


double sinetick(OSCIL *p_osc, double freq) {
	/* use like this */
	//for(i=0;i<nframes;i++)
	//	outframe[i] = sinetick(osc,freq); /* modify freq ad lib */

	double val;
	val = sin(p_osc->curphase);
	if(p_osc->curfreq != freq) {
		p_osc->curfreq = freq;
		p_osc->incr = p_osc->twopiovrsr * freq;
	}
	p_osc->curphase += p_osc->incr;
	if(p_osc->curphase >= TWOPI)
		p_osc->curphase -= TWOPI;
	if(p_osc->curphase < 0.0)
		p_osc->curphase += TWOPI;
	return val;
}
