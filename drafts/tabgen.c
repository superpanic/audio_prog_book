#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define TABLEN 1024
#define TWOPI 6.283185307179586476925287

int main(int argc, char *argv[]) {
	int err = 0;
	uint32_t i;
	uint32_t len = TABLEN;
	double step;
	uint8_t guard_flag = 0;
	FILE *fp;

	if(argc > 1) {
		for(i=0; i<argc; i++) {
			if(argv[i][0] == '-') {
				if(argv[i][1]) {
					if(argv[i][1] == 'g') {
						printf("\tguard flag found, adding guard point to table\n");
						guard_flag = 1;	
						len += 1;		
					} else {
						printf("\terror: unknown flag argument found\n");
					}
				} else {
					printf("\terror: no flag\n");
				}
			}
		}
	}

	double table[len];

		// generate table
	step = TWOPI/len;
	for(i=0;i<len;i++) {
		table[i] = sin(step * i);
	}

	if(guard_flag) table[len-1] = table[0];

	if(argc > 1) {
		// print to file
		fp = fopen(argv[1], "w");
		if(fp == NULL) {
			printf("\tfailed to create file named %s\n"
				"\taborting...\n", argv[1]);
			err++;
			return err;
		}
		for(i=0;i<len;i++) {
			fprintf(fp, "%lf", table[i]);
			if(i<len-1) fprintf(fp, ", ");
		}
		fclose(fp);
		printf("\tfinished printing %i values to file '%s'\n", len, argv[1]);
	} else {
		// print to console
		for(i=0;i<len;i++) {
			printf("%lf", table[i]);
			if(i<len-1) printf(", ");
		}
		printf("\n\tfinished printing %i values\n", len);
	}
	
	return 0;
}
