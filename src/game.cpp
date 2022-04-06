#include "graphics.h"
#include "menus.h"
#include "main.h"

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");


Color::Color(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

Vec_u2::Vec_u2(unsigned int a, unsigned int b)
{
	this->x = a;
	this->y = b;
}

Vec_i2::Vec_i2( int a,  int b)
{
	this->x = a;
	this->y = b;
}

Vec_f2::Vec_f2( float a,  float b)
{
	this->x = a;
	this->y = b;
}


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

}

void threadGame()
{

}



void gameGraphics()
{
	/** your graphics logic here. turn your data into floats and pack it into vertex_buffer_data. The sequence is r, g, b, a, x, y; repeat for each point. **/

	unsigned int nVertsToRenderThisTurn = 0;
	unsigned int nIndicesToUseThisTurn = 0;

	// std::list<PhysicalObject>::iterator object;
	// for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	// {
	// 	unsigned int nObjectVerts = object->vertices.size();
	// 	nVertsToRenderThisTurn += nObjectVerts;
	// 	nIndicesToUseThisTurn  += nObjectVerts + 1;
	// }

	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
	unsigned int vertex_buffer_cursor = 0;
	float vertex_buffer_data[totalNumberOfFields];
	unsigned int index_buffer_cursor = 0;
	unsigned int index_buffer_content = 0;
	unsigned int index_buffer_data[nIndicesToUseThisTurn];

	// std::list<PhysicalObject>::iterator object;
	// for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	// {

	// 	b2Vec2 bodyPosition = object->p_body->GetWorldCenter();
	// 	float bodyAngle = object->p_body->GetAngle();
	// 	float bodyAngleSin = sin(bodyAngle);
	// 	float bodyAngleCos = cos(bodyAngle);

	// 	std::vector<b2Vec2>::iterator vert;
	// 	for (vert = std::begin(object->vertices); vert !=  std::end(object->vertices); ++vert)
	// 	{
	// 		// add the position and rotation of the game-world object that the vertex belongs to.
	// 		b2Vec2 rotatedPoint = b2Vec2(   vert->x + bodyPosition.x, vert->y + bodyPosition.y   );
	// 		rotatedPoint = b2RotatePointPrecomputed( bodyPosition, bodyAngleSin, bodyAngleCos, rotatedPoint);

	// 		vertex_buffer_data[(vertex_buffer_cursor) + 0] = object->color.r;
	// 		vertex_buffer_data[(vertex_buffer_cursor) + 1] = object->color.g;
	// 		vertex_buffer_data[(vertex_buffer_cursor) + 2] = object->color.b;
	// 		vertex_buffer_data[(vertex_buffer_cursor) + 3] = object->color.a;
	// 		vertex_buffer_data[(vertex_buffer_cursor) + 4] = rotatedPoint.x;
	// 		vertex_buffer_data[(vertex_buffer_cursor) + 5] = rotatedPoint.y ;
	// 		(vertex_buffer_cursor) += 6;

	// 		index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
	// 		(index_buffer_cursor)++;
	// 		(index_buffer_content)++;
	// 	}

	// 	index_buffer_data[(index_buffer_cursor)] = PRIMITIVE_RESTART;
	// 	(index_buffer_cursor)++;
	// }

	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);
	glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );
}

void threadGraphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	preDraw();

	prepareForWorldDraw();

	gameGraphics();


	cleanupAfterWorldDraw();
	drawMenus ();
	drawCaptureText ();


	// b2Vec2 worldMousePos = transformScreenPositionToWorld( b2Vec2(mouseX, mouseY) );

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGraphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}