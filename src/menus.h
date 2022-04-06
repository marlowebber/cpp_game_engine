#ifndef MENUS_H
#define MENUS_H

enum menuDirection { LEFT, RIGHT, ABOVE, BELOW };


struct menuItem 
{
    std::string text;
    Color textColor  ;
    Color panelColor ;
    int x;
    int y;
    int size;

    bool scaffold; // this is a menu that deploys along with its parent. it is used as a handle if you want a vertical list to one side of another menuitem (instead of extending away expandSubMenus(menu); in a line).
    bool collapsed;
    bool clicked;
    int visualDelayCount;
    float alpha;

    AABB aabb ;

    std::list<menuItem> subMenus;

    menuItem * parentMenu;

    menuDirection direction;

    // how far the submenus extend in each direction.
    int left;
    int right;
    int above;
    int below;

    void * onClick;
    void * userData;
    bool printValue;
    bool flagDelete ;
    bool collapsible;
    bool editable;

    menuItem(  );
};


extern float viewportScaleFactorX;
extern float viewportScaleFactorY;


void exampleMenuCallback(void * userData);
void cleanupText2D();

void setupMenus();

void drawMenus ();
int checkMenus (int mouseX, int mouseY);
void resetMenus () ;


void rebaseMenu (menuItem * menu, int newX, int newY);

extern std::list<menuItem> menus;


extern bool capturingText;
extern std::string capturedString;


menuItem * setupMenu ( std::string menuName , menuDirection direction, menuItem * parentMenu, void * callback=nullptr, void * userData=nullptr, Color color=Color(0.1f, 0.1f, 0.1f, 1.0f), Vec_f2 position=Vec_f2(0.0f, 0.0f));

void editUserData (uDataWrap * itemToEdit);

void editUserDataCallback ();


void drawCaptureText ();

Vec_f2 transformScreenPositionToWorld( Vec_f2 screen );

Vec_f2 transformWorldPositionToScreen( Vec_f2 world );

void drawTestCoordinate (float x, float y) ;


void setDraggingMenu ( menuItem * menu ) ;
void clearDraggingMenu() ;

extern menuItem * draggedMenu;

#endif // menus_h