#include "utilities.h"

#include <fstream>
#include<iostream>
//https://github.com/edrosten/8bit_rng/blob/master/rng-4261412736.c
uint32_t x, y, z, a;

Vec_u2::Vec_u2()
{
	this->x = 0;
	this->y = 0;
}

Vec_i2::Vec_i2()
{
	this->x = 0;
	this->y = 0;
}

Vec_f2::Vec_f2()
{
	this->x = 0.0f;
	this->y = 0.0f;
}


Vec_u2::Vec_u2(unsigned int a, unsigned int b)
{
	this->x = a;
	this->y = b;
}

Vec_i2::Vec_i2( int a,  int b)
{
	this->x = a;
	this->y = b;
}

Vec_f2::Vec_f2( float a,  float b)
{
	this->x = a;
	this->y = b;
}


AABB::AABB()
{
	this->upperBound = Vec_f2(0.0f, 0.0f);
	this->lowerBound = Vec_f2(0.0f, 0.0f);;
}

AABB::AABB(Vec_f2 upperBound, Vec_f2 lowerBound)
{
	this->upperBound = upperBound;
	this->lowerBound = lowerBound;
}


Color::Color()
{
	this->r = 0.0f;
	this->g = 0.0f;
	this->b = 0.0f;
	this->a = 0.0f;
}


Color::Color(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

uDataWrap::uDataWrap()
{
	uData = nullptr;
	dataType = TYPE_UNINIT;
}

uDataWrap::uDataWrap(void * dat, unsigned int typ)
{
	uData = dat;
	dataType = typ;
}


Vec_f2 rotatePointPrecomputed( Vec_f2 center, float s, float c, Vec_f2 point)
{
	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	// rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// translate point back:
	point.x = xnew + center.x;
	point.y = ynew + center.y;
	return Vec_f2(point.x, point.y);
};

float magnitude_int( int x,  int y)
{
	float mag = sqrt(x * x + y * y);
	return mag;
}

int distanceBetweenPoints( Vec_i2 a, Vec_i2 b )
{
	int diffX = abs(a.x - b.x);
	int diffY = abs(a.y - b.y);
	return magnitude_int( diffX,  diffY);
}

float RNG()
{
	static std::default_random_engine e;
	e.seed(std::chrono::system_clock::now().time_since_epoch().count());
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

inline uint32_t extremelyFastRandomByte()
{
	// it used to be an actual byte, but that makes it eventually run out of randomness and always choose the same number!!
	// mask off the top 8 if you really need a byte.
	uint32_t t = x ^ (x << 8);
	x = y;
	y = z;
	z = a;
	a = z ^ t ^ ( z >> 1) ^ (t << 1);
	return a;
}

uint32_t extremelyFastNumberInRange(uint32_t from, uint32_t to)
{
	return from + ( extremelyFastRandomByte() % ( to - from + 1 ) );
}

uint32_t extremelyFastNumberFromZeroTo( uint32_t to)
{
	return ( extremelyFastRandomByte() % ( to + 1 ) );
}

void seedExtremelyFastNumberGenerators()
{
	if (RNG() < 0.5)  {x = 1;}
	if (RNG() < 0.5)  {y = 1;}
	if (RNG() < 0.5)  {z = 1;}
}


void setupExtremelyFastNumberGenerators()
{
	x = 0;
	y = 0;
	z = 0;
	a = 1;
}



float fast_sigmoid(float in)
{
	// https://stackoverflow.com/questions/10732027/fast-sigmoid-algorithm
	float out = (in / (1 + abs(in)));
	return  out;
}


bool exists_test3(std::string filename)
{
	// using namespace std;
	// int main() {
	/* try to open file to read */
	std::ifstream ifile;
	ifile.open(filename.c_str());
	if (ifile) {
		return true;//cout << "file exists";
	}
	// else {
	return false;//cout << "file doesn't exist";
	// }
	// }
}


float clamp(float in, float min, float max)
{
float out = in;
if (out < min) { out = min;}
else if (out > max){ out = max;}
return out;

}