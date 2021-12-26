#include "game.h"
#include "graphics.h"

#include <ctime>
#include <chrono>
#include <iostream>

#include <box2d.h>

// this class pins together the parts you need for a box2d physical-world object.
// if you make your own classes that represent physical objects, you should either have them inherit from this, or have a copy of one of these as a member.
class PhysicalObject
{
public:
	// b2RevoluteJointDef jointDef;
	// b2RevoluteJoint * p_joint;
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

b2World * m_world = nullptr;
std::list<b2Body* > rayContacts;
std::list<PhysicalObject> physicalObjects;

static const unsigned int nominalFramerate = 60;
const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */

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

void initializeGame ()
{
	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	setupExtremelyFastNumberGenerators();
	srand((unsigned int)time(NULL));

	b2Vec2 gravity = b2Vec2(0.0f, -10.0f);
	m_world = new b2World(gravity);

	m_world->SetContactListener(&listener);

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
}

void threadGame ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	// custom logic goes here!
	if (!m_world->IsLocked() )
	{
		float timeStep = nominalFramerate > 0.0f ? 1.0f / nominalFramerate : float(0.0f);
		m_world->Step(timeStep, 1, 1);
	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGame " << elapsed.count() << " microseconds." << std::endl;
#endif
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

void threadGraphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	preDraw();




	unsigned int nVertsToRenderThisTurn = 0;
	unsigned int nIndicesToUseThisTurn = 0;

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

	/** your graphics logic here. turn your data into floats and pack it into vertex_buffer_data. The sequence is r, g, b, a, x, y; repeat for each point. **/

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


	prepareForWorldDraw();
	
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_buffer_data ), vertex_buffer_data, GL_DYNAMIC_DRAW );
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_DYNAMIC_DRAW);
    glDrawElements( GL_TRIANGLE_FAN, nIndicesToUseThisTurn, GL_UNSIGNED_INT, index_buffer_data );

    cleanupAfterWorldDraw();




	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGraphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}


