#include "physics.h"
#include "graphics.h"
#include "utilities.h"
#include "menus.h"

#include <box2d.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

unsigned int Text2DTextureID;
unsigned int Text2DVertexBufferID;
unsigned int Text2DUVBufferID;
unsigned int Text2DShaderID;
unsigned int Text2DUniformID;

float viewportScaleFactorX = 2.4;
float viewportScaleFactorY = 1.8;

bool flagRebuildMenus = false;
// bool capturingNumber = false;
bool capturingText = false;
std::string capturedString;
b2Vec2 captureTextPosition;
uDataWrap * editItem;
bool capitalizing = false;

menuItem * draggedMenu = nullptr;

unsigned int menuTextSize = 10;
float vertexKerning = 0.5f;
float uvKerning = 0.3f;

menuItem::menuItem( )
{
    scaffold = true;
    collapsed = true;
    clicked = false;
    panelColor = b2Color(0.5f, 0.5f, 0.5f);
    textColor = b2Color(0.0f, 0.0f, 0.0f);
    x = 0.0f;
    y = 0.0f;
    alpha = 0.8f;
    size = 50;
    aabb.lowerBound = b2Vec2(0.0f, 0.0f);
    aabb.upperBound = b2Vec2(0.0f, 0.0f);
    onClick = nullptr;
    printValue = true;
    flagDelete = false;
    collapsible = true;
    editable = true;
}

menuItem * lastActiveMenu;
std::list<menuItem> menus;

b2Vec2 transformScreenPositionToWorld( b2Vec2 screen )
{
    float worldX =       ( ((screen.x - (width / 2))) * (viewZoom  ) / 96) + viewPanX ;
    float worldY =     -1 *  ( ((screen.y - (height / 2))) * (viewZoom  ) / 96) + viewPanY;
    return b2Vec2(worldX, worldY);
}

b2Vec2 transformWorldPositionToScreen( b2Vec2 world )
{
    float screenX = (  192 * world.x - 192 * viewPanX + width  * viewZoom ) / (2 * viewZoom);
    float screenY = (  -192 * (world.y * -1) + 192 * viewPanY + height * viewZoom ) / (2 * viewZoom);
    screenX = screenX / viewportScaleFactorX;
    screenY = screenY / viewportScaleFactorY;
    return b2Vec2(screenX, screenY);
}

GLuint loadBMP_custom(const char * imagepath)
{
    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imagepath, "rb");
    if (!file) {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        getchar();
        return 0;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file) != 54 ) {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    // A BMP files always begins with "BM"
    if ( header[0] != 'B' || header[1] != 'M' ) {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }
    // Make sure this is a 24bpp file
    if ( *(int*) & (header[0x1E]) != 0  )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}
    if ( *(int*) & (header[0x1C]) != 24 )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}

    // Read the information about the image
    dataPos    = *(int*) & (header[0x0A]);
    imageSize  = *(int*) & (header[0x22]);
    width      = *(int*) & (header[0x12]);
    height     = *(int*) & (header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0)    imageSize = width * height * 3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

    // Create a buffer
    data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(data, 1, imageSize, file);

    // Everything is in memory now, the file can be closed.
    fclose (file);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete [] data;

    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;
}

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw call
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

    glDisable(GL_BLEND);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

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



menuItem * setupMenu ( std::string menuName , menuDirection direction, menuItem * parentMenu, void * callback, void * userData, b2Color color, b2Vec2 position)
{

    menuItem * newMenu = new menuItem();
    newMenu->text = menuName;

    newMenu->panelColor = color;

    newMenu->left = menuTextSize;
    newMenu->right = (menuTextSize * menuName.length() * vertexKerning) + menuTextSize;
    newMenu->below = menuTextSize;
    newMenu->above = menuTextSize + menuTextSize;


    newMenu->parentMenu = parentMenu;

    newMenu->direction = direction;

    if (parentMenu != nullptr)
    {


        switch (direction)
        {
        case LEFT:
            newMenu->x = parentMenu->x - (parentMenu->left) ;
            newMenu->y = parentMenu->y ;

            parentMenu->left += (menuTextSize * menuName.length() * vertexKerning) + menuTextSize;

            break;

        case RIGHT:
            newMenu->x = parentMenu->x + (parentMenu->right) ;
            newMenu->y = parentMenu->y ;

            parentMenu->right += (menuTextSize * menuName.length() * vertexKerning) + menuTextSize;
            break;

        case ABOVE:
            newMenu->x = parentMenu->x  ;
            newMenu->y = parentMenu->y + (parentMenu->above);

            parentMenu->above += menuTextSize + menuTextSize;
            break;

        case BELOW:
            newMenu->x = parentMenu->x  ;
            newMenu->y = parentMenu->y - (parentMenu->above);

            parentMenu->above += menuTextSize + menuTextSize;
            break;
        }
    }
    else
    {
        newMenu->x = position.x;
        newMenu->y = position.y;
    }

    // draw a rectangle around the menu to calculate aabb
    newMenu->aabb.lowerBound = b2Vec2(  newMenu->x - (0.5 * menuTextSize) ,  newMenu->y - (0.5 * menuTextSize)  );
    newMenu->aabb.upperBound = b2Vec2( newMenu->x  + (newMenu->text.length() * menuTextSize * vertexKerning) + (0.5 * menuTextSize) ,    newMenu->y + menuTextSize + (0.5 * menuTextSize));

    newMenu->size = menuTextSize;
    newMenu->collapsed = true;
    newMenu->clicked = false;

    newMenu->onClick = callback;
    newMenu->userData = userData;

    if (parentMenu == nullptr)
    {
        return newMenu;
    }
    else
    {
        parentMenu->subMenus.push_back( *newMenu );
        return &(parentMenu->subMenus.back());
    }
}

void expandMenu (menuItem * menu)
{



    menu->collapsed = false;
    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        subMenu->collapsed = false;
        if (subMenu->scaffold)
        {
            std::list<menuItem>::iterator subSubMenu;
            for (subSubMenu = subMenu->subMenus.begin(); subSubMenu !=  subMenu->subMenus.end(); ++subSubMenu)
            {
                subSubMenu->collapsed = false;
            }
        }
    }
}







int expandMenuRecursive (menuItem * targetMenu , menuItem * menu);

int expandMenuRecursive (menuItem * targetMenu , menuItem * menu)
{

    // bool foundIt = false;




    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {


        if (!(menu->collapsed) && subMenu->scaffold)
        {
            subMenu->collapsed = false;
        }


        // else {
        if  (expandMenuRecursive (targetMenu, &(*subMenu) ) )
        {
            // subMenu->collapsed = false;
            // menu->collapsed = false;
            // break;

            std::list<menuItem>::iterator subMenu2;
            for (subMenu2 = menu->subMenus.begin(); subMenu2 !=  menu->subMenus.end(); ++subMenu2)
            {

                subMenu2->collapsed = false;
            }



            return 1;
        }
        // }



    }

    if ( menu == targetMenu )
    {
        // foundIt = true;
        // subMenu->collapsed = false;
        menu->collapsed = false;
        // break;
        return 1;
    }




    // if (foundIt)
    // {
    //     menu->collapsed = false;
    //     return 1;
    // }



    return 0;
}

void searchAndExpandMenuChain(menuItem * targetMenu)
{
    std::list<menuItem>::iterator menu;
    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {

        menu->collapsed = false; // top level menus always expand
        expandMenuRecursive(targetMenu,  &(*menu)  );
    }
}


void collapseMenu (menuItem * menu)
{
    if (menu->collapsible)
    {
        menu->collapsed = true;
    }

    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        collapseMenu(&(*subMenu));
    }
}



void collapseAllMenus ()
{
    std::list<menuItem>::iterator menu;
    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {
        collapseMenu( &(*menu)  );
    }

    // expand root level menus and their scaffolds again.
    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {
        expandMenu( &(*menu) );
    }
}

void drawMenuText (menuItem * menu) ;

void drawMenuText (menuItem * menu)
{
    if (!menu->collapsed)
    {
        printText2D( menu->text, menu->x, menu->y, menu->size);

        if (menu->userData != nullptr) {

            if (menu->userData != editItem || !capturingText)
            {

                uDataWrap * tempDataWrap = (uDataWrap *) menu->userData;

                if (tempDataWrap->dataType == TYPE_UDATA_INT )
                {
                    printText2D( std::to_string( *((int *)tempDataWrap->uData) ) , menu->x + menu->right, menu->y, menu->size);
                }
                else  if (tempDataWrap->dataType == TYPE_UDATA_UINT )
                {
                    printText2D( std::to_string( *((unsigned int *)tempDataWrap->uData)  ) , menu->x + menu->right, menu->y, menu->size);
                }
                else  if (tempDataWrap->dataType == TYPE_UDATA_BOOL )
                {
                    printText2D( std::to_string( *((bool *)tempDataWrap->uData)  ) , menu->x + menu->right, menu->y, menu->size);
                }
                else  if (tempDataWrap->dataType == TYPE_UDATA_FLOAT )
                {
                    printText2D( std::to_string( *((float *)tempDataWrap->uData)  ) , menu->x + menu->right, menu->y, menu->size);
                }
                else if (tempDataWrap->dataType == TYPE_UDATA_STRING )
                {
                    printText2D( *((std::string *)tempDataWrap->uData) , menu->x + menu->right, menu->y, menu->size);
                }
            }
        }
    }
    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        drawMenuText(&(*subMenu));
    }
}

void prepareForMenuDraw()
{
    // glUseProgram( program );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    glEnableVertexAttribArray( attrib_position );
    glEnableVertexAttribArray( attrib_color );

    glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
    glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

    t_mat4x4 menu_matrix;

    // mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar )
    mat4x4_ortho(
        menu_matrix,
        0,
        width / viewportScaleFactorX,
        0,
        height / viewportScaleFactorY,
        -10.0f,
        +10.0f
    );
    glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, menu_matrix );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void cleanupAfterMenuDraw ()
{
    glDisable(GL_BLEND);

    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_color);
}

void drawPanel ( menuItem * menu , unsigned int * cursor, GLfloat * vertex_buffer_data, unsigned int * index_buffer_cursor, unsigned int * index_buffer_content, unsigned int * index_buffer_data ) ;

void drawPanel ( menuItem * menu , unsigned int * cursor, GLfloat * vertex_buffer_data, unsigned int * index_buffer_cursor, unsigned int * index_buffer_content, unsigned int * index_buffer_data ) {

    std::list<menuItem>::iterator subMenu;

    if ( menu->clicked)
    {
        if (menu->visualDelayCount > 3 || menu->visualDelayCount < 0) {
            menu->clicked = false;
            // menu->alpha = 0.0f;
            menu->visualDelayCount = 0;
        }
        menu->visualDelayCount ++;
    }

    if (!menu->collapsed)
    {
        float alpha = 1.0f;

        vertToBuffer ( vertex_buffer_data, cursor, menu->panelColor, alpha,  b2Vec2(  menu->aabb.lowerBound.x ,   menu->aabb.upperBound.y ) ) ;
        advanceIndexBuffers (index_buffer_data, index_buffer_content, index_buffer_cursor) ;

        vertToBuffer ( vertex_buffer_data, cursor, menu->panelColor, alpha, menu->aabb.lowerBound ) ;
        advanceIndexBuffers (index_buffer_data, index_buffer_content, index_buffer_cursor) ;

        vertToBuffer ( vertex_buffer_data, cursor, menu->panelColor, alpha, b2Vec2 ( menu->aabb.upperBound.x , menu->aabb.lowerBound.y  )  ) ;
        advanceIndexBuffers (index_buffer_data, index_buffer_content, index_buffer_cursor) ;

        vertToBuffer ( vertex_buffer_data, cursor, menu->panelColor, alpha, menu->aabb.upperBound ) ;
        advanceIndexBuffers (index_buffer_data, index_buffer_content, index_buffer_cursor) ;

        index_buffer_data[(*index_buffer_cursor)] = 0xffff;
        (*index_buffer_cursor)++;
    }


    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        drawPanel( &(*subMenu ) , cursor, vertex_buffer_data, index_buffer_cursor, index_buffer_content, index_buffer_data );
    }
}

void drawPanels(unsigned int * j, GLfloat * vertex_buffer_data, unsigned int * index_buffer_cursor, unsigned int * index_buffer_content, unsigned int * index_buffer_data) {
    std::list<menuItem>::iterator menu;
    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {
        drawPanel( &(*menu ) , j, vertex_buffer_data, index_buffer_cursor, index_buffer_content, index_buffer_data );
    }
}


struct polyCounter {
    unsigned int verts;
    unsigned int indices;
};


polyCounter analyzeMenu ( menuItem * menu) ;

polyCounter analyzeMenu ( menuItem * menu) {
    polyCounter n;
    n.verts = 0;
    n.indices = 0;

    if (!menu->collapsed) {
        // if (  menu->clicked) {
        n.verts += 4;
        n.indices += 5;

        // }
    }

    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        polyCounter m = analyzeMenu(&(*subMenu));
        n.verts += m.verts;
        n.indices += m.indices;
    }
    return n;
}

polyCounter analyzeMenus() {
    polyCounter n;
    n.verts = 0;
    n.indices = 0;
    std::list<menuItem>::iterator menu;
    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {
        polyCounter m = analyzeMenu(&(*menu));
        n.verts += m.verts;
        n.indices += m.indices;
    }
    return n;
}



void drawMenus ()
{
    std::list<menuItem>::iterator menu;


    polyCounter rocks = analyzeMenus();

    unsigned int nVertsToRenderThisTurn = rocks.verts;
    unsigned int nIndicesToUseThisTurn = rocks.indices;
    unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

    // Create the buffer.
    unsigned int g_vertex_buffer_cursor = 0;
    GLfloat vertex_buffer_data[totalNumberOfFields];

    unsigned int index_buffer_cursor = 0;
    unsigned int index_buffer_content = 0;
    unsigned int index_buffer_data[nIndicesToUseThisTurn];

    prepareForMenuDraw();

    drawPanels(&g_vertex_buffer_cursor, vertex_buffer_data, &index_buffer_cursor, &index_buffer_content, index_buffer_data);

    glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);
    glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );

    cleanupAfterMenuDraw();

    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {
        drawMenuText(&(*menu));
    }
}


void resetMenus ()
{

    // go through menus and DELETE all the udatawraps.
    std::list<menuItem>::iterator menu;

    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {
        if (menu->userData != nullptr)
        {
            delete ( (uDataWrap*)(menu->userData));
        }
    }

    menus.clear();

    rebuildMenus();
}


void editUserDataCallback ()
{
    if (editItem->dataType == TYPE_UDATA_STRING)
    {
        *(std::string *)editItem->uData = capturedString;
    }

    else if (editItem->dataType == TYPE_UDATA_INT)
    {
        if (capturedString.length() > 0)
        {
            *(int *)editItem->uData = std::stoi( capturedString );
        }
        else {

            *(int *)editItem->uData = 0;
        }
    }

    else if (editItem->dataType == TYPE_UDATA_UINT)
    {
        if (capturedString.length() > 0)
        {
            int result = std::stoi( capturedString );

            if (result > 0)
            {
                unsigned int applicable = result;
                *(unsigned int *)editItem->uData = applicable;
            }
            else
            {
                *(unsigned int *)editItem->uData = 0;
            }
        }
        else
        {

            *(unsigned int *)editItem->uData = 0;
        }

    }
    else if (editItem->dataType == TYPE_UDATA_FLOAT)
    {
        float * jemima = (float *)  editItem->uData ;
        *jemima = std::stof( capturedString );
    }

    capturingText = false;
    // resetMenus();
}

void editUserData (uDataWrap * itemToEdit)
{
    editItem = itemToEdit;
    capturingText  = true;
    if (itemToEdit->dataType == TYPE_UDATA_STRING)
    {

        capturedString = *(std::string *)(itemToEdit->uData);
    }
    else  if (itemToEdit->dataType == TYPE_UDATA_INT)
    {

        capturedString =  std::to_string(  *(int *)(itemToEdit->uData) );
    }
    else  if (itemToEdit->dataType == TYPE_UDATA_UINT)
    {

        capturedString =  std::to_string(  *(unsigned int *)(itemToEdit->uData) );
    }
    else  if (itemToEdit->dataType == TYPE_UDATA_FLOAT)
    {

        capturedString =  std::to_string(  *(float *)(itemToEdit->uData) );
    }
    else  if (itemToEdit->dataType == TYPE_UDATA_BOOL)
    {

        capturedString =  std::to_string(  *(bool *)(itemToEdit->uData) );
    }
}

void drawCaptureText ()
{
    if (capturingText)
    {
        printText2D( capturedString, lastActiveMenu->x + lastActiveMenu->right, lastActiveMenu->y, 10);
    }
}

int checkMenu (menuItem * menu, float mouseX, float mouseY) ;

// check to see which menu has been clicked on
int checkMenu (menuItem * menu, float mouseX, float mouseY)
{
    if (!menu->collapsed)
    {
        if (mouseX > menu->aabb.lowerBound.x && mouseX < menu->aabb.upperBound.x)
        {
            if (mouseY > menu->aabb.lowerBound.y && mouseY < menu->aabb.upperBound.y)
            {
                menu->clicked = true;

                collapseAllMenus();
                // expandMenu(menu);

                searchAndExpandMenuChain(menu);

                lastActiveMenu = &(*menu);
                uDataWrap * menuUserData = (uDataWrap*)(menu->userData);

                if (menu->onClick != nullptr)
                {
                    if (menu->editable) // why do you need editable to run the function? editUserData function is also used to display values. but sometimes you want to disable editing.
                    {
                        ((void (*)(void*)) menu->onClick)(menu->userData);
                    }
                }




                return 1;
            }
        }
    }
    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        if ( checkMenu(&(*subMenu), mouseX, mouseY)  == 1)
        {
            menu->collapsed = false;
            subMenu->collapsed = false;
            return 1;
        }
    }
    return 0;
}



void setDraggingMenu ( menuItem * menu )
{

    draggedMenu = menu;
}


void clearDraggingMenu()
{


    draggedMenu = nullptr;


}


void resetAccumulatedSubmenuPositions(menuItem * menu)
{


    menu->left = 0;
    menu->right = 0;
    menu->below = 0;
    menu->above = 0;

    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {


        resetAccumulatedSubmenuPositions( &(*subMenu) );
    }

}


// move a menu and all its submenus on the screen
void rebaseMenu (menuItem * menu, int moveByX, int moveByY)
{




    menu->x += moveByX;
    menu->y += moveByY;


    menu->aabb.upperBound.x += moveByX;
    menu->aabb.upperBound.y += moveByY;
    menu->aabb.lowerBound.x += moveByX;
    menu->aabb.lowerBound.y += moveByY;




    std::list<menuItem>::iterator subMenu;
    for (subMenu = menu->subMenus.begin(); subMenu !=  menu->subMenus.end(); ++subMenu)
    {
        rebaseMenu( &(*subMenu),  moveByX, moveByY);
    }


}




// check to see which menu has been clicked on
int checkMenus (int mouseX, int mouseY)
{

    // if (capturingText)
    // {
    //     editUserDataCallback () ;
    // }



    mouseX = (mouseX / viewportScaleFactorX);
    mouseY = (((mouseY) * -1) / viewportScaleFactorY) + (1080 / viewportScaleFactorY);

    std::list<menuItem>::iterator menu;
    for (menu = menus.begin(); menu !=  menus.end(); ++menu)
    {

        if (!menu->collapsed)
        {
            if (mouseX > menu->aabb.lowerBound.x && mouseX < menu->aabb.upperBound.x)
            {
                if (mouseY > menu->aabb.lowerBound.y && mouseY < menu->aabb.upperBound.y)
                {

                    setDraggingMenu( &(*menu) );
                }
            }
        }


        if (  checkMenu(&(*menu), mouseX, mouseY) == 1) { return 1;}




    }

    collapseAllMenus();

    return 0;
}

void setupMenus()
{

    resetMenus();
    initText2D();

}
t_mat4x4 egg_matrix;

// this function just draws a mark on the screen in world coordinates.
void drawTestCoordinate (float x, float y)
{

    // printf( " %f %f\n ", x, y);

    unsigned int nVertsToRenderThisTurn = 0;
    unsigned int nIndicesToUseThisTurn = 0;

    nVertsToRenderThisTurn = 4 ;
    nIndicesToUseThisTurn = 5 ;
    unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

    // Create the buffer.
    unsigned int g_vertex_buffer_cursor = 0;
    GLfloat vertex_buffer_data[totalNumberOfFields];

    unsigned int index_buffer_cursor = 0;
    unsigned int index_buffer_content = 0;
    unsigned int index_buffer_data[nIndicesToUseThisTurn];

    glUseProgram( program );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    glEnableVertexAttribArray( attrib_position );
    glEnableVertexAttribArray( attrib_color );

    glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
    glVertexAttribPointer( attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )(4 * sizeof(float)) );

    unsigned int cursor = 0;

    unsigned int bloopSize = 0.1 * viewZoom;
    b2Color bloopColor = b2Color( 1.0f, 1.0f, 1.0f);

    vertex_buffer_data[(cursor) + 0] = bloopColor.r;
    vertex_buffer_data[(cursor) + 1] = bloopColor.g;
    vertex_buffer_data[(cursor) + 2] = bloopColor.b;
    vertex_buffer_data[(cursor) + 3] = 1.0f;
    vertex_buffer_data[(cursor) + 4] = x - bloopSize;
    vertex_buffer_data[(cursor) + 5] = y + bloopSize ;
    cursor += 6;

    index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
    (index_buffer_cursor)++;
    (index_buffer_content)++;

    vertex_buffer_data[(cursor) + 0] = bloopColor.r;
    vertex_buffer_data[(cursor) + 1] = bloopColor.g;
    vertex_buffer_data[(cursor) + 2] = bloopColor.b;
    vertex_buffer_data[(cursor) + 3] = 1.0f;
    vertex_buffer_data[(cursor) + 4] = x - bloopSize;
    vertex_buffer_data[(cursor) + 5] = y - bloopSize ;
    cursor += 6;

    index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
    (index_buffer_cursor)++;
    (index_buffer_content)++;

    vertex_buffer_data[(cursor) + 0] = bloopColor.r;
    vertex_buffer_data[(cursor) + 1] = bloopColor.g;
    vertex_buffer_data[(cursor) + 2] = bloopColor.b;
    vertex_buffer_data[(cursor) + 3] = 0.5f;
    vertex_buffer_data[(cursor) + 4] = x + bloopSize;
    vertex_buffer_data[(cursor) + 5] = y - bloopSize;
    cursor += 6;

    index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
    (index_buffer_cursor)++;
    (index_buffer_content)++;

    vertex_buffer_data[(cursor) + 0] = bloopColor.r;
    vertex_buffer_data[(cursor) + 1] = bloopColor.g;
    vertex_buffer_data[(cursor) + 2] = bloopColor.b;
    vertex_buffer_data[(cursor) + 3] = 0.5f;
    vertex_buffer_data[(cursor) + 4] = x + bloopSize;
    vertex_buffer_data[(cursor) + 5] = y + bloopSize;

    index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
    (index_buffer_cursor)++;
    (index_buffer_content)++;

    index_buffer_data[(index_buffer_cursor)] = 0xffff;
    (index_buffer_cursor)++;

    glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);

    mat4x4_ortho(
        egg_matrix,
        -10   * viewZoom + viewPanX,
        +10   * viewZoom + viewPanX,
        +5.625 * viewZoom + viewPanY,
        -5.625 * viewZoom + viewPanY,
        -10.0f,
        +10.0f
    );

    glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, egg_matrix );

    glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );

    glDisableVertexAttribArray(attrib_position);
    glDisableVertexAttribArray(attrib_color);



}
