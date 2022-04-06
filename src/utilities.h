#ifndef UTILITIES_H
#define UTILITIES_H


#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"
#include <vector>
#include <list>
#include <string>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <random>

#include <boost/thread.hpp>


struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color();
	Color(float r, float g, float b, float a);
};


struct Vec_u2
{
	unsigned int x;
	unsigned int y;

	Vec_u2();
	Vec_u2(unsigned int a, unsigned int b);

};

struct Vec_i2
{
	int x;
	int y;

	Vec_i2();
	Vec_i2( int a,  int b);
};

struct Vec_f2
{
	float x;
	float y;

	Vec_f2();
	Vec_f2(float  a, float  b);
};


#define TYPE_UDATA_STRING 	(1<<1)
#define TYPE_UDATA_UINT   	(1<<2)
#define TYPE_UDATA_INT    	(1<<3)
#define TYPE_UDATA_FLOAT  	(1<<4)
#define TYPE_UNINIT		  	(1<<5)
#define TYPE_UDATA_BOOL	  	(1<<6)

struct uDataWrap
{
	void * uData;
	unsigned int dataType;

	uDataWrap();
	uDataWrap(void * dat, unsigned int typ);

};

struct AABB
{
	Vec_f2 upperBound;
	Vec_f2 lowerBound;

	AABB();
	AABB(Vec_f2 upperBound, Vec_f2 lowerBound);
};



Vec_f2 rotatePointPrecomputed( Vec_f2 center, float s, float c, Vec_f2 point);

int alphanumeric (char c);
char numeralphabetic (int i);

float RNG();

float magnitude_int( int x,  int y);

int distanceBetweenPoints( Vec_i2 a, Vec_i2 b );

void seedExtremelyFastNumberGenerators();
uint32_t extremelyFastNumberInRange    (uint32_t from, uint32_t to);
uint32_t extremelyFastNumberFromZeroTo( uint32_t to);


void setupExtremelyFastNumberGenerators();

#endif