#include "wave.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

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
		// set only once!
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

double sqtick(OSCIL *p_osc, double freq) {
	double val;
	if(p_osc->curfreq != freq) {
		// set only once!
		p_osc->curfreq = freq;
		p_osc->incr = p_osc->twopiovrsr * freq;
	}
	// flip if under or above one pi
	if(p_osc->curphase <= M_PI) {
		val = 1.0;
	} else {
		val = -1.0;
	}

	p_osc->curphase += p_osc->incr;
	if(p_osc->curphase >= TWOPI) 
		p_osc->curphase -= TWOPI;
	if(p_osc->curphase < 0.0) 
		p_osc->curphase += TWOPI;
	
	return val;
}


double falltick(OSCIL *p_osc, double freq) {
	double val;
	if(p_osc->curfreq != freq) {
		// set only once!
		p_osc->curfreq = freq;
		p_osc->incr = p_osc->twopiovrsr * freq;
	}
	
	val = 1.0 - 2.0 * (p_osc->curphase * (1.0/TWOPI));

	p_osc->curphase += p_osc->incr;
	if(p_osc->curphase >= TWOPI) 
		p_osc->curphase -= TWOPI;
	if(p_osc->curphase < 0.0) 
		p_osc->curphase += TWOPI;
	
	return val;
}


double risetick(OSCIL *p_osc, double freq) {
	double val;
	if(p_osc->curfreq != freq) {
		// set only once!
		p_osc->curfreq = freq;
		p_osc->incr = p_osc->twopiovrsr * freq;
	}
	
	val = (2.0 * (p_osc->curphase * (1.0/TWOPI))) -1.0;

	p_osc->curphase += p_osc->incr;
	if(p_osc->curphase >= TWOPI) 
		p_osc->curphase -= TWOPI;
	if(p_osc->curphase < 0.0) 
		p_osc->curphase += TWOPI;
	
	return val;
}

double tritick(OSCIL *p_osc, double freq) {
	double val;
	if(p_osc->curfreq != freq) {
		// set only once!
		p_osc->curfreq = freq;
		p_osc->incr = p_osc->twopiovrsr * freq;
	}
	
	val = (2.0 * (p_osc->curphase * (1.0/TWOPI))) -1.0;
	if(val < 0.0) val = -val;
	val = 2.0*(val - 0.5);

	p_osc->curphase += p_osc->incr;
	if(p_osc->curphase >= TWOPI) 
		p_osc->curphase -= TWOPI;
	if(p_osc->curphase < 0.0) 
		p_osc->curphase += TWOPI;
	
	return val;
}