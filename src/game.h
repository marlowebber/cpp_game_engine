#ifndef GAME_H
#define GAME_H


#include "utilities.h"

void initializeGame ();
void rebuildMenus ();
void threadGame();
void gameGraphics() ;




void drawIsometricTile( Vec_f2 position , Color finalColor);

void drawTile( Vec_f2 position , Color finalColor);

void threadGraphics();
#endif 