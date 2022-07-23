#include <ctime>
#include <chrono>
#include <iostream>

#include "graphics.h"
// #include "menus.h"
#include "untitled_marlo_project.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

void quit ()
{
	shutdownGraphics();
	cleanupText2D();
	SDL_Quit();
	flagQuit = true;
}

void threadMainMenuInterface()
{
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		switch ( event.type )
		{
		case SDL_KEYDOWN:
		{
			switch ( event.key.keysym.sym )
			{
			case SDLK_j:
				flagCreate = true;
				break;
			case SDLK_i:
				flagLoad = true;
				break;
			case SDLK_ESCAPE:
				quit();
				return;
			}
		}
		}
	}
}

void threadInterface()
{
	ZoneScoped;
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		switch ( event.type )
		{
		case SDL_KEYUP:
		{
			switch ( event.key.keysym.sym )
			{
			}
			break;
		}
		case SDL_KEYDOWN:
		{
			// if (capturingText)
			// {
			// 	if (event.key.keysym.sym > 0x20 && event.key.keysym.sym < 0x7f)
			// 	{
			// 		capturedString += event.key.keysym.sym;
			// 	}

			// 	switch ( event.key.keysym.sym )
			// 	{
			// 	case SDLK_RETURN:
			// 		editUserDataCallback();
			// 		break;
			// 	}
			// 	break;
			// }

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
			case SDLK_LSHIFT:
				break;
			case SDLK_EQUALS:
				viewZoomSetpoint = viewZoomSetpoint * 0.9f;
				if (viewZoomSetpoint < minZoom)
				{
					viewZoomSetpoint = minZoom;
				}
				break;
			case SDLK_MINUS:
				viewZoomSetpoint = viewZoomSetpoint * 1.1f;
				if (viewZoomSetpoint > maxZoom)
				{
					viewZoomSetpoint = maxZoom;
				}

				break;
			case SDLK_p:
				togglePause();
				break;
			case SDLK_r:
				spawnPlayer();
				break;
			case SDLK_l:
				// lockfps = !lockfps;

				toggleFPSLimit();
				break;
			case SDLK_y:
				incrementSelectedOrgan();
				break;
			case SDLK_h:
				decrementSelectedOrgan();
				break;
			case SDLK_g:
				playerGrab = true;
				break;
			case SDLK_f:
				playerDrop = true;
				break;
			case SDLK_d:
			{
				adjustPlayerPos(Vec_f2(0.0f, playerSpeed));
				break;
			}
			case SDLK_s:
			{
				adjustPlayerPos(Vec_f2(-playerSpeed, 0.0f));
				break;
			}
			case SDLK_a:
			{
				adjustPlayerPos(Vec_f2(0.0f, -playerSpeed));
				break;
			}
			case SDLK_w:
			{
				adjustPlayerPos(Vec_f2(playerSpeed, 0.0f));
				break;
			}
			case SDLK_o:
			{
				save();
				break;
			}
			case SDLK_e:
			{
				selectCursorAnimal();
				break;
			}
			case SDLK_k:
			{
				saveSelectedAnimal();
				break;
			}

			case SDLK_u:
			{
				toggleInstructions();
				break;
			}
			case SDLK_m:
			{
				viewAdversary();
				break;
			}
			case SDLK_t:
			{
				incrementSelectedGrabber();
				break;
			}

			case SDLK_COMMA:

			{
				incrementVisualizer();
				break;
			}
			case SDLK_n:
			{


				lookAtNextNonretiredAnimal();
				break;
			}
			case SDLK_q:
			{
				toggleRain();
				break;
			}
			case SDLK_v:
			{

				spawnAnimalAtCursor();
				break;
			}

			case SDLK_b:
			{

				setupPlantAtCursor();
				break;
			}


			case SDLK_LEFTBRACKET:
			{
				incrementSideText();
				break;
			}
			case SDLK_RIGHTBRACKET:
			{
				decrementSideText();
				break;
			}
			case SDLK_SLASH:
			{
				 scrambleSelectedAnimal();
				break;
			}
			case SDLK_PERIOD:
			{
				if (getPause())
				{
					model();
				}
				break;
			}

			case SDLK_ESCAPE:
				flagReturn = true;
				return;
			}
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{
				activateGrabbedMachine();
				break;
			}
			case SDL_BUTTON_RIGHT:
			{
				rightClickCallback();
				break;
			}
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{
				notifyLMBUp();
				break;
			}
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			mouseX = event.motion.x;
			mouseY = event.motion.y;
			Vec_f2 mouseWorldPos = transformScreenPositionToWorld( Vec_f2( mouseX, mouseY ) );
			setMousePosition(Vec_i2 (mouseWorldPos.x, mouseWorldPos.y));
			break;
		}
		}
	}
}

void setFlagReady()
{
	flagReady = true;
}
void initDeepSea()
{
	worldCreationStage = 0;
}

int main( int argc, char * argv[] )
{
	fastReset();
	setupGraphics();
	setupExtremelyFastNumberGenerators();
	if ( ! test_all())
	{
		return 1;
	}
	initText2D();
	initDeepSea();
	for (;;)
	{
		flagCreate = false;
		flagReady = false;
		flagQuit = false;
		flagLoad = false;
		mainMenuDraw();
		threadMainMenuInterface();
		if (flagCreate)
		{
			flagCreate = false;
			flagReady = false;
			boost::thread t7{ setupRandomWorld }; // takes ages, so run it in a thread and print progress statements.
		}

		if (flagLoad)
		{
			flagLoad = false;
			flagReady = false;
			boost::thread t7{ load };
		}

		if (flagReady)
		{
			flagReady = false;
			startSimulation();
			worldCreationStage = 0;
		}

		if (flagQuit)
		{
			flagQuit = false;
			return 0;
		}
	}

	return 0;
}
