#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "game.h"

#define PRIMITIVE_RESTART 0xffff

extern float viewZoomSetpoint ;
extern float viewPanSetpointX;
extern float viewPanSetpointY;

void setupGraphics() ;
void shutdownGraphics() ;

void preDraw() ;
void postDraw();


void prepareForWorldDraw ();

void cleanupAfterWorldDraw();

void advanceIndexBuffers (unsigned int * index_buffer_data, unsigned int * index_buffer_content, unsigned int * index_buffer_cursor);

#endif