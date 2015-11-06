/***********************************************************************
FROM: https://github.com/FeepingCreature/SimplexNoise
************************************************************************

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain this list of conditions
      and the following disclaimer.
    * Redistributions in binary form must reproduce this list of conditions
      and the following disclaimer in the documentation and/or other
      materials provided with the distribution.
    * The names of its contributors may not be used to endorse or
      promote products derived from this software without specific
      prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/
#include "x86intrin.h"

typedef float v4sf __attribute__ ((vector_size (16)));
typedef int   v4si __attribute__ ((vector_size (16)));

typedef struct {
  unsigned char *perm, *mperm; // perm mod 12
} NoiseContext;

#define LET(A, B) typeof(B) A = B

inline v4sf vec4f(float a, float b, float c, float d) {
  return (v4sf) _mm_set_ps(d, c, b, a);
}

inline v4sf vec1_4f(float f) {
  return (v4sf) _mm_set1_ps(f);
}

inline int isum(v4si vec) {
  int i[4];
  *(v4si*) &i = vec;
  return i[0] + i[1] + i[2];
}

inline float sum3(v4sf vec) {
  float f[4];
  *(v4sf*) &f = vec;
  return f[0] + f[1] + f[2];
}

inline float sum4(v4sf vec) {
  float f[4];
  *(v4sf*) &f = vec;
  return f[0] + f[1] + f[2] + f[3];
}

extern NoiseContext noiseContext;
extern void permsetup(NoiseContext *nc);

inline float sse_simplex3(float x, float y, float z)
{
	extern float sse_noise3(float x, float y, float z, NoiseContext *nc);
	return sse_noise3(x, y, z, &noiseContext);
}
