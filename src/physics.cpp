#include "physics.h"
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
 PhysicalObject::
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

void initializePhysics ()
{
	// The first parts are necessary to set up the game world. Don't change them unless you know what you're doing.

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	setupExtremelyFastNumberGenerators();
	srand((unsigned int)time(NULL));
	b2Vec2 gravity = b2Vec2(0.0f, -10.0f);
	m_world = new b2World(gravity);
	m_world->SetContactListener(&listener);
	setupForMouseJoint();



}







void threadPhysics ()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	if (!m_world->IsLocked() )
	{


		threadInterface(); // if you do this while the world is unlocked, you can add and remove stuff.

		threadGame();

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

	prepareForWorldDraw();

	gameGraphics();


	cleanupAfterWorldDraw();
	drawMenus ();
	drawCaptureText ();


	b2Vec2 worldMousePos = transformScreenPositionToWorld( b2Vec2(mouseX, mouseY) );

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadGraphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}
