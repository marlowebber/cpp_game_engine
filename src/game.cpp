#include "game.h"
#include "graphics.h"
#include "menus.h"
#include "main.h"

#include <ctime>
#include <chrono>
#include <iostream>

#include <box2d.h>


b2World * m_world = nullptr;

b2MouseJoint* m_mouseJoint;
b2Body * mouseDraggingBody;
b2BodyDef groundBodyDef;
b2Body * m_groundBody;



// -----------------------------------------------------------------------------------

// add in the variables and constants for your code here.

int exampleNumberCapture = 15;
std::string exampleTextCapture = std::string("exampleText");








// -----------------------------------------------------------------------------------



class MyDestructionListener : public b2DestructionListener
{
	void SayGoodbye(b2Joint* joint)
	{
		// remove all references to joint.
		if (m_mouseJoint == joint)
		{
			m_mouseJoint = NULL;
		}
	}
};

b2DestructionListener * myDestructionListener;

void setupForMouseJoint()
{
	m_groundBody = m_world->CreateBody(&groundBodyDef);
	m_world->SetDestructionListener(myDestructionListener);
}

void destroyMouseJoint ()
{
	if (m_mouseJoint)
	{
		printf("destroyMouseJoint\n");
		m_world->DestroyJoint(m_mouseJoint);
		m_mouseJoint = nullptr;
		mouseDraggingBody = nullptr;
	}
}

void maintainMouseJoint (b2Vec2 p)
{
	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
		m_groundBody->SetTransform(p, 0.0f);
	}
}

bool getMouseJointStatus ()
{
	if (m_mouseJoint)
	{
		return true;
	}
	return false;
}

void initMouseJointWithBody (b2Vec2 p, b2Body * body)
{
	if (m_mouseJoint != NULL)
	{
		return;
	}

	b2MouseJointDef md;
	md.bodyA = m_groundBody;
	md.bodyB = body;
	md.target = p;
	md.maxForce = 10000.0f * body->GetMass();
	m_mouseJoint = (b2MouseJoint*)m_world->CreateJoint(&md);
	body->SetAwake(true);

	m_mouseJoint->SetStiffness(10000.00f);
	m_mouseJoint->SetDamping(100.00f);

	mouseDraggingBody = body;

	printf("mouse joint\n");

	return;
}


// this class pins together the parts you need for a box2d physical-world object.
// if you make your own classes that represent physical objects, you should either have them inherit from this, or have a copy of one of these as a member.
class PhysicalObject
{
public:
	b2BodyDef bodyDef;
	b2Body * p_body;
	b2PolygonShape shape;
	b2Fixture * p_fixture;
	float fraction;
	bool flagDelete;
	bool flagReady;
	b2Color color;

	std::vector<b2Vec2>  vertices;

	PhysicalObject (std::vector<b2Vec2>   vertices, bool flagStatic)
	{
		this->flagReady = false;
		this->flagDelete = false;
		this->fraction = 0;

		// this->jointDef =  b2RevoluteJointDef();
		this->bodyDef = b2BodyDef();
		this->bodyDef.userData = b2BodyUserData();

		this->vertices = vertices;

		if (flagStatic)
		{
			this->bodyDef.type = b2_staticBody;
		}
		else
		{
			this->bodyDef.type = b2_dynamicBody;
		}

		this->shape.Set(this->vertices.data(), this->vertices.size());

		this->color = b2Color (0.1f, 0.1f, 0.1f, 1.0f);
	}
};

std::list<b2Body* > rayContacts;
std::list<PhysicalObject> physicalObjects;

int checkClickObjects (b2Vec2 worldClick)
{


	printf("testing world position %f %f\n", worldClick.x, worldClick.y);
	std::list<PhysicalObject>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{

		if (object->p_fixture->TestPoint( worldClick) )
		{

			initMouseJointWithBody (worldClick, object->p_body);
			return 1;
		}

	}

	return 0;
}

void collisionHandler (b2Contact * contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
}

class MyListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact)
	{
		const b2Manifold* manifold = contact->GetManifold();

		if (manifold->pointCount == 0)
		{
			return;
		}

		collisionHandler (contact) ;
	}

	void EndContact(b2Contact* contact)
	{
		;
	}
};

MyListener listener;

class RayCastClosestCallback : public b2RayCastCallback
{
public:
	RayCastClosestCallback()
	{
		m_hit = false;
	}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{

		m_hit = true;
		m_point = point;
		m_normal = normal;

		b2Body* body = fixture->GetBody();

		rayContacts.push_back(body);

		return fraction;
	}

	bool m_hit;
	b2Vec2 m_point;
	b2Vec2 m_normal;
};

void shine (b2Vec2 p1, b2Vec2 p2)
{
	b2RayCastInput sunbeam;

	sunbeam.maxFraction = 1.0f;

	rayContacts.clear();

	sunbeam.p1 = p1;
	sunbeam.p2 = p2;

	RayCastClosestCallback jeremiahSnailMan;
	m_world->RayCast( &jeremiahSnailMan, sunbeam.p1, sunbeam.p2);

	// go through the list of contacts and only flag the closest one.
	std::list<b2Body*>::iterator contact;

	if (rayContacts.size() > 0)
	{
		unsigned int contactIndex = 0;
		for (contact = rayContacts.begin(); contact !=  rayContacts.end(); ++contact)
		{
			// do stuff to them
			contactIndex++;
		}
	}
}

void addToWorld(PhysicalObject object, b2Vec2 position, float angle)
{

	physicalObjects.push_back( object  );
	PhysicalObject * pushedObject = &(physicalObjects.back());
	pushedObject->p_body = m_world->CreateBody( &(pushedObject->bodyDef) );

	pushedObject->p_fixture = pushedObject->p_body->CreateFixture(&(pushedObject->shape), 1.2f);	// this endows the shape with mass and is what adds it to the physical world.

	pushedObject->p_body ->SetTransform(position, angle);

	pushedObject->flagReady = true;
}

void deleteFromWorld (PhysicalObject * object)
{
	// m_world->DestroyJoint(object->p_joint);
	object->p_body->DestroyFixture(object->p_fixture);
	m_world->DestroyBody(object->p_body); 	// this action is for real, you can't grow it back.
}

void exampleMenuCallback(void * userData)
{
	printf("a menu was clicked\n");
}

void initializeGame ()
{
	// The first parts are necessary to set up the game world. Don't change them unless you know what you're doing.

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	setupExtremelyFastNumberGenerators();
	srand((unsigned int)time(NULL));
	b2Vec2 gravity = b2Vec2(0.0f, -10.0f);
	m_world = new b2World(gravity);
	m_world->SetContactListener(&listener);
	setupForMouseJoint();

	// -----------------------------------------------------------------------------------
	// Add your initialization code in below:


	float exampleBoxSize = 10.0f;
	std::vector<b2Vec2> exampleBoxVertices =
	{
		b2Vec2( +1 * exampleBoxSize ,  -1 * exampleBoxSize), //b2Vec2 rootVertexA =
		b2Vec2( -1 * exampleBoxSize ,  -1 * exampleBoxSize), // b2Vec2 rootVertexB =
		b2Vec2( -1 * exampleBoxSize ,  +1 * exampleBoxSize), //b2Vec2 tipVertexA =
		b2Vec2( +1 * exampleBoxSize ,  +1 * exampleBoxSize) // b2Vec2 tipVertexB =
	};
	addToWorld( PhysicalObject(exampleBoxVertices, false), b2Vec2(0.0f, 20.0f) , 0.0f);

	std::vector<b2Vec2> exampleBox2Vertices =
	{
		b2Vec2( +10 * exampleBoxSize ,  -1 * exampleBoxSize), //b2Vec2 rootVertexA =
		b2Vec2( -10 * exampleBoxSize ,  -1 * exampleBoxSize), // b2Vec2 rootVertexB =
		b2Vec2( -10 * exampleBoxSize ,  +1 * exampleBoxSize), //b2Vec2 tipVertexA =
		b2Vec2( +10 * exampleBoxSize ,  +1 * exampleBoxSize) // b2Vec2 tipVertexB =
	};
	addToWorld( PhysicalObject(exampleBox2Vertices, true) , b2Vec2(0.0f, -20.0f), 3.0f );




	// -----------------------------------------------------------------------------------
}




void rebuildMenus ()
{
	int spacing = 10;

	// -----------------------------------------------------------------------------------
	// Add your menu code in below:




	menuItem * exampleMenuRoot = setupMenu ( std::string ("test") , RIGHT, nullptr, (void *)exampleMenuCallback, nullptr, b2Color(0.1f, 0.1f, 0.1f, 1.0f), b2Vec2(200, 200));
	exampleMenuRoot->collapsed = false;


	uDataWrap *     tempDataWrap = new uDataWrap( (void*)&exampleNumberCapture, TYPE_UDATA_INT  );
	menuItem * exampleMenuNumber = setupMenu ( std::string ("editable number") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap, b2Color(0.1f, 0.1f, 0.1f, 1.0f), b2Vec2(200, 200));
	exampleMenuNumber->collapsed = false;


	  tempDataWrap = new uDataWrap( (void*)&exampleTextCapture, TYPE_UDATA_STRING  );
	menuItem * exampleMenuText = setupMenu ( std::string ("editable text") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap, b2Color(0.1f, 0.1f, 0.1f, 1.0f), b2Vec2(200, 200));
	exampleMenuText->collapsed = false;

	menus.push_back(*exampleMenuRoot);
	// -----------------------------------------------------------------------------------


}



void threadGame ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	if (!m_world->IsLocked() )
	{


		threadInterface(); // if you do this while the world is unlocked, you can add and remove stuff.

		// -----------------------------------------------------------------------------------
		// Your custom game logic goes here!











		// -----------------------------------------------------------------------------------

		float timeStep = nominalFramerate > 0.0f ? 1.0f / nominalFramerate : float(0.0f);
		m_world->Step(timeStep, 1, 1);

	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGame " << elapsed.count() << " microseconds." << std::endl;
#endif
}


void threadGraphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	preDraw();

	unsigned int nVertsToRenderThisTurn = 0;
	unsigned int nIndicesToUseThisTurn = 0;


	// -----------------------------------------------------------------------------------
	/** your graphics logic here. turn your data into floats and pack it into vertex_buffer_data. The sequence is r, g, b, a, x, y; repeat for each point. **/
	std::list<PhysicalObject>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{
		unsigned int nObjectVerts = object->vertices.size();
		nVertsToRenderThisTurn += nObjectVerts;
		nIndicesToUseThisTurn  += nObjectVerts + 1;
	}

	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;
	unsigned int vertex_buffer_cursor = 0;
	float vertex_buffer_data[totalNumberOfFields];
	unsigned int index_buffer_cursor = 0;
	unsigned int index_buffer_content = 0;
	unsigned int index_buffer_data[nIndicesToUseThisTurn];

	// std::list<PhysicalObject>::iterator object;
	for (object = physicalObjects.begin(); object !=  physicalObjects.end(); ++object)
	{

		b2Vec2 bodyPosition = object->p_body->GetWorldCenter();
		float bodyAngle = object->p_body->GetAngle();
		float bodyAngleSin = sin(bodyAngle);
		float bodyAngleCos = cos(bodyAngle);


		std::vector<b2Vec2>::iterator vert;
		for (vert = std::begin(object->vertices); vert !=  std::end(object->vertices); ++vert)
		{

			// add the position and rotation of the game-world object that the vertex belongs to.
			b2Vec2 rotatedPoint = b2Vec2(   vert->x + bodyPosition.x, vert->y + bodyPosition.y   );
			rotatedPoint = b2RotatePointPrecomputed( bodyPosition, bodyAngleSin, bodyAngleCos, rotatedPoint);

			vertex_buffer_data[(vertex_buffer_cursor) + 0] = object->color.r;
			vertex_buffer_data[(vertex_buffer_cursor) + 1] = object->color.g;
			vertex_buffer_data[(vertex_buffer_cursor) + 2] = object->color.b;
			vertex_buffer_data[(vertex_buffer_cursor) + 3] = object->color.a;
			vertex_buffer_data[(vertex_buffer_cursor) + 4] = rotatedPoint.x;
			vertex_buffer_data[(vertex_buffer_cursor) + 5] = rotatedPoint.y ;
			(vertex_buffer_cursor) += 6;

			index_buffer_data[(index_buffer_cursor)] = (index_buffer_content);
			(index_buffer_cursor)++;
			(index_buffer_content)++;
		}

		index_buffer_data[(index_buffer_cursor)] = PRIMITIVE_RESTART;
		(index_buffer_cursor)++;
	}


	// -----------------------------------------------------------------------------------
	prepareForWorldDraw();
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);
	glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );
	cleanupAfterWorldDraw();
	drawMenus ();


	b2Vec2 worldMousePos = transformScreenPositionToWorld( b2Vec2(mouseX, mouseY) );
	// drawTestCoordinate(worldMousePos.x, worldMousePos.y);

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGraphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}
