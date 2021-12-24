#ifndef DEEPGARDEN_UTILITIES_H
#define DEEPGARDEN_UTILITIES_H

#include "game.h"

extern const float pi;

struct vec_u2
{
	unsigned int x;
	unsigned int y;
vec_u2(unsigned int a, unsigned int b);

};

struct vec_i2
{
	int x;
	int y;

vec_i2( int a,  int b);
};

struct vec_f2
{
	float x;
	float y;

vec_f2(float  a, float  b);
};

vec_f2 rotatePointPrecomputed( vec_f2 center, float s, float c, vec_f2 point);
int alphanumeric (char c);

float RNG();

float magnitude_int( int x,  int y);


// inline uint8_t extremelyFastRandomByte();

void setupExtremelyFastNumberGenerators();
 uint16_t extremelyFastNumberInRange(uint16_t from, uint16_t to);

 uint16_t extremelyFastNumberFromZeroTo( uint16_t to);

#endif