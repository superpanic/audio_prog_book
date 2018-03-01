// gtable.c
#include <stdlib.h>
#include "gtable.h"
#include "../include/common.h"

GTABLE *new_sine(uint32_t length) {
	uint32_t i;
	double step;
	GTABLE *gtable = NULL;

	if(length == 0) return NULL;
	gtable = (GTABLE*) malloc(sizeof(GTABLE));
	if(gtable == NULL) return NULL;
	gtable->table = (double*) malloc((length+1) * sizeof(double));
	if(gtable->table == NULL) {
		free(gtable);
		return NULL;
	}

	gtable->length = length;
	step = TWOPI / length;
	for(i=0; i<length; i++) gtable->table[i] = sin(step * i);
	gtable->table[i] = gtable->table[0]; // guard point;
	return gtable;
}

// gtable destruction
void gtable_free(GTABLE** gtable) {
	if(gtable && *gtable &&(*gtable)->table) {
		free((*gtable)->table);
		free(*gtable);
		*gtable = NULL;
	}
}

OSCIL* new_oscilt(double srate, const GTABLE* gtable, double phase) {
	OSCILT* p_osc;
	// do we have a good gtable?
	if(gtable == NULL || gtable->table == NULL || gtable->length == 0) return NULL;
	p_osc = (OSCILT *) malloc(sizeof(OSCILT));
	if(p_osc == NULL) return NULL;
	p_osc->osc.curfreq = 0.0;
	p_osc->osc.curphase = gtable->length * phase;
	p_osc->osc.incr = 0.0;
	// then the GTABLE-specific things
	p_osc->gtable = gtable;
	p_osc->dtablen = (double) gtable->length;
	p_osc->sizeovrsr = p_osc->dtablen / (double) srate;
	return p_osc;
}

double tabtick(OSCILT* p_osc, double freq) {
	int index = (int) (p_osc->osc.curphase);
	double val;
	double dtablen = p_osc->dtablen;
	double curphase = p_osc->osc.curphase;
	double *table = p_osc->gtable->table;
	if(p_osc->osc.curfreq != freq) {
		p_osc->osc.curfreq = freq;
		p_osc->osc.incr = p_osc->sizeovrsr * p_osc->osc.curfreq;
	}
	curphase += p_osc->osc.incr;
	// wrap
	while(curphase >= dtablen) curphase -= dtablen;
	while(curphase < 0.0) curphase += dtablen;
	p_osc->osc.curphase = curphase;
	return table[index];
}

double tabitick(OSCILT* p_osc, double freq) {
	int base_index = (int) p_osc->osc.curphase;
	unsigned long next_index = base_index + 1;
	double frac, slope, val;
	double dtablen = p_osc->dtablen;
	double curphase = p_osc->osc.curphase;
	double *table = p_osc->gtable->table;
	if(p_osc->osc.curfreq != freq) {
		p_osc->osc.curfreq = freq;
		p_osc->osc.incr = p_osc->sizeovrsr;
		p_osc->osc.curfreq;
	}
	frac = curphase - base_index;
	val = table[base_index];
	slope = table[next_index] - val;
	val += (frac * slope);
	curphase += p_osc->osc.incr;
	while(curphase >= dtablen) curphase -= dtablen;
	while(curphase < 0.0) curphase += dtablen;
	p_osc->osc.curphase = curphase;
	return val;
}
