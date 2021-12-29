#include <ctime>
#include <chrono>
#include <iostream>

#include "physics.h"
#include "graphics.h"
#include "menus.h"

bool paused = false;
bool flagQuit = false;

int mouseX;
int mouseY;
b2Vec2 worldMousePos;

float panSpeed = 0.5f;

void quit ()
{
	shutdownGraphics();
	cleanupText2D();
	SDL_Quit();
	flagQuit = true;
}

void togglePause ()
{
	paused = !paused;
}

void threadInterface()
{

#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif

	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		switch ( event.type )
		{

		case SDL_KEYDOWN:
		{

			if (capturingText)
			{
				if (event.key.keysym.sym > 0x20 && event.key.keysym.sym < 0x7f)
				{
					capturedString += event.key.keysym.sym;
				}

				switch ( event.key.keysym.sym )
				{
				case SDLK_RETURN:
					editUserDataCallback();
					break;
				}
				break;
			}



			switch ( event.key.keysym.sym )
			{

			case SDLK_LEFT:
				viewPanSetpointX = viewPanSetpointX - (panSpeed * viewZoomSetpoint);
				break;
			case SDLK_RIGHT:
				viewPanSetpointX = viewPanSetpointX + (panSpeed * viewZoomSetpoint);
				break;
			case SDLK_UP:
				viewPanSetpointY = viewPanSetpointY + (panSpeed * viewZoomSetpoint);
				break;
			case SDLK_DOWN:
				viewPanSetpointY = viewPanSetpointY - (panSpeed * viewZoomSetpoint);
				break;
			case SDLK_EQUALS:
				viewZoomSetpoint = viewZoomSetpoint * 0.9f;
				break;
			case SDLK_MINUS:
				viewZoomSetpoint = viewZoomSetpoint * 1.1f;
				break;
			case SDLK_p:
				togglePause();
				break;
			case SDLK_ESCAPE:
				quit();
			}
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{



				if (capturingText)
				{
					editUserDataCallback () ;
				}


				if ( checkMenus ( mouseX,  mouseY) )
				{

					return;



				}
				else
				{

					if (  checkClickObjects ( worldMousePos) )
					{
						return;
					}
				}





				break;
			}
			break;
			}

		}
		case SDL_MOUSEBUTTONUP:
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{
				if (getMouseJointStatus())
				{
					destroyMouseJoint();
				}

				if ( draggedMenu != nullptr ) 
				{
					clearDraggingMenu();
				}
				break;
			}
			break;
			}
		}
		case SDL_MOUSEMOTION:
		{

			int prevMouseX = mouseX;
			int prevMouseY = mouseY;

			mouseX = event.motion.x;
			mouseY = event.motion.y;

			int deltaMouseX = ((mouseX - prevMouseX) / viewportScaleFactorX ) ;
			int deltaMouseY = (-1 * (mouseY - prevMouseY) / viewportScaleFactorY  ) ;

			if ( draggedMenu != nullptr) 
			{



			 rebaseMenu (draggedMenu, deltaMouseX, deltaMouseY);



			}

			worldMousePos = transformScreenPositionToWorld( b2Vec2(mouseX, mouseY) );


			if (getMouseJointStatus())
			{
				maintainMouseJoint (worldMousePos) ;
			}
			break;
		}
		}
	}

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "threadInterface " << elapsed.count() << " microseconds." << std::endl;
#endif
}

int main( int argc, char * argv[] )
{
	setupGraphics();
	initializePhysics();
	initializeGame();
	setupMenus();

	for ( ;; )
	{
		// you can start your threads like this:
		// boost::thread t2{ threadInterface };
		boost::thread t3{ threadPhysics };

		// graphics only works in this thread, because it is the process the SDL context was created in.
		threadGraphics();



		// you can have this thread wait for another to end by saying:
		// t2.join();
		t3.join();

		if (flagQuit)
		{
			flagQuit = false;
			return 0;
		}
	}

	return 0;
}
