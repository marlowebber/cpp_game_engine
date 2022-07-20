#include "graphics.h"
// #include "menus.h"
#include "untitled_marlo_project.h"


// #include "graphics.h"
// #include "menus.h"
#include "utilities.h"

// #include <stdio.h>

#include "untitled_marlo_project.h"



#include <SDL.h> // The SDL library
#include <math.h> // sin, fmod

#include <stdio.h> // printf


#include <iostream>
#include <fstream>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif


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



float vertexKerning = 0.5f;
float uvKerning = 0.3f;

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath)
{
    unsigned char header[124];

    FILE *fp;

    /* try to open the file */
    fp = fopen(imagepath, "rb");
    if (fp == NULL) {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
        return 0;
    }

    /* verify the type of file */
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }

    /* get the surface desc */
    fread(&header, 124, 1, fp);

    unsigned int height      = *(unsigned int*) & (header[8 ]);
    unsigned int width       = *(unsigned int*) & (header[12]);
    unsigned int linearSize  = *(unsigned int*) & (header[16]);
    unsigned int mipMapCount = *(unsigned int*) & (header[24]);
    unsigned int fourCC      = *(unsigned int*) & (header[80]);

    unsigned char * buffer;
    unsigned int bufsize;
    /* how big is it going to be including all mipmaps? */
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    /* close the file pointer */
    fclose(fp);

    unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    switch (fourCC)
    {
    case FOURCC_DXT1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        free(buffer);
        return 0;
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;

    /* load the mipmaps */
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
    {
        unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
                               0, size, buffer + offset);

        offset += size;
        width  /= 2;
        height /= 2;

        // Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    free(buffer);

    return textureID;
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    } else {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}



Vec_f2 transformScreenPositionToWorld( Vec_f2 screen )
{
    float worldX =           ( ((screen.x - (width  / 2))) * (viewZoom  ) / (width  * 0.5f) ) + viewPanX ;
    float worldY =     -1 *  ( ((screen.y - (height / 2))) * (viewZoom  ) / (height * 0.9f) ) + viewPanY;
    return Vec_f2  (worldX, worldY);
}


unsigned int Text2DTextureID;
unsigned int Text2DVertexBufferID;
unsigned int Text2DUVBufferID;
unsigned int Text2DShaderID;
unsigned int Text2DUniformID;

void cleanupText2D()
{
    // Delete buffers
    glDeleteBuffers(1, &Text2DVertexBufferID);
    glDeleteBuffers(1, &Text2DUVBufferID);

    // Delete texture
    glDeleteTextures(1, &Text2DTextureID);

    // Delete shader
    glDeleteProgram(Text2DShaderID);
}

void initText2D()
{
    const char * texturePath = "fonts/ubuntuMonoR.DDS";

    // Initialize textureF
    Text2DTextureID = loadDDS(texturePath);

    // Initialize VBO
    glGenBuffers(1, &Text2DVertexBufferID);
    glGenBuffers(1, &Text2DUVBufferID);

    // Initialize Shader
    Text2DShaderID = LoadShaders( "fonts/TextVertexShader.vertexshader", "fonts/TextVertexShader.fragmentshader" );

    // Initialize uniforms' IDs
    Text2DUniformID = glGetUniformLocation( Text2DShaderID, "myTextureSampler" );
}




void printText2D(std::string m_text, int x, int y, int size)
{



    const char * text = m_text.c_str();

    unsigned int length = strlen(text);
    // // Fill buffers
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;

    for ( unsigned int i = 0 ; i < length ; i++ )
    {
        glm::vec2 vertex_up_left    = glm::vec2( x + i * (size * vertexKerning)                             , y + size );
        glm::vec2 vertex_up_right   = glm::vec2( x + i * (size * vertexKerning) + (size * vertexKerning)    , y + size );
        glm::vec2 vertex_down_right = glm::vec2( x + i * (size * vertexKerning) + (size * vertexKerning)    , y      );
        glm::vec2 vertex_down_left  = glm::vec2( x + i * (size * vertexKerning)                             , y      );

        vertices.push_back(vertex_up_left   );
        vertices.push_back(vertex_down_left );
        vertices.push_back(vertex_up_right  );

        vertices.push_back(vertex_down_right);
        vertices.push_back(vertex_up_right);
        vertices.push_back(vertex_down_left);

        char character = text[i];
        float uv_x = (character % 16) / 16.0f;
        float uv_y = (character / 16) / 16.0f;

        glm::vec2 uv_up_left    = glm::vec2( uv_x +  (1.0f / 16.0f / 2) - (1.0f / 16.0f * uvKerning)      ,  uv_y );
        glm::vec2 uv_up_right   = glm::vec2( uv_x +  (1.0f / 16.0f / 2) + (1.0f / 16.0f * uvKerning)      ,  uv_y );
        glm::vec2 uv_down_right = glm::vec2( uv_x +  (1.0f / 16.0f / 2) + (1.0f / 16.0f * uvKerning)      , (uv_y + 1.0f / 16.0f) );
        glm::vec2 uv_down_left  = glm::vec2( uv_x +  (1.0f / 16.0f / 2) - (1.0f / 16.0f * uvKerning)      , (uv_y + 1.0f / 16.0f) );
        UVs.push_back(uv_up_left   );
        UVs.push_back(uv_down_left );
        UVs.push_back(uv_up_right  );

        UVs.push_back(uv_down_right);
        UVs.push_back(uv_up_right);
        UVs.push_back(uv_down_left);
    }
    glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);

    // Bind shader
    glUseProgram(Text2DShaderID);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(Text2DUniformID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw call
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

    // glDisable(GL_BLEND);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}






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
	c = c / 3.0f ; // because there are 3 color components, normalise the result to 1.

	c      *= a.a;

	return c;
}

Color mutateColor(Color in)
{
	Color out = in;
	out.r += (RNG() - 0.5) * 0.1f;
	out.g += (RNG() - 0.5) * 0.1f;
	out.b += (RNG() - 0.5) * 0.1f;
	return clampColor(out);
}

Color normalizeColor(Color in)
{
	Color out = in;
	float max = 0.0f;
	if (in.r > max) { max = in.r;}
	if (in.g > max) { max = in.g;}
	if (in.b > max) { max = in.b;}
	float scale = 1 / max;
	out.r *= scale;
	out.g *= scale;
	out.b *= scale;
	return out;
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



void drawRectangle( Vec_f2 position , Color finalColor, float panelWidth, float panelHeight)
{

	vertToBuffer ( finalColor, Vec_f2 (  position.x -    panelWidth          , position.y -     panelHeight            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x +    panelWidth          , position.y +     panelHeight            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x -    panelWidth          , position.y +     panelHeight            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x -    panelWidth          , position.y -     panelHeight            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x +    panelWidth          , position.y +     panelHeight            ) );
	vertToBuffer ( finalColor, Vec_f2 (  position.x +    panelWidth          , position.y -     panelHeight            ) );

}



void drawLine(  Vec_f2 a, Vec_f2 b, float thickness, Color finalColor )
{

	Vec_f2 pos = Vec_f2(  (a.x + b.x) * 0.5f ,  (a.y + b.y) * 0.5f  );

	Vec_f2 diff = Vec_f2(b.x - a.x, b.y - a.y);

	float len = (sqrt( (diff.x * diff.x) + ( diff.y * diff.y)  ) * 0.5f);

	float angle = atan2(diff.y, diff.x) +  (0.5f * const_pi);

	Vec_f2 pointA =  Vec_f2 (  pos.x -    thickness          , pos.y -     len           );
	Vec_f2 pointB =  Vec_f2 (  pos.x +    thickness          , pos.y +     len           );
	Vec_f2 pointC =  Vec_f2 (  pos.x -    thickness          , pos.y +     len           );
	Vec_f2 pointD =  Vec_f2 (  pos.x -    thickness          , pos.y -     len           );
	Vec_f2 pointE =  Vec_f2 (  pos.x +    thickness          , pos.y +     len           );
	Vec_f2 pointF =  Vec_f2 (  pos.x +    thickness          , pos.y -     len           );

	float sa = sin(angle);
	float ca = cos(angle);

	pointA = rotatePointPrecomputed( pos, sa, ca, pointA);
	pointB = rotatePointPrecomputed( pos, sa, ca, pointB);
	pointC = rotatePointPrecomputed( pos, sa, ca, pointC);
	pointD = rotatePointPrecomputed( pos, sa, ca, pointD);
	pointE = rotatePointPrecomputed( pos, sa, ca, pointE);
	pointF = rotatePointPrecomputed( pos, sa, ca, pointF);

	vertToBuffer ( finalColor, pointA ); // A
	vertToBuffer ( finalColor, pointB ); // B
	vertToBuffer ( finalColor, pointC ); // C
	vertToBuffer ( finalColor, pointD ); // A
	vertToBuffer ( finalColor, pointE ); // B
	vertToBuffer ( finalColor, pointF ); // C

}


void drawPointerTriangle( Vec_f2 position , Color finalColor, float angle)
{
	// one triangle
	Vec_f2 pointA = Vec_f2 (  position.x -    (tileWidth / 2)          , position.y -     tileWidth            );
	Vec_f2 pointB = Vec_f2 (  position.x +    (tileWidth / 2)          , position.y -     tileWidth            );
	Vec_f2 pointC = Vec_f2 (  position.x                              , position.y +     tileWidth            ) ;
	pointA = rotatePointPrecomputed( position, sin(angle), cos(angle), pointA);
	pointB = rotatePointPrecomputed( position, sin(angle), cos(angle), pointB);
	pointC = rotatePointPrecomputed( position, sin(angle), cos(angle), pointC);
	vertToBuffer ( finalColor, pointA ); // A
	vertToBuffer ( finalColor, pointB ); // B
	vertToBuffer ( finalColor, pointC ); // C
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

void resetColorgridCursor()
{

	colorGridCursor = 0;
}

void preDraw()
{
	// colorGridCursor = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	resetColorgridCursor();
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

	viewPanSetpointX = clamp(viewPanSetpointX, -1.0f * viewPanLimit, viewPanLimit );
	viewPanSetpointY = clamp(viewPanSetpointY, -1.0f * viewPanLimit, viewPanLimit );
}

void addExamplePanelToBuffer(float x , float y)
{

	float s = 10000;

	vertToBuffer(color_lightblue, Vec_f2(-s + x, -s + y));
	vertToBuffer(color_lightblue, Vec_f2( s + x,  s + y));
	vertToBuffer(color_lightblue, Vec_f2(-s + x,  s + y));

	vertToBuffer(color_lightblue, Vec_f2(-s + x, -s + y));
	vertToBuffer(color_lightblue, Vec_f2( s + x,  s + y));
	vertToBuffer(color_lightblue, Vec_f2( s + x, -s + y));
}



void drawPanel(Vec_f2 lowerBound , Vec_f2 upperBound, Color color)
{
	vertToBuffer(color, Vec_f2(lowerBound.x, lowerBound.y));
	vertToBuffer(color, Vec_f2(upperBound.x, upperBound.y));
	vertToBuffer(color, Vec_f2(lowerBound.x, upperBound.y));
	vertToBuffer(color, Vec_f2(lowerBound.x, lowerBound.y));
	vertToBuffer(color, Vec_f2(upperBound.x, upperBound.y));
	vertToBuffer(color, Vec_f2(upperBound.x, lowerBound.y));
}


void mainMenuDraw()
{
	preDraw();

	prepareForMenuDraw();

	drawMainMenuText();

	postDraw();
}


void commitBufferToScreen()
{
	glBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, energyColorGrid);
	glDrawArrays(GL_TRIANGLES, 0,  colorGridCursor);

}

void threadGraphics()
{

	ZoneScoped;


	preDraw();


	// if (!fastCam)
	// {
	prepareForWorldDraw ();


	camera();

	if (getFPSLimit())
	{
		model();
	}

	commitBufferToScreen();

	// unsigned int endOfWorldVertexRegion = colorGridCursor;
	resetColorgridCursor();
	
	prepareForMenuDraw();


	// addExamplePanelToBuffer(0, 0);

	

	// addExamplePanelToBuffer(10000,0);
	// addExamplePanelToBuffer(0, -10000);
	// addExamplePanelToBuffer(0, 10000);
	// // drawPanels();
	// drawInterfacePanel();

	// drawPalette2();

	// glBufferSubData(GL_ARRAY_BUFFER, endOfWorldVertexRegion, (colorGridCursor - endOfWorldVertexRegion), energyColorGrid);
	// glDrawArrays   (GL_TRIANGLES,    endOfWorldVertexRegion,  (colorGridCursor - endOfWorldVertexRegion));

	drawGameInterfaceText();
	// }
	// else
	// {

	// 	prepareForMenuDraw();
	// 	drawFastCamText();
	// }
	// // drawAllMenuText ();

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