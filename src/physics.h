#ifndef GAME_H
#define GAME_H

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

#include "utilities.h"

#include <box2d.h>

// #define THREAD_TIMING

// struct Color
// {
// 	float r;
// 	float g;
// 	float b;
// 	float a;

// 	Color(float r, float g, float b, float a);
// };

;
// extern b2MouseJoint* m_mouseJoint;


class PhysicalObject
{
public:
	b2BodyDef bodyDef;
	b2Body * p_body;
	b2PolygonShape shape;
	b2Fixture * p_fixture;
	float fraction;
	bool flagDelete;
	bool flagReady;
	b2Color color;

	std::vector<b2Vec2>  vertices;

	PhysicalObject (std::vector<b2Vec2>   vertices, bool flagStatic);
};


extern std::list<b2Body* > rayContacts;
extern std::list<PhysicalObject> physicalObjects;

void initializePhysics ();

void threadPhysics ();

// void threadGame () ;

void threadGraphics () ;

void maintainMouseJoint (b2Vec2 p);

void destroyMouseJoint ();

bool getMouseJointStatus () ;


void exampleMenuCallback(void * userData);
int checkClickObjects (b2Vec2 worldClick);


void addToWorld(PhysicalObject object, b2Vec2 position, float angle);

void deleteFromWorld (PhysicalObject * object);

#endif