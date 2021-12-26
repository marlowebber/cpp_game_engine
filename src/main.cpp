#include <ctime>
#include <chrono>
#include <iostream>

#include "game.h"
#include "graphics.h"

bool paused = false;
bool flagQuit = false;

int mouseX;
int mouseY;

float panSpeed = 0.5f;

void quit ()
{
	shutdownGraphics();
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
				break;
			}

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
	initializeGame();

	for ( ;; )
	{
		// you can start your threads like this:
		boost::thread t2{ threadInterface };
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
