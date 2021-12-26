#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "game.h"

#define PRIMITIVE_RESTART 0xffff

extern float viewZoomSetpoint ;
extern float viewPanSetpointX;
extern float viewPanSetpointY;

extern float viewZoom;
extern float viewPanX;
extern float viewPanY;

const unsigned int nominalFramerate = 60;
const unsigned int width = 1920;
const unsigned int height = 1080;

extern GLuint vs, fs, program;
extern GLuint vao, vbo;
extern GLuint IndexBufferId;

typedef float t_mat4x4[16];
static inline void mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
{
#define T(a, b) (a * 4 + b)

	out[T(0, 0)] = 2.0f / (right - left);
	out[T(0, 1)] = 0.0f;
	out[T(0, 2)] = 0.0f;
	out[T(0, 3)] = 0.0f;

	out[T(1, 1)] = 2.0f / (top - bottom);
	out[T(1, 0)] = 0.0f;
	out[T(1, 2)] = 0.0f;
	out[T(1, 3)] = 0.0f;

	out[T(2, 2)] = -2.0f / (zfar - znear);
	out[T(2, 0)] = 0.0f;
	out[T(2, 1)] = 0.0f;
	out[T(2, 3)] = 0.0f;

	out[T(3, 0)] = -(right + left) / (right - left);
	out[T(3, 1)] = -(top + bottom) / (top - bottom);
	out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
	out[T(3, 3)] = 1.0f;

#undef T
}

const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

void setupGraphics() ;
void shutdownGraphics() ;

void preDraw() ;
void postDraw();

void prepareForWorldDraw ();
void cleanupAfterWorldDraw();

void vertToBuffer (GLfloat * vertex_buffer_data, unsigned int * cursor, b2Color color, float alpha, b2Vec2 vert) ;

void advanceIndexBuffers (unsigned int * index_buffer_data, unsigned int * index_buffer_content, unsigned int * index_buffer_cursor);

#endif