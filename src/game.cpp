#include "game.h"
#include "graphics.h"

#include <ctime>
#include <chrono>
#include <iostream>

#define THREAD_TIMING

const unsigned int numberOfFieldsPerVertex = 6; /*  R, G, B, A, X, Y  */


std::string exampleSentence = "";


void initialize ()
{
	setupExtremelyFastNumberGenerators();

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));

}

void thread_game () 
{

	// custom logic goes here!

}


void thread_graphics()
{
#ifdef THREAD_TIMING
	auto start = std::chrono::steady_clock::now();
#endif
	preDraw();

	unsigned int nVertsToRenderThisTurn = 1;
	long unsigned int totalNumberOfFields = nVertsToRenderThisTurn * numberOfFieldsPerVertex;

	float colorGrid[nVertsToRenderThisTurn];

	/** your graphics logic here **/

	glBufferData( GL_ARRAY_BUFFER, sizeof( float  ) * totalNumberOfFields, colorGrid, GL_DYNAMIC_DRAW );
	glDrawArrays(GL_POINTS, 0,  nVertsToRenderThisTurn);

	postDraw();

#ifdef THREAD_TIMING
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout << "thread_graphics " << elapsed.count() << " microseconds." << std::endl;
#endif
}


