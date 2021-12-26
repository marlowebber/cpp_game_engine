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

void initializeGame ();

void threadGame () ;

void threadGraphics () ;
void rebuildMenus ();

void maintainMouseJoint (b2Vec2 p);

void destroyMouseJoint ();

bool getMouseJointStatus () ;


int checkClickObjects (b2Vec2 worldClick);

#endif