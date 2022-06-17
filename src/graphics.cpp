#include "graphics.h"
#include "menus.h"
#include "untitled_marlo_project.h"


#include <SDL.h> // The SDL library
#include <math.h> // sin, fmod

#include <stdio.h> // printf
const unsigned int width = 1920;
const unsigned int height = 1080;
const float fwidth = 1920;
const float fheight = 1080;
const unsigned int bufferSize = 4 * 6 * 1920 * 1080;

float viewZoom = 10.0f;
float viewPanX = 0.0f;
float viewPanY = 0.0f;

float energyColorGrid[bufferSize];
unsigned int colorGridCursor = 0; // keeps track of how many verts to draw this turn.

float viewZoomSetpoint = 250.0f;
float viewPanSetpointX = 0.0f;
float viewPanSetpointY = 0.0f;
float cameraTrackingResponse = 10;

SDL_Window * window;
SDL_GLContext context;

GLuint vs, fs, program;
GLuint vao, vbo;
GLuint IndexBufferId;

// The projection matrix efficiently handles all panning, zooming, and rotation.
t_mat4x4 projection_matrix;

Color clampColor (Color in)
{
	Color out = in;
	if      (out.r > 1.0f) {out.r = 1.0f;}
	else if (out.r < 0.0f) {out.r = 0.0f;}
	if      (out.g > 1.0f) {out.g = 1.0f;}
	else if (out.g < 0.0f) {out.g = 0.0f;}
	if      (out.b > 1.0f) {out.b = 1.0f;}
	else if (out.b < 0.0f) {out.b = 0.0f;}
	if      (out.a > 1.0f) {out.a = 1.0f;}
	else if (out.a < 0.0f) {out.a = 0.0f;}
	return out;
}

Color averageColor (Color a, Color b)
{
	Color c;
	c.r = (a.r + b.r) / 2;
	c.g = (a.g + b.g) / 2;
	c.b = (a.b + b.b) / 2;
	c.a = (a.a + b.a) / 2;
	return clampColor(c);
}

// add both colors together.
// in life, this is like two lights shining together. the result is a mix of both, depending on their strengths.
Color addColor (Color a, Color b)
{
	Color c;
	c.r = (a.r * a.a) + (b.r * b.a);
	c.g = (a.g * a.a) + (b.g * b.a);
	c.b = (a.b * a.a) + (b.b * b.a);
	c.a = a.a + b.a;
	return clampColor(c);
}

// multiply the amount of color in A by the amount of color in B.
// in life, this is like colored light falling on a colored object. If they are the same color, the result will be brighter.
Color multiplyColor (Color a, Color b)
{
	Color c;
	c.r = (a.r * a.a) * (b.r * b.a);
	c.g = (a.g * a.a) * (b.g * b.a);
	c.b = (a.b * a.a) * (b.b * b.a);
	c.a = a.a * b.a ;
	return clampColor(c);
}

Color multiplyColorByScalar(Color a, float b)
{
	Color c;
	c.r = a.r ;
	c.g = a.g ;
	c.b = a.b ;
	c.a = a.a * b;
	return clampColor(c);
}

// allow B to block A.
// in life, this is like a color image shining through a color window. the image is filtered by the color and opacity of the window.
Color filterColor( Color a, Color b)
{
	Color c;
	c.r = (b.r ) + ((1.0f - (b.a)) * (a.r));
	c.g = (b.g ) + ((1.0f - (b.a)) * (a.g));
	c.b = (b.b ) + ((1.0f - (b.a)) * (a.b));
	c.a = a.a + b.a;
	return clampColor(c);
}

// mix A and B
Color mixColor (Color a, Color b, float mix)
{
	if (mix > 1.0f) {mix = 1.0f;}
	else if (mix < 0.0f) { mix = 0.0f;}
	Color c;
	c.r = (a.r * mix) + ( b.r * (1.0f - mix) );
	c.g = (a.g * mix) + ( b.g * (1.0f - mix) );
	c.b = (a.b * mix) + ( b.b * (1.0f - mix) );
	c.a = (a.a * mix) + ( b.a * (1.0f - mix) );
	return c;
}

float colorAmplitude(Color a )
{
	float c = a.r;
	c      += a.g;
	c      += a.b;
	c      *= a.a;

	c = c / 3.0f ; // because there are 3 color components, normalise the result to 1.

	return c;
}

static const char * vertex_shader =
    "#version 330\n"
    "in vec2 i_position;\n"
    "in vec4 i_color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = i_color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position, 0.0, 1.0 );\n"
    "}\n";

static const char * fragment_shader =
    "#version 330\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";


void shutdownGraphics()
{
	SDL_GL_DeleteContext( context );
	SDL_DestroyWindow( window );
}











const unsigned int mapsize = 10;
const float tileHeight =  1.0f;
const float tileWidth = 0.5f;
unsigned int heightmap[mapsize * mapsize];


Vec_f2 worldToIsometric( Vec_f2 input )
{
	// https://stackoverflow.com/questions/892811/drawing-isometric-game-worlds
	// screenX = (cellX * tile_width  / 2) + (cellY * tile_width  / 2)
	// screenY = (cellY * tile_height / 2) - (cellX * tile_height / 2)

	return Vec_f2(
	           (input.x * tileWidth  / 2) + (input.y * tileWidth  / 2)
	           ,
	           (input.y * tileHeight  / 2) + (input.x * tileHeight  / 2)
	       );
}


void drawIsometricTile( Vec_f2 position , Color finalColor)
{
	Vec_f2 isometricPos = worldToIsometric( position );
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + 0.0f,            isometricPos.y +  -(tileHeight / 2)));
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + 0.0f,            isometricPos.y +  (tileHeight / 2)));
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + -(tileWidth / 2), isometricPos.y +  0.0f));
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + 0.0f,            isometricPos.y + -(tileHeight / 2)));
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + 0.0f,            isometricPos.y + (tileHeight / 2)));
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + (tileWidth / 2), isometricPos.y + 0.0f));
}



void drawTile( Vec_f2 position , Color finalColor)
{
	vertToBuffer ( finalColor, Vec_f2 (  position.x -    tileWidth          , position.y -     tileWidth            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x +    tileWidth          , position.y +     tileWidth            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x -    tileWidth          , position.y +     tileWidth            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x -    tileWidth          , position.y -     tileWidth            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x +    tileWidth          , position.y +     tileWidth            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x +    tileWidth          , position.y -     tileWidth            ) );
}


void drawLine(  Vec_f2 a, Vec_f2 b, float thickness, Color finalColor )
{





}


void drawPointerTriangle( Vec_f2 position , Color finalColor, float angle)
{




	// one triangle


	Vec_f2 pointA = Vec_f2 (  position.x -    (tileWidth / 2)          , position.y -     tileWidth            );
	Vec_f2 pointB = Vec_f2 (  position.x +    (tileWidth / 2)          , position.y -     tileWidth            );
	Vec_f2 pointC = Vec_f2 (  position.x                              , position.y +     tileWidth            ) ;


// Vec_f2
	pointA = rotatePointPrecomputed( position, sin(angle), cos(angle), pointA);
	pointB = rotatePointPrecomputed( position, sin(angle), cos(angle), pointB);
	pointC = rotatePointPrecomputed( position, sin(angle), cos(angle), pointC);


	vertToBuffer ( finalColor, pointA ); // A
	vertToBuffer ( finalColor, pointB ); // B
	vertToBuffer ( finalColor, pointC ); // C


	// vertToBuffer ( finalColor, Vec_f2 (  position.x -    (tileWidth/10)          , position.y -     tileWidth            ) ); // A
	// vertToBuffer ( finalColor, Vec_f2 (  position.x +    (tileWidth/10)          , position.y +     tileWidth            ) ); // B
	// vertToBuffer ( finalColor, Vec_f2 (  position.x +    (tileWidth/10)          , position.y -     tileWidth            ) ); // D


}














#define BUFFER_DURATION 1 // Length of the buffer in seconds
#define FREQUENCY 48000 // Samples per second
#define BUFFER_LEN (BUFFER_DURATION*FREQUENCY) // Samples in the buffer

int buffer_pos = 0;

void play_buffer(void* userdata, unsigned char* stream, int len) ;

// // Global audio format variable
// // If the sound card doesn't support this format, SDL handles the
// // conversions seemlessly for us
SDL_AudioSpec spec = {
	.freq = FREQUENCY,
	.format = AUDIO_S16SYS, // Signed 16 bit integer format
	.channels = 1,
	.samples = 4096, // The size of each "chunk"
	.callback = play_buffer, // user-defined function that provides the audio data
	.userdata = NULL // an argument to the callback function (we dont need any)
};


// // Buffer that gets filled with the audio samples
int16_t buffer[BUFFER_LEN];

void play_buffer(void* userdata, unsigned char* stream, int len) {
	// Silence the whole stream in case we don't touch some parts of it
	// This fills the stream with the silence value (almost always just 0)
	// SDL implements the standard library (SDL_memset, SDL_memcpy etc.) to support more platforms
	SDL_memset(stream, spec.silence, len);

	// Dividing the stream size by 2 gives us the real length of the buffer (our format is 2 bytes per sample)
	len /= 2;
	// Prevent overflows (if we get to the end of the sound buffer, we don't want to read beyond it)
	len = (buffer_pos + len < BUFFER_LEN ? len : BUFFER_LEN - buffer_pos);

	// If we are at the end of the buffer, keep the silence and return
	if (len == 0) return;

	// // Copy the samples from the current position in the buffer to the stream
	// // Notice that the length gets multiplied back by 2 because we need to specify the length IN BYTES
	SDL_memcpy(stream, buffer + buffer_pos, len * 2);

	// // Move the buffer position
	buffer_pos += len;


	printf("play buffer callback\n");
}


// // This maps our [0, 1] value to the sound card format we chose (signed 16 bit integer)
// // Amplitude is the height of the wave, hence the volume
// Sint16 format(double sample, double amplitude) {
// 	// 32567 is the maximum value of a 16 bit signed integer (2^15-1)
// 	return (Sint16)(sample*32567*amplitude);
// }

// // Generate a sine wave
// double tone(double hz, unsigned long time) {
// 	return sin(time * hz * M_PI * 2 / FREQUENCY);
// }

// // Generate a sawtooth wave
// double saw(double hz, unsigned long time) {
// 	return fmod(time*hz/FREQUENCY, 1)*2-1;
// }

// // Generate a square wave
// double square(double hz, unsigned long time) {
// 	double sine = tone(hz, time);
// 	return sine > 0.0 ? 1.0 : -1.0;
// }



void setupGraphics()
{
	int sdl_error_code = SDL_Init( SDL_INIT_VIDEO );
	// | SDL_INIT_AUDIO);

	// std::cout << SDL_GetError() << '\n';




	// SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);


	// for (int i = 0; i < BUFFER_LEN; i++)
	// {
//        buffer[i] = sin(i * 440 * 3.1415 * 2 / FREQUENCY);

	// }


	// SDL_PauseAudioDevice(dev, 0);















	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	window = SDL_CreateWindow( "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	context = SDL_GL_CreateContext( window );

	/**
	You have to bind and unbind the buffer to copy to it from 'C' and draw with it from openGL. Think of it as locking/unlocking between the program and the graphics.

	So the sequence is:
	create
	bind
	stuff data
	unbind

	bind
	display
	unbind

	https://stackoverflow.com/questions/8599264/why-the-second-call-to-glbindbuffer
	**/

	vs = glCreateShader( GL_VERTEX_SHADER );
	// gs = glCreateShader( GL_GEOMETRY_SHADER );
	fs = glCreateShader( GL_FRAGMENT_SHADER );

	int length = strlen( vertex_shader );
	glShaderSource( vs, 1, ( const GLchar ** )&vertex_shader, &length );
	glCompileShader( vs );

	GLint status;
	glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
	if ( status == GL_FALSE )
	{
		fprintf( stderr, "vertex shader compilation failed\n" );
	}

	// this is an example of how a geometry shader might be implemented. leave the commented code as reference for this useful feature.
	//  length = strlen( geometry_shader );
	// glShaderSource( gs, 1, ( const GLchar ** )&geometry_shader, &length );
	// glCompileShader( gs );

	// GLint status;
	// glGetShaderiv( gs, GL_COMPILE_STATUS, &status );
	// if ( status == GL_FALSE )
	// {
	// 	fprintf( stderr, "geometry shader compilation failed\n" );
	// }

	length = strlen( fragment_shader );
	glShaderSource( fs, 1, ( const GLchar ** )&fragment_shader, &length );
	glCompileShader( fs );

	glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
	if ( status == GL_FALSE )
	{
		fprintf( stderr, "fragment shader compilation failed\n" );
	}

	program = glCreateProgram();
	glAttachShader( program, vs );
	// glAttachShader( program, gs );
	glAttachShader( program, fs );

	glBindAttribLocation( program, attrib_position, "i_position" );
	glBindAttribLocation( program, attrib_color, "i_color" );
	glLinkProgram( program );

	glDisable( GL_DEPTH_TEST );
	glViewport( 0, 0, width, height );

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xffff);

	glGenBuffers(1, &IndexBufferId);
	glGenVertexArrays( 1, &vao );
	glGenBuffers( 1, &vbo );

	glUseProgram( program );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);

	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );

	glEnableVertexAttribArray( attrib_position );
	glEnableVertexAttribArray( attrib_color );

	glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
	glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

	glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(3);

	glBufferData( GL_ARRAY_BUFFER, bufferSize, energyColorGrid, GL_DYNAMIC_DRAW );

	for (int i = 0; i < bufferSize; ++i)
	{
		energyColorGrid[i] = 0.0f;
	}
}

void prepareForWorldDraw ()
{
	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glEnableVertexAttribArray( attrib_position );
	glEnableVertexAttribArray( attrib_color );
	glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
	glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

	// mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
	mat4x4_ortho(
	    projection_matrix,
	    (-1 * viewZoom) + viewPanX,
	    (+1 * viewZoom) + viewPanX,
	    (-1 * (fheight / fwidth) * viewZoom) + viewPanY,
	    (+1 * (fheight / fwidth) * viewZoom) + viewPanY,
	    -10.0f,
	    +10.0f
	);
	glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );
}

void prepareForMenuDraw ()
{
	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glEnableVertexAttribArray( attrib_position );
	glEnableVertexAttribArray( attrib_color );
	glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
	glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

	// mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
	mat4x4_ortho(
	    projection_matrix,
	    0,
	    (fwidth / 2.0f) * 0.835f,
	    0,
	    (fheight / 2.0f) * 1.1f,
	    -10.0f,
	    +10.0f
	);
	glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );
}

const unsigned int floats_per_color = 16;

void vertToBuffer ( Color color, Vec_f2 vert )
{
	if ( ( (colorGridCursor * 6) + 5 ) < bufferSize )
	{
		float floatx = vert.x;
		float floaty = vert.y;
		energyColorGrid[ (colorGridCursor * 6) + 0 ] = color.r;
		energyColorGrid[ (colorGridCursor * 6) + 1 ] = color.g;
		energyColorGrid[ (colorGridCursor * 6) + 2 ] = color.b;
		energyColorGrid[ (colorGridCursor * 6) + 3 ] = color.a;
		energyColorGrid[ (colorGridCursor * 6) + 4 ] = vert.x;
		energyColorGrid[ (colorGridCursor * 6) + 5 ] = vert.y;
		colorGridCursor ++;
	}
}

void advanceIndexBuffers (unsigned int * index_buffer_data, unsigned int * index_buffer_content, unsigned int * index_buffer_cursor)
{
	index_buffer_data[(*index_buffer_cursor)] = (*index_buffer_content);
	(*index_buffer_cursor)++;
	(*index_buffer_content)++;
}

void preDraw()
{
	colorGridCursor = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	colorGridCursor = 0;
}

void postDraw ()
{
	SDL_GL_SwapWindow( window );
	float zoomDifference = (viewZoom - viewZoomSetpoint);
	float zoomResponse = (zoomDifference * -1) / cameraTrackingResponse;
	viewZoom = viewZoom + zoomResponse;
	float panDifferenceX = (viewPanX - viewPanSetpointX);
	float panResponseX = (panDifferenceX * -1) / cameraTrackingResponse;
	float panDifferenceY = (viewPanY - viewPanSetpointY);
	float panResponseY = (panDifferenceY * -1) / cameraTrackingResponse;
	viewPanX +=  panResponseX ;
	viewPanY += panResponseY ;
}

void addExamplePanelToBuffer()
{
	vertToBuffer(color_lightblue, Vec_f2(0.0f, 0.0f));
	vertToBuffer(color_lightblue, Vec_f2(250.0f, 300.0f));
	vertToBuffer(color_lightblue, Vec_f2(0.0f, 300.0f));

	vertToBuffer(color_lightblue, Vec_f2(0.0f, 0.0f));
	vertToBuffer(color_lightblue, Vec_f2(250.0f, 300.0f));
	vertToBuffer(color_lightblue, Vec_f2(250.0f, 0.0f));
}


void mainMenuDraw()
{
	preDraw();

	prepareForMenuDraw();
	drawMainMenuText();

	postDraw();
}

void threadGraphics()
{
	preDraw();

	prepareForWorldDraw ();


	camera();

	if (lockfps)
	{
		model();
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, energyColorGrid);
	glDrawArrays(GL_TRIANGLES, 0,  colorGridCursor);

	unsigned int endOfWorldVertexRegion = colorGridCursor;

	prepareForMenuDraw();
	// addExamplePanelToBuffer();
	drawPanels();
	glBufferSubData(GL_ARRAY_BUFFER, endOfWorldVertexRegion, (colorGridCursor - endOfWorldVertexRegion), energyColorGrid);
	glDrawArrays   (GL_TRIANGLES,    endOfWorldVertexRegion,  (colorGridCursor - endOfWorldVertexRegion));

	drawGameInterfaceText();

	drawAllMenuText ();

	postDraw();
}



Vec_f2 GetOGLPos(int x, int y)
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return Vec_f2(posX, posY);
}