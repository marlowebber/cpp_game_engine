#include "graphics.h"
#include "menus.h"
#include "main.h"

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

void initializeGame ()
{
	setupExtremelyFastNumberGenerators();
	srand((unsigned int)time(NULL));
	setupMenus();
}

void threadGame()
{

}
