#include <ctime>
#include <chrono>
#include <iostream>

#include "graphics.h"
#include "menus.h"
#include "untitled_marlo_project.h"

bool paused = false;
bool flagQuit = false;

int mouseX;
int mouseY;

float panSpeed = 0.1f;

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
				// cameraPositionX ++;
				break;
			case SDLK_RIGHT:
				viewPanSetpointX = viewPanSetpointX + (panSpeed * viewZoomSetpoint);
				// cameraPositionX --;
				break;
			case SDLK_UP:
				viewPanSetpointY = viewPanSetpointY + (panSpeed * viewZoomSetpoint);
				// cameraPositionY++;
				break;
			case SDLK_DOWN:
				viewPanSetpointY = viewPanSetpointY - (panSpeed * viewZoomSetpoint);
				// cameraPositionY--;
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
			case SDLK_r:
				spawnPlayer();
				break;
			case SDLK_l:
				// spawnPlayer();
				lockfps = !lockfps;
				break;



			case SDLK_d:
			{

				cameraPositionX += cameraPanSpeed;
				cameraPositionX = cameraPositionX % worldSize;
				// unsigned int destination = getPlayerDestination();
				// destination += 1;
				// setPlayerDestination(destination);
				break;
			}


			case SDLK_s:
			{

				cameraPositionY -= cameraPanSpeed;
				cameraPositionY = cameraPositionY % worldSize;
				// unsigned int destination = getPlayerDestination();
				// destination -= worldSize;
				// setPlayerDestination(destination);
				break;
			}



			case SDLK_a:
			{

				cameraPositionX -= cameraPanSpeed;
				cameraPositionX = cameraPositionX % worldSize;
				// unsigned int destination = getPlayerDestination();
				// destination -= 1;
				// setPlayerDestination(destination);
				break;
			}


			case SDLK_w: // w
			{
				cameraPositionY += cameraPanSpeed;
				cameraPositionY = cameraPositionY % worldSize;
				// unsigned int destination = getPlayerDestination();
				// destination += worldSize;
				// setPlayerDestination(destination);
				break;
			}


			case SDLK_i: // w
			{
				load();
				break;
			}
			case SDLK_o: // w
			{
				save();
				break;
			}


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

					// if (  checkClickObjects ( worldMousePos) )
					// {
					// 	return;
					// }
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
				// if (getMouseJointStatus())
				// {
				// 	destroyMouseJoint();
				// }



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

// drawTestCoordinate (mouseX, mouseY) ;

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

			// worldMousePos = transformScreenPositionToWorld( b2Vec2(mouseX, mouseY) );


			// if (getMouseJointStatus())
			// {
			// 	maintainMouseJoint (worldMousePos) ;
			// }
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
	initializeGame();
	setupMenus();

	for ( ;; )
	{
		// you can start your threads like this:
		boost::thread t2{ threadInterface };
		// boost::thread t3{ threadPhysics };
		boost::thread t3{ threadGame };

		// graphics only works in this thread, because it is the process the SDL context was created in.
		threadGraphics();



		// you can have this thread wait for another to end by saying:
		t2.join();
		t3.join();

		if (flagQuit)
		{
			flagQuit = false;
			return 0;
		}
	}

	return 0;
}
