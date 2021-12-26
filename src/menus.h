#ifndef MENUS_H
#define MENUS_H

enum menuDirection { LEFT, RIGHT, ABOVE, BELOW };

typedef enum t_attrib_id
{
    attrib_position,
    attrib_color
} t_attrib_id;

struct menuItem 
{
    std::string text;
    b2Color textColor;
    b2Color panelColor;
    float x;
    float y;
    int size;

    bool scaffold; // this is a menu that deploys along with its parent. it is used as a handle if you want a vertical list to one side of another menuitem (instead of extending away expandSubMenus(menu); in a line).
    bool collapsed;
    bool clicked;
    int visualDelayCount;
    float alpha;

    b2AABB aabb;

    std::list<menuItem> subMenus;

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


void cleanupText2D();

void setupMenus();

void drawMenus ();
int checkMenus (int mouseX, int mouseY);
void resetMenus () ;

extern std::list<menuItem> menus;

menuItem * setupMenu ( std::string menuName , menuDirection direction, menuItem * parentMenu, void * callback, void * userData, b2Color color, b2Vec2 position);


b2Vec2 transformScreenPositionToWorld( b2Vec2 screen );

b2Vec2 transformWorldPositionToScreen( b2Vec2 world );

void drawTestCoordinate (float x, float y) ;

#endif // menus_h