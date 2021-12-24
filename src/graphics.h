#ifndef DEEPGARDEN_GRAPHICS_H
#define DEEPGARDEN_GRAPHICS_H

#include "game.h"

extern float viewZoomSetpoint ;
extern float viewPanSetpointX;
extern float viewPanSetpointY;

void setupGraphics() ;
void shutdownGraphics() ;
void draw( unsigned int ** grid );

void preDraw() ;
void postDraw();

void vertToBuffer (GLfloat * vertex_buffer_data, unsigned int * cursor, Color vert_color, unsigned int x, unsigned int y);

void advanceIndexBuffers (unsigned int * index_buffer_data, unsigned int * index_buffer_content, unsigned int * index_buffer_cursor);

#endif