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

void initializeGame ();
void rebuildMenus ();
void threadGame();
void gameGraphics() ;




void threadGraphics();
#endif 