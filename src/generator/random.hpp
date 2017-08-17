///
/// hash algorithm for randomness that doesn't need good distribution
///
#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdint>

inline unsigned int hash(unsigned int x)
{
	x = (x ^ 61) ^ (x >> 16);
	x = x + (x << 3);
	x = x ^ (x >> 4);
	x = x * 0x27d4eb2d;
	x = x ^ (x >> 15);
	return x;
}
inline int ihash(int x)
{
	return hash(x) & INT32_MAX;
}

inline float randf(int x)
{
	return ihash(x) / (float)INT32_MAX;
}
inline float randf(int x, int y)
{
	return randf(x xor ihash(y));
}
inline float randf(int x, int y, int z)
{
	return randf(x xor ihash(y) xor ihash(z));
}

inline int rand2d(int x, int z)
{
	return ihash(x + 5953) xor ihash(z + 7639);
}

#endif
