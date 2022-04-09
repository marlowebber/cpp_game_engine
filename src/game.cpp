#include "graphics.h"
#include "menus.h"
#include "main.h"
#include "untitled_marlo_project.h"

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");

void rebuildMenus ()
{
	int spacing = 10;

	menuItem * exampleMenuRoot = setupMenu ( std::string ("menu") , RIGHT, nullptr, (void *)exampleMenuCallback, nullptr, Color(0.1f, 0.1f, 0.1f, 1.0f), Vec_f2(200, 200));
	exampleMenuRoot->collapsed = false;

	uDataWrap *     tempDataWrap = new uDataWrap( (void*)&exampleNumberCapture, TYPE_UDATA_INT  );
	menuItem * exampleMenuNumber = setupMenu ( std::string ("editable number") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap);
	exampleMenuNumber->collapsed = false;

	tempDataWrap = new uDataWrap( (void*)&exampleTextCapture, TYPE_UDATA_STRING  );
	menuItem * exampleMenuText = setupMenu ( std::string ("editable text") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap );
	exampleMenuText->collapsed = false;

	menuItem * exampleMenu2 = setupMenu ( std::string ("submenu") , BELOW, exampleMenuRoot);
	exampleMenu2->collapsed = false;

	menuItem * exampleMenu3 = setupMenu ( std::string ("submenu with pin") , BELOW, exampleMenuRoot);
	exampleMenu3->collapsed = false;

	menuItem * exampleMenu4 = setupMenu ( std::string ("") , RIGHT, exampleMenu3);
	exampleMenu4->collapsed = false;
	exampleMenu4->scaffold = true;

	tempDataWrap = new uDataWrap( (void*)&exampleNumberCapture, TYPE_UDATA_INT  );
	menuItem * exampleMenu5 = setupMenu ( std::string ("sub-submenu") , BELOW, exampleMenu4, (void *)editUserData, (void*)tempDataWrap);
	exampleMenu5->collapsed = true;

	tempDataWrap = new uDataWrap( (void*)&exampleTextCapture, TYPE_UDATA_STRING  );
	menuItem * exampleMenu6 = setupMenu ( std::string ("sub-submenu") , BELOW, exampleMenu4, (void *)editUserData, (void*)tempDataWrap);
	exampleMenu6->collapsed = true;

	menus.push_back(*exampleMenuRoot);
}



const unsigned int mapsize = 10;
const float tileHeight =  1.0f;
const float tileWidth = 2.0f;
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
	vertToBuffer(finalColor, Vec_f2( isometricPos.x + -(tileWidth / 2),isometricPos.y +  0.0f));
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


































void initializeGame ()
{
	setupExtremelyFastNumberGenerators();
	srand((unsigned int)time(NULL));
	setupMenus();

	startSimulation();
}

void threadGame()   // the main loop for the game, this get run as its own thread.
{
model();

}

void gameGraphics() // process the game visual elements, this gets run inside the graphics thread.
{

	// drawTile( Vec_f2(0.0f, 0.0f) , color_lightblue);

	camera();


}














