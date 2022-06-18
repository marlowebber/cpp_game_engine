// Untitled Marlo Project.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cstring>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"

#include <boost/thread.hpp>
#include <random>
#include <time.h>
#include <pthread.h>
#include <iostream>
#include <fstream>

#define TRACY_ENABLE

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

#include "utilities.h"
#include "graphics.h"
#include "untitled_marlo_project.h"
#include "menus.h"
#include "main.h"

#include <chrono>
#include <thread>

#include "SimplexNoise.h"

#include "TinyErode.h"

const bool immortality           = false;
const bool doReproduction        = true;
const bool doMuscles             = true;
const bool growingCostsEnergy    = true;
const bool taxIsByMass           = true;
const bool threading             = true;
const bool cameraFollowsChampion = false;
const bool cameraFollowsPlayer   = true;
const bool variedGrowthCost      = false;
const bool variedUpkeep          = false;
const bool respawnLowSpecies     = true;
const bool doMutation            = true;
const bool setOrSteerAngle       = true;
const bool printLogs             = false;

const int prelimSize = 256;
const int cameraPanSpeed = 10;

const float baseLungCapacity = 1.0f;
const unsigned int prelimSquareSize = prelimSize * prelimSize;
const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.
const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfSpecies = 8;
const unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);

const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs game.animals to make new cells.
const float taxEnergyScale         = 0.00002f;        // a multiplier for how much it costs game.animals just to exist.
const float movementEnergyScale    = 0.00002f;        // a multiplier for how much it costs game.animals to move.
const float foodEnergy             = 0.9f;         // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float grassEnergy            = 0.3f;         // how much you get from eating a square of grass
const float neuralNoise = 0.1f;
const float liverStorage = 20.0f;
const unsigned int baseLifespan = 50000;			// if the lifespan is long, the animal's strategy can have a greater effect on its success. If it's very short, the animal is compelled to be just a moving mouth.
const float signalPropagationConstant = 0.1f;      // how strongly sensor organs compel the animal.
const float musclePower = 40.0f;
const float thresholdOfBoredom = 0.1f;
const unsigned int numberOfSpeakerChannels = 16;
const float const_pi = 3.1415f;

const int paletteMenuX = 200;
const int paletteMenuY = 50;
const int paletteTextSize = 10;
const int paletteSpacing = 20;
const unsigned int paletteWidth = 3;
const unsigned int nLogs = 32;
const unsigned int logLength = 64;

// these are the parameters that set up the physical geography of the game.world.
const float seaLevel =  0.5f * worldSize;;;
const float biome_marine  = seaLevel + (worldSize / 20);
const float biome_coastal = seaLevel + (worldSize / 3);
const float sunXangle = 0.35f;
const float sunYangle = 0.35f;
const unsigned int baseSize = 100;
const unsigned int wallThickness = 8;
const unsigned int doorThickness = 16;

const float panSpeed = 0.1f;
const float playerSpeed = 0.3f;

const int neighbourOffsets[] =
{
	- 1,
	- worldSize - 1,
	- worldSize ,
	- worldSize  + 1,
	+ 1,
	+worldSize + 1,
	+worldSize,
	+worldSize - 1
};

// tinyerode stuff
auto getHeight = [](int x, int y) -> float {
	unsigned int address = (y * prelimSize) + x;
	return  prelimMap[address];  ///* return height value at (x, y) */ 0.0f;
};

auto addHeight = [](int x, int y, float deltaHeight) {
	unsigned int address = (y * prelimSize) + x;
	prelimMap[address] += deltaHeight ;   /* add 'deltaHeight' to the location (x, y) */
};

auto getWater = [](int x, int y) -> float {
	unsigned int address = (y * prelimSize) + x;
	return  prelimWater[address]; //	return /* return water level at (x, y) */ return 0.0f;
};

auto addWater = [](int x, int y, float deltaWater) -> float {
	/* Note: 'deltaWater' can be negative. */
	unsigned int address = (y * prelimSize) + x;
	float previousWaterLevel =  prelimWater[address];
	prelimWater[address] += deltaWater;
	if (prelimWater[address] < 0.0f)
	{
		prelimWater[address]  = 0.0f;
	}
	/* The function returns the new water level. It should not
	 * fall below zero. */
	return std::max(0.0f, previousWaterLevel + deltaWater);
};

auto carryCapacity = [](int x, int y) -> float {
	return 0.1;
};

auto deposition = [](int x, int y) -> float {
	return 0.1;
};

auto erosion = [](int x, int y) -> float {
	return 0.1;
};

auto evaporation = [](int x, int y) -> float {
	return 0.1;
};



	GameState game;

	// these are variables which are only needed per session, and never need to be stored.
	
	float prelimMap[prelimSquareSize];
	float prelimWater[prelimSquareSize];
	unsigned int modelFrameCount = 0;
	unsigned int usPerFrame = 0;
	float fps = 1.0f;

	bool mainMenu = true;
	bool flagQuit = false;
	bool flagCreate = false;
	bool flagLoad = false;
	bool flagReady = false;
	bool flagReturn = false;
	int mouseX;
	int mouseY;
	unsigned int worldCreationStage = 0;




// --------------------- DeepSea public methods



void activateGrabbedMachine()// occurs whenever a left click is received.
{
	if (game.playerCreature >= 0)
	{
		for (int i = 0; i < game.animals[game.playerCreature].cellsUsed; ++i)
		{
			if (game.animals[game.playerCreature].body[i].organ == ORGAN_GRABBER)
			{
				if (game.animals[game.playerCreature].body[i].grabbedCreature >= 0 && game.animals[game.playerCreature].body[i].grabbedCreature < numberOfAnimals)
				{
					if (game.animals [   game.animals[game.playerCreature].body[i].grabbedCreature  ].isMachine)
					{
						if (game.animals [   game.animals[game.playerCreature].body[i].grabbedCreature  ].machineCallback !=  MATERIAL_NOTHING)
						{
							switch (game.animals [   game.animals[game.playerCreature].body[i].grabbedCreature  ].machineCallback )
							{
							case MACHINECALLBACK_KNIFE :
								knifeCallback(game.animals[game.playerCreature].body[i].grabbedCreature , game.playerCreature  );
								break;
							case MACHINECALLBACK_PISTOL :
								exampleGunCallback(game.animals[game.playerCreature].body[i].grabbedCreature , game.playerCreature  );
								break;
							case MACHINECALLBACK_LIGHTER :
								lighterCallback(game.animals[game.playerCreature].body[i].grabbedCreature , game.playerCreature  );
								break;
							case MACHINECALLBACK_HOSPITAL :
								paletteCallback(game.animals[game.playerCreature].body[i].grabbedCreature , game.playerCreature  );
								break;
							case MACHINECALLBACK_MESSAGECOMPUTER :
								communicationComputerCallback(game.animals[game.playerCreature].body[i].grabbedCreature , game.playerCreature  );
								break;
							}
							break;
						}
					}
				}
			}
		}
	}
}


void selectCursorAnimal()
{
	if (game.selectedAnimal >= 0)
	{
		game.selectedAnimal = -1;
	}
	else
	{
		int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
		int cursorPosY = game.cameraPositionY + game.mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
		if (game.cursorAnimal >= 0 && game.cursorAnimal < numberOfAnimals)
		{
			int occupyingCell = isAnimalInSquare(game.cursorAnimal, worldCursorPos);
			if ( occupyingCell >= 0)
			{
				game.selectedAnimal = game.cursorAnimal;
			}
		}
	}
}

void rightClickCallback ()
{
	if (game.palette)
	{
		paletteEraseAtMouse();
	}
}


void viewAdversary()
{
	if (game.cameraTargetCreature == game.adversary)
	{
		game.cameraTargetCreature = -1;
		if (game.playerCreature >= 0)
		{
			game.cameraTargetCreature = game.playerCreature;
		}
	}
	else
	{
		if (game.adversary >= 0)
		{
			game.cameraTargetCreature = game.adversary;
		}
	}
}

void toggleInstructions()
{
	game.showInstructions = !game.showInstructions;
}


void resetMouseCursor(  )
{
	game.mousePositionX = 0;
	game.mousePositionY = 0;
}

void togglePause ()
{
	game.paused = !game.paused;
}

void incrementSelectedOrgan()
{
	game.paletteSelectedOrgan++;
	game.paletteSelectedOrgan = game.paletteSelectedOrgan % numberOfOrganTypes;
}
void decrementSelectedOrgan()
{
	game.paletteSelectedOrgan--;
	game.paletteSelectedOrgan = game.paletteSelectedOrgan % numberOfOrganTypes;
}

void playerGrab()
{
	if (game.playerCreature >= 0)
	{
		for (int i = 0; i < game.animals[game.playerCreature].cellsUsed; ++i)
		{
			if (game.animals[game.playerCreature].body[i].organ == ORGAN_GRABBER)
			{
				if (game.animals[game.playerCreature].body[i].grabbedCreature == -1)
				{
					game.animals[game.playerCreature].body[i].signalIntensity = 1;
				}
			}
		}
	}
}

void playerDrop()
{
	if (game.playerCreature >= 0)
	{
		for (int i = 0; i < game.animals[game.playerCreature].cellsUsed; ++i)
		{
			if (game.animals[game.playerCreature].body[i].organ == ORGAN_GRABBER)
			{
				if (game.animals[game.playerCreature].body[i].grabbedCreature >= 0)
				{
					game.animals[game.playerCreature].body[i].signalIntensity = -1;
				}
			}
		}
	}
}

void adjustPlayerPos(Vec_f2 pos)
{
	if (game.playerCreature >= 0)
	{
		game.animals[game.playerCreature].fAngle = 0.0f;
		int strafeMuscle = getRandomCellOfType(game.playerCreature, ORGAN_MUSCLE_STRAFE);
		int muscle = getRandomCellOfType(game.playerCreature, ORGAN_MUSCLE);
		if (strafeMuscle >= 0)
		{
			game.animals[game.playerCreature].body[strafeMuscle].signalIntensity = pos.y;
		}
		if (muscle >= 0)
		{
			game.animals[game.playerCreature].body[muscle].signalIntensity = pos.x;
		}
	}
}







// --------------------- DeepSea private methods



void resetGameState()
{
	// these variables are how the player drives their character, and what they get back from it.
	game.playerGrabState = false;
	game.playerCanSee = true;
	game.playerCanHear = true;
	game.playerCanSmell = true;
	game.palette = false;
	game.playerCanPickup = false;
	game.playerCanPickupItem = -1;
	game.lockfps           = true;
	game.paused = false;
	game.mousePositionX =  -430;
	game.mousePositionY =  330;


	// these variables keep track of the main characters in the game game.world.
	game.playerCreature = -1;
	game.championScore = 0;
	game.championEnergyScore = 0.0f;
	game.adversary = -1;
	game.adversaryRespawnPos;
	game.selectedAnimal = -1;
	game.cursorAnimal = -1;
	game.playerRespawnPos;



	// camera view
	game.cameraPositionX = 0 ;
	game.cameraPositionY = 0 ;
	game.cameraTargetCreature = -1;


	// these variables govern the display of menus and other texts.
	game.showInstructions = false;
	game.ecologyComputerDisplay = false;
	game.visualizer = VISUALIZER_TRUECOLOR;
	game.computerdisplays[5];
	game.logs[logLength][nLogs];
	game.paletteSelectedOrgan = 0;


	resetAnimals();
	resetGrid();
}



void appendLog( std::string input)
{
	for (int i = nLogs; i > 0; i--)
	{
		memcpy(  &game.logs[i] , &game.logs[i - 1], sizeof(char) * logLength   );
	}
	strcpy( &game.logs[0][0] , input.c_str() );
}




void resetConnection(unsigned int animalIndex, unsigned int cellLocalPositionI, unsigned int i)
{
	game.animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;
	game.animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	game.animals[animalIndex].body[cellLocalPositionI].connections[i].weight = RNG() - 0.5f;
}



void resetCell(unsigned int animalIndex, unsigned int cellLocalPositionI)
{
	game.animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
	game.animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
	game.animals[animalIndex].body[cellLocalPositionI].color  = color_darkgrey;
	game.animals[animalIndex].body[cellLocalPositionI].damage = 0.0f;
	game.animals[animalIndex].body[cellLocalPositionI].eyeLookX = 0;
	game.animals[animalIndex].body[cellLocalPositionI].eyeLookY = 0;
	game.animals[animalIndex].body[cellLocalPositionI].localPosX = 0;
	game.animals[animalIndex].body[cellLocalPositionI].localPosY = 0;
	game.animals[animalIndex].body[cellLocalPositionI].grabbedCreature = -1;

	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		resetConnection(animalIndex, cellLocalPositionI, i);
	}
	game.animals[animalIndex].genes[cellLocalPositionI] = game.animals[animalIndex].body[cellLocalPositionI];
}

void paintAnimal(unsigned int animalIndex)
{
	Color newAnimalColorA = Color(RNG(), RNG(), RNG(), 1.0f);
	Color newAnimalColorB = Color(RNG(), RNG(), RNG(), 1.0f);
	for (int i = 0; i < animalSquareSize; ++i)
	{
		game.animals[animalIndex].body[i].color = filterColor(  newAnimalColorA , multiplyColorByScalar( newAnimalColorB , RNG())  );
		game.animals[animalIndex].genes[i].color = game.animals[animalIndex].body[i].color ;
	}
}

void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		std::string gunDescription = std::string("An animal");
		strcpy( &game.animals[animalIndex].displayName[0] , gunDescription.c_str() );
		game.animals[animalIndex].mass = 0;
		game.animals[animalIndex].numberOfTimesReproduced = 0;
		game.animals[animalIndex].damageDone = 0;
		game.animals[animalIndex].damageReceived = 0;
		game.animals[animalIndex].birthLocation = 0;
		game.animals[animalIndex].age = 0;
		game.animals[animalIndex].lifespan = baseLifespan;
		game.animals[animalIndex].parentIdentity = -1;
		game.animals[animalIndex].offspringEnergy = 1.0f;
		game.animals[animalIndex].energy   = 0.0f;
		game.animals[animalIndex].energyDebt   = 0.0f;
		game.animals[animalIndex].maxEnergy   = 0.0f;
		game.animals[animalIndex].fPosX = 0.0f;
		game.animals[animalIndex].fPosY = 0.0f;
		game.animals[animalIndex].position = 0;
		game.animals[animalIndex].uPosX = 0;
		game.animals[animalIndex].uPosY = 0;
		game.animals[animalIndex].parentAmnesty = true;
		game.animals[animalIndex].retired = true;
		game.animals[animalIndex].cellsUsed = 0;
		game.animals[animalIndex].identityColor = Color(RNG(), RNG(), RNG(), 1.0f);
		if (animalIndex == game.adversary)
		{
			game.animals[animalIndex].identityColor = color_white;
		}
		game.animals[animalIndex].isMachine = false;
		game.animals[animalIndex].machineCallback = MATERIAL_NOTHING;
		game.animals[animalIndex].temp_limit_low = 273.0f;
		game.animals[animalIndex].temp_limit_high = 323.0f;
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			resetCell(animalIndex, cellLocalPositionI );
		}
	}
}


bool isCellAnEdge(unsigned int animalIndex, unsigned int cellIndex)// check if a cell has an empty neighbour.
{
	// go through the list of other cells and see if any of neighbour indexes match any of them. if so, mark the square as not an edge.
	unsigned int neighbourtally = 0;
	Vec_i2 locations_to_check[nNeighbours] =
	{
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX - 1 , game.animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX     , game.animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX + 1 , game.animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX - 1 , game.animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX + 1 , game.animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX - 1 , game.animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX     , game.animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX + 1 , game.animals[animalIndex].genes[cellIndex].localPosY + 1  ),
	};
	for (int potentialNeighbour = 0; potentialNeighbour < game.animals[animalIndex].cellsUsed; ++potentialNeighbour)
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			if (game.animals[animalIndex].genes[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        game.animals[animalIndex].genes[potentialNeighbour].localPosY == locations_to_check[i].y  )
			{
				neighbourtally++;
			}
		}
	}
	if (neighbourtally < nNeighbours)
	{
		return true;
	}
	return false;
}

unsigned int getRandomEdgeCell(unsigned int animalIndex)
{
	while (true)
	{
		unsigned int i = extremelyFastNumberFromZeroTo(game.animals[animalIndex].cellsUsed);
		if (i < animalSquareSize)
		{
			if (isCellAnEdge(animalIndex, i))
			{
				return i;
			}
		}
	}
}

Vec_i2 getRandomEmptyEdgeLocation(unsigned int animalIndex)
{
	unsigned int cellIndex = getRandomEdgeCell(animalIndex);
	Vec_i2 result = Vec_i2(0, 0);
	// get an edge cell at random then search its neighbours to find the empty one. return the position of the empty neighbour.
	Vec_i2 locations_to_check[nNeighbours] =
	{
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX - 1 , game.animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX     , game.animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX + 1 , game.animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX - 1 , game.animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX + 1 , game.animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX - 1 , game.animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX     , game.animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].genes[cellIndex].localPosX + 1 , game.animals[animalIndex].genes[cellIndex].localPosY + 1  ),
	};
	for (int i = 0; i < nNeighbours; ++i)
	{
		bool empty = true;
		for (int potentialNeighbour = 0; potentialNeighbour < game.animals[animalIndex].cellsUsed; ++potentialNeighbour)
		{
			if (game.animals[animalIndex].genes[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        game.animals[animalIndex].genes[potentialNeighbour].localPosY == locations_to_check[i].y  )
			{
				empty = false;
			}
		}
		if (empty)
		{
			return locations_to_check[i];
		}
	}
	return result;
}



// choose a random cell of any type that can be connected to, which includes all neurons and all sensors.
int getRandomConnectableCell( unsigned int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnectable(  game.animals[animalIndex].genes[cellIndex].organ ))
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

void appendCell(unsigned int animalIndex, unsigned int organType, Vec_i2 newPosition)
{
	// pick a random location for the new cell which is adjacent to a normal cell.
	// we can avoid ever having to check for valid placement of the cell if we are careful about where to place it!
	// figure out the lowest index in the animal array and put the new cell there
	unsigned int cellIndex = game.animals[animalIndex].cellsUsed;
	if (cellIndex < animalSquareSize)
	{
		game.animals[animalIndex].cellsUsed ++;
		game.animals[animalIndex].genes[cellIndex].localPosX = newPosition.x;
		game.animals[animalIndex].genes[cellIndex].localPosY = newPosition.y;
		game.animals[animalIndex].genes[cellIndex].organ = organType;

		if (  isCellConnecting(organType)) // if the cell is supposed to have connections, go hook it up
		{
			unsigned int randomNumberOfConnections = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS);
			for (int i = 0; i < randomNumberOfConnections; ++i)
			{

				unsigned int connectableCell = getRandomConnectableCell( animalIndex);// pick a random connectable cell to connect to.
				bool alreadyConnected =  false;	// check if you are already connected to it.
				for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
				{
					if (  game.animals[animalIndex].genes[cellIndex].connections[j].connectedTo == connectableCell &&
					        game.animals[animalIndex].genes[cellIndex].connections[j] .used)
					{
						alreadyConnected = true;
					}
				}
				if (!alreadyConnected)	// make the new connection if appropriate.
				{
					for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
					{
						if ( ! (game.animals[animalIndex].genes[cellIndex].connections[j].used))
						{
							game.animals[animalIndex].genes[cellIndex].connections[j].used = true;
							game.animals[animalIndex].genes[cellIndex].connections[j].connectedTo = connectableCell;
							game.animals[animalIndex].genes[cellIndex].connections[j].weight = (RNG() - 0.5f ) * 2;
							break;
						}
					}
				}
			}
		}
	}
	game.animals[animalIndex].body[cellIndex] = game.animals[animalIndex].genes[cellIndex] ;
}

// add a cell to an animal germline in a guided but random way. Used to messily construct new game.animals, for situations where lots of variation is desirable.
void animalAppendCell(unsigned int animalIndex, unsigned int organType)
{
	Vec_i2 newPosition   = getRandomEmptyEdgeLocation(animalIndex); 	// figure out a new position anywhere on the animal edge
	appendCell(animalIndex, organType,  newPosition);
}


void resetAnimals()
{
	for ( int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		resetAnimal(animalIndex);
	}
	int j = 1;
	setupExampleAnimal2(j);
	game.champion = game.animals[j];
	game.championScore = 0;
}

Vec_f2 getTerrainSlope(unsigned int worldPositionI)
{
	float xSurfaceAngle = game.world[worldPositionI].height - game.world[worldPositionI + 1].height ;
	float ySurfaceAngle = game.world[worldPositionI].height - game.world[worldPositionI + worldSize].height ;
	return Vec_f2(xSurfaceAngle, ySurfaceAngle);
}


void detailTerrain(unsigned int worldPositionI)
{
	Vec_f2 slope = getTerrainSlope(worldPositionI);
	float grade = sqrt( (slope.x * slope.x) +  (slope.y * slope.y)  );
	float colorNoise = 1 + (((RNG() - 0.5f) * 0.35)) ; // map -1,1 to 0,0.8
	if ( game.world[worldPositionI]. height < biome_marine)
	{
		if (grade < 5.0f)
		{
			game.world[worldPositionI].terrain = MATERIAL_SAND;
		}

		else
		{
			game.world[worldPositionI].terrain = MATERIAL_BASALT;
		}
	}

	else if (game.world[worldPositionI]. height  > biome_marine && game.world[worldPositionI]. height  < biome_coastal )
	{
		if (grade < 2.5f)
		{
			game.world[worldPositionI].terrain = MATERIAL_SOIL;
		}
		else  if (grade < 5.0f)
		{
			game.world[worldPositionI].terrain = MATERIAL_DIRT;
		}
		else
		{
			game.world[worldPositionI].terrain = MATERIAL_BASALT;
			game.world[worldPositionI].wall = MATERIAL_BASALT;
		}
	}
	else if (game.world[worldPositionI]. height  > biome_coastal)
	{
		if (grade < 2.5f)
		{
			game.world[worldPositionI].terrain = MATERIAL_GRAVEL;

		}
		else if (grade < 5.0f)
		{
			game.world[worldPositionI].terrain = MATERIAL_DUST;
		}
		else
		{
			game.world[worldPositionI].terrain = MATERIAL_BASALT;
			game.world[worldPositionI].wall = MATERIAL_BASALT;
		}
	}
}

void resetGrid()
{
	for (int i = 0; i < worldSquareSize; ++i)
	{
		game.world[i].terrain = MATERIAL_NOTHING;
		game.world[i].material = MATERIAL_NOTHING;
		game.world[i].identity = -1;
		game.world[i].trail = 0.0f;
		game.world[i].height = 0.0f;
		game.world[i].light = color_white;
		game.world[i].pheromoneIntensity = 0.0f;
		game.world[i].pheromoneChannel = -1;
		game.world[i].grassColor =  color_green;
	}
}

int getNewIdentity(unsigned int speciesIndex)
{
	if (!game.speciesVacancies[speciesIndex])
	{
		return -1;
	}
	int animalIndex;
	for ( animalIndex = speciesIndex * numberOfAnimalsPerSpecies; animalIndex < (speciesIndex + 1) * numberOfAnimalsPerSpecies; ++animalIndex)
	{
		if (animalIndex < numberOfAnimals && animalIndex >= 0)
		{
			if (game.animals[animalIndex].retired)
			{
				return animalIndex;
			}
		}
	}
	game.speciesVacancies[speciesIndex] = false;
	return -1;
}

// // some genes have permanent effects, or effects that need to be known immediately at birth. Compute them here.
// this function studies the phenotype, not the genotype.
// returns whether the animal is fit to live.
bool measureAnimalQualities(unsigned int animalIndex)
{
	game.animals[animalIndex].mass = 0;
	game.animals[animalIndex].energyDebt = 0.0f;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		game.animals[animalIndex].mass++;
		game.animals[animalIndex].energyDebt += 1.0f;
	}
	game.animals[animalIndex].totalMuscle = 0;
	game.animals[animalIndex].offspringEnergy = 1.0f;
	game.animals[animalIndex].lifespan = baseLifespan;
	unsigned int totalGonads = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE ||
		        game.animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_TURN ||
		        game.animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_STRAFE)
		{
			game.animals[animalIndex].totalMuscle ++;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_ADDOFFSPRINGENERGY)
		{
			game.animals[animalIndex].offspringEnergy += game.animals[animalIndex].offspringEnergy ;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_ADDLIFESPAN)
		{
			game.animals[animalIndex].lifespan += baseLifespan;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_GONAD)
		{
			totalGonads ++;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_COLDADAPT)
		{
			game.animals[animalIndex].temp_limit_low -= 35.0f;
			game.animals[animalIndex].temp_limit_high -= 35.0f;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_HEATADAPT)
		{
			game.animals[animalIndex].temp_limit_high += 35.0f;
			game.animals[animalIndex].temp_limit_low  += 35.0f;
		}
	}
	game.animals[animalIndex].lifespan *= 0.75 + (RNG() * 0.5);
	if (game.animals[animalIndex].mass > 0 && totalGonads > 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

// choose a random cell of any type that can put forth a connection, which includes all neurons and actuators.
int getRandomConnectingCell( unsigned int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnecting(  game.animals[animalIndex].genes[cellIndex].organ ))
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}

	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

// choose a random cell of a particular organ type in a given animal, or MATERIAL_NOTHING if the organ doesn't exist.
int getRandomCellOfType(unsigned int animalIndex, unsigned int organType)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].genes[cellIndex].organ == organType)
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

// find a random speaker channel that cells of organType are using.
int findOccupiedChannel(unsigned int animalIndex, unsigned int organType)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].genes[cellIndex].organ == organType)
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return game.animals[animalIndex] .body[(*iterator)].speakerChannel;
	}
	else
	{
		return -1;
	}
}

// modify every cell which uses channel, into using a cnew hannel which is the sum of channel and increment. Increment may be negative.
void modifyChannel(unsigned int animalIndex, int channel, int increment)
{
	int newChannel = abs((channel + increment)) % numberOfSpeakerChannels;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].body[cellIndex].speakerChannel == channel)
		{
			game.animals[animalIndex].body[cellIndex].speakerChannel = newChannel;
		}
	}
}

// choose any random populated cell.
int getRandomPopulatedCell(unsigned int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		cellsOfType.push_back(cellIndex);
		found++;
	}
	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

// the opposite of append cell- remove a cell from the genes and body, shifting all other cells backwards and updating all connections.
void eliminateCell( unsigned int animalIndex, unsigned int cellToDelete )
{
	for (int cellIndex = cellToDelete + 1; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex) // shift array of cells down 1, overwriting the lowest modified cell (the cell to delete)
	{
		game.animals[animalIndex].genes[cellIndex - 1] = game.animals[animalIndex].genes[cellIndex];
	}
	game.animals[animalIndex].cellsUsed--; // clear the end cell which would have been duplicated
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)	// go through all cells and update connections
	{
		for (int connectionIndex = 0; connectionIndex < NUMBER_OF_CONNECTIONS; ++connectionIndex)
		{
			if (game.animals[animalIndex].genes[cellIndex].connections[connectionIndex].connectedTo == cellToDelete	)
			{
				game.animals[animalIndex].genes[cellIndex].connections[connectionIndex].used = false;
			}
			else if (game.animals[animalIndex].genes[cellIndex].connections[connectionIndex].connectedTo > cellToDelete)
			{
				game.animals[animalIndex].genes[cellIndex].connections[connectionIndex].connectedTo --;
			}
		}
		game.animals[animalIndex].body[cellIndex] = game.animals[animalIndex].genes[cellIndex] ;
	}
}

void mutateAnimal(unsigned int animalIndex)
{
	// some mutations are chosen more commonly than others. They are classed into groups, and each group is associated with a normalized likelyhood of occurring.
	// the reason for this is that the development of the brain must always occur faster and in more detail than the development of the body, or else intelligent behavior can never arise.
	float group1Probability = 1.0f;
	float group2Probability = 0.25f;
	float group3Probability = 0.0625f;
	float group4Probability = 0.03125f;
	float sum = group1Probability + group2Probability + group3Probability + group4Probability;
	float groupChoice = RNG() * sum;
	int group = 0;
	if (groupChoice < group1Probability ) // chosen group 1
	{
		group = 1;
	}
	else if (groupChoice > (group1Probability) &&  groupChoice < (group1Probability + group2Probability) ) // chosen group 2
	{
		group = 2;
	}
	else if (groupChoice > (group1Probability + group2Probability) &&  groupChoice < (group1Probability + group2Probability + group3Probability) ) // chosen group 3
	{
		group = 3;
	}
	else if (groupChoice > (group1Probability + group2Probability + group3Probability) &&  groupChoice < (sum) ) // chosen group 4
	{
		group = 4;
	}
	int mutation = MATERIAL_NOTHING;// choose a mutation from the group randomly.
	if (group == 1)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(2);

		if (mutationChoice == 0)
		{
			mutation = MUTATION_CONNECTIONWEIGHT;
		}
		else if (mutationChoice == 1)
		{
			mutation = MUTATION_ALTERBIAS;
		}
		else if (mutationChoice == 2)
		{
			mutation = MUTATION_SKINCOLOR;
		}
	}
	else if (group == 2)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(1);

		if (mutationChoice == 0)
		{
			mutation = MUTATION_SWITCHCONNECTION;
		}
		else if (mutationChoice == 1)
		{
			mutation = MUTATION_RECONNECT;
		}
	}
	else if (group == 3)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(1);

		if (mutationChoice == 0)
		{
			mutation = MUTATION_MOVECELL;
		}
		else if (mutationChoice == 1)
		{
			mutation = MUTATION_EYELOOK;
		}
	}
	else if (group == 4)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(2);

		if (mutationChoice == 0)
		{
			mutation = MUTATION_ERASEORGAN;
		}
		else if (mutationChoice == 1)
		{
			mutation = MUTATION_ADDORGAN;
		}
		else if (mutationChoice == 2)
		{
			mutation = MUTATION_SPEAKERCHANNEL;
		}
	}
	switch (mutation)
	{
	case MUTATION_ERASEORGAN:
	{
		int mutantCell = getRandomPopulatedCell( animalIndex);
		if (mutantCell >= 0)
		{
			eliminateCell(animalIndex, mutantCell);
		}
		break;
	}
	case MUTATION_ADDORGAN:
	{
		unsigned int newOrgan = randomLetter();
		animalAppendCell(animalIndex, newOrgan);
		break;
	}
	case MUTATION_SWITCHCONNECTION:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].used = !(game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
		}
		break;
	}
	case MUTATION_RECONNECT:// randomise a connection partner.
	{
		int mutantCell =  getRandomConnectingCell( animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			int mutantPartner =  getRandomConnectableCell( animalIndex);
			if (mutantPartner >= 0)
			{
				game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].connectedTo = mutantPartner;
			}
		}
		break;
	}
	case MUTATION_CONNECTIONWEIGHT:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			if (extremelyFastNumberFromZeroTo(1) == 0) // multiply it
			{
				game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].weight *= ((RNG() - 0.5) * 4);
			}
			else // add to it
			{
				game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].weight += ((RNG() - 0.5 ) * 2);
			}
		}
		break;
	}
	case MUTATION_ALTERBIAS:// randomise a bias neuron's strength.
	{
		int mutantCell = getRandomCellOfType(animalIndex, ORGAN_BIASNEURON);
		if (mutantCell >= 0)
		{
			if (extremelyFastNumberFromZeroTo(1) == 0) // multiply it
			{
				game.animals[animalIndex].genes[mutantCell].signalIntensity *= ((RNG() - 0.5 ) * 4);;
			}
			else // add to it
			{
				game.animals[animalIndex].genes[mutantCell].signalIntensity += ((RNG() - 0.5 ) * 2);;
			}
		}
		break;
	}
	case MUTATION_MOVECELL:// swap an existing cell location without messing up the connections.
	{
		int mutantCell = getRandomPopulatedCell(animalIndex);
		if (mutantCell >= 0)
		{
			Vec_i2 destination  = getRandomEmptyEdgeLocation(animalIndex);
			game.animals[animalIndex].body[mutantCell].localPosX = destination.x;
			game.animals[animalIndex].body[mutantCell].localPosY = destination.y;
		}
		break;
	}
	case MUTATION_SPEAKERCHANNEL:// mutate a speaker channel used by groups of cells; change the whole group
	{
		int occupiedChannel = -1;
		int typeOfChannel   = -1;
		unsigned int startingRandomCell = extremelyFastNumberFromZeroTo(game.animals[animalIndex].cellsUsed) - 1;
		for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
		{
			unsigned int cellIndex = (startingRandomCell + i) % game.animals[animalIndex].cellsUsed;
			if ( organUsesSpeakerChannel( game.animals[animalIndex].body[cellIndex].organ )  )
			{
				occupiedChannel =	 findOccupiedChannel( animalIndex, game.animals[animalIndex].body[cellIndex].organ);
			}
		}
		if (occupiedChannel >= 0 && occupiedChannel < numberOfSpeakerChannels)
		{
			int increment = extremelyFastNumberFromZeroTo(numberOfSpeakerChannels);
			modifyChannel(animalIndex, occupiedChannel, increment);
		}
		break;
	}
	case MUTATION_EYELOOK:// mutate an eyelook
	{
		int mutantCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_EYE);
		if (mutantCell >= 0 && mutantCell < animalSquareSize)
		{
			if (extremelyFastNumberFromZeroTo(1) == 0)
			{
				game.animals[animalIndex].genes[mutantCell].eyeLookX += (extremelyFastNumberFromZeroTo(2) - 1);
			}
			else
			{
				game.animals[animalIndex].genes[mutantCell].eyeLookY += (extremelyFastNumberFromZeroTo(2) - 1);
			}
		}
		break;
	}
	case MUTATION_SKINCOLOR:// mutate a cell's skin color
	{
		int mutantCell = getRandomPopulatedCell(animalIndex);
		if (mutantCell >= 0 && mutantCell < animalSquareSize)
		{
			game.animals[animalIndex].genes[mutantCell].color = mutateColor(	game.animals[animalIndex].genes[mutantCell].color);
		}
		break;
	}
	}
}

void spawnAnimalIntoSlot( unsigned int animalIndex,
                          Animal parent,
                          unsigned int position, bool mutation) // copy genes from the parent and then copy body from own new genes.
{
	resetAnimal(animalIndex);
	for (int i = 0; i < animalSquareSize; ++i)
	{
		game.animals[animalIndex].genes[i] = parent.genes[i];
		game.animals[animalIndex].body[i] = parent.genes[i];
	}
	game.animals[animalIndex].isMachine = parent.isMachine;
	game.animals[animalIndex].machineCallback = parent.machineCallback;
	game.animals[animalIndex].cellsUsed = parent.cellsUsed;
	game.animals[animalIndex].retired = false;
	game.animals[animalIndex].position = position;
	game.animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	game.animals[animalIndex].fPosY = position / worldSize;
	game.animals[animalIndex].birthLocation = position;
	game.animals[animalIndex].fAngle = ( (RNG() - 0.5f) * 2 * 3.141f  );
	mutateAnimal( animalIndex);
	measureAnimalQualities(animalIndex);
	memcpy( &( game.animals[animalIndex].displayName[0]), &(parent.displayName[0]), sizeof(char) * displayNameSize  );
}

int spawnAnimal( unsigned int speciesIndex,
                 Animal parent,
                 unsigned int position, bool mutation)
{
	int animalIndex = getNewIdentity(speciesIndex);
	if (animalIndex >= 0) // an animalIndex was available
	{
		spawnAnimalIntoSlot(animalIndex,
		                    // genes,
		                    parent,
		                    position, mutation);
	}
	return animalIndex;
}

void killAnimal(int animalIndex)
{
	if (animalIndex == game.playerCreature)
	{
		game.playerCreature = -1;
	}
	if (animalIndex == game.cameraTargetCreature)
	{
		game.cameraTargetCreature = -1;
	}
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	game.speciesVacancies[speciesIndex] = true;
	game.animals[animalIndex].retired = true;
	unsigned int animalWorldPositionX    = game.animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = game.animals[animalIndex].position / worldSize;
	for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex) // process organs and signals and clear animalIndex on grid
	{
		unsigned int cellLocalPositionX  = game.animals[animalIndex].body[cellIndex].localPosX;
		unsigned int cellLocalPositionY  = game.animals[animalIndex].body[cellIndex].localPosY;
		unsigned int cellWorldPositionX  = cellLocalPositionX + animalWorldPositionX;
		unsigned int cellWorldPositionY  = cellLocalPositionY + animalWorldPositionY;
		unsigned int cellWorldPositionI  = (cellWorldPositionY * worldSize) + cellWorldPositionX;
		if (cellWorldPositionI < worldSquareSize)
		{
			if (game.animals[animalIndex].body[cellIndex].organ != MATERIAL_NOTHING)
			{
				game.world[cellWorldPositionI].pheromoneChannel = 13;
				game.world[cellWorldPositionI].pheromoneIntensity = 1.0f;
				if (game.world[cellWorldPositionI].material == MATERIAL_NOTHING)
				{
					game.world[cellWorldPositionI].material = MATERIAL_FOOD;
				}
				if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_BONE)
				{
					game.world[cellWorldPositionI].material = MATERIAL_BONE;
				}
			}
		}
	}
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionI)
{
	if (cellWorldPositionI < worldSquareSize && game.world[cellWorldPositionI].identity >= 0 )
	{
		if (!game.animals[animalIndex].retired)
		{
			unsigned int cellIndex = game.world[cellWorldPositionI].occupyingCell;

			if (game.animals[animalIndex].body[cellIndex].worldPositionI == cellWorldPositionI)
			{
				if (game.animals[animalIndex].body[cellIndex].damage < 1.0f)
				{
					return cellIndex;
				}
			}
		}
	}
	return -1;
}


Color whatColorIsThisSquare(  unsigned int worldI)
{
	Color displayColor = color_black;
	int viewedAnimal = -1;
	unsigned int animalIndex = game.world[worldI].identity;
	unsigned int occupyingCell = 0;
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		occupyingCell = isAnimalInSquare(  animalIndex , worldI    );
		if (occupyingCell != -1)
		{
			viewedAnimal = animalIndex;
		}
	}
	if (viewedAnimal != -1)
	{
		if ( game.animals[viewedAnimal].body[occupyingCell].damage > 0.5f || organVisible(game.animals[viewedAnimal].body[occupyingCell].organ ) )
		{
			displayColor = organColors(game.animals[viewedAnimal].body[occupyingCell].organ );
		}
		else
		{
			displayColor = game.animals[viewedAnimal].body[occupyingCell].color;
		}
		if (viewedAnimal == game.selectedAnimal) // highlight selected animal.
		{
			displayColor =  addColor(displayColor, tint_selected);
		}
	}
	else
	{
		Color materialColor ;
		if (game.world[worldI].material == MATERIAL_GRASS )
		{
			materialColor = game.world[worldI].grassColor;
		}
		else
		{
			materialColor = materialColors(game.world[worldI].material);
		}
		displayColor = filterColor( materialColors(game.world[worldI].terrain) ,  materialColor);		// you can see the three material layers in order, wall then material then floor.
		Color wallColor = addColor(materialColors(game.world[worldI].wall), tint_wall);
		displayColor = filterColor( displayColor, wallColor  );
	}
	displayColor = multiplyColor(displayColor, game.world[worldI].light);
	return displayColor;
}

float getNormalisedHeight(unsigned int worldPositionI)
{
	float answer =   game.world[worldPositionI].height / (worldSize);
	return answer;
}

void computeLight(unsigned int worldPositionI, float xLightAngle, float yLightAngle)
{
	Vec_f2 slope = getTerrainSlope( worldPositionI);
	float xSurfaceDifference = (xLightAngle - slope.x);
	float ySurfaceDifference = (yLightAngle - slope.y);
	if (worldPositionI + worldSize < worldSquareSize)
	{
		float brightness = 1.0f - ((xSurfaceDifference + ySurfaceDifference) / (2.0f * 3.14f));
		if (brightness < 0.2f) { brightness = 0.2f;}
		game.world[worldPositionI].light = multiplyColorByScalar(color_white, brightness);
	}
}

void smoothSquare(unsigned int worldPositionI, float strength)
{
	float avg = game.world[worldPositionI].height;
	unsigned int count = 1;
	for (int n = 0; n < nNeighbours; ++n)
	{
		unsigned int neighbour = worldPositionI + neighbourOffsets[n];
		if (neighbour < worldSquareSize)
		{
			avg += game.world[neighbour].height;
			count ++;
		}
	}
	avg = avg / count;
	for (int n = 0; n < nNeighbours; ++n)
	{
		unsigned int neighbour = worldPositionI + neighbourOffsets[n];
		if (neighbour < worldSquareSize)
		{
			game.world[neighbour].height += (avg - game.world[neighbour].height) * strength;
		}
	}
}

void smoothHeightMap(unsigned int passes, float strength)
{
	for (int pass = 0; pass < passes ; ++pass)
	{
		for (unsigned int i = 0; i < worldSquareSize; ++i)
		{
			smoothSquare(i,  strength);
		}
		for (unsigned int i = worldSquareSize - 1; i > 0; --i)
		{
			smoothSquare(i,  strength);
		}
	}
}

void updateMap()
{
	ZoneScoped;
	unsigned int mapUpdateFidelity = worldSquareSize / 25000;
	for (unsigned int i = 0; i < mapUpdateFidelity; ++i)
	{
		unsigned int randomX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomY = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomI = (randomY * worldSize) + randomX;
		if (randomI < worldSquareSize)// slowly reduce pheromones over time.
		{
			if (game.world[randomI].pheromoneIntensity > 0.2f)
			{
				game.world[randomI].pheromoneIntensity -= 0.2f;
			}
			else
			{
				game.world[randomI].pheromoneChannel = -1;
			}
			if (game.world[randomI].height < seaLevel)
			{
				if (game.world[randomI].wall == MATERIAL_NOTHING)
				{
					game.world[randomI].wall = MATERIAL_WATER;
				}
			}
			else
			{
				if (game.world[randomI].wall == MATERIAL_WATER)
				{
					game.world[randomI].wall = MATERIAL_NOTHING;
				}
			}
			if (game.world[randomI].material == MATERIAL_FIRE)
			{
				for (int i = 0; i < nNeighbours; ++i)
				{
					unsigned int neighbour = randomI + neighbourOffsets[i];
					if (neighbour < worldSquareSize)
					{
						if (game.world[neighbour].material == MATERIAL_GRASS)
						{
							game.world[randomI].material = MATERIAL_FIRE;
						}
					}
				}
				if (extremelyFastNumberFromZeroTo(1) == 0)
				{
					game.world[randomI].material = MATERIAL_NOTHING;
				}
				else
				{
					unsigned int neighbour = randomI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
					if (neighbour < worldSquareSize)
					{
						game.world[neighbour].material = MATERIAL_FIRE;
					}
				}
			}
			if (game.world[randomI].material == MATERIAL_GRASS)
			{
				for (int n = 0; n < nNeighbours; ++n)
				{
					unsigned int neighbour = randomI + neighbourOffsets[n];
					if (neighbour < worldSquareSize)
					{
						if (game.world[neighbour].material == MATERIAL_NOTHING && !materialBlocksMovement(game.world[neighbour].wall ) &&  materialSupportsGrowth(game.world[neighbour].terrain ) )
						{
							float growthChance =  0.0f;// grow speed proportional to light brightness
							if (RNG() < game.world[neighbour].light.a)
							{
								game.world[neighbour].material = MATERIAL_GRASS;
								game.world[neighbour].grassColor = game.world[randomI].grassColor;
								game.world[neighbour].grassColor.r += (RNG() - 0.5f) * 0.1f;
								game.world[neighbour].grassColor.g += (RNG() - 0.5f) * 0.1f;
								game.world[neighbour].grassColor.b += (RNG() - 0.5f) * 0.1f;
								game.world[neighbour].grassColor = clampColor(game.world[neighbour].grassColor);
							}
						}
					}
				}
			}
			if ( materialDegrades( game.world[randomI].material) )
			{
				game.world[randomI].material = MATERIAL_NOTHING;
			}
		}
	}
}

int defenseAtWorldPoint(unsigned int animalIndex, unsigned int cellWorldPositionI)
{
	int nBones = 0;
	for (unsigned int n = 0; n < nNeighbours; ++n)
	{
		unsigned int worldNeighbour = cellWorldPositionI + neighbourOffsets[n];
		int occupyingCell = isAnimalInSquare(animalIndex, worldNeighbour) ;
		if ( occupyingCell >= 0)
		{
			if (game.animals[animalIndex].body[occupyingCell].organ == ORGAN_BONE)
			{
				nBones++;
			}
		}
	}
	return nBones;
}

Vec_i2 getMousePositionRelativeToAnimal(unsigned int animalIndex)
{
	int newPosX = game.mousePositionX -   ( game.cameraPositionX  - game.animals[animalIndex].uPosX);
	int newPosY = game.mousePositionY -   ( game.cameraPositionY  - game.animals[animalIndex].uPosY);
	return Vec_i2(newPosX, newPosY);
}

void paletteCallback( int gunIndex, int shooterIndex )	// add the selected organ to the selected animal
{
	if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals)
	{
		if (game.paletteSelectedOrgan >= 0 && game.paletteSelectedOrgan < numberOfOrganTypes)
		{
			Vec_i2 newpos = getMousePositionRelativeToAnimal(game.selectedAnimal);
			appendCell(game.selectedAnimal, game.paletteSelectedOrgan, newpos);
		}
	}
}

void paletteEraseAtMouse()
{
	if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals)
	{
		Vec_i2 newpos = getMousePositionRelativeToAnimal(game.selectedAnimal);
		for (int i = 0; i < game.animals[game.selectedAnimal].cellsUsed; ++i)
		{
			if (game.animals[game.selectedAnimal].body[i].localPosX == newpos.x &&
			        game.animals[game.selectedAnimal].body[i].localPosY == newpos.y )
			{
				eliminateCell(game.selectedAnimal, i);
			}
		}
	}
}

void drawPalette()
{
	int closestValue = 1000;
	unsigned int tempClosestToMouse = 0;
	for (int i = 0; i < numberOfOrganTypes; ++i)
	{
		unsigned int paletteX = i % paletteWidth;
		unsigned int paletteY = i / paletteWidth;
		int paletteFinalX = paletteMenuX + (paletteX * paletteSpacing * 6);
		int paletteFinalY = paletteMenuY + (paletteY * paletteSpacing);
		if (i == game.paletteSelectedOrgan)
		{
			printText2D(  std::string("X ") +  tileShortNames(i) , paletteFinalX, paletteFinalY, paletteTextSize);
		}
		else
		{
			printText2D(   tileShortNames(i) , paletteFinalX, paletteFinalY, paletteTextSize);
		}
	}
}

void communicationComputerCallback( int gunIndex, int shooterIndex)
{
	if (gunIndex == 9) { game.computerdisplays[0] = !game.computerdisplays[0] ;}
	if (gunIndex == 10) { game.computerdisplays[1] = !game.computerdisplays[1] ;}
	if (gunIndex == 11) { game.computerdisplays[2] = !game.computerdisplays[2] ;}
	if (gunIndex == 12 ) { game.computerdisplays[3] = !game.computerdisplays[3] ;}
	if (gunIndex == 13) { game.computerdisplays[4] = !game.computerdisplays[4] ;}
}

void spillBlood(unsigned int worldPositionI)
{
	if (game.world[worldPositionI].material == MATERIAL_NOTHING)
	{
		game.world[worldPositionI].material = MATERIAL_BLOOD;
	}
	else
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			unsigned int neighbour = worldPositionI += neighbourOffsets[i];
			if ( neighbour < worldSquareSize)
			{
				if (game.world[neighbour].material == MATERIAL_NOTHING)
				{
					game.world[neighbour].material = MATERIAL_BLOOD;
					break;
				}
			}
		}
	}
}

// return true if you blow the limb off, false if its still attached.
bool hurtAnimal(unsigned int animalIndex, unsigned int cellIndex, float amount)
{
	unsigned int cellWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
	float defense = defenseAtWorldPoint(game.world[cellWorldPositionI].identity, cellWorldPositionI);
	if (defense > 0)
	{
		amount = amount / defense;
	}
	game.animals[animalIndex].body[cellIndex].damage += amount;
	spillBlood(cellWorldPositionI);
	int painCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_PAIN);
	if (painCell >= 0)
	{
		game.animals[animalIndex].body[painCell].signalIntensity += amount;
	}
	if (game.animals[animalIndex].body[cellIndex].damage > 1.0f)
	{
		game.animals[animalIndex].damageReceived++;
		game.animals[animalIndex].mass--;
		return true;
	}
	return false;
}

void knifeCallback( int gunIndex, int shooterIndex )
{
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	if (game.cursorAnimal >= 0 && game.cursorAnimal < numberOfAnimals)
	{
		int occupyingCell = isAnimalInSquare(game.cursorAnimal, worldCursorPos);
		if ( occupyingCell >= 0)
		{
			hurtAnimal(game.cursorAnimal, occupyingCell, 0.3f);
		}
	}
}

void exampleGunCallback( int gunIndex, int shooterIndex)
{
	if (gunIndex >= 0)
	{
		unsigned int range = 1000;
		float bulletPosX = game.animals[gunIndex].fPosX;// trace a line from the gun and destroy any tissue found on the way.
		float bulletPosY = game.animals[gunIndex].fPosY;
		float angle      =  game.animals[gunIndex].fAngle;
		for (int i = 0; i < range; ++i)
		{
			bulletPosX += 1.0f * (cos(angle));
			bulletPosY += 1.0f * (sin(angle));
			unsigned int ubulletPosX = bulletPosX;
			unsigned int ubulletPosY = bulletPosY;
			unsigned int shootWorldPosition = (ubulletPosY * worldSize) + ubulletPosX;
			if (shootWorldPosition < worldSquareSize)
			{
				if (game.world[shootWorldPosition].identity >= 0 && game.world[shootWorldPosition].identity != gunIndex && game.world[shootWorldPosition].identity < numberOfAnimals && game.world[shootWorldPosition].identity != shooterIndex)
				{
					unsigned int shotOffNub = isAnimalInSquare(game.world[shootWorldPosition].identity, shootWorldPosition);
					if (shotOffNub >= 0 && shotOffNub < animalSquareSize)
					{
						hurtAnimal(game.world[shootWorldPosition].identity, shotOffNub, 0.35f + RNG());
					}
				}
				if (game.world[shootWorldPosition].wall == MATERIAL_NOTHING )
				{
					game.world[shootWorldPosition].wall = MATERIAL_SMOKE;
				}
				if ( materialBlocksMovement( game.world[shootWorldPosition].wall)  )
				{
					game.world[shootWorldPosition].wall = MATERIAL_NOTHING;
					break;
				}
			}
		}
	}
}

void lighterCallback( int gunIndex, int shooterIndex )
{
	if (gunIndex >= 0)
	{
		int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
		int cursorPosY = game.cameraPositionY + game.mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;

		if ( game.world[worldCursorPos].material == MATERIAL_NOTHING ||  game.world[worldCursorPos].material == MATERIAL_GRASS )
		{
			game.world[worldCursorPos].material = MATERIAL_FIRE;
		}
	}
}

void healAllDamage(unsigned int animalIndex)
{
	if (animalIndex < numberOfAnimals)
	{
		game.animals[animalIndex].damageReceived = 0;
		for (int i = 0; i < animalSquareSize; ++i)
		{
			game.animals[animalIndex].body[i].damage = 0.0f;
		}
	}
}

int getGrabbableItem(unsigned int animalIndex, unsigned int cellIndex)
{
	int grabArea = 5;
	int result = -1;
	bool gotSomething = false;
	for (int y = -grabArea; y < grabArea; ++y)
	{
		for (int x = -grabArea; x < grabArea; ++x)
		{
			unsigned int neighbour = game.animals[animalIndex].body[cellIndex].worldPositionI + (y * worldSize) + x;
			if (neighbour < worldSquareSize)
			{
				if (game.world[neighbour].identity >= 0 && game.world[neighbour].identity != animalIndex && game.world[neighbour].identity < numberOfAnimals)
				{
					int targetLocalPositionI = isAnimalInSquare( game.world[neighbour].identity, neighbour);
					if (targetLocalPositionI >= 0)
					{
						bool grabbedByAnotherGrabber = false;// finally, make sure the item is not grabbed by another of your own grabbers.
						for (unsigned int cellIndexB = 0; cellIndexB < game.animals[animalIndex].cellsUsed; cellIndexB++)                                      // place animalIndex on grid and attack / eat. add captured energy
						{
							if (game.animals[animalIndex].body[cellIndexB].organ == ORGAN_GRABBER)
							{
								if (game.animals[animalIndex].body[cellIndexB].grabbedCreature == game.world[neighbour].identity )
								{
									grabbedByAnotherGrabber = true;
									break;
								}
							}
						}
						if (!grabbedByAnotherGrabber)
						{
							gotSomething = true;
							result = game.world[neighbour].identity;
							break;
						}
					}
				}
			}
		}
		if (gotSomething)
		{
			break;
		}
	}
	return result;
}

// the animal is a grid of living cells that do different things. this function describes what they do each turn.
void organs_all()
{
	ZoneScoped;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!game.animals[animalIndex].retired)
		{
			float totalLiver = 0;
			unsigned int totalGonads = 0;
			float highestIntensity = 0.0f;
			for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; cellIndex++)
			{
				unsigned int cellWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
				unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
				unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;
				if (cellWorldPositionI >= worldSquareSize) {continue;}
				if (game.animals[animalIndex].body[cellIndex].damage > 1.0f) { continue;}
				unsigned int organ = game.animals[animalIndex].body[cellIndex].organ;
				switch (organ)
				{

				case ORGAN_SENSOR_AGE:
				{
					if (game.animals[animalIndex].lifespan > 0.0f)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = game.animals[animalIndex].age / game.animals[animalIndex].lifespan;
					}
					break;

				}

				case ORGAN_GRABBER:
				{
					if (animalIndex != game.playerCreature)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
							{
								unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
								}
							}
						}
					}
					else
					{
						int potentialGrab = getGrabbableItem(game.playerCreature, cellIndex);// check if there is anything grabbable.
						if (potentialGrab >= 0)
						{
							game.playerCanPickup = true;
							game.playerCanPickupItem = potentialGrab;
						}
						else
						{
							game.playerCanPickup = false;
							game.playerCanPickupItem = -1;
						}
					}

					if (game.animals[animalIndex].body[cellIndex].signalIntensity  >= 1.0f && game.animals[animalIndex].body[cellIndex].grabbedCreature  == -1)// if greater than 0, grab.
					{
						int potentialGrab = getGrabbableItem (animalIndex, cellIndex);
						if (potentialGrab >= 0)
						{
							game.animals[animalIndex].body[cellIndex].grabbedCreature = potentialGrab;//game.world[neighbour].identity;
							game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
							if (  game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].isMachine  )// some machines have stuff that activates on pickup
							{
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) == (MACHINECALLBACK_HOSPITAL))
								{
									game.palette = true;
									healAllDamage(game.playerCreature);
								}
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_ECOLOGYCOMPUTER)
								{
									game.ecologyComputerDisplay = true;
								}
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_NEUROGLASSES )
								{
									game.visualizer = VISUALIZER_NEURALACTIVITY;
								}
								if ( game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback ==   MACHINECALLBACK_TRACKERGLASSES  )
								{
									game.visualizer = VISUALIZER_TRACKS;
								}
							}
						}
					}
					if (game.animals[animalIndex].body[cellIndex].grabbedCreature >= 0 )// if there is a grabbed creature, adjust its position to the grabber.
					{
						game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosX = cellWorldPositionX;
						game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosY = cellWorldPositionY;
						game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosX = cellWorldPositionX;
						game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosY = cellWorldPositionY;
						game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].position = cellWorldPositionI;

						// also, if grabbed by the player, adjust the angle of the grabbed object so it points at the mouse cursor. for aiming weapons.
						float fposx = cellWorldPositionX;
						float fposy = cellWorldPositionY;
						float fmousePositionX = game.mousePositionX;
						float fmousePositionY = game.mousePositionY;
						float angleToCursor = atan2(   fmousePositionY - (  game.cameraPositionY - fposy)  ,  fmousePositionX - (game.cameraPositionX - fposx));
						game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].fAngle = angleToCursor;

						if (  game.animals[animalIndex].body[cellIndex].signalIntensity  <= -1.0f)
						{
							if (  game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].isMachine  )
							{
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_HOSPITAL)
								{
									game.palette = false;
								}
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_MESSAGECOMPUTER)
								{
									for (int i = 0; i < 5; ++i)
									{
										game.computerdisplays[i] = false;
									}
								}
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_ECOLOGYCOMPUTER)
								{
									game.ecologyComputerDisplay = false;
								}
								if ((game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_NEUROGLASSES ||
								        game.animals[game.animals[game.playerCreature].body[cellIndex].grabbedCreature].machineCallback ==   MACHINECALLBACK_TRACKERGLASSES  )
								{
									game.visualizer = VISUALIZER_TRUECOLOR;
								}
							}
							game.animals[animalIndex].body[cellIndex].grabbedCreature = -1;
							game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						}
					}
					break;
				}

				case ORGAN_SENSOR_PAIN:
				{
					game.animals[animalIndex].body[cellIndex].signalIntensity *= 0.95f;
				}

				case ORGAN_SENSOR_HUNGER:
				{
					if (game.animals[animalIndex].maxEnergy > 0.0f)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = game.animals[animalIndex].energy / game.animals[animalIndex].maxEnergy;
					}
					break;
				}

				case ORGAN_SENSOR_BIRTHPLACE:
				{
					if (game.animals[animalIndex].birthLocation > 0 && game.animals[animalIndex].birthLocation < worldSquareSize)
					{
						float targetWorldPositionX =   game.animals[animalIndex]. birthLocation % worldSize;  ;
						float targetWorldPositionY =   game.animals[animalIndex]. birthLocation / worldSize;  ;
						float fdiffx = targetWorldPositionX - game.animals[animalIndex].fPosX;
						float fdiffy = targetWorldPositionY - game.animals[animalIndex].fPosY;
						float targetAngle = atan2( fdiffy, fdiffx );
						game.animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
					}
					break;
				}

				case ORGAN_SENSOR_PARENT:
				{
					if (game.animals[animalIndex].parentIdentity >= 0 && game.animals[animalIndex].parentIdentity < numberOfAnimals)
					{
						if (!( game.animals[  game.animals[animalIndex].parentIdentity   ]  .retired  )   )
						{
							float targetWorldPositionX = game.animals[  game.animals[animalIndex].parentIdentity   ]  .fPosX;
							float targetWorldPositionY = game.animals[  game.animals[animalIndex].parentIdentity   ]  .fPosY;
							float fdiffx = targetWorldPositionX - game.animals[animalIndex].fPosX;
							float fdiffy = targetWorldPositionY - game.animals[animalIndex].fPosY;
							float targetAngle = atan2( fdiffy, fdiffx );
							game.animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
						}
					}
					break;
				}

				case ORGAN_SENSOR_LAST_STRANGER:
				{
					if (game.animals[animalIndex].lastTouchedStranger >= 0 && game.animals[animalIndex].lastTouchedStranger < numberOfAnimals)
					{
						if (!( game.animals[  game.animals[animalIndex].lastTouchedStranger   ]  .retired  )   )
						{
							float targetWorldPositionX = game.animals[  game.animals[animalIndex].lastTouchedStranger   ]  .fPosX;
							float targetWorldPositionY = game.animals[  game.animals[animalIndex].lastTouchedStranger   ]  .fPosY;
							float fdiffx = targetWorldPositionX - game.animals[animalIndex].fPosX;
							float fdiffy = targetWorldPositionY - game.animals[animalIndex].fPosY;
							float targetAngle = atan2( fdiffy, fdiffx );
							game.animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
						}
					}
					break;
				}

				case ORGAN_SENSOR_LAST_KIN:
				{
					if (game.animals[animalIndex].lastTouchedKin >= 0 && game.animals[animalIndex].lastTouchedKin < numberOfAnimals)
					{
						if (!( game.animals[  game.animals[animalIndex].lastTouchedKin   ]  .retired  )   )
						{
							float targetWorldPositionX = game.animals[  game.animals[animalIndex].lastTouchedKin   ]  .fPosX;
							float targetWorldPositionY = game.animals[  game.animals[animalIndex].lastTouchedKin   ]  .fPosY;
							float fdiffx = targetWorldPositionX - game.animals[animalIndex].fPosX;
							float fdiffy = targetWorldPositionY - game.animals[animalIndex].fPosY;
							float targetAngle = atan2( fdiffy, fdiffx );
							game.animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
						}
					}
					break;
				}

				case ORGAN_LUNG:
				{
					if (game.world[cellWorldPositionI].wall != MATERIAL_WATER)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = baseLungCapacity;
					}
					else
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity -= 0.01f;
					}
					if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						int otherLung = getRandomCellOfType(animalIndex, ORGAN_LUNG);
						if (otherLung < 0)
						{
							otherLung = getRandomCellOfType(animalIndex, ORGAN_GILL); // compatible with gills
						}
						if (otherLung >= 0)// if there is another reservoir of air, equalize them.
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity +=	game.animals[animalIndex].body[otherLung].signalIntensity;
							game.animals[animalIndex].body[cellIndex].signalIntensity *= 0.5f;
							game.animals[animalIndex].body[otherLung].signalIntensity = game.animals[animalIndex].body[cellIndex].signalIntensity;
						}
						if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
						{
							game.animals[animalIndex].damageReceived++;
						}
					}
					break;
				}
				case ORGAN_GILL:
				{
					if (game.world[cellWorldPositionI].wall == MATERIAL_WATER)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = baseLungCapacity;
					}
					else
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity -= 0.01f;
					}
					if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						int otherLung = getRandomCellOfType(animalIndex, ORGAN_GILL);
						if (otherLung < 0)
						{
							otherLung = getRandomCellOfType(animalIndex, ORGAN_LUNG); // compatible with lungs
						}
						if (otherLung >= 0)	// if there is another reservoir of air, equalize them.
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity +=	game.animals[animalIndex].body[otherLung].signalIntensity;
							game.animals[animalIndex].body[cellIndex].signalIntensity *= 0.5f;
							game.animals[animalIndex].body[otherLung].signalIntensity = game.animals[animalIndex].body[cellIndex].signalIntensity;
						}
						if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
						{
							game.animals[animalIndex].damageReceived++;
						}
					}
					break;
				}
				case ORGAN_MEMORY_RX:
				{
					break;	// don't need to do anything, the tx part does all the work.
				}
				case ORGAN_MEMORY_TX:
				{
					// sum inputs. if exceeding a threshold, find a corresponding memory RX cell and copy it the input sum.
					// because nothing else is altering its signal intensity, it will keep that value until changed again.
					game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
						{
							unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
							}
						}
					}
					if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f || game.animals[animalIndex].body[cellIndex].signalIntensity  < -1.0f)
					{
						std::list<unsigned int> cellsOfType;
						unsigned int found = 0;
						for (int i = 0; i < animalSquareSize; ++i)
						{
							if (game.animals[animalIndex].body[i].organ == ORGAN_MEMORY_RX)
							{
								cellsOfType.push_back(i);
								found++;
							}
						}
						int correspondingCellRX = -1;
						if (found > 0)
						{
							std::list<unsigned int>::iterator iterator = cellsOfType.begin();
							for (iterator = cellsOfType.begin(); iterator != cellsOfType.end(); ++iterator)
							{
								if ( game.animals[animalIndex].body[(*iterator)].speakerChannel == game.animals[animalIndex].body[cellIndex].speakerChannel  )
								{
									correspondingCellRX = (*iterator);
								}
							}
						}
						if (correspondingCellRX >= 0 && correspondingCellRX < animalSquareSize)
						{
							game.animals[animalIndex].body[correspondingCellRX].signalIntensity = game.animals[animalIndex].body[cellIndex].signalIntensity ;
						}
					}
					break;
				}

				case ORGAN_SENSOR_PHEROMONE:
				{
					game.animals[animalIndex].body[cellIndex].signalIntensity = 0;
					if (game.world[cellWorldPositionI].pheromoneChannel >= 0)
					{
						if (game.animals[animalIndex].body[cellIndex]. speakerChannel ==   game.world[cellWorldPositionI].pheromoneChannel)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity  = game.world[cellWorldPositionI].pheromoneIntensity;
						}
					}
					break;
				}

				case ORGAN_EMITTER_PHEROMONE:
				{
					game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
						{
							unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
							}
						}
					}
					game.world[cellWorldPositionI].pheromoneChannel = game.animals[animalIndex].body[cellIndex]. speakerChannel ;
					game.world[cellWorldPositionI].pheromoneIntensity = game.animals[animalIndex].body[cellIndex].signalIntensity;
					break;
				}

				case ORGAN_SPEAKER:
				{
					if ( game.animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
							{
								unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
								}
							}
						}
						if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
						}
						else if (game.animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
						}
						game.speakerChannels[  game.animals[animalIndex].body[cellIndex].speakerChannel ] += game.animals[animalIndex].body[cellIndex].signalIntensity ;
					}
					else
					{
						game.animals[animalIndex].body[cellIndex].speakerChannel = 0;
					}
					break;
				}

				case ORGAN_SENSOR_EAR:
				{
					if (game.animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = game.speakerChannelsLastTurn[ game.animals[animalIndex].body[cellIndex].speakerChannel ];
					}
					else
					{
						game.animals[animalIndex].body[cellIndex].speakerChannel = 0;
					}
					break;
				}

				case ORGAN_SENSOR_TRACKER:
				{
					game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					if ( game.world [cellWorldPositionI].identity != animalIndex )
					{
						game.animals[animalIndex].body[cellIndex].signalIntensity = game.world[cellWorldPositionI].trail;
					}
					break;
				}

				case ORGAN_SENSOR_BODYANGLE:
				{
					game.animals[animalIndex].body[cellIndex].signalIntensity = game.animals[animalIndex].fAngle;
					break;
				}

				case ORGAN_SENSOR_EYE:
				{
					Vec_f2 eyeLook = Vec_f2(game.animals[animalIndex].body[cellIndex].eyeLookX , game.animals[animalIndex].body[cellIndex].eyeLookY);
					Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), game.animals[animalIndex].fAngleSin, game.animals[animalIndex].fAngleCos, eyeLook);
					unsigned int eyeLookWorldPositionX = cellWorldPositionX + rotatedEyeLook.x;
					unsigned int eyeLookWorldPositionY = cellWorldPositionY + rotatedEyeLook.y;
					unsigned int eyeLookWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
					Color receivedColor = whatColorIsThisSquare(eyeLookWorldPositionI);
					Color perceivedColor = multiplyColor( receivedColor, game.animals[animalIndex].body[cellIndex].color  );
					game.animals[animalIndex].body[cellIndex].signalIntensity = colorAmplitude(perceivedColor );
					break;
				}

				case ORGAN_SENSOR_TOUCH:
				{
					game.animals[animalIndex].body[cellIndex].signalIntensity = 0;
					for (int i = 0; i < nNeighbours; ++i)
					{
						unsigned int neighbour = cellWorldPositionI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							if (game.world[neighbour].identity >= 0)
							{
								if (isAnimalInSquare( game.world[neighbour].identity , neighbour ))
								{
									game.animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
								else if (game.world[neighbour].material != MATERIAL_NOTHING)
								{
									game.animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
							}
						}
					}
					unsigned int touchedAnimal = game.world[cellWorldPositionI].identity;
					if (touchedAnimal < numberOfAnimals)
					{
						if (touchedAnimal >= 0)
						{
							if (touchedAnimal != animalIndex)
							{
								if (isAnimalInSquare( touchedAnimal , cellWorldPositionI ))
								{
									game.animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
								else if (game.world[cellWorldPositionI].material != MATERIAL_NOTHING)
								{
									game.animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
							}
						}
					}
					break;
				}

				case ORGAN_NEURON:
				{
					float sum = 0.0f; // go through the list of connections and sum their values.
					sum += neuralNoise * ((RNG() - 0.5f) * 2); // add noise all throughout the brain, this makes everything more robust and lifelike
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
						{
							unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								float connected_signal = game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
								sum += connected_signal;
							}
						}
					}
					game.animals[animalIndex].body[cellIndex].signalIntensity = fast_sigmoid(sum);
					break;
				}

				case ORGAN_GONAD:
				{
					totalGonads++;
					if (doReproduction && game.animals[animalIndex].energyDebt <= 0.0f )
					{
						if (game.animals[animalIndex].energy > ((game.animals[animalIndex].mass / 2 ) + game.animals[animalIndex].offspringEnergy ))
						{
							if (cellWorldPositionI < worldSquareSize)
							{
								unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;
								int result = spawnAnimal( speciesIndex, game.animals[animalIndex], game.animals[animalIndex].position, true );
								if (result >= 0)
								{
									game.animals[animalIndex].body[cellIndex].organ = MATERIAL_NOTHING;
									game.animals[animalIndex].numberOfTimesReproduced++;
									game.animals[animalIndex].energy -= game.animals[animalIndex].offspringEnergy;
									game.animals[result].energy       =  game.animals[animalIndex].offspringEnergy;
									game.animals[result].parentIdentity       = animalIndex;
								}
							}
						}
					}
					break;
				}
				case ORGAN_LIVER :
				{
					totalLiver += 1.0f;
					break;
				}

				case ORGAN_MOUTH_VEG :
				{
					if (game.world[cellWorldPositionI].material == MATERIAL_GRASS)
					{
						game.animals[animalIndex].energy += grassEnergy ;
						game.world[cellWorldPositionI].material = MATERIAL_NOTHING;
					}
					break;
				}

				case ORGAN_MOUTH_SCAVENGE :
				{
					if (game.world[cellWorldPositionI].material == MATERIAL_FOOD)
					{
						game.animals[animalIndex].energy += foodEnergy ;
						game.world[cellWorldPositionI].material = MATERIAL_NOTHING;
					}
					break;
				}

				case ORGAN_MUSCLE :
				{
					if (doMuscles)
					{
						if (animalIndex != game.playerCreature)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;

							for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
							{
								if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
								{
									unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
									if (connected_to_cell < animalSquareSize)
									{
										game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
									}
								}
							}
						}
						if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
						}
						else if (game.animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
						}
						game.animals[animalIndex].fPosX += game.animals[animalIndex].body[cellIndex].signalIntensity * 10 * cos(game.animals[animalIndex].fAngle);
						game.animals[animalIndex].fPosY += game.animals[animalIndex].body[cellIndex].signalIntensity * 10 * sin(game.animals[animalIndex].fAngle);
						game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					}
					break;
				}

				case ORGAN_MUSCLE_STRAFE :
				{
					if (doMuscles)
					{
						if (animalIndex != game.playerCreature)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;	// go through the list of connections and sum their values.
							for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
							{
								if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
								{
									unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
									if (connected_to_cell < animalSquareSize)
									{
										game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
									}
								}
							}
						}
						if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
						}
						else if (game.animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
						}
						game.animals[animalIndex].fPosX += game.animals[animalIndex].body[cellIndex].signalIntensity * 10 * sin(game.animals[animalIndex].fAngle);// on the strafe muscle the sin and cos are reversed, that's all.
						game.animals[animalIndex].fPosY += game.animals[animalIndex].body[cellIndex].signalIntensity * 10 * cos(game.animals[animalIndex].fAngle);

						game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					}
					break;
				}

				case ORGAN_MUSCLE_TURN:
				{
					if (doMuscles)
					{
						if (animalIndex != game.playerCreature)
						{
							game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;// go through the list of connections and sum their values.
							for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
							{
								if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
								{
									unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
									if (connected_to_cell < animalSquareSize)
									{
										game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
									}
								}
							}
						}
						if (setOrSteerAngle)
						{
							game.animals[animalIndex].fAngle = (game.animals[animalIndex].body[cellIndex].signalIntensity ) ;
						}
						else
						{
							game.animals[animalIndex].fAngle += (game.animals[animalIndex].body[cellIndex].signalIntensity ) * 0.1f;
						}
						if (game.animals[animalIndex].fAngle > const_pi)
						{
							game.animals[animalIndex].fAngle -= 2 * const_pi;
						}
						if (game.animals[animalIndex].fAngle < -const_pi)
						{
							game.animals[animalIndex].fAngle += 2 * const_pi;
						}
						game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					}
					break;
				}
				}
			}
			game.animals[animalIndex].totalGonads = totalGonads;
			game.animals[animalIndex].maxEnergy = game.animals[animalIndex].mass + (totalLiver * liverStorage);
		}
	}

	for (unsigned int i = 0; i < numberOfSpeakerChannels; ++i)
	{
		game.speakerChannelsLastTurn [i] = game.speakerChannels[i];
		game.speakerChannels[i] = 0.0f;
	}
}

void move_all()
{
	ZoneScoped;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!game.animals[animalIndex].retired)
		{
			bool trailUpdate = false;// calculate direction of movement.
			float dAngle = 0.0f;
			if (game.animals[animalIndex].fPosX != game.animals[animalIndex].lastfposx || game.animals[animalIndex].fPosY != game.animals[animalIndex].lastfposy  )
			{
				float fdiffx =  game.animals[animalIndex].fPosX - game.animals[animalIndex].lastfposx;
				float fdiffy =  game.animals[animalIndex].fPosY - game.animals[animalIndex].lastfposy;
				dAngle = atan2(fdiffy, fdiffx);// use atan2 to turn the diff into an angle.
				dAngle -= 0.5 * const_pi;
				if (dAngle < const_pi)
				{
					dAngle += (2 * const_pi);
				}
				trailUpdate = true;
				game.animals[animalIndex].lastfposx = game.animals[animalIndex].fPosX;
				game.animals[animalIndex].lastfposy = game.animals[animalIndex].fPosY;
			}
			game.animals[animalIndex].fAngleCos = cos(game.animals[animalIndex].fAngle);
			game.animals[animalIndex].fAngleSin = sin(game.animals[animalIndex].fAngle);
			unsigned int newPosX  = game.animals[animalIndex].fPosX;
			unsigned int newPosY  = game.animals[animalIndex].fPosY;
			unsigned int newPosition  =  (newPosY * worldSize) + newPosX;
			if (newPosition < worldSquareSize)
			{
				if (  materialBlocksMovement( game.world[newPosition].wall ) )
				{
					game.animals[animalIndex].fPosX  = game.animals[animalIndex].uPosX;
					game.animals[animalIndex].fPosY  = game.animals[animalIndex].uPosY;
				}
				else
				{
					game.animals[animalIndex].uPosX  = game.animals[animalIndex].fPosX;
					game.animals[animalIndex].uPosY  = game.animals[animalIndex].fPosY;
				}
				game.animals[animalIndex].position = newPosition;
				if (false)
				{
					if (game.world[newPosition].temperature > game.animals[animalIndex].temp_limit_high)
					{
						game.animals[animalIndex].damageReceived += abs(game.world[newPosition].temperature  - game.animals[animalIndex].temp_limit_high);
					}
					if (game.world[newPosition].temperature < game.animals[animalIndex].temp_limit_low)
					{
						game.animals[animalIndex].damageReceived += abs(game.world[newPosition].temperature  - game.animals[animalIndex].temp_limit_low);
					}
				}
			}

			for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				if (taxIsByMass)
				{
					game.animals[animalIndex].energy -= taxEnergyScale *  organUpkeepCost(game.animals[animalIndex].body[cellIndex].organ); // * speciesEnergyOuts[speciesIndex] ;
				}
				bool okToStep = true;
				int rotatedX = game.animals[animalIndex].body[cellIndex].localPosX * game.animals[animalIndex].fAngleCos - game.animals[animalIndex].body[cellIndex].localPosY * game.animals[animalIndex].fAngleSin;
				int rotatedY = game.animals[animalIndex].body[cellIndex].localPosX * game.animals[animalIndex].fAngleSin + game.animals[animalIndex].body[cellIndex].localPosY * game.animals[animalIndex].fAngleCos ;
				unsigned int cellWorldPositionX = game.animals[animalIndex].uPosX + rotatedX;
				unsigned int cellWorldPositionY = game.animals[animalIndex].uPosY + rotatedY;
				unsigned int cellWorldPositionI = ((cellWorldPositionY * worldSize) + (cellWorldPositionX)) % worldSquareSize;
				if (game.world[cellWorldPositionI].identity >= 0 && game.world[cellWorldPositionI].identity != animalIndex && game.world[cellWorldPositionI].identity < numberOfAnimals)
				{
					int targetLocalPositionI = isAnimalInSquare( game.world[cellWorldPositionI].identity, cellWorldPositionI);
					if (targetLocalPositionI >= 0)
					{
						okToStep = false;
						if (!game.animals[   game.world[cellWorldPositionI].identity  ].isMachine)
						{
							unsigned int fellowSpeciesIndex = (game.world[cellWorldPositionI].identity) / numberOfAnimalsPerSpecies;
							if (fellowSpeciesIndex == speciesIndex)
							{
								game.animals[animalIndex].lastTouchedKin = game.world[cellWorldPositionI].identity;
							}
							else
							{
								game.animals[animalIndex].lastTouchedStranger = game.world[cellWorldPositionI].identity;
							}
							if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_WEAPON || game.animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_CARNIVORE )
							{
								if (game.animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
								{
									if (game.world[cellWorldPositionI].identity == game.animals[animalIndex].parentIdentity)
									{
										continue;
									}
								}
								bool meatAvailable = hurtAnimal(game.world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f );
								if (meatAvailable)
								{
									okToStep = true;
									game.animals[animalIndex].damageDone++;
									game.speciesAttacksPerTurn[speciesIndex] ++;
									if (game.animals[game.world[cellWorldPositionI].identity].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
									{
										if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_WEAPON)
										{
											if (game.world[cellWorldPositionI].material == MATERIAL_NOTHING)
											{
												game.world[cellWorldPositionI].material = MATERIAL_FOOD;
											}
										}
										if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_CARNIVORE)
										{
											game.animals[animalIndex].energy += foodEnergy ;
										}
									}
								}
							}
							else if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_PARASITE )
							{
								float amount = (game.animals[game.world[cellWorldPositionI].identity].energy) / animalSquareSize;
								float defense = defenseAtWorldPoint(game.world[cellWorldPositionI].identity, cellWorldPositionI);
								amount = amount / defense;
								game.animals[animalIndex].energy += amount;
								game.animals[game.world[cellWorldPositionI].identity].energy -= amount;
							}
						}
					}
					else
					{
						okToStep = true;
					}
				}
				if (okToStep)
				{
					if (game.world[cellWorldPositionI].identity == -1 && speciesIndex != 0)
					{
						if (game.world[cellWorldPositionI].material == MATERIAL_NOTHING || game.world[cellWorldPositionI].material == MATERIAL_GRASS)
						{
							game.world[cellWorldPositionI].material = MATERIAL_GRASS;
							game.world[cellWorldPositionI].grassColor =  addColor( color_green , multiplyColorByScalar(game.animals[animalIndex].identityColor, 0.5f ));//
						}
					}
					game.world[cellWorldPositionI].identity = animalIndex;
					game.world[cellWorldPositionI].occupyingCell = cellIndex;
					if (trailUpdate)
					{
						game.world[cellWorldPositionI].trail    = dAngle;
					}
					game.animals[animalIndex].body[cellIndex].worldPositionI = cellWorldPositionI;
				}
			}
		}
		else
		{
			game.animals[animalIndex].position =  game.animals[animalIndex].position % worldSquareSize;
		}
	}
}

void energy_all() // perform energies.
{
	ZoneScoped;
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		game.populationCountUpdates[i] = 0;
	}
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;
		if (!game.animals[animalIndex].retired && speciesIndex < numberOfSpecies)
		{
			game.populationCountUpdates[speciesIndex]++;
			game.animals[animalIndex].age++;
			if (game.animals[animalIndex].energy > game.animals[animalIndex].maxEnergy)
			{
				game.animals[animalIndex].energy = game.animals[animalIndex].maxEnergy;
			}
			if (game.animals[animalIndex].energyDebt > 0.0f)
			{
				if (game.animals[animalIndex].energy > (game.animals[animalIndex].maxEnergy / 2))
				{
					float repayment = game.animals[animalIndex].energy  - (game.animals[animalIndex].maxEnergy / 2)  ;
					game.animals[animalIndex].energyDebt -= repayment;
					game.animals[animalIndex].energy -= repayment;
				}
			}
			else
			{
				if (game.animals[animalIndex].parentAmnesty)
				{
					game.animals[animalIndex].parentAmnesty = false;
				}
			}
			bool execute = false;
			if (!immortality && !game.animals[animalIndex].isMachine) // reasons an npc can die
			{
				if (game.speciesPopulationCounts[speciesIndex] > (( numberOfAnimals / numberOfSpecies) / 4) && animalIndex != game.playerCreature) // only kill off weak game.animals if there is some population.
					if (game.animals[animalIndex].energy < 0.0f)
					{
						execute = true;
					}
				if (game.animals[animalIndex].age > game.animals[animalIndex].lifespan && animalIndex != game.playerCreature)
				{
					execute = true;
				}
				if (game.animals[animalIndex].totalGonads == 0)
				{
					execute = true;
				}
				if (game.animals[animalIndex].damageReceived > game.animals[animalIndex].mass)
				{
					execute = true;
				}
				if (game.animals[animalIndex].mass <= 0)
				{
					execute = true;
				}
			}
			if (execute)
			{
				if (animalIndex != game.adversary)
				{
					killAnimal( animalIndex);
				}
			}
			bool nominate = false;
			int animalScore = game.animals[animalIndex].damageDone + game.animals[animalIndex].damageReceived  + game.animals[animalIndex].numberOfTimesReproduced ;
			if (animalIndex != game.playerCreature && speciesIndex > 0) // player & player species cannot be nominated
			{
				if ( animalScore > game.championScore)
				{
					nominate = true;
				}
				else if (animalScore == game.championScore)
				{
					if (game.animals[animalIndex].energy > game.championEnergyScore)
					{
						nominate = true;
					}
				}
			}
			if (nominate)
			{
				game.championScore = animalScore;
				game.championEnergyScore = game.animals[animalIndex].energy;
				game.champion = game.animals[animalIndex];
			}
		}
	}
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		game.speciesPopulationCounts[i] = game.populationCountUpdates[i];
		game.speciesAttacksPerTurn[i] = 0;
	}
}

void computeAllAnimalsOneTurn()
{
	if (threading)
	{
		boost::thread t8{ organs_all };
		boost::thread t9{ move_all   };
		boost::thread t10{ energy_all };
		t10.join();
		t9.join();
		t8.join();
	}
	else
	{
		energy_all();
		organs_all();
		move_all();
	}
}

void camera()
{
	if (game.cameraTargetCreature >= 0)
	{
		game.cameraPositionX = game.animals[game.cameraTargetCreature].position % worldSize;
		game.cameraPositionY = game.animals[game.cameraTargetCreature].position / worldSize;
	}
	if (game.playerCreature >= 0 && game.cameraTargetCreature == game.playerCreature)	// if the player doesn't have any eyes, don't draw anything!
	{
		int playerEye = getRandomCellOfType(game.playerCreature, ORGAN_SENSOR_EYE);
		if (playerEye >= 0)
		{
			game.playerCanSee = true;
		}
		else
		{
			game.playerCanSee = false;
		}
	}
	else
	{
		game.playerCanSee = true; // if you're in the spectator view, basically not 'the player' or any other creature, you stil want to be able to see.
	}
	if (game.playerCanSee)
	{
		int viewFieldMax = +(viewFieldY / 2);
		int viewFieldMin = -(viewFieldX / 2);
		for ( int vy = viewFieldMin; vy < viewFieldMax; ++vy)
		{
			for ( int vx = viewFieldMin; vx < viewFieldMax; ++vx)
			{
				unsigned int x = (vx + game.cameraPositionX) % worldSize;
				unsigned int y = (vy + game.cameraPositionY) % worldSize;
				Color displayColor = color_black;
				unsigned int worldI = (y * worldSize) + x;
				if (worldI < worldSquareSize)
				{
					float fx = vx;
					float fy = vy;
					switch (game.visualizer)
					{
					case VISUALIZER_TRUECOLOR:
					{
						displayColor = whatColorIsThisSquare(worldI);
						drawTile( Vec_f2( fx, fy ), displayColor);
						break;
					}
					case VISUALIZER_TRACKS:
					{
						displayColor = color_grey;
						float amount = 0.5f;
						if (game.world[worldI].wall != MATERIAL_NOTHING)
						{
							amount -= 0.25f;
						}
						if (game.world[worldI].material != MATERIAL_NOTHING)
						{
							amount -= 0.0625f;
						}
						if (game.world[worldI].identity < numberOfAnimals && game.world[worldI].identity >= 0)
						{
							if (game.animals[ game.world[worldI].identity].isMachine  )
							{
								displayColor = color_white;
								drawTile( Vec_f2( fx, fy ), displayColor);
							}
							displayColor = game.animals[ game.world[worldI].identity ].identityColor;
							drawPointerTriangle( Vec_f2( fx, fy ), displayColor, game.world[worldI].trail );
						}
						break;
					}
					case VISUALIZER_NEURALACTIVITY:
					{
						displayColor = color_grey;
						float amount = 0.5f;
						if (game.world[worldI].wall != MATERIAL_NOTHING)
						{
							amount -= 0.25f;
						}
						if (game.world[worldI].material != MATERIAL_NOTHING)
						{
							amount -= 0.0625f;
						}
						if (game.world[worldI].identity < numberOfAnimals && game.world[worldI].identity >= 0)
						{
							int occupyingCell = isAnimalInSquare(game.world[worldI].identity, worldI);
							if ( occupyingCell >= 0)
							{
								unsigned int organ = game.animals[ game.world[worldI].identity] .body[occupyingCell].organ;
								if (organIsANeuron(  organ ) || organIsASensor(organ))
								{
									amount = game.animals[game.world[worldI].identity].body[occupyingCell].signalIntensity ; //* 2.0f;
								}
								if (game.animals[game.world[worldI].identity].isMachine)
								{
									amount = 1.0f;
								}
							}
						}
						displayColor.r = (amount * 0.5f) + 0.5f ; // map 1,-1 to 1,0
						displayColor.g = (amount * 0.5f) + 0.5f;
						displayColor.b = (amount * 0.5f) + 0.5f;
						drawTile( Vec_f2( fx, fy ), displayColor);
						break;
					}
					}
				}
			}
		}
	}

	Color displayColor = color_white;// draw the cursor.
	Vec_f2 worldMousePos = Vec_f2( game.mousePositionX, game.mousePositionY);
	drawTile( worldMousePos, displayColor);
}

void displayComputerText()
{
	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;
	if (game.ecologyComputerDisplay)
	{
		for (int i = 0; i < numberOfSpecies; ++i)
		{
			printText2D(   std::string("Species ") + std::to_string(i) +   std::string(" pop. " + std::to_string(game.speciesPopulationCounts[i])) + " hits " + std::to_string(game.speciesAttacksPerTurn[i]) , menuX, menuY, textSize);
			menuY -= spacing;
		}
		menuY -= spacing;
		printText2D(   std::string("Grass energy: ") + std::to_string(grassEnergy)  + std::string(", meat energy: ") + std::to_string(foodEnergy)   , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Resting tax: ") + std::to_string(taxEnergyScale)  + std::string(", movement tax: ") + std::to_string(movementEnergyScale) + std::string(", growth tax: ") + std::to_string(growthEnergyScale)   , menuX, menuY, textSize);
		menuY -= spacing;
	}
// First terminal is near the player at the start.  Explain how to pick up and use items. The player is given a pistol.
// The second terminal contains a hospital and explains how anatomy works in the game. The player is encouraged to add a gill to themselves to allow breathing underwater. It is located at the shoreline
// The 4th terminal is under water in a teeming coral reef. It contains tracker glasses that allow the game.adversary to be identified and found.
// The game.adversary is killed and life no longer has a source, but will continue existing where it does. The game.adversary drops neuro glasses that the player needs to edit brain connections.
// If all life in the simulation is destroyed, a message will become available stating that the game.animals broke out into the real game.world and caused widespread disaster
	if (game.computerdisplays[0])
	{
		printText2D(   std::string("game.animals are groups of tiles that move around. Each tile has a dedicated purpose.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Your body is made this way too. ") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("If your tiles are damaged, you will lose the tile's function,") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("which can include your sight or movement. ") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Find the hospital terminal! It is in a black building on land, just like this one.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (game.computerdisplays[1])
	{
		printText2D(   std::string("Use the hospital terminal to add a gill to your body. It will enable you to explore underwater.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Find a building under the water and retrieve the tracker glasses. These can identity the game.adversary.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (game.computerdisplays[2])
	{
		printText2D(   std::string("Activate the tracker glasses to see the trails that game.animals leave.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("You will recognize the game.adversary by its white trail.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Take the weapon, find the game.adversary and kill it.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (game.computerdisplays[3])
	{
		printText2D(   std::string("Neuro glasses allow you to see the minute electrical activity of living flesh.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("You can use them, in combination with the hospital, to edit the connection map of a living creature.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	printText2D(  "    \n" , menuX, menuY, textSize);
	menuY -= spacing;
}

void drawGameInterfaceText()
{
	int menuX = 50;
	int menuY = 50;
	int textSize = 10;
	int spacing = 20;
	printText2D(   std::string("FPS ") + std::to_string(fps ) , menuX, menuY, textSize);
	menuY += spacing;
	if (game.showInstructions)
	{
		printText2D(   std::string("[u] hide instructions"), menuX, menuY, textSize);
		menuY += spacing;
	}
	else
	{
		printText2D(   std::string("[u] instructions"), menuX, menuY, textSize);
		menuY += spacing;
	}
	bool holding = false;	// print grabber states
	for (int i = 0; i < game.animals[game.playerCreature].cellsUsed; ++i)
	{
		if (game.animals[game.playerCreature].body[i].organ == ORGAN_GRABBER)
		{
			if (game.animals[game.playerCreature].body[i].grabbedCreature >= 0)
			{
				std::string stringToPrint = std::string("Holding ") + game.animals[  game.animals[game.playerCreature].body[i].grabbedCreature ].displayName + std::string(". [f] to drop.");
				if (game.animals[  game.animals[game.playerCreature].body[i].grabbedCreature ].isMachine)
				{
					stringToPrint += std::string(" [lmb, rmb] to use.");
				}
				printText2D( stringToPrint   , menuX, menuY, textSize);
				menuY += spacing;
				holding = true;
			}
		}
	}
	if (!holding && game.playerCanPickup && game.playerCanPickupItem >= 0 && game.playerCanPickupItem < numberOfAnimals)
	{
		printText2D(   std::string("[g] pick up ") + std::string(game.animals[game.playerCanPickupItem].displayName) , menuX, menuY, textSize);
		menuY += spacing;
	}
	if (game.palette)
	{
		printText2D(   std::string("[lmb] add, [rmb] delete ") , menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[y] select next, [h] select last ") , menuX, menuY, textSize);
		menuY += spacing;
	}
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	if (worldCursorPos < worldSquareSize)
	{
		int heightInt = game.world[worldCursorPos].height;
		printText2D(   std::string("x ") + std::to_string(cursorPosX ) + std::string(" y ") + std::to_string(cursorPosY) + std::string(" height ") + std::to_string(heightInt) , menuX, menuY, textSize);
		menuY += spacing;
		game.cursorAnimal = game.world[worldCursorPos].identity;
		bool animalInSquare = false;
		if (game.cursorAnimal >= 0 && game.cursorAnimal < numberOfAnimals)
		{
			unsigned int cursorAnimalSpecies = game.cursorAnimal / numberOfAnimalsPerSpecies;
			int occupyingCell = isAnimalInSquare(game.cursorAnimal, worldCursorPos);
			std::string selectString( " [e] to select.");
			if (game.selectedAnimal >= 0)
			{
				std::string selectString( " [e] to deselect. [k] save animal.");
			}
			if ( occupyingCell >= 0)
			{
				if (cursorAnimalSpecies == 0)
				{
					if (game.cursorAnimal == game.playerCreature)
					{
						printText2D(   std::string("This is you.") + selectString, menuX, menuY, textSize);
						menuY += spacing;
					}
					else
					{
						printText2D(  std::string("A ") +  std::string(game.animals[game.cursorAnimal].displayName) + std::string(".") + selectString , menuX, menuY, textSize);
						menuY += spacing;
					}
				}
				else
				{
					printText2D(   std::string("An animal of species ") + std::to_string(cursorAnimalSpecies ) + std::string(".") + selectString, menuX, menuY, textSize);
					menuY += spacing;
				}
				printText2D(  tileDescriptions(  game.animals[  game.cursorAnimal].body[occupyingCell].organ ), menuX, menuY, textSize);
				menuY += spacing;
				animalInSquare = true;
			}
		}
		if (!animalInSquare)
		{
			if (game.world[worldCursorPos].material != MATERIAL_NOTHING)
			{
				printText2D(  tileDescriptions(game.world[worldCursorPos].material ), menuX, menuY, textSize);
				menuY += spacing;
			}
			else
			{
				printText2D(  tileDescriptions (game.world[worldCursorPos].terrain ), menuX, menuY, textSize);
				menuY += spacing;
			}
		}
	}
	if (game.playerCreature >= 0)
	{
		int playerPheromoneSensor = getRandomCellOfType( game.playerCreature, ORGAN_SENSOR_PHEROMONE ) ;// if the player has a nose, print what it smells like here.
		if (playerPheromoneSensor >= 0)
		{
			unsigned int playerPheromoneSensorWorldPos = game.animals[game.playerCreature].body[playerPheromoneSensor].worldPositionI;
			if (game.world[playerPheromoneSensorWorldPos].pheromoneChannel >= 0 &&  game.world[playerPheromoneSensorWorldPos].pheromoneChannel < numberOfSpeakerChannels)
			{
				printText2D(   pheromoneChannelDescriptions[  game.world[playerPheromoneSensorWorldPos].pheromoneChannel ] , menuX, menuY, textSize);
				menuY += spacing;
			}
			else
			{
				printText2D(   std::string("You can't smell anything in particular.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
		if (!game.playerCanSee)// if the player is blind, say so!
		{
			printText2D(   std::string("You can't see anything. ") , menuX, menuY, textSize);
			menuY += spacing;
		}
		int playerGill = getRandomCellOfType( game.playerCreature, ORGAN_GILL ) ;
		if (playerGill >= 0)
		{
			if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 0.0f)
			{
				printText2D(   std::string("You have no oxygen left.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (game.animals[game.playerCreature].body[playerGill].signalIntensity < baseLungCapacity / 2)
			{
				printText2D(   std::string("You're half out of oxygen.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
		playerGill = getRandomCellOfType( game.playerCreature, ORGAN_LUNG ) ;
		if (playerGill >= 0)
		{
			if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 0.0f)
			{
				printText2D(   std::string("You have no air left.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (game.animals[game.playerCreature].body[playerGill].signalIntensity < baseLungCapacity / 2)
			{
				printText2D(   std::string("You're half out of air.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
		if (game.animals[game.playerCreature].damageReceived < (game.animals[game.playerCreature].mass) * 0.5)
		{
			;
		}
		if (game.animals[game.playerCreature].damageReceived < (game.animals[game.playerCreature].mass) * 0.75)
		{
			printText2D(   std::string("You're hurt.") , menuX, menuY, textSize);
			menuY += spacing;
		}
		else
		{
			printText2D(   std::string("You are mortally wounded.") , menuX, menuY, textSize);
			menuY += spacing;
		}
	}
	displayComputerText();
	if (printLogs)
	{
		menuY += spacing;
		for (int i = 0; i < 8; ++i)
		{
			printText2D(   game.logs[i] , menuX, menuY, textSize);
			menuY += spacing;
		}
		menuY += spacing;
	}
	if (game.showInstructions)
	{
		menuY += spacing;
		printText2D(   std::string("[esc] quit"), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[arrows] pan, [-,=] zoom"), menuX, menuY, textSize);
		menuY += spacing;
		std::string pauseString = std::string("[p] pause ");
		if (game.paused)
		{
			pauseString = std::string("[p] resume ");
		}
		printText2D(   std::string("[l] limit frame rate"), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[o] save, ") + pauseString, menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[space] return mouse") , menuX, menuY, textSize);
		menuY += spacing;
		if (game.playerCreature >= 0)
		{
			printText2D(   std::string("[w,a,s,d] move") , menuX, menuY, textSize);
			menuY += spacing;
			printText2D(   std::string("Explore to find useful items.") , menuX, menuY, textSize);
			menuY += spacing;
		}
		else
		{
			printText2D(   std::string("[r] spawn") , menuX, menuY, textSize);
			menuY += spacing;
		}
	}

	if (game.palette)
	{
		drawPalette();
	}
}

void a( unsigned int animalIndex , unsigned int organ,  Vec_i2 * p , Color color)
{
	appendCell( animalIndex, organ, *p);
	game.animals[animalIndex].body[game.animals[animalIndex].cellsUsed - 1].color = color;
	p->x++;
}

void setupCreatureFromCharArray( unsigned int animalIndex, char * start, unsigned int len, unsigned int width )
{
	Vec_i2 o = Vec_i2(0, 0);
	Vec_i2 p = Vec_i2(0, 0);
	Color c = color_peach_light;
	if (len > animalSquareSize)
	{
		len = animalSquareSize;
	}
	for (unsigned int i = 0; i < len; ++i)
	{
		char c = start[i];
		unsigned int newOrgan = MATERIAL_NOTHING;
		switch (c)
		{

		case 'B':
			newOrgan = ORGAN_BONE;
			break;
		case 'E':
			newOrgan = ORGAN_SENSOR_EYE;
			break;
		case 'N':
			newOrgan = ORGAN_SENSOR_PHEROMONE;
			break;
		case 'S':
			newOrgan = ORGAN_SPEAKER;
			break;
		case 'M':
			newOrgan = ORGAN_MUSCLE;
			break;
		case 'T':
			newOrgan = ORGAN_MUSCLE_TURN;
			break;
		case 'A':
			newOrgan = ORGAN_MUSCLE_STRAFE;
			break;
		case 'G':
			newOrgan = ORGAN_GRABBER;
			break;
		case 'L':
			newOrgan = ORGAN_LIVER;
			break;
		case 'U':
			newOrgan = ORGAN_LUNG;
			break;
		case 'D':
			newOrgan = ORGAN_GONAD;
			break;
		case 'O':
			newOrgan = ORGAN_ADDOFFSPRINGENERGY;
		case '1':
			newOrgan = MATERIAL_METAL;
		}
		if (newOrgan != MATERIAL_NOTHING)
		{
			appendCell( animalIndex, newOrgan, p);
		}
		p.x++;
		if (p.x == width)
		{
			p.x = 0;
			p.y --;
		}
	}
}

void setupExampleLighter(int i)
{
	resetAnimal(i);
	game.animals[i].isMachine = true;
	game.animals[i].machineCallback = MACHINECALLBACK_LIGHTER;
	std::string gunDescription = std::string("lighter");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );

	appendCell( i, MATERIAL_METAL, Vec_i2(0, 1) );

	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 0) );

	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 1) );

	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 2) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 2) );
}

void setupExampleHuman(int i)
{
	resetAnimal(i);
	std::string gunDescription = std::string("human");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );
	char humanBody[] =
	{
		' ', ' ', ' ', 'B', ' ', ' ', ' ',
		' ', ' ', 'B', 'B', 'B', ' ', ' ',
		' ', 'B', 'E', 'B', 'E', 'B', ' ',
		' ', ' ', 'B', 'N', 'B', ' ', ' ',
		' ', ' ', ' ', 'S', ' ', ' ', ' ',
		' ', ' ', ' ', 'B', ' ', ' ', ' ',
		' ', 'B', 'B', 'B', 'B', 'B', ' ',
		'M', 'M', 'U', 'B', 'U', 'M', 'M',
		'M', ' ', 'B', 'B', 'B', ' ', 'M',
		'B', ' ', 'L', 'B', 'L', ' ', 'B',
		'B', ' ', 'B', 'B', 'B', ' ', 'B',
		'G', ' ', 'A', 'O', 'A', ' ', 'G',
		' ', ' ', 'B', 'B', 'B', ' ', ' ',
		' ', ' ', 'B', 'D', 'B', ' ', ' ',
		' ', ' ', 'B', ' ', 'B', ' ', ' ',
		' ', ' ', 'B', ' ', 'B', ' ', ' ',
		' ', ' ', 'T', ' ', 'T', ' ', ' ',
		' ', ' ', 'B', ' ', 'B', ' ', ' ',
		' ', ' ', 'B', ' ', 'B', ' ', ' ',
		' ', ' ', 'B', ' ', 'B', ' ', ' ',
		' ', ' ', 'B', ' ', 'B', ' ', ' ',
	};
	setupCreatureFromCharArray( i, humanBody, animalSquareSize, 7 );
}

void setupExampleGlasses(int i)
{
	resetAnimal(i);
	game.animals[i].isMachine = true;
	appendCell( i, MATERIAL_GLASS, Vec_i2(1, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(2, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(2, -1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(2, 1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(3, 0) );

	appendCell( i, MATERIAL_METAL, Vec_i2(0, 0) );

	appendCell( i, MATERIAL_GLASS, Vec_i2(-1, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-2, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-2, -1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-2, 1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-3, 0) );
}

void setupTrackerGlasses(int i)
{
	game.animals[i].machineCallback = MACHINECALLBACK_TRACKERGLASSES;
	std::string gunDescription = std::string("tracker glasses");
	strcpy( &(game.animals[i].displayName[0]) , gunDescription.c_str() );
}

void setupNeuroGlasses(int i)
{
	game.animals[i].machineCallback = MACHINECALLBACK_NEUROGLASSES;
	std::string gunDescription = std::string("neuro glasses");
	strcpy( &(game.animals[i].displayName[0]) , gunDescription.c_str() );
}

void setupExampleGun(int i)
{
	resetAnimal(i);
	game.animals[i].isMachine = true;
	game.animals[i].machineCallback = MACHINECALLBACK_PISTOL;
	std::string gunDescription = std::string("pistol");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );

	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(1, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(2, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-1, -1) );
}

void setupExampleKnife(int i)
{
	resetAnimal(i);
	game.animals[i].isMachine = true;
	game.animals[i].machineCallback = MACHINECALLBACK_KNIFE;
	std::string gunDescription = std::string("knife");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );

	appendCell( i, MATERIAL_METAL, Vec_i2(0, 3) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 2) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2(+1, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, -1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, -2) );
}

void setupExampleComputer(int i)
{
	resetAnimal(i);
	game.animals[i].isMachine = true;
	game.animals[i].fAngle = 0.0f;

	appendCell( i, MATERIAL_METAL, Vec_i2(-2, 2) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 2) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 0, 2) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 1, 2) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 2, 2) );

	appendCell( i, MATERIAL_METAL, Vec_i2(-2, 1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-1, 1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2( 0, 1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2( 1, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 2, 1) );

	appendCell( i, MATERIAL_METAL, Vec_i2(-2, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-1, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2( 0, 0) );
	appendCell( i, MATERIAL_GLASS, Vec_i2( 1, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 2, 0) );


	appendCell( i, MATERIAL_METAL, Vec_i2(-2, -1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2(-1, -1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2( 0, -1) );
	appendCell( i, MATERIAL_GLASS, Vec_i2( 1, -1) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 2, -1) );


	appendCell( i, MATERIAL_METAL, Vec_i2(-2, -2) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-1, -2) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 0, -2) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 1, -2) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 2, -2) );


	appendCell( i, MATERIAL_METAL, Vec_i2(0, -3) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-2, -4) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-1, -4) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 0, -4) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 1, -4) );
	appendCell( i, MATERIAL_METAL, Vec_i2( 2, -4) );
}

void setupEcologyCompter(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("ecology terminal");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );
	game.animals[i].machineCallback = MACHINECALLBACK_ECOLOGYCOMPUTER;
}

void setupMessageComputer(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("message terminal");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );
	game.animals[i].machineCallback = MACHINECALLBACK_MESSAGECOMPUTER;
}

void setupHospitalComputer(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("hospital");
	strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );
	game.animals[i].machineCallback = MACHINECALLBACK_HOSPITAL;
}

void setupBuilding_playerBase(unsigned int worldPositionI)
{
	unsigned int worldPositionX = worldPositionI % worldSize;
	unsigned int worldPositionY = worldPositionI / worldSize;
	float avgHeight = 0.0f;
	unsigned int tally = 0;
	for (unsigned int i = 0; i < worldSquareSize; ++i)
	{
		int x = i % worldSize;
		int y = i / worldSize;
		int xdiff = x - worldPositionX;
		int ydiff = y - worldPositionY;
		if (abs(xdiff) < baseSize && abs(ydiff) < baseSize)// set all the tiles around the position to a floor tile
		{
			avgHeight += game.world[i].height;
			tally++;
			game.world[i].terrain = MATERIAL_VOIDMETAL;
			if (  !(game.world[i].wall == MATERIAL_NOTHING || game.world[i].wall == MATERIAL_WATER) )
			{
				game.world[i].wall = MATERIAL_NOTHING;
			}
		}
		if ((((x > worldPositionX - baseSize - wallThickness) && (x < worldPositionX - baseSize + wallThickness) ) || // make walls around it // a square border of certain thickness
		        ((x > worldPositionX + baseSize - wallThickness) && (x < worldPositionX + baseSize + wallThickness) ) ||
		        ((y > worldPositionY - baseSize - wallThickness) && (y < worldPositionY - baseSize + wallThickness) ) ||
		        ((y > worldPositionY + baseSize - wallThickness) && (y < worldPositionY + baseSize + wallThickness) ) )
		        &&
		        (abs(xdiff) < (baseSize + wallThickness) && abs(ydiff) < (baseSize + wallThickness))
		        &&
		        ((abs(xdiff) > doorThickness) &&  (abs(ydiff) > doorThickness) )) // with doors in the middle of each wall
		{
			game.world[i].wall = MATERIAL_VOIDMETAL;
		}
	}
	avgHeight = avgHeight / tally;
	for (unsigned int i = 0; i < worldSquareSize; ++i)
	{
		int x = i % worldSize;
		int y = i / worldSize;
		int xdiff = x - worldPositionX;
		int ydiff = y - worldPositionY;
		if (abs(xdiff) < baseSize && abs(ydiff) < baseSize)
		{
			game.world[i].height = avgHeight;
		}
	}
	game.cameraPositionX  = worldPositionX;
	game.cameraPositionY = worldPositionY;
}

void spawnAdversary(unsigned int targetWorldPositionI)
{
	game.adversary = numberOfAnimalsPerSpecies + 1; // game.adversary animal is a low number index in the 1th species. 0th is for players and machines.
	spawnAnimalIntoSlot(game.adversary, game.champion, targetWorldPositionI, false);
	game.animals[game.adversary].position = targetWorldPositionI;
	game.animals[game.adversary].uPosX = targetWorldPositionI % worldSize;
	game.animals[game.adversary].uPosY = targetWorldPositionI / worldSize;
	game.animals[game.adversary].fPosX = game.animals[game.adversary].uPosX;
	game.animals[game.adversary].fPosY = game.animals[game.adversary].uPosY;
}

void spawnPlayer()
{
	if (game.playerCreature == -1)
	{
		unsigned int targetWorldPositionI =  game.playerRespawnPos;
		int i = 1;
		setupExampleHuman(i);
		game.playerCreature = 0;
		spawnAnimalIntoSlot(game.playerCreature, game.animals[i], targetWorldPositionI, false);
		game.cameraTargetCreature = game.playerCreature;
		int randomLung = getRandomCellOfType(game.playerCreature, ORGAN_LUNG);
		if (randomLung >= 0)
		{
			game.animals[game.playerCreature].body[randomLung].signalIntensity = baseLungCapacity;
		}
		game.animals[game.playerCreature].energy = game.animals[game.playerCreature].maxEnergy;
		game.animals[game.playerCreature].damageReceived = 0;
		appendLog( std::string("Spawned the player.") );
	}
	else
	{
		killAnimal(game.playerCreature);
	}
}

void saveParticularAnimal(unsigned int animalIndex, std::string filename )
{
	std::ofstream out7( filename .c_str());
	out7.write( (char*)(&game.animals[game.selectedAnimal]), sizeof(Animal));
	out7.close();
}

void loadParticlarAnimal(unsigned int animalIndex, std::string filename)
{
	std::ifstream in7(filename.c_str());
	in7.read( (char*)(&game.animals[game.selectedAnimal]), sizeof(Animal));
	in7.close();
}

void saveSelectedAnimal ( )
{
	if (game.selectedAnimal >= 0)
	{
		saveParticularAnimal(game.selectedAnimal, std::string("save/game.selectedAnimal") );
	}
}

void spawnTournamentAnimals()
{
	if (game.adversary >= 0)
	{
		for (int i = (1 * numberOfAnimalsPerSpecies); i < numberOfAnimals; ++i)// game.animals in the tournament are not in the 0th species, which is for players and machines.
		{
			unsigned int targetWorldPositionI = game.animals[game.adversary].position;
			int j = 1;
			setupExampleAnimal2(j);
			spawnAnimalIntoSlot(i, game.animals[j], targetWorldPositionI, true);
		}
	}
}

void normalizeTerrainHeight()
{
	float maxHeight = 0.0f;
	float minHeight = 0.0f;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (game.world[worldPositionI].height > maxHeight)
		{
			maxHeight = game.world[worldPositionI].height;
		}
		if (game.world[worldPositionI].height < minHeight)
		{
			minHeight = game.world[worldPositionI].height;
		}
	}
	float heightRange =  maxHeight - minHeight ;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		game.world [ worldPositionI] .height =  ((game.world [ worldPositionI] .height - minHeight) / (  heightRange )  ) * (worldSize);
	}
	float postMaxHeight = 0.0f;
	float postMinHeight = 0.0f;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (game.world[worldPositionI].height > postMaxHeight)
		{
			postMaxHeight = game.world[worldPositionI].height;
		}
		if (game.world[worldPositionI].height < postMinHeight)
		{
			postMinHeight = game.world[worldPositionI].height;
		}
	}
	for (int i = 0; i < worldSquareSize; ++i)
	{
		computeLight(i, sunXangle, sunYangle);
	}
}

void copyPrelimToRealMap()
{
	unsigned int pixelsPer = worldSize / prelimSize;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		unsigned int x = worldPositionI % worldSize;
		unsigned int y = worldPositionI / worldSize;
		unsigned int px = x / pixelsPer;
		unsigned int py = y / pixelsPer;
		unsigned int prelimSampleIndex = (prelimSize * py) + px;
		game.world[worldPositionI].height = prelimMap[prelimSampleIndex] ;
	}
}

void recomputeTerrainLighting()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		computeLight( worldPositionI, sunXangle, sunYangle);
		if (game.world[worldPositionI].height < seaLevel)
		{
			game.world[worldPositionI].wall = MATERIAL_WATER;
			float depth = (seaLevel - game.world[worldPositionI].height);
			float brightness = (1 / (1 + (depth / (worldSize / 8))) );
			if (brightness < 0.2f) { brightness = 0.2f;}
			game.world[worldPositionI].light = multiplyColorByScalar(game.world[worldPositionI].light, brightness   );
		}
		float steps = 8;
		float b = game.world[worldPositionI].light.a * steps;//100.0f;
		int ib = b;
		float betoot =  (ib / steps);
		game.world[worldPositionI].light.a = betoot;
	}
}

unsigned int getRandomPosition(bool underwater)
{
	while (true)
	{
		unsigned int randomI = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
		if (!underwater)
		{
			if (game.world[randomI].height > biome_coastal)
			{
				return randomI;
			}
		}
		else
		{
			if (game.world[randomI].height < seaLevel)
			{
				return randomI;
			}
		}
	}
}

void setupGameItems()
{
	unsigned int targetWorldPositionI =  getRandomPosition(false);

	setupBuilding_playerBase(targetWorldPositionI);

	int i = 1;
	setupEcologyCompter( i);
	spawnAnimalIntoSlot(3, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25;
	setupMessageComputer( i);
	spawnAnimalIntoSlot(9, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25 * worldSize;
	game.playerRespawnPos = targetWorldPositionI;
	spawnPlayer();

	targetWorldPositionI =  getRandomPosition(true);
	game.adversaryRespawnPos = targetWorldPositionI;
	spawnAdversary(targetWorldPositionI);

	targetWorldPositionI =  getRandomPosition(false);
	setupBuilding_playerBase(targetWorldPositionI);
	setupHospitalComputer(i);
	spawnAnimalIntoSlot(5, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25 * worldSize;
	setupMessageComputer( i);
	spawnAnimalIntoSlot(10, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI =  getRandomPosition(true);
	setupBuilding_playerBase(targetWorldPositionI);
	setupTrackerGlasses(i);
	spawnAnimalIntoSlot(4, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25;
	setupExampleGun(i);
	spawnAnimalIntoSlot(2, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25 * worldSize;
	setupMessageComputer( i);
	spawnAnimalIntoSlot(11, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI =  getRandomPosition(true);
	setupBuilding_playerBase(targetWorldPositionI);
	setupExampleKnife(i);
	spawnAnimalIntoSlot(6, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25;
	setupExampleLighter(i);
	spawnAnimalIntoSlot(7, game.animals[i], targetWorldPositionI, false);

	targetWorldPositionI =  getRandomPosition(true);
	setupBuilding_playerBase(targetWorldPositionI);
	setupNeuroGlasses(i);
	spawnAnimalIntoSlot(8, game.animals[i], targetWorldPositionI, false);
}

void applyPretties()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (  game.world[worldPositionI].terrain == MATERIAL_ROCK )
		{
			detailTerrain(worldPositionI);
		}
	}
}

void setupRandomWorld()
{
	worldCreationStage = 0;
	worldCreationStage++;
	worldCreationStage++;
	float initWaterLevel = 1.0f;
	for (unsigned int pp = 0; pp < prelimSquareSize; pp++)
	{
		unsigned int x = pp % prelimSize;
		unsigned int y = pp / prelimSize;
		float hDistance = x;
		float hMax = worldSize;
		prelimMap[pp] = hDistance / hMax;
		float noiseScaleFactor = 0.005f;
		float fx = x * noiseScaleFactor;
		float fy = y * noiseScaleFactor;
		float noise =   SimplexNoise::noise(fx, fy);   // Get the noise value for the coordinate
		prelimMap[pp] += noise;
		noiseScaleFactor = 0.05f;
		fx = x * noiseScaleFactor;
		fy = y * noiseScaleFactor;
		noise =   SimplexNoise::noise(fx, fy) * 0.1f;   // Get the noise value for the coordinate
		prelimMap[pp] += noise;
		noiseScaleFactor = 0.5f;
		fx = x * noiseScaleFactor;
		fy = y * noiseScaleFactor;
		noise =   SimplexNoise::noise(fx, fy) * 0.01f;   // Get the noise value for the coordinate
		prelimMap[pp] += noise;
		prelimWater[pp] = initWaterLevel;
	}
	worldCreationStage++;
	bool erode = false;
	if (erode)
	{
		TinyErode::Simulation simulation(prelimSize, prelimSize);
		simulation.SetMetersPerX(1000.0f / prelimSize);
		simulation.SetMetersPerY(1000.0f / prelimSize);
		int iterations = 256;
		for (int i = 0; i < iterations; i++)
		{
			// Determines where the water will flow.
			simulation.ComputeFlowAndTilt(getHeight, getWater);
			// Moves the water around the terrain based on the previous computed values.
			simulation.TransportWater(addWater);
			// Where the magic happens. Soil is picked up from the terrain and height
			// values are subtracted based on how much was picked up. Then the sediment
			// moves along with the water and is later deposited.
			simulation.TransportSediment(carryCapacity, deposition, erosion, addHeight);
			// Due to heat, water is gradually evaported. This will also cause soil
			// deposition since there is less water to carry soil.
			simulation.Evaporate(addWater, evaporation);
		}
		// Drops all suspended sediment back into the terrain.
		simulation.TerminateRainfall(addHeight);
	}
	worldCreationStage++;
	copyPrelimToRealMap();
	worldCreationStage++;
	smoothHeightMap( 8, 0.5f );
	worldCreationStage++;
	normalizeTerrainHeight();
	worldCreationStage++;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)// place items and terrain
	{
		unsigned int x = worldPositionI % worldSize;
		unsigned int y = worldPositionI / worldSize;
		game.world[worldPositionI].terrain = MATERIAL_ROCK;
		if (x < wallThickness || x > worldSize - wallThickness || y < wallThickness  || y > worldSize - wallThickness)	// walls around the game.world edge
		{
			game.world[worldPositionI].wall = MATERIAL_VOIDMETAL;
		}
		if (game.world[worldPositionI].height < seaLevel)
		{
			game.world[worldPositionI].material = MATERIAL_GRASS;
		}
	}
	applyPretties();
	worldCreationStage++;
	setupGameItems();
	worldCreationStage++;
	recomputeTerrainLighting();
	worldCreationStage++;
	setFlagReady();
}

void tournamentController()
{
	ZoneScoped;
	if (game.adversary < 0)
	{
		spawnAdversary(game.adversaryRespawnPos);
	}
	if (game.adversary >= 0 && game.adversary < numberOfAnimals)
	{
		if (game.animals[game.adversary].retired)
		{
			spawnAdversary(game.adversaryRespawnPos);
		}
		else
		{
			if (game.animals[game.adversary].position >= 0 && game.animals[game.adversary].position < worldSquareSize)
			{
				game.adversaryRespawnPos = game.animals[game.adversary].position;
				unsigned int adversaryRespawnPosX = game.adversaryRespawnPos % worldSize;
				unsigned int adversaryRespawnPosY = game.adversaryRespawnPos / worldSize;
				if (adversaryRespawnPosX < baseSize)
				{
					adversaryRespawnPosX = baseSize;
				}
				else if (adversaryRespawnPosX > worldSize - baseSize )
				{
					adversaryRespawnPosX = worldSize - baseSize;
				}
				if (adversaryRespawnPosY < baseSize)
				{
					adversaryRespawnPosY = baseSize;
				}
				else if (adversaryRespawnPosY > worldSize - baseSize )
				{
					adversaryRespawnPosY = worldSize - baseSize;
				}
				game.adversaryRespawnPos = (adversaryRespawnPosY * worldSize ) + adversaryRespawnPosX;
			}
		}
	}

	if (respawnLowSpecies)
	{
		unsigned int totalpop = 0;
		for (unsigned int speciesIndex = 1; speciesIndex < numberOfSpecies; speciesIndex++) // start at 1 to ignore the non-natural species 0.
		{
			totalpop += game.speciesPopulationCounts[speciesIndex] ;
			if (game.speciesPopulationCounts[speciesIndex] == 0)
			{
				int foundAnimal = -1;// if there is another species who is successful, duplicate an animal from them.
				int foundSpecies = -1;
				for (unsigned int j = 1; j < numberOfSpecies; ++j)
				{
					if (game.speciesPopulationCounts[j] >= 1)
					{
						for (unsigned int k = extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies - 2); k < numberOfAnimalsPerSpecies; ++k)
						{
							unsigned int animalToCopy = (j * numberOfAnimalsPerSpecies) + k;
							if (!game.animals[animalToCopy].retired)
							{
								foundAnimal = animalToCopy;
								foundSpecies = j;
								break;
							}
						}
					}
					if (foundAnimal >= 0)
					{
						break;
					}
				}
				if (foundAnimal >= 0 && foundAnimal < numberOfAnimals)
				{
					int ispeciesindex = speciesIndex;
					memcpy(    &game.animals[ (speciesIndex * numberOfAnimalsPerSpecies) ] , &game.animals[ foundAnimal ], sizeof(Animal)    );
					resetAnimal(foundAnimal);
				}
			}
		}

		if (totalpop == 0 && game.adversary >= 0)// life went extinct but the game.adversary is still alive. Spawn a bunch more stuff to get it going again.
		{
			int j = 1;
			for (int k = j + 1; k < numberOfAnimals / 2; ++k)// spawn lots of the example animal
			{
				if (k != game.adversary)
				{
					setupExampleAnimal2(j);
					int domingo = spawnAnimal( 1, game.animals[j], game.animals[game.adversary].position, true);

					if (domingo >= 0)
					{
						paintAnimal(domingo);
						game.animals[domingo].energy = game.animals[domingo].maxEnergy;
						game.animals[domingo].damageReceived = 0;
					}
				}
			}

			for (int k = (numberOfAnimals / 2) + 1; k < numberOfAnimals; ++k)// spawn lots of the game.champion
			{
				if (k != game.adversary)
				{
					int domingo = spawnAnimal( 1,  game.champion, game.animals[game.adversary].position, true);
					if (domingo >= 0)
					{
						paintAnimal(domingo);
						game.animals[domingo].energy = game.animals[domingo].maxEnergy;
						game.animals[domingo].damageReceived = 0;
					}
				}
			}
		}
	}
}

void model()
{
	auto start = std::chrono::steady_clock::now();
	ZoneScoped;
	if (!game.paused)
	{
		tournamentController();
		computeAllAnimalsOneTurn();
		updateMap();
	}
	modelFrameCount++;
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	usPerFrame = elapsed.count();

	if (!game.lockfps && usPerFrame > 0)
	{
		fps = (1000000.0f / usPerFrame) ;
	}
	if (game.lockfps) { fps = 1.0f;}
}

void modelSupervisor()
{
	while (true)
	{
		if (!game.lockfps)
		{
			model();
		}
#ifdef TRACY_ENABLE
		FrameMark;
#endif
	}
}

void drawMainMenuText()
{
	bool saveExists = false;
	if (exists_test3(std::string("save/game.animals")) && exists_test3(std::string("save/game.world")) )
	{
		saveExists = true;
	}
	switch (worldCreationStage)
	{
	case 0:
		printText2D(   std::string("DEEP SEA "), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[esc] quit "), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[j] new "), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[i] load "), menuX, menuY, textSize);
		menuY += spacing;
		break;
	case 1:
		printText2D(   std::string("clear grids "), menuX, menuY, textSize);
		break;
	case 2:
		printText2D(   std::string("seed preliminary map with noise "), menuX, menuY, textSize);
		break;
	case 3:
		printText2D(   std::string("hydraulic erosion "), menuX, menuY, textSize);
		break;
	case 4:
		printText2D(   std::string("copy prelim to real map "), menuX, menuY, textSize);
		break;
	case 5:
		printText2D(   std::string("smooth heightmap "), menuX, menuY, textSize);
		break;
	case 6:
		printText2D(   std::string("normalize heightmap "), menuX, menuY, textSize);
		break;
	case 7:
		printText2D(   std::string("place terrain materials "), menuX, menuY, textSize);
		break;
	case 8:
		printText2D(   std::string("place game items "), menuX, menuY, textSize);
		break;
	case 9:
		printText2D(   std::string("compute terrain lighting "), menuX, menuY, textSize);
		break;
	case 10:
		printText2D(   std::string("ready "), menuX, menuY, textSize);
		break;
	case 11:
		printText2D(   std::string("loading game.animals from file "), menuX, menuY, textSize);
		break;
	case 12:
		printText2D(   std::string("loading map from file "), menuX, menuY, textSize);
		break;
	}
}

void startSimulation()
{
	srand((unsigned int)time(NULL));
	seedExtremelyFastNumberGenerators();
	int j = 1;
	setupExampleAnimal2(j);
	game.champion = game.animals[j];
	boost::thread t7{ modelSupervisor };
	for ( ;; )
	{
		boost::thread t2{ threadInterface };
		threadGraphics();// graphics only works in this thread, because it is the process the SDL context was created in.
		t2.join();
		if (flagReturn)
		{
			flagReturn = false;
			return;
		}
	}
}

void save()
{
	std::ofstream out6(std::string("save/game.world").c_str());
	out6.write( (char*)(game), sizeof(GameState) );
	out6.close();
}

void load()
{
	worldCreationStage = 12;
	std::ifstream in6(std::string("save/game.world").c_str());
	in6.read( (char *)(&(game)), sizeof(GameState));
	in6.close();
	worldCreationStage = 10;
	setFlagReady();
}

