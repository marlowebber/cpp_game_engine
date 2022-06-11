#ifndef UTILITIES_H
#define UTILITIES_H


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
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

#include <iostream>

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


const Color color_darkblue         = Color( 0.0f, 0.05f, 0.35f, 1.0f );

const Color color_lightblue          = Color( 0.1f, 0.3f, 0.65f, 1.0f );
const Color color_yellow             = Color( 1.0f, 1.0f, 0.0f, 1.0f );
const Color color_lightgrey          = Color( 0.75f, 0.75f, 0.75f, 1.0f );
const Color color_grey               = Color( 0.50f, 0.50f, 0.50f, 1.0f );
const Color color_darkgrey           = Color( 0.25f, 0.25f, 0.25f, 1.0f );

const Color color_charcoal           = Color( 0.1f, 0.1f, 0.1f, 1.0f );
const Color color_black              = Color( 0.0f, 0.0f, 0.0f, 1.0f );
const Color color_white              = Color (1.0f, 1.0f, 1.0f, 1.0f);
const Color color_purple             = Color( 0.8f, 0.0f, 0.8f, 1.0f );
const Color color_orange             = Color( 1.0f, 0.8f, 0.0f, 1.0f);
const Color color_offwhite           = Color( 0.9f, 1.0f, 0.8f, 1.0f );
const Color color_brightred          = Color( 0.9f, 0.1f, 0.0f, 1.0f);
const Color color_darkred            = Color( 0.5f, 0.05f, 0.0f, 1.0f);
const Color color_brown              = Color(  0.25f, 0.1f, 0.0f, 1.0f );
const Color color_green              = Color(  0.25f, 0.8f, 0.25f, 1.0f );
const Color color_darkgreen              = Color(  0.1f, 0.35f, 0.15f, 1.0f );
const Color color_clear              = Color( 0.0f, 0.0f, 0.0f, 0.0f );
const Color color_pink              = Color(  1.0f, 0.8f, 0.8f, 1.0f );



const Color tint_selected              = Color (1.0f, 1.0f, 1.0f, 0.25f);

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