
typedef struct NoiseContext {
	int offsets[8][2][4];
	unsigned char *perm, *mperm; // perm mod 12
} NoiseContext;

float noise3(float x, float y, float z, NoiseContext *nc);
float noise3(float x, float y, float z, NoiseContext *nc) __attribute__ ((force_align_arg_pointer));
