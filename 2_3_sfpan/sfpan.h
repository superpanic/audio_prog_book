
typedef struct panpos {
	double left;
	double right;
} PANPOS;

PANPOS simplepan(double position);
PANPOS constpowerpan(double position);
double max_samp(float* buf, unsigned long block_size);
