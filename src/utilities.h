#ifndef UTILITIES_H
#define UTILITIES_H

#include "physics.h"
#include <box2d.h>

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

struct uDataWrap
{
	void * uData;
	unsigned int dataType;

	uDataWrap(void * dat, unsigned int typ);

};

#define TYPE_UDATA_STRING 	(1<<1)
#define TYPE_UDATA_UINT   	(1<<2)
#define TYPE_UDATA_INT    	(1<<3)
#define TYPE_UDATA_FLOAT  	(1<<4)
#define TYPE_UNINIT		  	(1<<5)
#define TYPE_UDATA_BOOL	  	(1<<6)



vec_f2 rotatePointPrecomputed( vec_f2 center, float s, float c, vec_f2 point);
b2Vec2 b2RotatePointPrecomputed( b2Vec2 center, float s, float c, b2Vec2 point);

int alphanumeric (char c);

float RNG();

float magnitude_int( int x,  int y);

void setupExtremelyFastNumberGenerators();
uint16_t extremelyFastNumberInRange(uint16_t from, uint16_t to);
uint16_t extremelyFastNumberFromZeroTo( uint16_t to);

#endif