#include "physics.h"
#include "graphics.h"

float viewZoom = 10.0f;
float viewPanX = 0.0f;
float viewPanY = 0.0f;


float viewZoomSetpoint = 10.0f;
float viewPanSetpointX = 0.0f;
float viewPanSetpointY = 0.0f;

float cameraTrackingResponse = 10;

SDL_Window * window;
SDL_GLContext context;

GLuint vs, fs, program;
GLuint vao, vbo;
GLuint IndexBufferId;

static const char * vertex_shader =
    "#version 130\n"
    "in vec2 i_position;\n"
    "in vec4 i_color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = i_color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position, 0.0, 1.0 );\n"
    "}\n";

static const char * fragment_shader =
    "#version 130\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

typedef enum t_attrib_id
{
	attrib_position,
	attrib_color
} t_attrib_id;



// The projection matrix efficiently handles all panning, zooming, and rotation.
t_mat4x4 projection_matrix;

void shutdownGraphics()
{
	SDL_GL_DeleteContext( context );
	SDL_DestroyWindow( window );
}

void setupGraphics()
{
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
	glPrimitiveRestartIndex(PRIMITIVE_RESTART);

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
        -1 * 10  * viewZoom + viewPanX,
        +1 * 10  * viewZoom + viewPanX,
        -1 * 5.625 * viewZoom + viewPanY,
        +1 * 5.625 * viewZoom + viewPanY,
        -10.0f,
        +10.0f
    );
	glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, projection_matrix );
}


void cleanupAfterWorldDraw() 
{
    glDisable(GL_BLEND);

    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_color);
}
void preDraw()
{
	// prepareForWorldDraw ();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void postDraw ()
{
	// cleanupAfterWorldDraw();
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

void vertToBuffer (GLfloat * vertex_buffer_data, unsigned int * cursor, b2Color color, float alpha, b2Vec2 vert) {
	vertex_buffer_data[(*cursor) + 0] = color.r;
	vertex_buffer_data[(*cursor) + 1] = color.g;
	vertex_buffer_data[(*cursor) + 2] = color.b;
	vertex_buffer_data[(*cursor) + 3] = alpha;
	vertex_buffer_data[(*cursor) + 4] = vert.x;
	vertex_buffer_data[(*cursor) + 5] = vert.y ;
	(*cursor) += 6;
}

void advanceIndexBuffers (unsigned int * index_buffer_data, unsigned int * index_buffer_content, unsigned int * index_buffer_cursor)
{
    index_buffer_data[(*index_buffer_cursor)] = (*index_buffer_content);
    (*index_buffer_cursor)++;
    (*index_buffer_content)++;
}