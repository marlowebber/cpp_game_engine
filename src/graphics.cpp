#include "graphics.h"
#include "menus.h"




float viewZoom = 10.0f;
float viewPanX = 0.0f;
float viewPanY = 0.0f;


// const unsigned int nominalFramerate = 60;
const unsigned int width = 1920;
const unsigned int height = 1080;
const float fwidth = 1920;
const float fheight = 1080;

const unsigned int bufferSize = 1000000;

float energyColorGrid[bufferSize];
// float * energyColorGrid = new float[bufferSize];
unsigned int colorGridCursor = 0; // keeps track of how many verts to draw this turn.

float viewZoomSetpoint = 1000.0f;
float viewPanSetpointX = 0.0f;
float viewPanSetpointY = 0.0f;

float cameraTrackingResponse = 10;

SDL_Window * window;
SDL_GLContext context;

GLuint vs, fs, program;
GLuint vao, vbo;
GLuint IndexBufferId;




// Color::Color(float r, float g, float b, float a)
// {
// 	this->r = r;
// 	this->g = g;
// 	this->b = b;
// 	this->a = a;
// }


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
	Color c;// = Color()
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
	Color c ;//= Color(0.0f, 0.0f, 0.0f, 0.0f);
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
	Color c ;//= Color(0.0f, 0.0f, 0.0f, 0.0f);
	c.r = (a.r * a.a) * (b.r * b.a);
	c.g = (a.g * a.a) * (b.g * b.a);
	c.b = (a.b * a.a) * (b.b * b.a);
	c.a = a.a * b.a ;
	return clampColor(c);
}

Color multiplyColorByScalar(Color a, float b)
{
	Color c;
	c.r = a.r * b;
	c.g = a.g * b;
	c.b = a.b * b;
	c.a = a.a * b;
	return clampColor(c);
}

// allow B to block A.
// in life, this is like a color image shining through a color window. the image is filtered by the color and opacity of the window.
Color filterColor( Color a, Color b)
{

	Color c ;//= Color(0.0f, 0.0f, 0.0f, 0.0f);

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

// static const char * geometry_shader =



static const char * fragment_shader =
    "#version 330\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

// typedef enum t_attrib_id
// {
// 	attrib_position,
// 	attrib_color
// } t_attrib_id;



// typedef float t_mat4x4[16];
// static inline void mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
// {
// #define T(a, b) (a * 4 + b)

// 	out[T(0, 0)] = 2.0f / (right - left);
// 	out[T(0, 1)] = 0.0f;
// 	out[T(0, 2)] = 0.0f;
// 	out[T(0, 3)] = 0.0f;

// 	out[T(1, 1)] = 2.0f / (top - bottom);
// 	out[T(1, 0)] = 0.0f;
// 	out[T(1, 2)] = 0.0f;
// 	out[T(1, 3)] = 0.0f;

// 	out[T(2, 2)] = -2.0f / (zfar - znear);
// 	out[T(2, 0)] = 0.0f;
// 	out[T(2, 1)] = 0.0f;
// 	out[T(2, 3)] = 0.0f;

// 	out[T(3, 0)] = -(right + left) / (right - left);
// 	out[T(3, 1)] = -(top + bottom) / (top - bottom);
// 	out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
// 	out[T(3, 3)] = 1.0f;

// #undef T
// }

// The projection matrix efficiently handles all panning, zooming, and rotation.
t_mat4x4 projection_matrix;


void shutdownGraphics()
{
	SDL_GL_DeleteContext( context );
	SDL_DestroyWindow( window );
}


void setupGraphics()
{


	// glPointSize(3);

	// Setup the game window with SDL2
	SDL_Init( SDL_INIT_VIDEO );
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
	 * */

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

	// glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );


	glClearColor( 0.00f, 0.00f, 0.00f, 1.0f );


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glPointSize(3);


	glBufferData( GL_ARRAY_BUFFER, bufferSize, energyColorGrid, GL_DYNAMIC_DRAW );
}

void prepareForWorldDraw ()
{
	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glEnableVertexAttribArray( attrib_position );
	glEnableVertexAttribArray( attrib_color );
	glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
	glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );


// unsigned int zoomInt = viewZoom;


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


// unsigned int zoomInt = viewZoom;


	// mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
	mat4x4_ortho(
	    projection_matrix,
	    (-(fwidth / 2)),
	    (+(fwidth / 2)),
	    (-(fheight / 2) ),
	    (+(fheight / 2) ),
	    -10.0f,
	    +10.0f
	);

	// mat4x4_ortho(
	//     projection_matrix,
	//     (-1 * viewZoom) ,
	//     (+1 * viewZoom) ,
	//     (-1 * (fheight / fwidth) * viewZoom),
	//     (+1 * (fheight / fwidth) * viewZoom),
	//     -10.0f,
	//     +10.0f
	// );

	glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );

}







// void cleanupAfterWorldDraw ()
// {
// 	glDisable(GL_BLEND);
// 	glDisableVertexAttribArray(attrib_position);
// 	glDisableVertexAttribArray(attrib_color);
// }

const unsigned int floats_per_color = 16;

void vertToBuffer ( Color color, Vec_f2 vert )
{
	// if (colorGridCursor < (bufferSize - 8))
	// {
	float floatx = vert.x;
	float floaty = vert.y;
	memcpy( &( energyColorGrid[colorGridCursor] ) , &color , sizeof(Color));
	memcpy(   (&( energyColorGrid[colorGridCursor] )) + sizeof(Color) , &vert , sizeof(Vec_f2));
	// colorGridCursor += (sizeof(Color) + sizeof(Vec_f2) );

	energyColorGrid[ (colorGridCursor * 6) + 0 ] = color.r;
	energyColorGrid[ (colorGridCursor * 6) + 1 ] = color.g;
	energyColorGrid[ (colorGridCursor * 6) + 2 ] = color.b;
	energyColorGrid[ (colorGridCursor * 6) + 3 ] = color.a;
	energyColorGrid[ (colorGridCursor * 6) + 4 ] = vert.x;
	energyColorGrid[ (colorGridCursor * 6) + 5 ] = vert.y;
	colorGridCursor ++;


	// }
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
	// cleanupAfterWorldDraw();

	SDL_GL_SwapWindow( window );
	float zoomDifference = (viewZoom - viewZoomSetpoint);
	float zoomResponse = (zoomDifference * -1) / cameraTrackingResponse;
	viewZoom = viewZoom + zoomResponse;





	// // if (firstPerson)
	// // {
	// 	viewPanSetpointX = (playerPosition) % sizeX;
	// 	viewPanSetpointY = (playerPosition) / sizeX;
	// }

	float panDifferenceX = (viewPanX - viewPanSetpointX);
	float panResponseX = (panDifferenceX * -1) / cameraTrackingResponse;
	float panDifferenceY = (viewPanY - viewPanSetpointY);
	float panResponseY = (panDifferenceY * -1) / cameraTrackingResponse;
	viewPanX +=  panResponseX ;
	viewPanY += panResponseY ;

}


const Color color_lightblue          = Color( 0.1f, 0.3f, 0.65f, 1.0f );




void addExamplePanelToBuffer()
{

	// unsigned int x = 0;
	// unsigned int y = 0;
	// for (
	// unsigned int i = 0;
	// ; i < (bufferSize / 6) - 6; ++i)
	// {
	// 	x = i % width;
	// 	y = i / width;
	// 	float fx = x;
	// 	float fy = y;


// 		energyColorGrid[ (i * 6) + 0 ] = 1.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 1 ] = 0.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 2 ] = 0.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 3 ] = 1.0f;
// 		energyColorGrid[ (i * 6) + 4 ] = 0.0f;
// 		energyColorGrid[ (i * 6) + 5 ] = 0.0f;

// i++;
// 		energyColorGrid[ (i * 6) + 0 ] = 1.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 1 ] = 0.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 2 ] = 0.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 3 ] = 1.0f;
// 		energyColorGrid[ (i * 6) + 4 ] = 100.0f;
// 		energyColorGrid[ (i * 6) + 5 ] = 100.0f;

// i++;

// 		energyColorGrid[ (i * 6) + 0 ] = 1.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 1 ] = 0.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 2 ] = 0.0f;//lifeGrid[i].energy / maximumDisplayEnergy;
// 		energyColorGrid[ (i * 6) + 3 ] = 1.0f;
// 		energyColorGrid[ (i * 6) + 4 ] = 0.0f;
// 		energyColorGrid[ (i * 6) + 5 ] = 100.0f;


	vertToBuffer(color_lightblue, Vec_f2(0.0f, 0.0f));
	vertToBuffer(color_lightblue, Vec_f2(100.0f, 100.0f));
	vertToBuffer(color_lightblue, Vec_f2(0.0f, 100.0f));


	// }


}


void threadGraphics()
{

	preDraw();


	prepareForWorldDraw ();
	addExamplePanelToBuffer();
	glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, energyColorGrid);
	glDrawArrays(GL_TRIANGLES, 0,  colorGridCursor);

	unsigned int next = colorGridCursor;

	prepareForMenuDraw();
	addExamplePanelToBuffer();
	drawPanels();
	glBufferSubData(GL_ARRAY_BUFFER, next, (colorGridCursor - next), energyColorGrid);
	glDrawArrays   (GL_TRIANGLES,    next,  (colorGridCursor - next));


	drawAllMenuText ();

	postDraw();

}

