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
const Color color_brightred          = Color( 0.9f, 0.1f, 0.0f, 1.0f);
const Color color_darkred            = Color( 0.5f, 0.05f, 0.0f, 1.0f);
const Color color_brown              = Color(  0.25f, 0.1f, 0.0f, 1.0f );
const Color color_green              = Color(  0.25f, 0.8f, 0.25f, 1.0f );
const Color color_darkgreen              = Color(  0.1f, 0.35f, 0.15f, 1.0f );
const Color color_clear              = Color( 0.0f, 0.0f, 0.0f, 0.0f );
const Color color_pink              = Color(  1.0f, 0.8f, 0.8f, 1.0f );

const Color color_cream          = Color(  0.91f, 0.92f, 0.782f, 1.0f );
const Color color_offwhite           = Color( 0.9f, 1.0f, 0.8f, 1.0f );


const Color color_puce               = Color(0.48, 0.204, 0.134, 1.0f);
const Color color_tan           = Color(0.910, 0.815, 0.673, 1.0f);

const Color color_lightbrown           = Color(0.5, 0.4, 0.3, 1.0f);


const Color color_muscles1     = Color(0.710, 0.140, 0.09, 1.0f);
const Color color_muscles2     = Color(0.780, 0.120, 0.0468, 1.0f);

const Color color_brains1      = Color(0.830, 0.506, 0.711, 1.0f);
const Color color_brains2      = Color(0.9, 0.648, 0.808, 1.0f);
const Color color_brains3      = Color(0.710, 0.582, 0.691, 1.0f);
const Color color_brains4      = Color(0.92, 0.856, 0.910, 1.0f);

const Color color_lungs1      = Color(0.99, 0.683, 0.867, 1.0f);

const Color color_violet     = Color(0.762, 0.560, 0.950, 1.0f);

const Color color_peach       = Color(	1.0, 0.375, 0.250, 1.0f);
const Color color_peach_light = Color(0.950, 0.681, 0.627, 1.0f);


const Color color_blue_thirdClear = Color(0.1, 0.35, 1.0, 0.35f);
const Color color_black_thirdClear = Color(0.1, 0.35, 1.0, 0.35f);



const Color tint_selected              = Color (1.0f, 1.0f, 1.0f, 0.25f);

const Color tint_wall              = Color (1.0f, 1.0f, 1.0f, 0.1f);

const Color tint_shadow              = Color (0.0f, 0.0f, 0.0f, 0.5f);

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
float fast_sigmoid(float in);


bool exists_test3(std::string filename);
// inline bool exists_test3 (const std::string& name)
// {
// 	struct stat buffer;
// 	return (stat (name.c_str(), &buffer) == 0);
// }

#endif