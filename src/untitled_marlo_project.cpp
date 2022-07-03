// Untitled Marlo Project.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cstring>
#include <random>
#include <time.h>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

#include <boost/thread.hpp>

#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"

// #define TRACY_ENABLE


#include "utilities.h"
#include "graphics.h"
#include "untitled_marlo_project.h"
#include "menus.h"
#include "main.h"
#include "SimplexNoise.h"
#include "TinyErode.h"
#include "content.h"

const bool doReproduction        = true;
// const bool horizontalTransfer    = false;
const bool taxIsByMass           = true;
const bool respawnLowSpecies     = true;
const bool setOrSteerAngle       = false;
const bool printLogs             = true;

const bool killLoiteringAdversary = false;
const bool erode = false;
const int prelimSize = worldSize / 10;
const int cameraPanSpeed = 10;
const float baseLungCapacity = 1.0f;
const unsigned int prelimSquareSize = prelimSize * prelimSize;
const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.
const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);
const float neuralNoise = 0.2f;
const float liverStorage = 20.0f;
const unsigned int baseLifespan = 2000;			// if the lifespan is long, the animal's strategy can have a greater effect on its success. If it's very short, the animal is compelled to be just a moving mouth.
const float musclePower = 20.0f;
const float turnMusclePower = 1.0f;
const float aBreath = 0.01f;
const float neuralMutationStrength = 0.5f;
const int paletteMenuX = 300;
const int paletteMenuY = 50;
const int paletteTextSize = 10;
const int paletteSpacing = 20;
const unsigned int paletteWidth = 4;
// these are the parameters that set up the physical geography of the world.
const float seaLevel =  0.5f * worldSize;;;
const float biome_marine  = seaLevel + (worldSize / 20);
const float biome_coastal = seaLevel + (worldSize / 3);
const float sunXangle = 0.35f;
const float sunYangle = 0.35f;
const unsigned int baseSize = 100;
const unsigned int wallThickness = 8;
const unsigned int doorThickness = 16;
const int destroyerRange = 250;
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

// map updating
const int sectors = 4;
const int sectorSize = worldSquareSize / sectors;
const unsigned int updateSize = (worldSquareSize / 250) / sectors;

std::string progressString = std::string("");

GameState game;

// these are variables which are only needed per session, and never need to be stored.
float prelimMap[prelimSquareSize];
float prelimWater[prelimSquareSize];

// tinyerode stuff
const float initWaterLevel = 1.0f;
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
	return  prelimWater[address]; // /* return water level at (x, y) */
};
auto addWater = [](int x, int y, float deltaWater) -> float {/* Note: 'deltaWater' can be negative. */
	unsigned int address = (y * prelimSize) + x;
	float previousWaterLevel =  prelimWater[address];
	prelimWater[address] += deltaWater;
	if (prelimWater[address] < 0.0f)	/* The function returns the new water level. It should not fall below zero. */
	{
		prelimWater[address]  = 0.0f;
	}
	return std::max(0.0f, previousWaterLevel + deltaWater);
};
auto carryCapacity = [](int x, int y) -> float {
	return 0.025;
};
auto deposition = [](int x, int y) -> float {
	return 0.5;
};
auto erosion = [](int x, int y) -> float {
	return 0.5;
};
auto evaporation = [](int x, int y) -> float {
	return 0.000001;
};
unsigned int modelFrameCount = 0;
unsigned int usPerFrame = 0;
float fps = 1.0f;

int adversaryLoiter = 0;
unsigned int adversaryLoiterPos = 0;

bool mainMenu = true;
bool flagQuit = false;
bool flagCreate = false;
bool flagLoad = false;
bool flagReady = false;
bool flagReturn = false;
bool flagSave = false;
int mouseX;
int mouseY;
int worldCreationStage = 0;



float foodWeb[numberOfSpecies][numberOfSpecies];


void viewAdversary()
{
	if (game.cameraTargetCreature == game.adversary)
	{
		if (game.playerCreature >= 0)
		{
			game.cameraTargetCreature = game.playerCreature;
		}
		else
		{
			game.cameraTargetCreature = -1;
		}
	}

	else if (game.cameraTargetCreature == game.playerCreature)
	{
		if (game.adversary >= 0)
		{
			game.cameraTargetCreature = game.adversary;
		}
		else
		{
			game.cameraTargetCreature = -1;
		}
	}
	else
	{
		if (game.playerCreature >= 0)
		{
			game.cameraTargetCreature = game.playerCreature;
		}
		else if (game.adversary >= 0)
		{
			game.cameraTargetCreature = game.adversary;
		}
		else
		{
			game.cameraTargetCreature = -1;
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
	if (game.palette)
	{
		game.paletteSelectedOrgan++;
		game.paletteSelectedOrgan = game.paletteSelectedOrgan % numberOfOrganTypes;
	}

	if (game.ecologyComputerDisplay)
	{
		game.activeEcoSetting++;
		game.activeEcoSetting = game.activeEcoSetting % numberOfEcologySettings;
	}
}
void decrementSelectedOrgan()
{
	if (game.palette)
	{
		game.paletteSelectedOrgan--;
		game.paletteSelectedOrgan = game.paletteSelectedOrgan % numberOfOrganTypes;
	}

	if (game.ecologyComputerDisplay)
	{
		game.activeEcoSetting++;
		game.activeEcoSetting = game.activeEcoSetting % numberOfEcologySettings;
	}
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

void setMousePosition(Vec_i2 in)
{
	game.mousePositionX = in.x;
	game.mousePositionY = in.y;
}

void toggleFPSLimit()
{
	game.lockfps = !game.lockfps;
}

bool getFPSLimit()
{
	return game.lockfps;
}

void appendLog( std::string input)
{
	for (int i = nLogs; i > 0; i--)
	{
		memcpy(  &game.logs[i] , &game.logs[i - 1], sizeof(char) * logLength   );
	}
	strcpy( &game.logs[0][0] , input.c_str() );
}

void resetConnection(int animalIndex, unsigned int cellLocalPositionI, unsigned int i)
{
	game.animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;
	game.animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = 0;//extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	game.animals[animalIndex].body[cellLocalPositionI].connections[i].weight = 0.0f;//RNG() - 0.5f;
}

void resetCell(int animalIndex, unsigned int cellLocalPositionI)
{
	game.animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
	game.animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
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
}

void paintAnimal(int animalIndex)
{
	ZoneScoped;
	Color newAnimalColorA = Color(RNG(), RNG(), RNG(), 1.0f);
	Color newAnimalColorB = Color(RNG(), RNG(), RNG(), 1.0f);
	for (int i = 0; i < animalSquareSize; ++i)
	{
		game.animals[animalIndex].body[i].color = filterColor(  newAnimalColorA , multiplyColorByScalar( newAnimalColorB , RNG())  );
		if (game.animals[animalIndex].body[i].organ == ORGAN_SENSOR_EYE)
		{
			game.animals[animalIndex].body[i].color = color_green;
		}
	}
}

void resetAnimal(int animalIndex)
{
	ZoneScoped;
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
		game.animals[animalIndex].isMachine = false;
		game.animals[animalIndex].machineCallback = MATERIAL_NOTHING;
		// game.animals[animalIndex].temp_limit_low = 273.0f;
		// game.animals[animalIndex].temp_limit_high = 323.0f;
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			resetCell(animalIndex, cellLocalPositionI );
		}
	}
}


// choose a random cell of any type that can be connected to, which includes all neurons and all sensors.
int getRandomConnectableCell( int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnectable(  game.animals[animalIndex].body[cellIndex].organ ))
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

void scrambleAnimal(int animalIndex)
{
	for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
	{
		for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
		{
			if (extremelyFastNumberFromZeroTo(1) == 0)
			{
				game.animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;//extremelyFastNumberFromZeroTo(1);
				game.animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = getRandomConnectableCell(animalIndex) ;//game.animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo;
				game.animals[animalIndex].body[cellLocalPositionI].connections[i].weight      = (RNG() - 0.5f ) * 2.0f; //game.animals[animalIndex].body[cellLocalPositionI].connections[i].weight;
			}
			else
			{
				game.animals[animalIndex].body[cellLocalPositionI].connections[i].used = false; // extremelyFastNumberFromZeroTo(1);
			}
		}
	}
}

void resetAnimals()
{
	for ( int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		resetAnimal(animalIndex);
	}
	int j = 1;
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		setupExampleAnimal2(j, true);
		paintAnimal(j);
		game.champions[i] = game.animals[j];
		game.championScores[i] = 0;
		game.speciesVacancies[i] = true;
		game.speciesPopulationCounts[i] = 0;
		game.populationCountUpdates[i] = 0;
		game.speciesAttacksPerTurn[i] = 0;
	}
}

void resetGrid()
{
	for (int i = 0; i < worldSquareSize; ++i)
	{
		game.world[i].terrain = MATERIAL_VOIDMETAL;
		game.world[i].wall = MATERIAL_NOTHING;
		game.world[i].identity = -1;
		game.world[i].trail = 0.0f;
		game.world[i].height = 1.0f;
		game.world[i].light = color_black;
		game.world[i].pheromoneIntensity = 0.0f;
		game.world[i].pheromoneChannel = -1;
#ifdef PLANTS
		game.world[i].grassColor =  color_green;
		memset(&(game.world[i].plantGenes[0]), MATERIAL_NOTHING, sizeof(char) * plantGenomeSize);
		game.world[i].plantState = MATERIAL_NOTHING;
		game.world[i].geneCursor = 0;
		game.world[i].plantIdentity = 0;
		game.world[i].energy = 0.0f;
		game.world[i].energyDebt = 0.0f;
		game.world[i].sequenceReturn = 0;
		game.world[i].sequenceNumber = 0;
		game.world[i].grown = false;;
		memset(&(game.world[i].seedGenes[0]), MATERIAL_NOTHING, sizeof(char) * plantGenomeSize);
		game.world[i].seedState = MATERIAL_NOTHING;
		game.world[i].seedIdentity =  0;
		game.world[i].seedColor = color_yellow;
#endif
	}
}

void fastReset()
{
	memset( &game, 0x00, sizeof(GameState)  );
	for (int i = 0; i < numberOfAnimals; ++i)
	{
		game.animals[i].retired = true;
	}
	for (int i = 0; i < worldSquareSize; ++i)
	{
		game.world[i].plantState = MATERIAL_NOTHING;
		game.world[i].seedState  = MATERIAL_NOTHING;
	}
}

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

	// these variables keep track of the main characters in the game
	game.playerCreature = -1;
	game.adversary = -1;
	game.adversaryRespawnPos;
	game.selectedAnimal = -1;
	game.cursorAnimal = -1;
	game.playerRespawnPos;
	game.adversaryDefeated = false;
	game.adversaryCreated = false;
	game.playerLMBDown = false;

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

	game.ecoSettings[3]       = 0.00001f; // tax energy scale
	game.ecoSettings[2]     = 0.0001f;      // movement energy scale
	game.ecoSettings[0]           = 0.95f; // food (meat) energy
	game.ecoSettings[1]          = 0.25f; // grass energy
	game.activeEcoSetting = 0;
	resetAnimals();
	resetGrid();
}


bool isCellAnEdge(int animalIndex, unsigned int cellIndex)// check if a cell has an empty neighbour.
{
	// go through the list of other cells and see if any of neighbour indexes match any of them. if so, mark the square as not an edge.
	unsigned int neighbourtally = 0;
	Vec_i2 locations_to_check[nNeighbours] =
	{
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX - 1 , game.animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX     , game.animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX + 1 , game.animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX - 1 , game.animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX + 1 , game.animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX - 1 , game.animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX     , game.animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX + 1 , game.animals[animalIndex].body[cellIndex].localPosY + 1  ),
	};
	for (int potentialNeighbour = 0; potentialNeighbour < game.animals[animalIndex].cellsUsed; ++potentialNeighbour)
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			if (game.animals[animalIndex].body[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        game.animals[animalIndex].body[potentialNeighbour].localPosY == locations_to_check[i].y  )
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

unsigned int getRandomEdgeCell(int animalIndex)
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

Vec_i2 getRandomEmptyEdgeLocation(int animalIndex)
{
	unsigned int cellIndex = getRandomEdgeCell(animalIndex);
	Vec_i2 result = Vec_i2(0, 0);
	// get an edge cell at random then search its neighbours to find the empty one. return the position of the empty neighbour.
	Vec_i2 locations_to_check[nNeighbours] =
	{
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX - 1 , game.animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX     , game.animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX + 1 , game.animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX - 1 , game.animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX + 1 , game.animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX - 1 , game.animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX     , game.animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  game.animals[animalIndex].body[cellIndex].localPosX + 1 , game.animals[animalIndex].body[cellIndex].localPosY + 1  ),
	};
	for (int i = 0; i < nNeighbours; ++i)
	{
		bool empty = true;
		for (int potentialNeighbour = 0; potentialNeighbour < game.animals[animalIndex].cellsUsed; ++potentialNeighbour)
		{
			if (game.animals[animalIndex].body[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        game.animals[animalIndex].body[potentialNeighbour].localPosY == locations_to_check[i].y  )
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

void appendCell( int animalIndex, unsigned int organType, Vec_i2 newPosition)
{
	// pick a random location for the new cell which is adjacent to a normal cell.
	// we can avoid ever having to check for valid placement of the cell if we are careful about where to place it!
	// figure out the lowest index in the animal array and put the new cell there
	unsigned int cellIndex = game.animals[animalIndex].cellsUsed;
	if (cellIndex < animalSquareSize)
	{
		game.animals[animalIndex].cellsUsed ++;
		game.animals[animalIndex].body[cellIndex].localPosX = newPosition.x;
		game.animals[animalIndex].body[cellIndex].localPosY = newPosition.y;
		game.animals[animalIndex].body[cellIndex].organ = organType;

		if (  isCellConnecting(organType)) // if the cell is supposed to have connections, go hook it up
		{
			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				unsigned int connectableCell = getRandomConnectableCell( animalIndex);// pick a random connectable cell to connect to.
				bool alreadyConnected =  false;	// check if you are already connected to it.
				for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
				{
					if (  game.animals[animalIndex].body[cellIndex].connections[j].connectedTo == connectableCell &&
					        game.animals[animalIndex].body[cellIndex].connections[j] .used)
					{
						alreadyConnected = true;
					}
				}
				if (!alreadyConnected)	// make the new connection if appropriate.
				{
					for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
					{
						if ( ! (game.animals[animalIndex].body[cellIndex].connections[j].used))
						{
							game.animals[animalIndex].body[cellIndex].connections[j].used = true;
							game.animals[animalIndex].body[cellIndex].connections[j].connectedTo = connectableCell;
							game.animals[animalIndex].body[cellIndex].connections[j].weight = (RNG() - 0.5f ) * 2;
							break;
						}
					}
				}
			}
		}
	}
}

// add a cell to an animal germline in a guided but random way. Used to messily construct new game.animals, for situations where lots of variation is desirable.
void animalAppendCell(int animalIndex, unsigned int organType)
{
	ZoneScoped;
	Vec_i2 newPosition   = getRandomEmptyEdgeLocation(animalIndex); 	// figure out a new position anywhere on the animal edge
	appendCell(animalIndex, organType,  newPosition);
}


void setupTestAnimal_eye(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_SENSOR_EYE, Vec_i2(0, 1) );
	appendCell( i, ORGAN_NEURON, Vec_i2(0, 2) );
	appendCell( i, ORGAN_MUSCLE_TURN, Vec_i2(0, 3) );
	game.animals[i].body[1].connections[0].used = true;
	game.animals[i].body[1].connections[0].connectedTo = 0;
	game.animals[i].body[1].connections[0].weight = 1.0f;
	game.animals[i].body[2].connections[0].used = true;
	game.animals[i].body[2].connections[0].connectedTo = 1;
	game.animals[i].body[2].connections[0].weight = 1.0f;
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 4) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 5) );
}

void setupTestAnimal_reproducer(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 0) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 1) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 2) );
}

void setupTestAnimal_straightline(int i)
{
	// test animal 2 is little more than a mouth that moves at a constant pace.
	resetAnimal(i);
	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 0) );
	appendCell( i, ORGAN_BIASNEURON, Vec_i2(0, 1) );
	appendCell( i, ORGAN_MUSCLE, Vec_i2(0, 2) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 4) );
	game.animals[i].body[2].connections[0].used = true;
	game.animals[i].body[2].connections[0].connectedTo = 1;
	game.animals[i].body[2].connections[0].weight = 1.0f;
	game.animals[i].body[1].signalIntensity = 0.1f;
}

void setupTestAnimal_amphibious(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 0) );
	appendCell( i, ORGAN_GILL, Vec_i2(0, 1) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, 2) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );
}

void setupTestAnimal_airbreathing(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 0) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, 1) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 2) );
}

void setupTestAnimal_waterbreathing(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_GILL, Vec_i2(0, 0) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, 1) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 2) );
}

Vec_f2 getTerrainSlope(unsigned int worldPositionI)
{
	float xSurfaceAngle = game.world[worldPositionI].height - game.world[worldPositionI + 1].height ;
	float ySurfaceAngle = game.world[worldPositionI].height - game.world[worldPositionI + worldSize].height ;
	return Vec_f2(xSurfaceAngle, ySurfaceAngle);
}

void detailTerrain()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (  game.world[worldPositionI].terrain == MATERIAL_ROCK )
		{
			Vec_f2 slope = getTerrainSlope(worldPositionI);
			float grade = sqrt( (slope.x * slope.x) +  (slope.y * slope.y)  );
			float colorNoise = 1 + (((RNG() - 0.5f) * 0.35)) ; // map -1,1 to 0,0.8
			if ( game.world[worldPositionI]. height < biome_marine)
			{
				if (grade < 5.0f)
				{
					game.world[worldPositionI].terrain = MATERIAL_SAND;

#ifdef PLANTS

					game.world[worldPositionI].plantState = MATERIAL_GRASS;
#else
					game.world[worldPositionI].wall = MATERIAL_GRASS;
#endif
				}

				else
				{
					game.world[worldPositionI].terrain = MATERIAL_BASALT;
				}

				if (game.world[worldPositionI].height < seaLevel)
				{
					game.world[worldPositionI].wall = MATERIAL_WATER;
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
bool measureAnimalQualities( int animalIndex)
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
			if (game.animals[animalIndex].offspringEnergy > game.animals[animalIndex].mass / 2)
			{
				game.animals[animalIndex].offspringEnergy = game.animals[animalIndex].mass / 2; // if its bigger than this, the animal will never be able to reproduce.
			}
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_ADDLIFESPAN)
		{
			game.animals[animalIndex].lifespan += baseLifespan;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_GONAD)
		{
			totalGonads ++;
		}
		// if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_COLDADAPT)
		// {
		// 	game.animals[animalIndex].temp_limit_low -= 35.0f;
		// 	game.animals[animalIndex].temp_limit_high -= 35.0f;
		// }
		// if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_HEATADAPT)
		// {
		// 	game.animals[animalIndex].temp_limit_high += 35.0f;
		// 	game.animals[animalIndex].temp_limit_low  += 35.0f;
		// }
	}
	game.animals[animalIndex].lifespan *= 0.75 + (RNG() * 0.5);

	if (game.animals[animalIndex].mass > 0 && totalGonads > 1 ) // the animal needs at least two gonads to be viable.
	{
		return true;
	}
	else
	{
		return false;
	}
}

// choose a random cell of any type that can put forth a connection, which includes all neurons and actuators.
int getRandomConnectingCell(  int animalIndex)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnecting(  game.animals[animalIndex].body[cellIndex].organ ))
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}

	if (found > 0)
	{
		std::list< int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

// choose a random cell of any type that has a pre-existing connection.
int getRandomConnectedCell(  int animalIndex)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnecting(  game.animals[animalIndex].body[cellIndex].organ ))
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		int tries = 0;
		while (true)
		{
			tries++; if (tries > game.animals[animalIndex].cellsUsed) {return -1; }
			std::list< int>::iterator iterator = cellsOfType.begin();
			std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				if (game.animals[animalIndex].body[(*iterator)].connections[i].used)
				{

					return *iterator;
				}
			}
		}
	}
	return -1;
}

int getRandomUsedConnection(   int animalIndex,  int cellIndex)
{
	int randomStart = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS);
	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		int connectionIndex = (i + randomStart) % NUMBER_OF_CONNECTIONS;
		if (game.animals[animalIndex].body[cellIndex].connections[i].used)
		{
			return i;
		}
	}
	return -1;
}

int getRandomUnusedConnection(   int animalIndex,  int cellIndex)
{
	int randomStart = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS);
	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		int connectionIndex = (i + randomStart) % NUMBER_OF_CONNECTIONS;
		if (!(game.animals[animalIndex].body[cellIndex].connections[i].used))
		{
			return i;
		}

	}
	return -1;
}

// choose a random cell of a particular organ type in a given animal, or MATERIAL_NOTHING if the organ doesn't exist.
int getRandomCellOfType( int animalIndex,  int organType)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].body[cellIndex].organ == organType)
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list< int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

int getCellWithAir( int animalIndex)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_GILL || game.animals[animalIndex].body[cellIndex].organ == ORGAN_LUNG )
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	int bestCell = -1;
	if (found > 0)
	{
		float best = 0.0f;
		std::list< int>::iterator iterator = cellsOfType.begin();
		for (iterator = cellsOfType.begin(); iterator != cellsOfType.end(); ++iterator)
		{
			if (game.animals[animalIndex].body[ *iterator ].signalIntensity > best)
			{
				best = game.animals[animalIndex].body[ *iterator ].signalIntensity;
				bestCell = *iterator;
			}
		}
	}
	return bestCell;
}


// find a random speaker channel that cells of organType are using.
int findOccupiedChannel( int animalIndex,  int organType)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (game.animals[animalIndex].body[cellIndex].organ == organType)
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list< int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return game.animals[animalIndex] .body[(*iterator)].speakerChannel;
	}
	else
	{
		return -1;
	}
}

// modify every cell which uses channel, into using a cnew hannel which is the sum of channel and increment. Increment may be negative.
void modifyChannel( int animalIndex, int channel, int increment)
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
int getRandomPopulatedCell( int animalIndex)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		cellsOfType.push_back(cellIndex);
		found++;
	}
	if (found > 0)
	{
		std::list< int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return *iterator;
	}
	return -1;
}

// the opposite of append cell- remove a cell from the genes and body, shifting all other cells backwards and updating all connections.
void eliminateCell(  int animalIndex,  int cellToDelete )
{
	// record the signal intensities, so when you reconstruct the cells from their genes, you can put the intensities back just so.
	float signalIntensities[animalSquareSize];
	for (int cellIndex = 0; cellIndex < animalSquareSize; ++cellIndex)
	{
		signalIntensities[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity;
	}
	for (int cellIndex = cellToDelete + 1; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex) // shift array of cells down 1, overwriting the lowest modified cell (the cell to delete)
	{
		game.animals[animalIndex].body[cellIndex - 1] = game.animals[animalIndex].body[cellIndex];
	}

	if (game.animals[animalIndex].cellsUsed > 0)
	{

		game.animals[animalIndex].cellsUsed--; // clear the end cell which would have been duplicated
		for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)	// go through all cells and update connections
		{
			for (int connectionIndex = 0; connectionIndex < NUMBER_OF_CONNECTIONS; ++connectionIndex)
			{
				if (game.animals[animalIndex].body[cellIndex].connections[connectionIndex].connectedTo == cellToDelete	)
				{
					game.animals[animalIndex].body[cellIndex].connections[connectionIndex].used = false;
				}
				else if (game.animals[animalIndex].body[cellIndex].connections[connectionIndex].connectedTo > cellToDelete)
				{
					game.animals[animalIndex].body[cellIndex].connections[connectionIndex].connectedTo --;
				}
			}
		}
		for (int cellIndex = 0; cellIndex < animalSquareSize - 1; ++cellIndex)
		{
			game.animals[animalIndex].body[cellIndex].signalIntensity = signalIntensities[cellIndex + 1];
		}
	}
}

void mutateAnimal( int animalIndex)
{
	// some mutations are chosen more commonly than others. They are classed into groups, and each group is associated with a normalized likelyhood of occurring.
	// the reason for this is that the development of the brain must always occur faster and in more detail than the development of the body, or else intelligent behavior can never arise.
	const float group1Probability = 1.0f;//1.0f;
	const float group2Probability = 0.75f;//0.5f;
	const float group3Probability = 0.5f;//0.125f;
	const float group4Probability = 0.25f;//0.0625f;
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

	// animal receives a skin color mutation every time.
	int mutantSkinCell = getRandomPopulatedCell(animalIndex);
	if (mutantSkinCell >= 0 && mutantSkinCell < animalSquareSize)
	{
		game.animals[animalIndex].body[mutantSkinCell].color = mutateColor(	game.animals[animalIndex].body[mutantSkinCell].color);
	}

	int mutation = MATERIAL_NOTHING;// choose a mutation from the group randomly.
	if (group == 1)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(1);
		if (mutationChoice == 0)
		{
			mutation = MUTATION_ADDWEIGHT;
		}
		else if (mutationChoice == 1)
		{
			mutation = MUTATION_MULTIPLYWEIGHT;
		}
		else if (mutationChoice == 2)
		{
			mutation = MUTATION_ALTERBIAS;
		}
	}
	else if (group == 2)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(2);
		if (mutationChoice == 0)
		{
			mutation = MUTATION_MAKECONNECTION;
		}
		else if (mutationChoice == 1)
		{
			mutation = MUTATION_BREAKCONNECTION;
		}
		else if (mutationChoice == 2)
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
		animalAppendCell(animalIndex, extremelyFastNumberFromZeroTo(numberOfOrganTypes));
		break;
	}
	case MUTATION_MAKECONNECTION:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			int mutantConnection = getRandomUnusedConnection(animalIndex, mutantCell);
			if (mutantConnection >= 0)
			{
				game.animals[animalIndex].body[mutantCell].connections[mutantConnection].used = true;//!(game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
				// connect it to whatever
				int target = getRandomConnectableCell(animalIndex);
				if (target >= 0)
				{
					game.animals[animalIndex].body[mutantCell].connections[mutantConnection].connectedTo = target;
					game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight = ((RNG() - 0.5f) * 2.0f );
				}
			}
		}
		break;
	}

	case MUTATION_BREAKCONNECTION:  // find an existing connection and break it
	{
		int mutantCell =  getRandomConnectedCell(animalIndex);
		if (mutantCell >= 0)
		{
			int mutantConnection = getRandomUsedConnection(animalIndex, mutantCell);
			if (mutantConnection >= 0)
			{
				game.animals[animalIndex].body[mutantCell].connections[mutantConnection].used = false;//!(game.animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
			}
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
				game.animals[animalIndex].body[mutantCell].connections[mutantConnection].connectedTo = mutantPartner;
			}
		}
		break;
	}
	case MUTATION_ADDWEIGHT:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight += ((RNG() - 0.5f) * neuralMutationStrength);
		}
		break;
	}
	case MUTATION_MULTIPLYWEIGHT:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight *= ((RNG() - 0.5f) * neuralMutationStrength);
		}
		break;
	}
	case MUTATION_ALTERBIAS:// randomise a bias neuron's strength.
	{
		int mutantCell = getRandomCellOfType(animalIndex, ORGAN_BIASNEURON);
		if (mutantCell >= 0)
		{
			game.animals[animalIndex].body[mutantCell].signalIntensity *= ((RNG() - 0.5f ) * neuralMutationStrength);
			game.animals[animalIndex].body[mutantCell].signalIntensity += ((RNG() - 0.5f ) * neuralMutationStrength);

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
				game.animals[animalIndex].body[mutantCell].eyeLookX += (extremelyFastNumberFromZeroTo(2) - 1);
			}
			else
			{
				game.animals[animalIndex].body[mutantCell].eyeLookY += (extremelyFastNumberFromZeroTo(2) - 1);
			}
		}
		break;
	}
	}
}

void rebuildBodyFromGenes( int animalIndex)
{
	if (animalIndex < numberOfAnimals)
	{
		game.animals[animalIndex].damageReceived = 0;
		for (int i = 0; i < animalSquareSize; ++i)
		{
			game.animals[animalIndex].body[i].damage = 0.0f;
		}

		measureAnimalQualities(animalIndex);
	}
}

void spawnAnimalIntoSlot(  int animalIndex,
                           Animal parent,
                           unsigned int position, bool mutation) // copy genes from the parent and then copy body from own new genes.
{
	resetAnimal(animalIndex);
	for (int i = 0; i < animalSquareSize; ++i)
	{
		game.animals[animalIndex].body[i] = parent.body[i];
	}
	game.animals[animalIndex].isMachine = parent.isMachine;
	game.animals[animalIndex].machineCallback = parent.machineCallback;
	game.animals[animalIndex].cellsUsed = parent.cellsUsed;	game.animals[animalIndex].position = position;
	game.animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	game.animals[animalIndex].fPosY = position / worldSize;
	game.animals[animalIndex].birthLocation = position;
	game.animals[animalIndex].fAngle = ( (RNG() - 0.5f) * 2 * const_pi );
	rebuildBodyFromGenes( animalIndex);
	if (mutation)
	{
		mutateAnimal( animalIndex);
	}
	memcpy( &( game.animals[animalIndex].displayName[0]), &(parent.displayName[0]), sizeof(char) * displayNameSize  );
	game.animals[animalIndex].retired = false;
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(int animalIndex, unsigned int cellWorldPositionI)
{
	ZoneScoped;
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



// updates the animal's position, and performs any actions resulting from that.
void place( int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	game.animals[animalIndex].fAngleCos = cos(game.animals[animalIndex].fAngle);
	game.animals[animalIndex].fAngleSin = sin(game.animals[animalIndex].fAngle);

	bool trailUpdate = false;
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

	unsigned int newPosX  = game.animals[animalIndex].fPosX;
	unsigned int newPosY  = game.animals[animalIndex].fPosY;
	unsigned int newPosition  =  (newPosY * worldSize) + newPosX;
	if (newPosition < worldSquareSize)
	{
		if (  materialBlocksMovement( game.world[game.animals[animalIndex].position].wall ) ) // vibrate out of wall if stuck.
		{
			game.animals[animalIndex].fPosX += ( (RNG() - 0.5f) * 10.0f  );
			game.animals[animalIndex].fPosY += ( (RNG() - 0.5f) * 10.0f  );
		}

		if (  materialBlocksMovement( game.world[newPosition].wall ) ) // don't move into walls.
		{
			game.animals[animalIndex].fPosX  = game.animals[animalIndex].uPosX;
			game.animals[animalIndex].fPosY  = game.animals[animalIndex].uPosY;
		}
		else
		{
			game.animals[animalIndex].uPosX  = game.animals[animalIndex].fPosX;
			game.animals[animalIndex].uPosY  = game.animals[animalIndex].fPosY;
			game.animals[animalIndex].position = newPosition;
		}

		// animal temperature limits
		// if (false)
		// {
		// 	if (game.world[newPosition].temperature > game.animals[animalIndex].temp_limit_high)
		// 	{
		// 		game.animals[animalIndex].damageReceived += abs(game.world[newPosition].temperature  - game.animals[animalIndex].temp_limit_high);
		// 	}
		// 	if (game.world[newPosition].temperature < game.animals[animalIndex].temp_limit_low)
		// 	{
		// 		game.animals[animalIndex].damageReceived += abs(game.world[newPosition].temperature  - game.animals[animalIndex].temp_limit_low);
		// 	}
		// }
	}

	for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)                                      // place animalIndex on grid and attack / eat. add captured energy
	{
		if (taxIsByMass)
		{
			game.animals[animalIndex].energy -= game.ecoSettings[3] ;
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

				}
			}
			else
			{
				okToStep = true;
			}
		}

		if (okToStep)
		{
			// spawn grass
			if (speciesIndex > 0)
			{
				if (game.world[cellWorldPositionI].identity < 0)
				{
					if (materialSupportsGrowth(game.world[cellWorldPositionI].terrain) && ! materialBlocksMovement(game.world[cellWorldPositionI].wall) )
					{
#ifdef PLANTS
						if (game.world[cellWorldPositionI].plantState == MATERIAL_NOTHING)
						{
							game.world[cellWorldPositionI].plantState = MATERIAL_GRASS;
						}
#else
						if (game.world[cellWorldPositionI].wall == MATERIAL_NOTHING)
						{

							game.world[cellWorldPositionI].wall = MATERIAL_GRASS;
						}
#endif
					}
				}
			}

			game.world[cellWorldPositionI].identity = animalIndex;
			game.world[cellWorldPositionI].occupyingCell = cellIndex;

			if (trailUpdate)
			{
				game.world[cellWorldPositionI].trail    = dAngle;
			}

			// move pollen along with animal
			unsigned int prevWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
			if (prevWorldPositionI < worldSquareSize)
			{
				if (game.world[  prevWorldPositionI].seedState == MATERIAL_POLLEN)
				{

					if (game.world[cellWorldPositionI].seedState == MATERIAL_NOTHING)
					{
						game.world[cellWorldPositionI].seedState = MATERIAL_POLLEN;
						memcpy( &(game.world[cellWorldPositionI].seedGenes[0]), &(game.world[prevWorldPositionI].seedGenes[0]) , sizeof(char)*plantGenomeSize);
						game.world[prevWorldPositionI].seedState = MATERIAL_NOTHING;
					}
				}
			}
			game.animals[animalIndex].body[cellIndex].worldPositionI = cellWorldPositionI;
		}
	}
}

int spawnAnimal( unsigned int speciesIndex,
                 Animal parent,
                 unsigned int position, bool mutation)
{
	int animalIndex = getNewIdentity(speciesIndex);
	if (animalIndex >= 0) // an animalIndex was available
	{
		spawnAnimalIntoSlot(animalIndex,
		                    parent,
		                    position, mutation);

		place(animalIndex);
	}
	return animalIndex;
}

void killAnimal(int animalIndex)
{
	if (animalIndex == game.playerCreature)
	{
		game.playerCreature = -1;
		appendLog(std::string("You died!"));
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
			if (game.animals[animalIndex].body[cellIndex].organ != MATERIAL_NOTHING && game.animals[animalIndex].body[cellIndex].damage < 1.0f)
			{
				game.world[cellWorldPositionI].pheromoneChannel = 13;
				game.world[cellWorldPositionI].pheromoneIntensity = 1.0f;
				if (game.world[cellWorldPositionI].wall == MATERIAL_NOTHING)
				{
					game.world[cellWorldPositionI].wall = MATERIAL_FOOD;
				}
				if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_BONE)
				{
					game.world[cellWorldPositionI].wall = MATERIAL_BONE;
				}
			}
		}
	}
}

Color whatColorIsThisSquare(  unsigned int worldI)
{
	ZoneScoped;
	Color displayColor = color_black;
	int viewedAnimal = -1;
	int animalIndex = game.world[worldI].identity;
	int occupyingCell = -1;
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		occupyingCell = isAnimalInSquare(  animalIndex , worldI    );
		if (occupyingCell >= 0)
		{
			viewedAnimal = animalIndex;
		}
	}
	if (viewedAnimal != -1)
	{
		if (organVisible(game.animals[viewedAnimal].body[occupyingCell].organ ))
		{
			displayColor = organColors(game.animals[viewedAnimal].body[occupyingCell].organ );
		}

		if ( game.animals[viewedAnimal].body[occupyingCell].damage > 0.5f  )
		{
			displayColor = organColors(game.animals[viewedAnimal].body[occupyingCell].organ );
			displayColor = mixColor( displayColor, color_brightred , ((game.animals[viewedAnimal].body[occupyingCell].damage ) - 0.5f) * 2.0f );
		}
		else
		{
			displayColor = game.animals[viewedAnimal].body[occupyingCell].color;
			displayColor = mixColor( color_brightred, displayColor, (game.animals[viewedAnimal].body[occupyingCell].damage) * 2.0f );
		}
		if (false)
		{
			if ( game.world[worldI].seedState == MATERIAL_POLLEN ) // pollen is visible over the top of animals, because it can cling to them.
			{
				displayColor = game.world[worldI].seedColor;
			}
		}
	}
	else
	{
		//1. terrain.
		displayColor = materialColors(game.world[worldI].terrain);
#ifdef PLANTS
		if ( game.world[worldI].seedState == MATERIAL_SEED )
		{
			displayColor = game.world[worldI].seedColor;
		}
		else if ( game.world[worldI].seedState == MATERIAL_POLLEN )
		{
			displayColor = game.world[worldI].seedColor;
		}
		else if ( game.world[worldI].plantState != MATERIAL_NOTHING)
		{
			displayColor = game.world[worldI].grassColor;
		}
#endif
		//2. wall
		if (game.world[worldI].wall != MATERIAL_NOTHING)
		{
			if (materialIsTransparent(game.world[worldI].wall))
			{
				displayColor = filterColor( displayColor,  multiplyColorByScalar( materialColors(game.world[worldI].wall), 0.5f ) );
			}
			else
			{
				displayColor = materialColors(game.world[worldI].wall);//filterColor( displayColor,  multiplyColorByScalar( wallColor, 0.5f ) );
			}
		}
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
		float brightness = 1.0f - ((xSurfaceDifference + ySurfaceDifference) / (2.0f * const_pi));
		brightness *= 0.5;
		brightness += 0.5f;
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
		progressString = std::string(" ") + std::to_string(pass) + std::string("/") + std::to_string(passes);
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

#ifdef PLANTS

unsigned int plantIdentityCursor = 0;

int getNewPlantIdentity()
{
	plantIdentityCursor++;
	return plantIdentityCursor;
}

void mutatePlants(unsigned int worldI)
{
	unsigned int mutationChoice = extremelyFastNumberFromZeroTo(2);
	unsigned int mutationIndex = extremelyFastNumberFromZeroTo(plantGenomeSize - 1);

	if (mutationChoice == 0)
	{	// swap a letter
		game.world[worldI].plantGenes[mutationIndex] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
	}
	else if (mutationChoice == 1)
	{	// insert a letter
		if (mutationIndex == 0) {mutationIndex = 1;}
		for (int i = plantGenomeSize - 1; i > mutationIndex; --i)
		{
			game.world[worldI].plantGenes[i] = game.world[worldI].plantGenes[i - 1] ;
		}
		game.world[worldI].plantGenes[mutationIndex] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);

	}
	else if (mutationChoice == 2)
	{	// remove a letter
		for (int i = mutationIndex ; i < plantGenomeSize - 1; ++i)
		{
			game.world[worldI].plantGenes[i] = game.world[worldI].plantGenes[i + 1] ;
		}
		game.world[worldI].plantGenes[plantGenomeSize - 1] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
	}
}

void growInto(unsigned int from, unsigned int to, unsigned int organ)
{
	if (!( materialBlocksMovement (game.world[to].wall)))
	{
		game.world[to].plantState = organ;

		game.world[to].identity = game.world[from].identity;
		game.world[to].energy = 0.0f;
		game.world[to].geneCursor = game.world[from].geneCursor;
		game.world[to].grassColor = game.world[from].grassColor;

		game.world[to].sequenceNumber = game.world[from].sequenceNumber;
		game.world[to].sequenceReturn = game.world[from].sequenceReturn;

		game.world[to].grown = false;

		if (organ == MATERIAL_SEED)
		{
			game.world[to].energyDebt = 2.0f;
		}
		else
		{

			game.world[to].energyDebt = 1.0f;
		}
		memcpy( &(game.world[to].plantGenes[0]), &(game.world[from].plantGenes[0]), sizeof(char) * plantGenomeSize   );
	}
}

void growPlants(unsigned int worldI)
{
	if (game.world[worldI].plantState == MATERIAL_GRASS)
	{
		for (int n = 0; n < nNeighbours; ++n)
		{
			unsigned int neighbour = worldI + neighbourOffsets[n];
			if (neighbour < worldSquareSize)
			{
				if (game.world[neighbour].plantState == MATERIAL_NOTHING && !materialBlocksMovement(game.world[neighbour].wall ) &&  materialSupportsGrowth(game.world[neighbour].terrain ) )
				{
					float growthChance =  1.0f - colorAmplitude(  multiplyColor( game.world[worldI].grassColor , game.world[neighbour].light )); // grow speed proportional to light brightness

					growthChance *= 0.10f;

					if (RNG() < growthChance )
					{
						growInto(worldI, neighbour, MATERIAL_GRASS);

						game.world[neighbour].grassColor = mutateColor (game.world[worldI].grassColor);
					}
				}
			}
		}
		return;
	}

	unsigned int skipTo = game.world[worldI].geneCursor;
	bool skip = false;
	unsigned int turns = 0;
	while (true)
	{
		turns++; if (turns > plantGenomeSize) { return; }
		game.world[worldI].geneCursor ++;
		if (game.world[worldI].geneCursor >= plantGenomeSize) {return;}
		char c = game.world[worldI].plantGenes[game.world[worldI].geneCursor];
		bool done = false;

		if (c < nNeighbours)
		{
			game.world[worldI].growthMatrix[c] = !(game.world[worldI].growthMatrix[c]);
			continue;
		}
		else
		{
			switch (c)
			{
			case PLANTGENE_GROW_SYMM_H:
			{
				bool originalGrowthMatrix[nNeighbours];
				memcpy(&(originalGrowthMatrix[0]), &(game.world[worldI].growthMatrix[0]), sizeof(bool)*nNeighbours  );
				memset(&(game.world[worldI].growthMatrix), false, sizeof(bool)*nNeighbours);
				if (originalGrowthMatrix[7]) { game.world[worldI].growthMatrix[5] = true;}
				if (originalGrowthMatrix[5]) { game.world[worldI].growthMatrix[7] = true;}
				if (originalGrowthMatrix[0]) { game.world[worldI].growthMatrix[4] = true;}
				if (originalGrowthMatrix[4]) { game.world[worldI].growthMatrix[0] = true;}
				if (originalGrowthMatrix[1]) { game.world[worldI].growthMatrix[3] = true;}
				if (originalGrowthMatrix[3]) { game.world[worldI].growthMatrix[1] = true;}
				break;
			}
			case PLANTGENE_GROW_SYMM_V:
			{
				bool originalGrowthMatrix[nNeighbours];
				memcpy(&(originalGrowthMatrix[0]), &(game.world[worldI].growthMatrix[0]), sizeof(bool)*nNeighbours  );
				memset(&(game.world[worldI].growthMatrix), false, sizeof(bool)*nNeighbours);
				if (originalGrowthMatrix[1]) { game.world[worldI].growthMatrix[5] = true;}
				if (originalGrowthMatrix[2]) { game.world[worldI].growthMatrix[6] = true;}
				if (originalGrowthMatrix[3]) { game.world[worldI].growthMatrix[7] = true;}
				if (originalGrowthMatrix[5]) { game.world[worldI].growthMatrix[1] = true;}
				if (originalGrowthMatrix[6]) { game.world[worldI].growthMatrix[2] = true;}
				if (originalGrowthMatrix[7]) { game.world[worldI].growthMatrix[3] = true;}
				break;
			}

			case PLANTGENE_SEQUENCE:
			{
				if (game.world[worldI].geneCursor + 2 < plantGenomeSize)
				{
					game.world[worldI].sequenceNumber = game.world[worldI].plantGenes[game.world[worldI].geneCursor + 1];
					game.world[worldI].sequenceReturn = game.world[worldI].geneCursor + 2;
				}
				break;
			}

			case PLANTGENE_BREAK:
			{
				// if the sequence number is greater than zero, return to the sequence origin and decrement the sequence number and depth.
				if (game.world[worldI].sequenceNumber > 0)
				{
					game.world[worldI].geneCursor = game.world[worldI].sequenceReturn;
					game.world[worldI].sequenceNumber--;
				}
				else
				{
					// if it is 0, you've completed doing the sequence n times, so you can exit it.
					// In this case, sequenceReturn of the next cells will be sampled from a cell before the sequence start, which allows nested sequences.
					int innerSequenceReturn = game.world[worldI].sequenceReturn;

					if (innerSequenceReturn > 3) // impossible to have a complete sequence header earlier than 3
					{
						// Why 3? Because sequence returns don't point at the sequence gene itself, they point at the first gene IN the sequence.
						// The header goes <last cell of outer sequence> <sequence gene> <length> <first cell> .. . you always return to the first cell inside the sequence
						// when breaking an inner sequence,  return to the last cell of outer sequence, which is 3 cells behind where the inner sequence returns to.
						unsigned int lastCellOfOuterSequence = innerSequenceReturn - 3;
						game.world[worldI].sequenceReturn = game.world[lastCellOfOuterSequence].sequenceReturn;
						game.world[worldI].sequenceNumber = game.world[lastCellOfOuterSequence].sequenceNumber;
					}
				}
				break;
			}

			case PLANTGENE_BUD:
			{
				for (int i = 0; i < nNeighbours; ++i)
				{
					if (game.world[worldI].growthMatrix[i])
					{
						unsigned int neighbour = worldI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							growInto(worldI, neighbour, MATERIAL_BUD);
							if (skip) { game.world[neighbour].geneCursor = skipTo;  }
						}
					}
				}
				done = true;
				break;
			}
			case PLANTGENE_LEAF:
			{
				for (int i = 0; i < nNeighbours; ++i)
				{
					if (game.world[worldI].growthMatrix[i])
					{
						unsigned int neighbour = worldI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							growInto(worldI, neighbour, MATERIAL_LEAF);
							if (skip) { game.world[neighbour].geneCursor = skipTo;  }
						}
					}
				}
				done = true;
				break;
			}
			case PLANTGENE_WOOD:
			{
				for (int i = 0; i < nNeighbours; ++i)
				{
					if (game.world[worldI].growthMatrix[i])
					{
						unsigned int neighbour = worldI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							growInto(worldI, neighbour, MATERIAL_WOOD);
							if (skip) { game.world[neighbour].geneCursor = skipTo;  }
						}
					}
				}
				done = true;
				break;
			}
			case PLANTGENE_GOTO:
			{
				unsigned int nextGene = game.world[worldI].geneCursor + 1;
				skipTo = game.world[worldI].plantGenes[nextGene];

				if (skipTo < nextGene) { skipTo = nextGene; }  // otherwise individual plants can will get into loops and grow enormously large. it's a good but boring strategy.

				skip = true;
				break;
			}
			case PLANTGENE_RED:
			{
				game.world[worldI].grassColor.r *= 1.35f;
				game.world[worldI].grassColor = clampColor(game.world[worldI].grassColor);
				break;
			}
			case PLANTGENE_GREEN:
			{
				game.world[worldI].grassColor.b *= 1.35f;

				game.world[worldI].grassColor = clampColor(game.world[worldI].grassColor);
				break;
			}
			case PLANTGENE_BLUE:
			{
				game.world[worldI].grassColor.b *= 1.35f;
				game.world[worldI].grassColor = clampColor(game.world[worldI].grassColor);
				break;
			}
			case PLANTGENE_LIGHT:
			{
				game.world[worldI].grassColor.r *= 1.35f;
				game.world[worldI].grassColor.g *= 1.35f;
				game.world[worldI].grassColor.b *= 1.35f;

				game.world[worldI].grassColor = clampColor(game.world[worldI].grassColor);
				break;
			}
			case PLANTGENE_DARK:
			{
				game.world[worldI].grassColor.r *= 0.75f;
				game.world[worldI].grassColor.g *= 0.75f;
				game.world[worldI].grassColor.b *= 0.75f;

				break;
			}
			}
			if (done)
			{
				game.world[worldI].grown = true;
				return;
			}
		}
	}
}

void updatePlants(unsigned int worldI)
{
	switch (game.world[worldI].seedState)
	{
	case MATERIAL_POLLEN:
	{
		// move seeds randomly
		unsigned int neighbour = worldI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
		if (neighbour < worldSquareSize)
		{
			if ( !materialBlocksMovement( game.world[neighbour].wall ) && game.world[neighbour].seedState == MATERIAL_NOTHING )
			{
				game.world[neighbour].seedState = MATERIAL_POLLEN;
				game.world[neighbour].identity = game.world[worldI].identity;
				memcpy( & (game.world[neighbour].seedGenes[0]) , &(game.world[worldI].seedGenes[0]),  plantGenomeSize * sizeof(char)  );
				game.world[worldI].seedState = MATERIAL_NOTHING;
			}
		}
		// pollen degrades if not attached to an animal
		bool bonded = false;
		if (game.world[worldI].identity >= 0 && game.world[worldI].identity < numberOfAnimals)
		{
			int bond = isAnimalInSquare( game.world[worldI].identity , worldI )  ;
			if (  bond >= 0 )
			{
				bonded = true;
			}
		}
		if (!bonded)
		{
			if (extremelyFastNumberFromZeroTo(100) == 0)
			{
				game.world[neighbour].seedState = MATERIAL_NOTHING;
			}
		}
		break;
	}
	case MATERIAL_SEED:
	{
		// spawn plants from seeds if applicable
		if (extremelyFastNumberFromZeroTo(10) == 0)
		{
			if (materialSupportsGrowth(game.world[worldI].terrain))
			{
				game.world[worldI].plantState = MATERIAL_WOOD;
				game.world[worldI].plantIdentity = getNewPlantIdentity();
				game.world[worldI].geneCursor = 0;
				game.world[worldI].energy = 1.0f;
				game.world[worldI].energyDebt = 0.0f;
				game.world[worldI].grassColor = color_green;

				memset(&(game.world[worldI].growthMatrix), false, sizeof(bool) * nNeighbours);
				memcpy( & (game.world[worldI].plantGenes[0]) , &(game.world[worldI].seedGenes[0]),  plantGenomeSize * sizeof(char)  );

				if (extremelyFastNumberFromZeroTo(1) == 0)
				{
					mutatePlants(worldI);
				}
			}
		}
		else
		{
			// move seeds randomly
			unsigned int neighbour = worldI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
			if (neighbour < worldSquareSize)
			{
				if ( !materialBlocksMovement( game.world[neighbour].wall ) && game.world[neighbour].seedState == MATERIAL_NOTHING )
				{
					game.world[neighbour].seedState = MATERIAL_SEED;
					game.world[neighbour].identity = game.world[worldI].identity;
					memcpy( & (game.world[neighbour].seedGenes[0]) , &(game.world[worldI].seedGenes[0]),  plantGenomeSize * sizeof(char)  );
					game.world[worldI].seedState = MATERIAL_NOTHING;
				}
			}
		}
		break;
	}
	}
	if (game.world[worldI].plantState == MATERIAL_BUD || game.world[worldI].plantState == MATERIAL_GRASS ||
	        game.world[worldI].plantState == MATERIAL_LEAF ||
	        game.world[worldI].plantState == MATERIAL_WOOD )
	{
		if (game.world[worldI].energyDebt < 0.0f)
		{
			growPlants(worldI);
		}
		else
		{
			if (game.world[worldI].energy > 0.0f)
			{
				if (game.world[worldI].plantState == MATERIAL_BUD)
				{
					if (game.world[worldI].energy > game.world[worldI].energyDebt) // the debt will finally be paid off.
					{
						for (int n = 0; n < nNeighbours; ++n)
						{
							unsigned int neighbour = worldI + neighbourOffsets[n];
							if (neighbour < worldSquareSize)
							{
								// create a pollen in the neighbouring square, it's not the same as normal growing.
								if (game.world[neighbour].seedState == MATERIAL_NOTHING)
								{
									memcpy( &(game.world[neighbour].seedGenes[0]), &(game.world[worldI].plantGenes[0]), sizeof(char) * plantGenomeSize   );
									game.world[neighbour].seedColor  = game.world[worldI].grassColor;
									game.world[neighbour].seedIdentity = game.world[worldI].identity;
									game.world[neighbour].seedState = MATERIAL_POLLEN;
									break;
								}
							}
						}
					}
				}
				game.world[worldI].energyDebt -= game.world[worldI].energy;
				game.world[worldI].energy = 0.0f;
			}
		}

		// grow plant into neighboring squares if applicable
		switch (game.world[worldI].plantState)
		{
		case MATERIAL_LEAF:
		{
			game.world[worldI].energy += colorAmplitude(    multiplyColor(  game.world[worldI].light , game.world[worldI].grassColor)) ;
			break;
		}
		case MATERIAL_WOOD:
		{
			// transfer energy between adjacent cells with same ID
			for (int i = 0; i < nNeighbours; ++i)
			{
				unsigned int neighbour = worldI + neighbourOffsets[  i ];
				if (neighbour < worldSquareSize)
				{
					if ( game.world[neighbour].identity == game.world[worldI].identity)
					{
						float avg = game.world[worldI].energy + game.world[neighbour].energy;
						avg *= 0.5f;
						game.world[worldI].energy = avg;
						game.world[neighbour].energy = avg;
					}
				}
			}
			break;
		}

		case MATERIAL_BUD:
		{
			if (game.world[worldI].energyDebt < 0.0f)
			{
				for (int n = 0; n < nNeighbours; ++n)
				{
					unsigned int neighbour = worldI + neighbourOffsets[n];
					if (neighbour < worldSquareSize)
					{
						if (game.world[neighbour].seedState == MATERIAL_POLLEN)
						{
							const float plantSpeciesThreshold = 0.85f;
							if (
							    // plant species is basically organized by the color of their seeds.
							    game.world[neighbour].seedColor.r > (game.world[worldI].grassColor.r * plantSpeciesThreshold) &&
							    game.world[neighbour].seedColor.g > (game.world[worldI].grassColor.g * plantSpeciesThreshold) &&
							    game.world[neighbour].seedColor.b > (game.world[worldI].grassColor.b * plantSpeciesThreshold) &&

							    game.world[neighbour].seedColor.r < (game.world[worldI].grassColor.r * (1.0 / plantSpeciesThreshold)) &&
							    game.world[neighbour].seedColor.g < (game.world[worldI].grassColor.g * (1.0 / plantSpeciesThreshold)) &&
							    game.world[neighbour].seedColor.b < (game.world[worldI].grassColor.b * (1.0 / plantSpeciesThreshold))
							)
							{
								// sex between two plants
								memcpy( & (game.world[worldI].seedGenes[0]) , &(game.world[worldI].plantGenes[0]),  plantGenomeSize * sizeof(char)  );
								for (int i = 0; i < plantGenomeSize; ++i)
								{
									if (extremelyFastNumberFromZeroTo(1) == 0)
									{
										game.world[worldI].seedGenes[i] =  game.world[neighbour].seedGenes[i] ;
									}
								}
								game.world[neighbour].seedState = MATERIAL_NOTHING;
								game.world[worldI].plantState = MATERIAL_SEED;
								game.world[worldI].seedColor = game.world[worldI].grassColor;
								break;
							}
						}
					}
				}
			}
			break;
		}

		case MATERIAL_GRASS:
		{
			game.world[worldI].energy += colorAmplitude(    multiplyColor(  game.world[worldI].light , game.world[worldI].grassColor)) ;
			break;
		}
		}
	}
}

#endif


void updateMapI(unsigned int randomI)
{
// slowly reduce pheromones over time.

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


	// if (game.world[randomI].wall == MATERIAL_FOOD)
	// {

	// 	game.world[randomI].wall = MATERIAL_NOTHING;
	// 	if (
	// 	    game.world[randomI].plantState == MATERIAL_NOTHING)
	// 	{

	// 		game.world[randomI].plantState = MATERIAL_GRASS;
	// 	}
	// }



	if (game.world[randomI].wall == MATERIAL_FIRE)
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			unsigned int neighbour = randomI + neighbourOffsets[i];
			if (neighbour < worldSquareSize)
			{
				if (game.world[neighbour].wall == MATERIAL_GRASS)
				{
					game.world[randomI].wall = MATERIAL_FIRE;
				}
			}
		}
		if (extremelyFastNumberFromZeroTo(2) == 0)
		{
			game.world[randomI].wall = MATERIAL_NOTHING;
		}
		else
		{
			unsigned int neighbour = randomI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
			if (neighbour < worldSquareSize)
			{
				game.world[neighbour].wall = MATERIAL_FIRE;
				game.world[randomI].wall = MATERIAL_NOTHING;
			}
		}
	}

	if ( materialDegrades( game.world[randomI].wall) )
	{
		game.world[randomI].wall = MATERIAL_NOTHING;
	}

#ifdef PLANTS

	if ( game.world[randomI].plantState != MATERIAL_NOTHING)
	{
		updatePlants(randomI);
	}

#else

	if (game.world[randomI].wall == MATERIAL_GRASS)
	{
		for (int n = 0; n < nNeighbours; ++n)
		{
			unsigned int neighbour = randomI + neighbourOffsets[n];
			if (neighbour < worldSquareSize)
			{
				if (game.world[neighbour].wall == MATERIAL_NOTHING && !materialBlocksMovement(game.world[neighbour].wall ) &&  materialSupportsGrowth(game.world[neighbour].terrain ) )
				{
					game.world[neighbour].wall = MATERIAL_GRASS;
					game.world[neighbour].pheromoneChannel = 6;
					game.world[neighbour].pheromoneIntensity = 1.0f; // the smell of grass
				}
			}
		}
	}
#endif
}


void updateMapSector( unsigned int sector )
{
	ZoneScoped;
	unsigned int from = sector * sectorSize;
	unsigned int to   = ((sector + 1 ) * sectorSize ) - 1;

	// generate a huge ass list of the map squares you're going to update. Then update them all at once. more than 2x faster than generating and visiting each one in turn.
	unsigned int squaresToUpdate[updateSize];

	for (int i = 0; i < updateSize; ++i)
	{
		squaresToUpdate[i] =  from + extremelyFastNumberFromZeroTo(to);
	}

	for (int i = 0; i < updateSize; ++i)
	{
		if (squaresToUpdate[i] < worldSquareSize)
		{

			updateMapI(squaresToUpdate[i]);
		}
	}
}

void updateMap()
{

	boost::thread mapSectors[sectors];

	for (int i = 0; i < sectors; ++i)
	{
		mapSectors[i] = boost::thread { updateMapSector, i };
	}
	for (int i = 0; i < sectors; ++i)
	{
		mapSectors[i].join();
	}

	// prevent grass from going extinct.
	// for (int i = 0; i < 100; ++i)
	// {
	unsigned int newGrass = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
	if (game.world[newGrass].plantState == MATERIAL_NOTHING && materialSupportsGrowth(game.world[newGrass].terrain))
	{
		game.world[newGrass].plantState = MATERIAL_GRASS;
	}
	// }

	for (unsigned int i = 0; i < numberOfSpeakerChannels; ++i)
	{
		game.speakerChannelsLastTurn [i] = game.speakerChannels[i];
		game.speakerChannels[i] = 0.0f;
	}
}

int defenseAtWorldPoint( int animalIndex, unsigned int cellWorldPositionI)
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

Vec_i2 getMousePositionRelativeToAnimal( int animalIndex)
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

void rightClickCallback ()
{
	if (game.palette)
	{
		paletteEraseAtMouse();
	}

	if (game.ecologyComputerDisplay)
	{
		game.ecoSettings[game.activeEcoSetting] *= (1.0f / 1.5f);
	}
}

void drawPalette(int menuX, int menuY)
{
	int closestValue = 1000;
	unsigned int tempClosestToMouse = 0;
	int paletteFinalX ;
	int paletteFinalY ;

	// draw the available tiles.
	for (int i = 0; i < numberOfOrganTypes; ++i)
	{
		unsigned int paletteX = i % paletteWidth;
		unsigned int paletteY = i / paletteWidth;
		paletteFinalX = menuX + (paletteX * paletteSpacing * 10);
		paletteFinalY = menuY + (paletteY * paletteSpacing);
		if (i == game.paletteSelectedOrgan)
		{
			printText2D(  std::string("X ") +  tileShortNames(i) , paletteFinalX, paletteFinalY, paletteTextSize);
		}
		else
		{
			printText2D(   tileShortNames(i) , paletteFinalX, paletteFinalY, paletteTextSize);
		}
	}

	// draw the selected creature all big like, i guess.
}



void drawPalette2()
{
	if (game.selectedAnimal >= 0)
	{
		for (int i = 0; i < game.animals[game.selectedAnimal].cellsUsed; ++i)
		{
			float startPosX = 500.0f;
			float startPosY = 500.0f;
			float bigSquareSize = 100.0f;

			drawRectangle( Vec_f2(startPosX + (game.animals[game.selectedAnimal].body[i].localPosX * bigSquareSize), startPosY + (game.animals[game.selectedAnimal].body[i].localPosY * bigSquareSize)) ,
			               organColors(game.animals[game.selectedAnimal].body[i].organ), bigSquareSize, bigSquareSize);

		}
	}
}

void spill(unsigned int material,  unsigned int worldPositionI)

// void spillBlood(unsigned int worldPositionI)
{
	if (game.world[worldPositionI].wall == MATERIAL_NOTHING)
	{
		game.world[worldPositionI].wall = material;
	}
	else
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			unsigned int neighbour = worldPositionI += neighbourOffsets[i];
			if ( neighbour < worldSquareSize)
			{
				if (game.world[neighbour].wall == MATERIAL_NOTHING)
				{
					game.world[neighbour].wall = material;
					break;
				}
			}
		}
	}
}

void defeatAdversary()
{
	int i = 1;
	setupNeuroGlasses(i);
	spawnAnimalIntoSlot(8, game.animals[i], game.animals[game.adversary].position, false);

	game.adversaryDefeated = true;
	killAnimal(game.adversary);

	appendLog( std::string("the adversary was killed!") );
}

void healAnimal( int animalIndex)
{
	int pre = 0;
	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		if (game.animals[animalIndex].body[i].damage > 1.0f)
		{
			pre++;
		}
	}

	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		if (game.animals[animalIndex].body[i].damage > 0.0f)
		{
			game.animals[animalIndex].body[i].damage -= 0.05f;
		}
		else
		{
			game.animals[animalIndex].body[i].damage = 0.0f;
		}
	}

	int post = 0;
	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		if (game.animals[animalIndex].body[i].damage > 1.0f)
		{
			post++;
		}
	}

	int diff = pre - post;
	if (diff <= game.animals[animalIndex].damageReceived)
	{
		game.animals[animalIndex].damageReceived -= diff;
	}

}

// return true if you blow the limb off, false if its still attached.
void hurtAnimal( int animalIndex, unsigned int cellIndex, float amount, int shooterIndex)
{
	std::string damageLog = std::string("");
	bool limbLost = false;

	unsigned int cellWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
	float defense = defenseAtWorldPoint(game.world[cellWorldPositionI].identity, cellWorldPositionI);
	if (defense > 0)
	{
		amount = amount / defense;
	}
	game.animals[animalIndex].body[cellIndex].damage += amount;
	// spillBlood(cellWorldPositionI);



	int painCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_PAIN);
	if (painCell >= 0)
	{
		game.animals[animalIndex].body[painCell].signalIntensity += amount;
	}
	if (game.animals[animalIndex].body[cellIndex].damage > 1.0f)
	{
		game.animals[animalIndex].damageReceived++;

		if (animalIndex == game.adversary && shooterIndex == game.playerCreature)
		{
			if (game.animals[game.adversary].damageReceived > game.animals[game.adversary].mass / 2)
			{
				defeatAdversary();
			}

		}
		limbLost = true;
	}

	if (animalIndex == game.playerCreature)
	{
		std::string num_text = std::to_string(amount);
		std::string rounded = num_text.substr(0, num_text.find(".") + 2);
		damageLog += std::string("Hit for ") + std::string( rounded );

		if (limbLost)
		{
			damageLog += std::string(", lost a ") + tileShortNames(game.animals[animalIndex].body[cellIndex].organ ) + std::string("!");
		}

		appendLog(damageLog);
	}

	bool dropped = false;
	if (limbLost)
	{
		if (game.animals[animalIndex].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
		{
			// if (game.world[cellWorldPositionI].wall == MATERIAL_NOTHING)
			// {
			// 	game.world[cellWorldPositionI].wall = organProduces(game.animals[animalIndex].body[cellIndex].organ);//MATERIAL_FOOD;
			// }
			spill(organProduces(game.animals[animalIndex].body[cellIndex].organ), cellWorldPositionI);
			dropped = true;
		}
	}

	if (!dropped)
	{
		spill(MATERIAL_BLOOD,  cellWorldPositionI);
	}

}

int lastCutSquare = 0;

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
			if (occupyingCell != lastCutSquare)
			{
				hurtAnimal(game.cursorAnimal, occupyingCell, 0.3f, shooterIndex);
				lastCutSquare = occupyingCell;
			}
		}
	}
}

void shoot(unsigned int gunIndex, int shooterIndex,  unsigned int shootWorldPosition, float angle)
{
	float bulletPosX = shootWorldPosition % worldSize;
	float bulletPosY = shootWorldPosition / worldSize;

	// unsigned int range = 250;
	for (int i = 0; i < destroyerRange; ++i)
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
				int shotOffNub = isAnimalInSquare(game.world[shootWorldPosition].identity, shootWorldPosition);
				if (shotOffNub >= 0 && shotOffNub < animalSquareSize)
				{
					hurtAnimal(game.world[shootWorldPosition].identity, shotOffNub, 0.35f + RNG(), shooterIndex);
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

void exampleGunCallback( int gunIndex, int shooterIndex)
{
	if (gunIndex >= 0)
	{
		shoot( gunIndex, shooterIndex,  game.animals[gunIndex].position, game.animals[gunIndex].fAngle);
	}
}

void lighterCallback( int gunIndex, int shooterIndex )
{
	if (gunIndex >= 0)
	{
		int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
		int cursorPosY = game.cameraPositionY + game.mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
		if ( game.world[worldCursorPos].wall == MATERIAL_NOTHING  )
		{
			game.world[worldCursorPos].wall = MATERIAL_FIRE;
		}
	}
}

int getGrabbableItem( int animalIndex, unsigned int cellIndex)
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
						for (unsigned int cellIndexB = 0; cellIndexB < game.animals[animalIndex].cellsUsed; ++cellIndexB)                                      // place animalIndex on grid and attack / eat. add captured energy
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

void notifyLMBUp()
{
	game.playerLMBDown = false;
}

void activateGrabbedMachine()// occurs whenever a left click is received.
{
	game.playerLMBDown = true;
	if (game.playerCreature >= 0)
	{
		if (game.playerActiveGrabber >= 0)
		{
			if (game.animals [ game.playerCreature].body[game.playerActiveGrabber].grabbedCreature >= 0)
			{
				if (game.animals [   game.animals [ game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].isMachine)
				{
					switch (game.animals [   game.animals [ game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback )
					{
					case MACHINECALLBACK_PISTOL :
						exampleGunCallback(game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
						break;
					case MACHINECALLBACK_LIGHTER :
						lighterCallback(game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
						break;
					case MACHINECALLBACK_HOSPITAL :
						paletteCallback(game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
						break;
					case MACHINECALLBACK_MESSAGECOMPUTER1:
						game.computerdisplays[0] = ! game.computerdisplays[0] ;
						break;
					case MACHINECALLBACK_MESSAGECOMPUTER2 :
						game.computerdisplays[1] = ! game.computerdisplays[1] ;
						break;
					case MACHINECALLBACK_MESSAGECOMPUTER3 :
						game.computerdisplays[2] = ! game.computerdisplays[2] ;
						break;
					case MACHINECALLBACK_MESSAGECOMPUTER4 :
						game.computerdisplays[3] = ! game.computerdisplays[3] ;
						break;
					case MACHINECALLBACK_MESSAGECOMPUTER5:
						game.computerdisplays[4] = ! game.computerdisplays[4] ;
						break;
					}
				}
			}
		}
	}
	if (game.ecologyComputerDisplay)
	{
		game.ecoSettings[game.activeEcoSetting] *= 1.5f;
	}
}

void sexBetweenTwoCreatures(unsigned int a, unsigned int b)
{
	if ( game.animals[a].energyDebt <= 0.0f && game.animals[b].energyDebt <= 0.0f  )
	{
		if (game.animals[a].energy > game.animals[a].offspringEnergy && game.animals[b].energy > game.animals[b].offspringEnergy )
		{
			float energyDonation = game.animals[a].offspringEnergy + game.animals[b].offspringEnergy ;
			game.animals[a].energy -= game.animals[a].offspringEnergy;
			game.animals[b].energy -= game.animals[b].offspringEnergy;
			unsigned int bSpecies = b % numberOfAnimalsPerSpecies;
			int newAnimal = spawnAnimal( bSpecies, game.animals[b], game.animals[b].position, true );
			if (newAnimal >= 0)
			{
				for (int i = 0; i < animalSquareSize; ++i)
				{
					if (extremelyFastNumberFromZeroTo(1) == 0)
					{
						game.animals[newAnimal].body[i] = game.animals[a].body[i];
					}
				}
				rebuildBodyFromGenes( newAnimal);
				game.animals[newAnimal].energy += energyDonation;
			}

			int pleasureCellA = getRandomCellOfType(a, ORGAN_SENSOR_PLEASURE);
			if (pleasureCellA >= 0)
			{
				game.animals[a].body[pleasureCellA].signalIntensity += 1.0f;
			}

			int pleasureCellB = getRandomCellOfType(b, ORGAN_SENSOR_PLEASURE);
			if (pleasureCellB >= 0)
			{
				game.animals[b].body[pleasureCellB].signalIntensity += 1.0f;
			}
		}
	}
}




float smallestAngleBetween(float x, float y)
{
	float a = fmod ( (x - y), (2 * const_pi));
	float b = fmod ( (y - x), (2 * const_pi));
	if (a < b)
	{
		return (a * -1.0f);
	}
	return b;
}

inline void sumInputs( int animalIndex,  int cellIndex)
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

void animal_organs( int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	float totalLiver = 0;
	unsigned int totalGonads = 0;
	float highestIntensity = 0.0f;

	float sensorium[game.animals[animalIndex].cellsUsed];

	for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{

		if (game.animals[animalIndex].body[cellIndex].damage > 1.0f) { continue;}
		unsigned int cellWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
		if (cellWorldPositionI >= worldSquareSize) {continue;}
		unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
		unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;
		unsigned int organ = game.animals[animalIndex].body[cellIndex].organ;
		switch (organ)
		{


		case ORGAN_EMITTER_WAX:
		{
			sumInputs(  animalIndex,   cellIndex);
			if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
			{
				spill(MATERIAL_WAX, cellWorldPositionI);
				game.animals[animalIndex].energy -= 1.0f;
			}
		}
		case ORGAN_EMITTER_HONEY:
		{
			sumInputs(  animalIndex,   cellIndex);
			if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
			{
				spill(MATERIAL_HONEY, cellWorldPositionI);
				game.animals[animalIndex].energy -= 1.0f;
			}
		}



		case ORGAN_SENSOR_AGE:
		{
			if (game.animals[animalIndex].lifespan > 0.0f)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity 
				float result = game.animals[animalIndex].age / game.animals[animalIndex].lifespan;
				sensorium[cellIndex] = result;
			}
			break;
		}
		case TILE_DESTROYER_EYE:
		{
			// pick a random tile within range, see if it contains an animal not of species 0, and shoot it if so.

			int closestvx ;
			int closestvy;
			float closestTargetDistance = destroyerRange;
			bool targetAcquired = false;
			for (int vy = -destroyerRange; vy < destroyerRange; vy += 4)
			{
				for (int vx = -destroyerRange; vx < destroyerRange; vx += 4)
				{
					int targetPosX = cellWorldPositionX + vx;
					int targetPosY = cellWorldPositionY + vy;
					unsigned int targetPosI = (targetPosY * worldSize) + targetPosX;
					if (targetPosI < worldSquareSize)
					{
						if (game.world[targetPosI].identity >= 0 && game.world[targetPosI].identity != animalIndex)
						{
							if ( ! (game.animals[game.world[targetPosI].identity ].isMachine) )
							{
								int targetCell = isAnimalInSquare( game.world[targetPosI].identity , targetPosI );
								if (targetCell >= 0)
								{
									float distanceToTarget = magnitude_int(  vx , vy   );
									if (distanceToTarget < closestTargetDistance)
									{
										targetAcquired = true;
										closestTargetDistance  = distanceToTarget;
										closestvx = vx;
										closestvy = vy;
									}
								}
							}
						}
					}
				}
			}
			if (targetAcquired)
			{
				float angleToTarget =  atan2( closestvy  , closestvx );
				const float destroyerInaccuracy = 0.1f;
				angleToTarget += (((extremelyFastNumberFromZeroTo(64) - 32.0f) / 32.0f )) * destroyerInaccuracy;
				shoot( animalIndex, animalIndex,  game.animals[animalIndex].position, angleToTarget);
			}
			break;
		}

		case ORGAN_SENSOR_RANDOM:
		{
			float randomness = ((extremelyFastNumberFromZeroTo(64) - 32.0f) / 32.0f );
			 sensorium[cellIndex] = randomness ;//game.animals[animalIndex].body[cellIndex].signalIntensity = //(RNG() - 0.5f) * 2.0f;
			break;
		}

		case ORGAN_GRABBER:
		{
			if (animalIndex == game.playerCreature)
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
			else
			{

				sumInputs(  animalIndex,   cellIndex);
				// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
				// for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
				// {
				// 	if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
				// 	{
				// 		unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
				// 		if (connected_to_cell < animalSquareSize)
				// 		{
				// 			game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
				// 		}
				// 	}
				// }
			}

			// Grab stuff.
			if (game.animals[animalIndex].body[cellIndex].signalIntensity  >= 1.0f && game.animals[animalIndex].body[cellIndex].grabbedCreature  == -1)
			{
				int potentialGrab = getGrabbableItem (animalIndex, cellIndex);
				if (potentialGrab >= 0)
				{
					game.animals[animalIndex].body[cellIndex].grabbedCreature = potentialGrab;
					// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					sensorium[cellIndex] = 0.0f;
				}
			}

			// Grabbed items behavior
			if (game.animals[animalIndex].body[cellIndex].grabbedCreature >= 0 )// if there is a grabbed creature, adjust its position to the grabber.
			{
				//Move grabbed items to the grabber position.
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


				// Dropping items.
				if (  game.animals[animalIndex].body[cellIndex].signalIntensity  <= -1.0f)
				{
					game.animals[animalIndex].body[cellIndex].grabbedCreature = -1;
					// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					sensorium[cellIndex] = 0.0f;
				}
			}
			break;
		}

		case ORGAN_SENSOR_PAIN:
		{
			// game.animals[animalIndex].body[cellIndex].signalIntensity *= 0.99f;
			sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity  * 0.99f;// 0.0f;
		}

		case ORGAN_SENSOR_HUNGER:
		{
			if (game.animals[animalIndex].maxEnergy > 0.0f)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity 
				float hungy = game.animals[animalIndex].energy / game.animals[animalIndex].maxEnergy;
				sensorium[cellIndex] = hungy;
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
				// game.animals[animalIndex].body[cellIndex].signalIntensity 
				sensorium [cellIndex] =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle);
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
					// game.animals[animalIndex].body[cellIndex].signalIntensity
				sensorium [cellIndex] 	 =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle);
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
					// game.animals[animalIndex].body[cellIndex].signalIntensity
					sensorium [cellIndex]  =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle);
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
					// game.animals[animalIndex].body[cellIndex].signalIntensity 
					sensorium [cellIndex] =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle);
				}
			}
			break;
		}

		case ORGAN_LUNG:
		{
			if (game.world[cellWorldPositionI].wall != MATERIAL_WATER)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity = baseLungCapacity;
				sensorium [cellIndex]  = baseLungCapacity;
			}
			else
			{
				bool hasGill = false;
				for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
				{
					if (game.animals[animalIndex].body[i].organ == ORGAN_GILL)
					{
						hasGill = true;
						break;
					}
				}
				if (!hasGill)
				{
					// game.animals[animalIndex].body[cellIndex].signalIntensity -= aBreath;

					sensorium [cellIndex]  = game.animals[animalIndex].body[cellIndex].signalIntensity - aBreath;
					if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						game.animals[animalIndex].damageReceived++;
					}
				}
			}
			break;
		}
		case ORGAN_GILL:
		{
			if (game.world[cellWorldPositionI].wall == MATERIAL_WATER)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity = baseLungCapacity;
				sensorium [cellIndex] = baseLungCapacity;
			}
			else
			{
				bool hasLung = false;
				for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
				{
					if (game.animals[animalIndex].body[i].organ == ORGAN_LUNG)
					{
						hasLung = true;
						break;
					}
				}
				if (!hasLung)
				{
					// game.animals[animalIndex].body[cellIndex].signalIntensity -= aBreath;
					sensorium [cellIndex]  = game.animals[animalIndex].body[cellIndex].signalIntensity - aBreath;
					if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						game.animals[animalIndex].damageReceived++;
					}
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
			// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			// for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			// {
			// 	if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
			// 	{
			// 		unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
			// 		if (connected_to_cell < animalSquareSize)
			// 		{
			// 			game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
			// 		}
			// 	}
			// }

			sumInputs(  animalIndex,   cellIndex);

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
				if (correspondingCellRX >= 0 && correspondingCellRX < game.animals[animalIndex].cellsUsed)
				{
					// game.animals[animalIndex].body[correspondingCellRX].signalIntensity
					sensorium[correspondingCellRX]   = game.animals[animalIndex].body[cellIndex].signalIntensity ;
					
				}
			}
			break;
		}

		case ORGAN_SENSOR_PHEROMONE:
		{
			// game.animals[animalIndex].body[cellIndex].signalIntensity = 0;
			if (game.world[cellWorldPositionI].pheromoneChannel >= 0)
			{
				if (game.animals[animalIndex].body[cellIndex]. speakerChannel ==   game.world[cellWorldPositionI].pheromoneChannel)
				{
					// game.animals[animalIndex].body[cellIndex].signalIntensity 
					sensorium[cellIndex] = game.world[cellWorldPositionI].pheromoneIntensity;
				}
			}
			break;
		}

		case ORGAN_EMITTER_PHEROMONE:
		{
			// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			// for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			// {
			// 	if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
			// 	{
			// 		unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
			// 		if (connected_to_cell < animalSquareSize)
			// 		{
			// 			game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
			// 		}
			// 	}
			// }

			sumInputs(  animalIndex,   cellIndex);

			game.world[cellWorldPositionI].pheromoneChannel = game.animals[animalIndex].body[cellIndex]. speakerChannel ;
			game.world[cellWorldPositionI].pheromoneIntensity = game.animals[animalIndex].body[cellIndex].signalIntensity;
			break;
		}

		case ORGAN_SPEAKER:
		{
			if ( game.animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
				// for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
				// {
				// 	if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
				// 	{
				// 		unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
				// 		if (connected_to_cell < animalSquareSize)
				// 		{
				// 			game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
				// 		}
				// 	}
				// }

			sumInputs(  animalIndex,   cellIndex);

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
				// game.animals[animalIndex].body[cellIndex].signalIntensity 
				sensorium[cellIndex] = game.speakerChannelsLastTurn[ game.animals[animalIndex].body[cellIndex].speakerChannel ];
			}
			else
			{
				game.animals[animalIndex].body[cellIndex].speakerChannel = 0;
			}
			break;
		}

		case ORGAN_SENSOR_TRACKER:
		{
			// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			if ( game.world [cellWorldPositionI].identity != animalIndex )
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity = game.world[cellWorldPositionI].trail;
				// game.animals[animalIndex].body[cellIndex].signalIntensity 
				sensorium[cellIndex]=  smallestAngleBetween(  game.world[cellWorldPositionI].trail, game.animals[animalIndex].fAngle);
			}
			break;
		}

		case ORGAN_SENSOR_BODYANGLE:
		{
			// game.animals[animalIndex].body[cellIndex].signalIntensity
			sensorium[cellIndex] = game.animals[animalIndex].fAngle;
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
			float perceivedColor = 0.0f;
			perceivedColor += (game.animals[animalIndex].body[cellIndex].color.r - receivedColor.r );
			perceivedColor += (game.animals[animalIndex].body[cellIndex].color.g - receivedColor.g );
			perceivedColor += (game.animals[animalIndex].body[cellIndex].color.b - receivedColor.b );
			perceivedColor = perceivedColor / 3.0f;
			// game.animals[animalIndex].body[cellIndex].signalIntensity 
			sensorium[cellIndex]= 1.0f - perceivedColor;
			break;
		}

		case ORGAN_SENSOR_TOUCH:
		{
			// game.animals[animalIndex].body[cellIndex].signalIntensity = 0;
			sensorium[cellIndex] = 0.0f;
			for (int i = 0; i < nNeighbours; ++i)
			{
				unsigned int neighbour = cellWorldPositionI + neighbourOffsets[i];
				if (neighbour < worldSquareSize)
				{
					if (game.world[neighbour].identity >= 0)
					{
						if (isAnimalInSquare( game.world[neighbour].identity , neighbour ) >= 0)
						{
							// game.animals[animalIndex].body[cellIndex].signalIntensity 
							sensorium[cellIndex] += 0.5f;
						}
						else if (game.world[neighbour].wall != MATERIAL_NOTHING)
						{
							// game.animals[animalIndex].body[cellIndex].signalIntensity
							sensorium[cellIndex] += 0.5f;
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
						if (isAnimalInSquare( touchedAnimal , cellWorldPositionI ) >= 0)
						{
							// game.animals[animalIndex].body[cellIndex].signalIntensity 
						sensorium[cellIndex] 	+= 0.5f;
						}
						else if (game.world[cellWorldPositionI].wall != MATERIAL_NOTHING)
						{
							// game.animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
							sensorium[cellIndex] 	+= 0.5f;
						}
					}
				}
			}
			break;
		}

		case ORGAN_NEURON:
		{
			// float sum = 0.0f; // go through the list of connections and sum their values.
			// for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			// {
			// 	if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
			// 	{
			// 		unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
			// 		if (connected_to_cell < animalSquareSize)
			// 		{
			// 			float connected_signal = game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
			// 			sum += connected_signal;
			// 		}
			// 	}
			// }

			sumInputs(  animalIndex,   cellIndex);
			// game.animals[animalIndex].body[cellIndex].signalIntensity
			sensorium[cellIndex]  = fast_sigmoid(game.animals[animalIndex].body[cellIndex].signalIntensity);
			break;
		}

		case ORGAN_GONAD:
		{
			if (game.animals[animalIndex].body[cellIndex].damage < 1.0f)
			{
				totalGonads++;
			}
			if (doReproduction && game.animals[animalIndex].energyDebt <= 0.0f )
			{
				float reproducesAt = ((game.animals[animalIndex].mass / 2 ) + game.animals[animalIndex].offspringEnergy );
				if (game.animals[animalIndex].energy > reproducesAt)
				{
					if (cellWorldPositionI < worldSquareSize)
					{
						unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;

						bool mutate = false;
						if (extremelyFastNumberFromZeroTo(1) == 0)
						{
							mutate = true;
						}

						int result = spawnAnimal( speciesIndex, game.animals[animalIndex], game.animals[animalIndex].position, mutate );
						if (result >= 0)
						{
							game.animals[animalIndex].body[cellIndex].damage = 1.0f;
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

#ifdef PLANTS

			bool ate_plant = false;

			if (game.world[cellWorldPositionI].plantState == MATERIAL_BUD)
			{
				ate_plant = true;
			}

			else if (game.world[cellWorldPositionI].plantState == MATERIAL_LEAF)
			{

				ate_plant = true;
			}

			else if (game.world[cellWorldPositionI].plantState == MATERIAL_GRASS)
			{

				ate_plant = true;
			}
			else if (game.world[cellWorldPositionI].plantState == MATERIAL_WOOD)
			{

				ate_plant = true;
			}


			if (ate_plant)
			{

				game.animals[animalIndex].energy += game.ecoSettings[1] ;
				game.world[cellWorldPositionI].plantState = MATERIAL_NOTHING;
			}

#else
			if (game.world[cellWorldPositionI].wall == MATERIAL_GRASS)
			{
				game.animals[animalIndex].energy += game.ecoSettings[1] ;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
#endif
			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			break;
		}
		case ORGAN_MOUTH_SCAVENGE :
		{
			if (game.world[cellWorldPositionI].wall == MATERIAL_FOOD)
			{
				game.animals[animalIndex].energy += game.ecoSettings[0] ;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			break;
		}
		case ORGAN_MOUTH_PARASITE:
		{
			if (game.world[cellWorldPositionI].identity != animalIndex && game.world[cellWorldPositionI].identity >= 0 && game.world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
			{
				int leechAttackVictim = isAnimalInSquare(game.world[cellWorldPositionI].identity , cellWorldPositionI);
				if (leechAttackVictim >= 0)
				{
					if (game.animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
					{
						if (game.world[cellWorldPositionI].identity == game.animals[animalIndex].parentIdentity)
						{
							continue;
						}
					}
					float amount = (game.animals[game.world[cellWorldPositionI].identity].energy) / animalSquareSize;
					float defense = defenseAtWorldPoint(game.world[cellWorldPositionI].identity, cellWorldPositionI);
					amount = amount / defense;
					game.animals[animalIndex].energy += amount;
					game.animals[game.world[cellWorldPositionI].identity].energy -= amount;

					unsigned int victimSpecies =  (game.world[cellWorldPositionI].identity / numberOfAnimalsPerSpecies) ;
					if (victimSpecies < numberOfSpecies)
					{
						foodWeb[speciesIndex][  victimSpecies] += amount ;
					}
				}
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			break;
		}
		case ORGAN_MOUTH_CARNIVORE:
		{
			if (game.world[cellWorldPositionI].identity != animalIndex && game.world[cellWorldPositionI].identity >= 0 && game.world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
			{
				int targetLocalPositionI = isAnimalInSquare( game.world[cellWorldPositionI].identity , cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					if (game.animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
					{
						if (game.world[cellWorldPositionI].identity == game.animals[animalIndex].parentIdentity)
						{
							continue;
						}
					}
					hurtAnimal(game.world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f, animalIndex );
					if (game.world[cellWorldPositionI].wall == MATERIAL_FOOD)
					{
						game.animals[animalIndex].energy += game.ecoSettings[0] ;
						unsigned int victimSpecies =  (game.world[cellWorldPositionI].identity / numberOfAnimalsPerSpecies) ;
						if (victimSpecies < numberOfSpecies)
						{
							foodWeb[speciesIndex][  victimSpecies] += game.ecoSettings[0] ;
						}
					}
				}
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			break;
		}
		case ORGAN_CLAW:
		{
			if (game.world[cellWorldPositionI].identity != animalIndex && game.world[cellWorldPositionI].identity >= 0 && game.world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
			{
				int targetLocalPositionI = isAnimalInSquare(game.world[cellWorldPositionI].identity , cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					hurtAnimal(game.world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f, animalIndex );
				}
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_WAX)
			{
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			break;
		}
		case ORGAN_MUSCLE :
		{
			if (animalIndex != game.playerCreature)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;

				// for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
				// {
				// 	if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
				// 	{
				// 		unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
				// 		if (connected_to_cell < animalSquareSize)
				// 		{
				// 			game.animals[animalIndex].body[cellIndex].signalIntensity  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
				// 		}
				// 	}
				// }

				sumInputs(  animalIndex,   cellIndex);
			}
			if (game.animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
			{
				game.animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
			}
			else if (game.animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
			{
				game.animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
			}

			float impulse = game.animals[animalIndex].body[cellIndex].signalIntensity * musclePower;
			game.animals[animalIndex].fPosX += (impulse / game.animals[animalIndex].mass) * game.animals[animalIndex].fAngleSin;
			game.animals[animalIndex].fPosY += (impulse / game.animals[animalIndex].mass) * game.animals[animalIndex].fAngleCos;
			game.animals[animalIndex].energy -= game.ecoSettings[2] * impulse ;

			game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			break;
		}

		case ORGAN_MUSCLE_STRAFE :
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
			float impulse = game.animals[animalIndex].body[cellIndex].signalIntensity * musclePower;
			game.animals[animalIndex].fPosX += (impulse / game.animals[animalIndex].mass) * game.animals[animalIndex].fAngleCos;
			game.animals[animalIndex].fPosY += (impulse / game.animals[animalIndex].mass) * game.animals[animalIndex].fAngleSin;
			game.animals[animalIndex].energy -= game.ecoSettings[2] * impulse ;

			game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			break;
		}
		case ORGAN_MUSCLE_TURN:
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
				game.animals[animalIndex].fAngle += (game.animals[animalIndex].body[cellIndex].signalIntensity ) * turnMusclePower;
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
			break;
		}
		case ORGAN_GENITAL_A:
		{
			if ( game.world[cellWorldPositionI].identity >= 0 &&  game.world[cellWorldPositionI].identity < numberOfAnimals)
			{
				int targetLocalPositionI = isAnimalInSquare( game.world[cellWorldPositionI].identity, cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					if (game.animals[game.world[cellWorldPositionI].identity].body[targetLocalPositionI].organ == ORGAN_GENITAL_B )
					{
						sexBetweenTwoCreatures( animalIndex, game.world[cellWorldPositionI].identity );
					}
				}
			}
			break;
		}
		case ORGAN_GENITAL_B:
		{
			if ( game.world[cellWorldPositionI].identity >= 0 &&  game.world[cellWorldPositionI].identity < numberOfAnimals)
			{
				int targetLocalPositionI = isAnimalInSquare( game.world[cellWorldPositionI].identity, cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					if (game.animals[game.world[cellWorldPositionI].identity].body[targetLocalPositionI].organ == ORGAN_GENITAL_A )
					{
						sexBetweenTwoCreatures( game.world[cellWorldPositionI].identity , animalIndex);
					}
				}
			}
			break;
		}
		case ORGAN_SENSOR_PLEASURE:
		{
			sensorium[cellIndex]  = game.animals[animalIndex].body[cellIndex].signalIntensity * 0.99f;
			break;
		}
		}

		// add noise to all neural pathways, except for bias neurons which would mess them up.
		if (organIsASensor(organ) || organIsANeuron(organ))
		{
			if (organ != ORGAN_BIASNEURON)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity
				sensorium[cellIndex]  += (((extremelyFastNumberFromZeroTo(64) - 32.0f) / 32.0f )) * neuralNoise;
			}
		}
	}


	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		game.animals[animalIndex].body[i].signalIntensity  = sensorium[i];
	}

	game.animals[animalIndex].totalGonads = totalGonads;
	game.animals[animalIndex].maxEnergy = game.animals[animalIndex].mass + (totalLiver * liverStorage);
}


void animalEnergy(int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	game.animals[animalIndex].age++;

	game.animals[animalIndex].energy -= game.ecoSettings[3] * game.animals[animalIndex].mass;

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
	if ( !game.animals[animalIndex].isMachine && game.animals[animalIndex].age > 0) // reasons an npc can die
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
		if (game.animals[animalIndex].damageReceived > game.animals[animalIndex].mass / 2)
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
		killAnimal( animalIndex);
	}
	bool nominate = false;
	int animalScore = game.animals[animalIndex].damageDone + game.animals[animalIndex].damageReceived  + game.animals[animalIndex].numberOfTimesReproduced ;
	if ( animalScore >= game.championScores[speciesIndex])
	{
		// nominee must be reproductively viable (1 gonad is not enough!) and have a breathing apparatus and a mouth
		unsigned int totalGonads = 0;
		unsigned int totalMouths = 0;
		unsigned int totalBreathing  = 0;
		for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
		{
			if ( game.animals[animalIndex].body[i].organ == ORGAN_GONAD)
			{
				totalGonads++;
			}

			if ( game.animals[animalIndex].body[i].organ == ORGAN_LUNG
			        || game.animals[animalIndex].body[i].organ == ORGAN_GILL )
			{
				totalBreathing++;
			}

			if ( game.animals[animalIndex].body[i].organ == ORGAN_MOUTH_CARNIVORE
			        || game.animals[animalIndex].body[i].organ == ORGAN_MOUTH_SCAVENGE
			        || game.animals[animalIndex].body[i].organ == ORGAN_MOUTH_PARASITE
			        || game.animals[animalIndex].body[i].organ == ORGAN_MOUTH_VEG
			   )
			{
				totalMouths++;
			}
		}

		bool nominate = false;
		// player & player species cannot be nominated, and machines cannot be nominated
		if (animalIndex != game.playerCreature
		        && animalIndex != game.adversary
		        && speciesIndex > 0
		        && game.animals[animalIndex].machineCallback == MATERIAL_NOTHING
		        && game.animals[animalIndex].mass > 0
		        && totalGonads > 1 && totalMouths > 0 && totalBreathing > 0)
		{
			if ( animalScore > game.championScores[speciesIndex])
			{
				nominate = true;
			}
			else if ( animalScore == game.championScores[speciesIndex])
			{
				if (game.animals[animalIndex].energy > game.championEnergies[speciesIndex])
				{
					nominate = true;
				}
			}
		}
		if (nominate)
		{
			game.championScores[speciesIndex] = animalScore;
			game.champions[speciesIndex] = game.animals[animalIndex];
			game.championEnergies[speciesIndex] = game.animals[animalIndex].energy;
		}
	}
}

void census()
{
	ZoneScoped;
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		game.populationCountUpdates[i] = 0;
	}
	for (int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;
		if (!game.animals[animalIndex].retired && speciesIndex < numberOfSpecies)
		{
			game.populationCountUpdates[speciesIndex]++;
		}
	}
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		game.speciesPopulationCounts[i] = game.populationCountUpdates[i];
		game.speciesAttacksPerTurn[i] = 0;
	}
}

void camera()
{
	ZoneScoped;
	if (flagSave) { return;}

	if (game.cameraTargetCreature >= 0)
	{
		game.cameraPositionX = game.animals[game.cameraTargetCreature].position % worldSize;
		game.cameraPositionY = game.animals[game.cameraTargetCreature].position / worldSize;
	}
	if (game.playerCreature >= 0 )	// if the player doesn't have any eyes, don't draw anything!
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
		const int viewFieldMax = +(viewFieldY / 2);
		const int viewFieldMin = -(viewFieldX / 2);
		const unsigned int shadowSquareSize = viewFieldX * viewFieldY;
		bool shadows[shadowSquareSize];

		for (int i = 0; i < shadowSquareSize; ++i)
		{
			shadows[i] = false;
		}
		for ( int vy = viewFieldMin; vy < viewFieldMax; ++vy)
		{
			for ( int vx = viewFieldMin; vx < viewFieldMax; ++vx)
			{
				Color displayColor = color_black;
				 int x = (vx + game.cameraPositionX) % worldSize;
				 int y = (vy + game.cameraPositionY) % worldSize;
				unsigned int worldI = (y * worldSize) + x;
				if (worldI < worldSquareSize)
				{
					float fx = vx;
					float fy = vy;
					displayColor = whatColorIsThisSquare(worldI);
					unsigned int shadowIndex = ( (viewFieldX * (vy + abs(viewFieldMin))  ) + (vx + abs(viewFieldMin)) ) ;
					if (shadowIndex < shadowSquareSize)
					{
						if (shadows[shadowIndex])
						{
							displayColor = filterColor(displayColor, tint_shadow);
						}
					}
					if (game.world[worldI].identity >= 0 && game.world[worldI].identity < numberOfAnimals)
					{
						int animalCell = isAnimalInSquare(game.world[worldI].identity, worldI );
						if (animalCell >= 0 && animalCell < game.animals[game.selectedAnimal].cellsUsed)
						{
							if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals)
							{


								if (game.world[worldI].identity == game.selectedAnimal )
								{

									;



// void drawLine(  Vec_f2 a, Vec_f2 b, float thickness, Color finalColor )



									for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
									{
										if (game.animals[game.selectedAnimal].body[animalCell].connections[i].used)
										{

											Vec_f2 start = Vec_f2(vx, vy);
											Vec_f2 end    = Vec_f2(vx, vy);

											int connected_to_cell = game.animals[game.selectedAnimal].body[animalCell].connections[i].connectedTo;
											if (connected_to_cell >= 0 && connected_to_cell < game.animals[game.selectedAnimal].cellsUsed)
											{
												// end.x -= (game.animals[game.selectedAnimal].body[connected_to_cell].localPosX - game.animals[game.selectedAnimal].body[animalCell].localPosX );
												// end.y -= (game.animals[game.selectedAnimal].body[connected_to_cell].localPosY - game.animals[game.selectedAnimal].body[animalCell].localPosX );

												unsigned int connectedPos = game.animals[game.selectedAnimal].body[connected_to_cell].worldPositionI;
												int connectedX = connectedPos% worldSize;
												int connectedY = connectedPos/ worldSize;

												end.x -= (x - connectedX);
												end.y -= (y - connectedY);


												Color signalColor = color_white;
												float brightness =game.animals[game.selectedAnimal].body[connected_to_cell].signalIntensity * game.animals[game.selectedAnimal].body[animalCell].connections[i].weight ;
												signalColor = multiplyColorByScalar(signalColor, brightness);
												drawLine(  start, end, 0.01f, signalColor );
											}
										}
									}


								}
								else
								{
									displayColor = filterColor(displayColor, tint_shadow);
								}
							}
						}
					}
					drawTile( Vec_f2( fx, fy ), displayColor);
				}
			}
		}
	}

	Color displayColor = color_white; // draw the mouse cursor.
	Vec_f2 worldMousePos = Vec_f2( game.mousePositionX, game.mousePositionY);
	drawTile( worldMousePos, displayColor);
	drawLine(  worldMousePos, Vec_f2(0,0), 0.01f, color_green );
}

void displayComputerText()
{
	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;
	if (game.ecologyComputerDisplay)
	{



		// draw the food web.
		int originalMenuX = menuX;
		for (int i = 0; i < numberOfSpecies; ++i)
		{
			printText2D( "species " + std::to_string(i) + " eats: " , menuX, menuY, textSize);
			// break;

			for (int j = 0; j < numberOfSpecies; ++j)
			{
				printText2D(  std::to_string(foodWeb[i][j]) , menuX, menuY, textSize);
				menuX += spacing * 5;
				// break;
			}
			menuX = originalMenuX;
			menuY -= spacing;
		}






		for (int i = 0; i < numberOfSpecies; ++i)
		{
			printText2D(   "Species " + std::to_string(i) +
			               " pop. " + std::to_string(game.speciesPopulationCounts[i]) +
			               " hits " + std::to_string(game.speciesAttacksPerTurn[i])
			               + " champion score " + std::to_string(game.championScores[i])
			               + " champion energy " + std::to_string(game.championEnergies[i])

			               , menuX, menuY, textSize);
			menuY -= spacing;
		}
		for (int j = 0; j < numberOfEcologySettings; ++j)
		{
			std::string selectString = std::string("");
			if (j == game.activeEcoSetting) { selectString = std::string("X "); }
			switch (j)
			{
			case 0:
			{

			}
			case 1:
			{
				printText2D( selectString +  std::string("Grass energy ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 2:
			{
				printText2D( selectString +  std::string("Movement tax ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 3:
			{
				printText2D( selectString +  std::string("Resting tax ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			}
			menuY -= spacing;
		}
	}

	if (game.computerdisplays[0])
	{
		menuY -= spacing;
		printText2D(   std::string("Animals are groups of tiles. Each tile is an organ that performs a dedicated bodily function. ") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Your body is made this way too. ") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("If your tiles are damaged, you will lose the tile's function,") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("which can include your sight, movement, or breathing, resulting in disorientation and death. ") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Find the hospital! It is in a black building on land, just like this one.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (game.computerdisplays[1])
	{
		printText2D(   std::string("The hospital will heal you if you pick it up.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("It can also be used to alter your body and mind.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Use it to add a gill anywhere on your body, so that you can survive underwater") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Find a building in the ocean and retrieve the tracker glasses.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("But beware, there are dangers other than the water itself.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (game.computerdisplays[2])
	{
		printText2D(   std::string("Activate the tracker glasses to see the trails that creatures leave.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("You will recognize the adversary by its white trail.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Take the weapon, find the adversary and destroy it.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("The adversary posesses neuro glasses, which allow you to see the minute electrical activity of living flesh.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("You can use them, in combination with the hospital, to edit the mind of a living creature.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (game.computerdisplays[3])
	{
		if (game.adversaryDefeated)
		{
			printText2D(   std::string("The adversary has been destroyed. Life will no longer be created in the world, but will persist from its current state,") , menuX, menuY, textSize);
			menuY -= spacing;
			printText2D(   std::string("or eventually be driven to extinction.") , menuX, menuY, textSize);
			menuY -= spacing;
		}
		else
		{
			printText2D(   std::string("You must defeat the adversary. Return here when it is done.") , menuX, menuY, textSize);
			menuY -= spacing;
		}
	}
	printText2D(  "    \n" , menuX, menuY, textSize);
	menuY -= spacing;
}

void incrementSelectedGrabber()
{
	for (unsigned int i = 0; i < game.animals[game.playerCreature].cellsUsed ; ++i)
	{
		unsigned int neighbour = (game.playerActiveGrabber + i) % game.animals[game.playerCreature].cellsUsed;
		if (game.animals[game.playerCreature].body[neighbour].organ == ORGAN_GRABBER && game.animals[game.playerCreature].body[neighbour].grabbedCreature >= 0
		        && neighbour != game.playerActiveGrabber
		   )
		{
			game.playerActiveGrabber = neighbour;
			break;
		}
	}
}

void drawGameInterfaceText()
{
	int menuX = 50;
	int menuY = 50;
	int textSize = 10;
	int spacing = 20;

	if (flagSave)
	{
		printText2D(   std::string("saving") , menuX, menuY, textSize);
		menuY += spacing;
		return;
	}
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = ((cursorPosY * worldSize) + cursorPosX ) % worldSquareSize;
	if (game.paused)
	{
		printText2D(   std::string("FPS 0 (Paused)")   , menuX, menuY, textSize);
		menuY += spacing;
	}
	else
	{
		printText2D(   std::string("FPS ") + std::to_string(modelFrameCount ) , menuX, menuY, textSize);
		menuY += spacing;
	}
	modelFrameCount = 0;
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

		if (game.lockfps)
		{
			printText2D(   std::string("[l] max simulation speed"), menuX, menuY, textSize);
			menuY += spacing;
		}
		else
		{
			printText2D(   std::string("[l] limit simulation speed"), menuX, menuY, textSize);
			menuY += spacing;
		}

		printText2D(   std::string("[o] save, ") + pauseString, menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[space] return mouse") , menuX, menuY, textSize);
		menuY += spacing;
		if (game.playerCreature >= 0)
		{
			printText2D(   std::string("[w,a,s,d] move") , menuX, menuY, textSize);
			menuY += spacing;
			printText2D(   std::string("[r] despawn") , menuX, menuY, textSize);
			menuY += spacing;
			printText2D(   std::string("Start by finding items in the world and picking them up.") , menuX, menuY, textSize);
			menuY += spacing;
		}
		else
		{
			printText2D(   std::string("[r] spawn") , menuX, menuY, textSize);
			menuY += spacing;
		}

		printText2D(   std::string("[u] hide instructions"), menuX, menuY, textSize);
		menuY += spacing;
	}
	else
	{
		printText2D(   std::string("[u] instructions"), menuX, menuY, textSize);
		menuY += spacing;
	}

	game.palette = false;
	game.ecologyComputerDisplay = false;


	unsigned int holding = 0;	// print grabber states
	for (int i = 0; i < game.animals[game.playerCreature].cellsUsed; ++i)
	{
		if (game.animals[game.playerCreature].body[i].organ == ORGAN_GRABBER)
		{
			if (game.animals[game.playerCreature].body[i].grabbedCreature >= 0)
			{
				holding++;
			}
		}
	}
	if (holding == 0)
	{
		game.playerActiveGrabber = -1;
	}

	if (holding > 0)
	{
		if (game.playerActiveGrabber < 0)
		{
			incrementSelectedGrabber();
		}

		std::string stringToPrint;

		printText2D(   std::string("Holding ") + std::to_string(holding )  + std::string(" items. [t] next"), menuX, menuY, textSize);
		menuY += spacing;

		if (game.playerActiveGrabber >= 0 && game.playerActiveGrabber < animalSquareSize)
		{
			stringToPrint += std::string("Holding ") + game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].displayName + std::string(" [f] drop ");

			if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].isMachine)
			{

				if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_HOSPITAL)
				{
					stringToPrint += std::string("[lmb, rmb] add, erase [y, h] next, prev");

					healAnimal(game.playerCreature);
					game.palette = true;
				}

				if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_ECOLOGYCOMPUTER)
				{
					stringToPrint += std::string("[lmb, rmb] +, - [y, h] next, last");
					game.ecologyComputerDisplay = true;
				}

				if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER1 ||
				        game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER2 ||
				        game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER3 ||
				        game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER4 ||
				        game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER5

				   )
				{
					stringToPrint += std::string("[lmb] read messages");
				}

				if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_LIGHTER)
				{
					stringToPrint += std::string("[lmb] start fire");
				}

				if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_KNIFE)
				{
					stringToPrint += std::string("[lmb] cut");

					if (game.playerLMBDown)
					{
						knifeCallback(game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
					}
				}

				if (game.animals[  game.animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_PISTOL)
				{
					stringToPrint += std::string("[lmb] shoot");
				}

			}
			printText2D( stringToPrint   , menuX, menuY, textSize);
			menuY += spacing;
		}
	}

	if (game.playerCanPickup && game.playerCanPickupItem >= 0 && game.playerCanPickupItem < numberOfAnimals)
	{
		printText2D(   std::string("[g] pick up ") + std::string(game.animals[game.playerCanPickupItem].displayName) , menuX, menuY, textSize);
		menuY += spacing;
	}

	if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals)
	{
		std::string selectString( "energy " + std::to_string(game.animals[game.selectedAnimal].energy ) + ". [e] to deselect. [k] save animal.");
	}

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


#ifdef PLANTS
			std::string plantInfo =  std::string(" energy ") + std::to_string( game.world[worldCursorPos].energy) + std::string(", debt ") + std::to_string( game.world[worldCursorPos].energyDebt  );

			if (game.world[worldCursorPos].plantState != MATERIAL_NOTHING)
			{
				printText2D(   tileDescriptions(game.world[worldCursorPos].plantState  ) + plantInfo , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (game.world[worldCursorPos].wall != MATERIAL_NOTHING)
			{
				printText2D(  tileDescriptions(game.world[worldCursorPos].wall ), menuX, menuY, textSize);
				menuY += spacing;
			}
			else
			{
				printText2D(  tileDescriptions (game.world[worldCursorPos].terrain ), menuX, menuY, textSize);
				menuY += spacing;
			}
#else
			if (game.world[worldCursorPos].wall != MATERIAL_NOTHING)
			{
				printText2D(  tileDescriptions(game.world[worldCursorPos].wall ), menuX, menuY, textSize);
				menuY += spacing;
			}
			else
			{
				printText2D(  tileDescriptions (game.world[worldCursorPos].terrain ), menuX, menuY, textSize);
				menuY += spacing;
			}

#endif
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

		int playerGill = getCellWithAir(game.playerCreature);
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

		playerGill = getRandomCellOfType( game.playerCreature, ORGAN_SENSOR_PAIN ) ;
		if (playerGill >= 0)
		{

			if (game.animals[game.playerCreature].damageReceived > (game.animals[game.playerCreature].mass) * 0.25 &&
			        game.animals[game.playerCreature].damageReceived < (game.animals[game.playerCreature].mass) * 0.375
			   )
			{
				printText2D(   std::string("You're badly hurt.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (game.animals[game.playerCreature].damageReceived > (game.animals[game.playerCreature].mass) * 0.375)
			{
				printText2D(   std::string("You are mortally wounded.") , menuX, menuY, textSize);
				menuY += spacing;
			}
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
	if (game.palette)
	{
		menuY += spacing;
		drawPalette(menuX, menuY);
	}
}

void paintCreatureFromCharArray( int animalIndex, const char * start, unsigned int len, unsigned int width )
{
	Vec_i2 o = Vec_i2(0, 0);
	Vec_i2 p = Vec_i2(0, 0);
	if (len > animalSquareSize)
	{
		len = animalSquareSize;
	}
	for (unsigned int i = 0; i < len; ++i)
	{
		char c = start[i];
		Color newColor = color_black;
		switch (c)
		{
		case 'B':
			newColor = color_black;
			break;
		case 'T':
			newColor = color_tan;
			break;
		case 'P':
			newColor = color_pink;
			break;
		case 'R':
			newColor = color_brown;
			break;
		case 'V':
			newColor = color_charcoal;
			break;
		case 'W':
			newColor = color_white;
			break;
		case 'L':
			newColor = color_lightblue;
			break;
		case 'D':
			newColor = color_darkgrey;
			break;
		case 'G':
			newColor = color_grey;
			break;
		case 'M':
			newColor = color_lightgrey;
			break;
		case 'K':
			newColor = color_brightred;
			break;
		case 'Q':
			newColor = color_darkred;
			break;
		}
		for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
		{
			if (game.animals[animalIndex].body[i].localPosX == p.x && game.animals[animalIndex].body[i].localPosY == p.y)
			{
				game.animals[animalIndex].body[i].color = newColor;
			}
		}
		p.x++;
		if (p.x == width)
		{
			p.x = 0;
			p.y --;
		}
	}
}

void printAnimalCells(int animalIndex)
{
	printf( "%s\n",   game.animals[animalIndex].displayName  );

	for (int i = 0; i < animalSquareSize; ++i)
	{
		printf( "%s\n",   tileShortNames(game.animals[animalIndex].body[i].organ).c_str()  );
	}
}

void setupCreatureFromCharArray( int animalIndex, const char * start, unsigned int len, unsigned int width, std::string newName, int newMachineCallback )
{
	resetAnimal(animalIndex);
	strcpy( &game.animals[animalIndex].displayName[0] , newName.c_str() );
	if (newMachineCallback >= 0)
	{
		game.animals[animalIndex].isMachine = true;
		game.animals[animalIndex].machineCallback = newMachineCallback;
	}
	Vec_i2 o = Vec_i2(0, 0);
	Vec_i2 p = Vec_i2(0, 0);
	Color c = color_peach_light;
	int i = 0;
	for (;;)
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
		case 'R':
			newOrgan = ORGAN_SENSOR_EAR;
			break;
		case 'H':
			newOrgan = ORGAN_SENSOR_BIRTHPLACE;
			break;
		case 'X':
			newOrgan = ORGAN_SENSOR_PAIN;
			break;
		case '/':
			newOrgan = ORGAN_HAIR;
			break;
		case 'O':
			newOrgan = ORGAN_ADDOFFSPRINGENERGY;
			break;
		case '1':
			newOrgan = MATERIAL_METAL;
			break;
		case '2':
			newOrgan = MATERIAL_GLASS;
			break;
		case '3':
			newOrgan = TILE_DESTROYER_EYE;
			break;
		}
		if (newOrgan != MATERIAL_NOTHING)
		{
			appendCell( animalIndex, newOrgan, p);
			if (game.animals[animalIndex].cellsUsed >= (animalSquareSize - 1)) { break;}
		}
		p.x++;
		if (p.x == width)
		{
			p.x = 0;
			p.y --;
		}
		i++;
		if (i > len) { break;}
	}
	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		game.animals[animalIndex].body[i].localPosX -= width / 2;
		game.animals[animalIndex].body[i].localPosY -= p.y / 2;
	}
}

void spawnAdversary(unsigned int targetWorldPositionI)
{
	game.adversary = numberOfAnimalsPerSpecies + 1; // game.adversary animal is a low number index in the 1th species. 0th is for players and machines.
	spawnAnimalIntoSlot(game.adversary, game.champions[1], targetWorldPositionI, true);
	game.animals[game.adversary].position = targetWorldPositionI;
	game.animals[game.adversary].uPosX = targetWorldPositionI % worldSize;
	game.animals[game.adversary].uPosY = targetWorldPositionI / worldSize;
	game.animals[game.adversary].fPosX = game.animals[game.adversary].uPosX;
	game.animals[game.adversary].fPosY = game.animals[game.adversary].uPosY;
	if (!game.adversaryCreated)
	{
		appendLog( std::string("Life has started in the oceans,") );
		appendLog( std::string("and begun to grow and change.") );
		game.adversaryCreated = true;
	}
}

void spawnPlayer()
{
	unsigned int targetWorldPositionI =  game.playerRespawnPos;
	int i = 1;
	setupExampleHuman(i);
	game.playerCreature = 0;
	spawnAnimalIntoSlot(game.playerCreature, game.animals[i], targetWorldPositionI, false);
	game.cameraTargetCreature = game.playerCreature;
	appendLog( std::string("Spawned the player.") );
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
	else
	{
		game.cameraPositionX += cameraPanSpeed * pos.x;
		game.cameraPositionX =  game.cameraPositionX % worldSize;
		game.cameraPositionY -= cameraPanSpeed * pos.y;
		game.cameraPositionY =  game.cameraPositionY % worldSize;
	}
}

void saveParticularAnimal(int animalIndex, std::string filename )
{
	std::ofstream out7( filename .c_str());
	out7.write( (char*)(&game.animals[game.selectedAnimal]), sizeof(Animal));
	out7.close();
}

void loadParticlarAnimal(int animalIndex, std::string filename)
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
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize - 1; worldPositionI++)
	{
		computeLight( worldPositionI, sunXangle, sunYangle);
		if (game.world[worldPositionI].height < seaLevel)
		{
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

int getRandomPosition(bool underwater)
{
	while (true)
	{
		unsigned int randomI = extremelyFastNumberFromZeroTo(worldSquareSize - 1);

		unsigned int x = randomI % worldSize;
		unsigned int y = randomI / worldSize;

		if (x > baseSize && x < (worldSize - baseSize) && y > baseSize && y < (worldSize - baseSize)  )
		{

			if (underwater)
			{
				if (game.world[randomI].height > seaLevel)
				{
					continue;
				}
			}
			else
			{
				if (game.world[randomI].height < biome_coastal)
				{
					continue;
				}
			}

			bool hasAir = false;
			bool hasWater = false;
			bool unsuitable = false;
			for (int k = -(baseSize / 2); k < (baseSize / 2); ++k)
			{
				for (int j = -(baseSize / 2); j < (baseSize / 2); ++j)
				{
					unsigned int scan = randomI + (k * worldSize) + j;
					if (game.world[scan].wall == MATERIAL_NOTHING) { hasAir = true; }
					if (game.world[scan].wall == MATERIAL_WATER) { hasWater = true; }
					if (game.world[scan].wall == MATERIAL_VOIDMETAL) { unsuitable = true;}

				}
			}
			if (unsuitable) { continue; }

			if (underwater)
			{
				if (!hasAir)
				{
					return randomI;
				}
			}

			if (!underwater)
			{
				if (!hasWater)
				{
					return randomI;
				}
			}
		}
	}
}

void setupBuilding_playerBase( int worldPositionI)
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
		if (abs(xdiff) < baseSize && abs(ydiff) < (baseSize + wallThickness )) // set all the tiles around the position to a floor tile
		{
			avgHeight += game.world[i].height;
			tally++;
			game.world[i].terrain = MATERIAL_VOIDMETAL;
			if (  !(game.world[i].wall == MATERIAL_NOTHING || game.world[i].wall == MATERIAL_WATER) )
			{
				game.world[i].wall = MATERIAL_NOTHING;
			}
		}
		if ((   ((x > worldPositionX - baseSize - wallThickness) && (x < worldPositionX - baseSize + wallThickness) ) || // make walls around it // a square border of certain thickness
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
		if (abs(xdiff) < baseSize + wallThickness && abs(ydiff) < baseSize + wallThickness)
		{
			game.world[i].height = avgHeight;
		}
	}
	game.cameraPositionX  = worldPositionX;
	game.cameraPositionY = worldPositionY;
}

void setupGameItems()
{
	// BUILDING 1
	// contains eco computer, player, adversary, and message terminal 1
	int building1 =  getRandomPosition(false);

	setupBuilding_playerBase(building1);

	int i = 1;
	setupEcologyCompter( i);
	spawnAnimalIntoSlot(2, game.animals[i], building1, false);

	building1 += 25;
	setupMessageComputer( i, 0);
	spawnAnimalIntoSlot(3, game.animals[i], building1, false);

	building1 += 25 * worldSize;
	game.playerRespawnPos = building1;
	spawnPlayer();


	// BUILDING 2
	// contains hospital and message computer 2
	int building2 =  getRandomPosition(false);
	setupBuilding_playerBase(building2);
	setupHospitalComputer(i);
	spawnAnimalIntoSlot(4, game.animals[i], building2, false);

	building2 += 25 * worldSize;
	setupMessageComputer( i, 1);
	spawnAnimalIntoSlot(5, game.animals[i], building2, false);


	// BUILDING 3
	//contains tracker glasses, pistol, and message computer 3
	int building3 =  getRandomPosition(true);
	setupBuilding_playerBase(building3);
	setupTrackerGlasses(i);
	spawnAnimalIntoSlot(6, game.animals[i], building3, false);

	building3 += 25;
	setupExampleGun(i);
	spawnAnimalIntoSlot(7, game.animals[i], building3, false);

	building3 += 25 * worldSize;
	setupMessageComputer( i, 2);
	spawnAnimalIntoSlot(8, game.animals[i], building3, false);


	// adversary is outside, under water
	game.adversaryRespawnPos =  getRandomPosition(true);
	spawnAdversary(game.adversaryRespawnPos);



	// BUILDING 4
	// contains knife, lighter, and message computer 4
	int building4 =  getRandomPosition(true);
	setupBuilding_playerBase(building4);
	setupExampleKnife(i);
	spawnAnimalIntoSlot(9, game.animals[i], building4, false);

	building4 += 25;
	setupExampleLighter(i);
	spawnAnimalIntoSlot(10, game.animals[i], building4, false);

	building4 += 25 * worldSize;
	setupMessageComputer( i, 3);
	spawnAnimalIntoSlot(11, game.animals[i], building4, false);

	building4 -= 25;
	setupDestroyer( i);
	spawnAnimalIntoSlot(12, game.animals[i], building4, false);
}

void setupRandomWorld()
{
	worldCreationStage = 1;
	resetGameState();
	worldCreationStage = 2;
	for (unsigned int pp = 0; pp < prelimSquareSize; pp++)
	{
		unsigned int x = pp % prelimSize;
		unsigned int y = pp / prelimSize;
		float hDistance = x;
		float hMax = prelimSize;
		prelimMap[pp] = hDistance / hMax;
		float noiseScaleFactor = 0.005f;
		float fx = x * noiseScaleFactor;
		float fy = y * noiseScaleFactor;
		float noise =   SimplexNoise::noise(fx, fy) * 0.35;   // Get the noise value for the coordinate
		prelimMap[pp] += noise;
		noiseScaleFactor = 0.05;
		fx = x * noiseScaleFactor;
		fy = y * noiseScaleFactor;
		noise =   SimplexNoise::noise(fx, fy) * 0.075;   // Get the noise value for the coordinate
		prelimMap[pp] += noise;
		noiseScaleFactor = 0.5f;
		fx = x * noiseScaleFactor;
		fy = y * noiseScaleFactor;
		noise =   SimplexNoise::noise(fx, fy) * 0.005;   // Get the noise value for the coordinate
		prelimMap[pp] += noise;
		prelimWater[pp] = initWaterLevel;
	}

	for (unsigned int pp = 0; pp < prelimSquareSize; pp++)
	{
		prelimMap[pp] *= prelimSize;
	}

	worldCreationStage = 3;
	if (erode)
	{
		TinyErode::Simulation simulation(prelimSize, prelimSize);
		simulation.SetMetersPerX(150.0f / prelimSize);
		simulation.SetMetersPerY(150.0f / prelimSize);
		const int iterations = 350;
		for (int i = 0; i < iterations; i++)
		{

			progressString = std::string(" ") + std::to_string(i) + std::string("/") + std::to_string(iterations);

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
	worldCreationStage = 4;
	copyPrelimToRealMap();
	worldCreationStage = 5;
	smoothHeightMap( 5, 0.5f );
	worldCreationStage = 6;
	normalizeTerrainHeight();
	worldCreationStage = 7;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)// place items and terrain
	{
		unsigned int x = worldPositionI % worldSize;
		unsigned int y = worldPositionI / worldSize;
		game.world[worldPositionI].terrain = MATERIAL_ROCK;
		if (x < wallThickness || x > worldSize - wallThickness || y < wallThickness  || y > worldSize - wallThickness)	// walls around the game.world edge
		{
			game.world[worldPositionI].wall = MATERIAL_VOIDMETAL;
		}

	}
	detailTerrain();
	worldCreationStage = 8;
	setupGameItems();
	worldCreationStage = 9;
	recomputeTerrainLighting();
	worldCreationStage = 13;

	save();

	worldCreationStage = 10;
	setFlagReady();
}

void tournamentController()
{
	ZoneScoped;
	if (! game.adversaryDefeated)
	{
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
					for (int i = 0; i < nNeighbours; ++i)
					{
						unsigned int neighbour = game.animals[game.adversary].position + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							if (materialSupportsGrowth(game.world[game.animals[game.adversary].position].terrain ))
							{
#ifdef PLANTS
								if (extremelyFastNumberFromZeroTo(1000) == 0)
								{
									// spawn some plants
									if (game.world[game.animals[game.adversary].position].seedState == MATERIAL_NOTHING)
									{
										game.world[game.animals[game.adversary].position].seedState = MATERIAL_SEED;
										for (int k = 0; k < plantGenomeSize; ++k)
										{
											game.world[game.animals[game.adversary].position].seedGenes[k] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
										}
									}
								}
								else
								{
									// spawn some grass
									if (game.world[game.animals[game.adversary].position].plantState == MATERIAL_NOTHING)
									{
										game.world[game.animals[game.adversary].position].plantState = MATERIAL_GRASS;
									}
								}
#else
								if (game.world[game.animals[game.adversary].position].wall == MATERIAL_NOTHING)
								{
									game.world[game.animals[game.adversary].position].wall = MATERIAL_GRASS;
								}
#endif
							}
						}
					}
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
				if (killLoiteringAdversary)
				{
					if (game.animals[game.adversary].position != adversaryLoiterPos)
					{
						adversaryLoiterPos = game.animals[game.adversary].position ;
						adversaryLoiter = 0;
					}
					else
					{
						adversaryLoiter++;
					}
					if (adversaryLoiter > 1000)
					{
						killAnimal(game.adversary);
						game.animals[game.adversary].retired = true;
					}
				}
			}
			if (respawnLowSpecies)
			{
				census();
				for (int k = 0; k < numberOfSpecies; ++k)// spawn lots of the example animal
				{
					if (game.speciesPopulationCounts[k] == 0 && k != 0)
					{
						int j = 1;
						int domingo = -1;
						unsigned int randomPos = game.animals[game.adversary].position;
						for (int ee = 0; ee < 60; ++ee)
						{
							// spawn 6 of the champion
							int randomCell = getRandomPopulatedCell(game.adversary);
							if (randomCell >= 0)
							{
								randomPos = game.animals[game.adversary].body[randomCell].worldPositionI;
							}

							domingo = spawnAnimal( k,  game.champions[k], randomPos, true);
							if (domingo >= 0)
							{

								game.animals[domingo].energy = game.animals[domingo].maxEnergy;
							}
						}
					}
				}
			}
		}
	}
}

void animalTurn( int i)
{
	ZoneScoped;
	place(i);
	animal_organs( i);
	animalEnergy( i);
}

void model_sector( int from,  int to)
{
	ZoneScoped;
	// includes 'from' but ends just before 'to'

	for ( int i = from; i < to; ++i)
	{
		if (! (game.animals[i].retired))
		{
			animalTurn(i);
		}
	}

}

void model()
{
	ZoneScoped;
	boost::thread t5{ tournamentController };
	boost::thread t6{ updateMap };
	const unsigned int numberOfSectors = 64;
	const unsigned int numberOfAnimalsPerSector = numberOfAnimals / numberOfSectors;
	boost::thread  sectors[numberOfSectors];
	for (int i = 1; i < numberOfSectors; ++i)
	{
		sectors[i] =	boost::thread { model_sector ,   (i - 1) *numberOfAnimalsPerSector   , i * numberOfAnimalsPerSector   };
	}
	for (int i = 0; i < numberOfSectors; ++i)
	{
		(sectors[i]).join();
	}
}

void modelSupervisor()
{
	while (true)
	{
		if (!game.lockfps)
		{
			if (!game.paused && !flagSave)
			{
				model();
				modelFrameCount++;
			}
		}
		if (flagReturn)
		{
			return;
		}
	}
}


void drawMainMenuText()
{
	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;

	bool saveExists = false;
	if (exists_test3(std::string("save/game") ))
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


		if (saveExists)
		{
			printText2D(   std::string("[i] load "), menuX, menuY, textSize);
			menuY += spacing;
			break;
		}
		else
		{
			printText2D(   std::string("[j] new "), menuX, menuY, textSize);
			menuY += spacing;
		}
		break;

	case 1:
		printText2D(   std::string("clear grids "), menuX, menuY, textSize);
		break;
	case 2:
		printText2D(   std::string("seed preliminary map with noise "), menuX, menuY, textSize);
		break;
	case 3:
		printText2D(   std::string("hydraulic erosion ") + progressString, menuX, menuY, textSize);
		break;
	case 4:
		printText2D(   std::string("copy prelim to real map "), menuX, menuY, textSize);
		break;
	case 5:
		printText2D(   std::string("smooth heightmap ") + progressString, menuX, menuY, textSize);
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
	case 13:
		printText2D(   std::string("saving"), menuX, menuY, textSize);
		break;
	}
}

void startSimulation()
{
	worldCreationStage = 0;
	boost::thread t7{ modelSupervisor };
	for ( ;; )
	{
#ifdef TRACY_ENABLE
		FrameMark;
#endif
		boost::thread t2 { threadInterface };
		threadGraphics(); // graphics only works in this thread, because it is the process the SDL context was created in.
		t2.join();
		if (flagReturn)
		{
			t7.join();
			flagReturn = false;
			return;
		}
	}
}

void save()
{
	flagSave = true;
	worldCreationStage = 13;
	std::ofstream out6(std::string("save/game").c_str());
	out6.write( (char*)(&game), sizeof(GameState) );
	out6.close();
	worldCreationStage = 10;
	flagSave = false;
}

void load()
{
	worldCreationStage = 12;
	std::ifstream in6(std::string("save/game").c_str());
	in6.read( (char *)(&game), sizeof(GameState));
	in6.close();
	worldCreationStage = 10;
	setFlagReady();
}

bool test_all()
{
	resetGameState();
	bool testResult_2 = false;
	bool testResult_3 = false;
	bool testResult_4 = false;
	bool testResult_5 = false;
	bool testResult_6 = false;
	int j = 1;

	unsigned int testPos = extremelyFastNumberFromZeroTo(worldSquareSize - 1); //worldSquareSize / 2;

	// 2. animals eat grass and gain energy
	// make a test animal which moves in a straight line at a constant pace, and a row of food for it to eat.
	// run the sim enough that it will eat the food.
	// measure it's energy to see that it ate the food.

	unsigned int testSpecies = numberOfSpecies - 1;
	setupTestAnimal_straightline(j);
	int testAnimal = spawnAnimal(  testSpecies, game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].fAngle = 0.0f;
	place(testAnimal); // apply the changes just made.
	game.animals[testAnimal].energy = game.animals[testAnimal].energy = 1.0f;
	const unsigned int howManyPlantsToEat = 10;
	float initialEnergy = game.animals[testAnimal].energy;
	for (int i = 0; i < howManyPlantsToEat; ++i)
	{
		game.world[testPos + (i * worldSize)].plantState = MATERIAL_GRASS;
	}

	for (int i = 0; i < howManyPlantsToEat; ++i)
	{
		animalTurn(testAnimal);
	}
	// printf("mamainal %f \n", );
	if (game.animals[testAnimal].energy != initialEnergy)
	{
		testResult_2 = true;
	}
	killAnimal(testAnimal);

	// 3. animals reproduce when they have enough energy and their debt is 0
	const int how_long_it_takes_to_make_sure = (baseLungCapacity / aBreath) * 2;
	setupTestAnimal_reproducer(j);
	testAnimal = spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = 2.5f ;
	game.animals[testAnimal].energyDebt = 0.0f;
	census();
	for (int i = 0; i < how_long_it_takes_to_make_sure; ++i)
	{
		animalTurn(testAnimal);
	}
	census();
	if (game.speciesPopulationCounts[testSpecies] == 2)
	{
		testResult_3 = true;
	}
	killAnimal(testAnimal);


	// 4. reproduction copies the animal wholly and exactly, except that lifetime stats are reset to 0 in the new generation, and some mutation may be carried along
	setupTestAnimal_reproducer(j);
	testAnimal =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy;
	game.animals[testAnimal].energyDebt = 0;
	animalTurn(testAnimal);
	int diffs = 0;
	for (int i = 0; i < numberOfAnimalsPerSpecies; ++i)
	{
		int child = (testSpecies * numberOfAnimalsPerSpecies) + i;
		if (game.animals[child].retired == false  )
		{
			if (game.animals[child].parentIdentity == testAnimal  )
			{
				Cell * childBody = game.animals[child].body;
				Cell * parentBody = game.animals[testAnimal].body;
				char * cchild = (char*)childBody;
				char * cparent = (char*)parentBody;
				unsigned int diffs = 0 ;
				for (int k = 0; k < ((animalSquareSize * sizeof(Cell)) / sizeof(char))  ; ++k)
				{
					if (cchild[k] != cparent[k])
					{
						diffs++;
					}
				}
				break;
			}
		}
	}
	if (diffs == 0)
	{
		testResult_4 = true;
	}
	killAnimal(testAnimal);

	// 5, the animal can sense stimulus from the environment, it can propagate through the brain and trigger an actuator.
	// note that this test uses a different test animal.
	setupTestAnimal_eye(j);
	testAnimal =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy;
	game.animals[testAnimal].energyDebt = 0;
	game.animals[testAnimal].energy = (game.animals[testAnimal].maxEnergy / 2) - 1.0f; // give it enough energy for the test
	int testEye = getRandomCellOfType(testAnimal, ORGAN_SENSOR_EYE);
	float originalAngle = game.animals[testAnimal].fAngle;
	unsigned int testEyePosition ;
	if (testEye >= 0)
	{
		testEyePosition = game.animals[testAnimal].body[testEye].worldPositionI;
		game.animals[testAnimal].body[testEye].color = color_white;
		game.world[testEyePosition].light = color_white;
	}
	animalTurn(testAnimal);
	if (game.animals[testAnimal].fAngle != originalAngle)
	{
		testResult_5 = true;
	}
	killAnimal(testAnimal);

// 6. lungs
	setupTestAnimal_airbreathing(j);
	testPos += 10;
	int testAnimal_air_in_air =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2 - 0.1f;
	game.animals[testAnimal].energyDebt = 0;

	setupTestAnimal_airbreathing(j);
	testPos += 10;
	int testAnimal_air_in_water =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2 - 0.1f;
	game.animals[testAnimal].energyDebt = 0;
	game.world[testPos].wall = MATERIAL_WATER;

	setupTestAnimal_waterbreathing(j);
	testPos += 10;
	int testAnimal_water_in_air =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2 - 0.1f;
	game.animals[testAnimal].energyDebt = 0;

	setupTestAnimal_waterbreathing(j);
	testPos += 10;
	int testAnimal_water_in_water =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2 - 0.1f;
	game.animals[testAnimal].energyDebt = 0;
	game.world[testPos].wall = MATERIAL_WATER;

	setupTestAnimal_amphibious(j);
	int testAnimal_amphi_in_air =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2 - 0.1f;
	game.animals[testAnimal].energyDebt = 0;

	setupTestAnimal_amphibious(j);
	testPos += 10;
	int testAnimal_amphi_in_water =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2 - 0.1f;
	game.animals[testAnimal].energyDebt = 0;

	game.world[testPos].wall = MATERIAL_WATER;
	testPos += worldSize;
	game.world[testPos].wall = MATERIAL_WATER;


	// an air breathing animal in air is fine
	// a water breathing animal in water is fine
	// a water breathing animal in air dies
	// an air breathing animal in water dies
	// an amphibious animal is fine in both situations


	for (int i = 0; i < how_long_it_takes_to_make_sure; ++i)
	{
		animalTurn(testAnimal_air_in_air);
		animalTurn(testAnimal_air_in_water);
		animalTurn(testAnimal_water_in_air);
		animalTurn(testAnimal_water_in_water);
		animalTurn(testAnimal_amphi_in_air);
		animalTurn(testAnimal_amphi_in_water);
	}

	if (!(game.animals[testAnimal_air_in_air].retired     ) &&
	        (game.animals[testAnimal_air_in_water].retired   ) &&
	        (game.animals[testAnimal_water_in_air].retired   ) &&
	        !(game.animals[testAnimal_water_in_water].retired ) &&
	        !(game.animals[testAnimal_amphi_in_air].retired   ) &&
	        !(game.animals[testAnimal_amphi_in_water].retired ))
	{
		testResult_6 = true;
	}

// 1. grass grows
// 2. animals eat grass and gain energy
// 3. animals reproduce when they have enough energy
// 4. reproduction copies the animal wholly and exactly, except that lifetime stats are reset to 0 in the new generation, and some mutation may be carried along
// 5. sensors take measurements of the game world to produce a signal
// 6. actuators use signals to move the animal.
// 7 lungs

	if (testResult_2 &&
	        testResult_3 &&
	        testResult_4 &&
	        testResult_5 &&
	        testResult_6)
	{
		return true;
	}
	else
	{
		// print the test report
		printf("DEEP SEA self test report\n");

		if (testResult_2)
		{
			printf("test 2: eat grass: PASS\n");
		}
		else
		{
			printf("test 2: eat grass: FAIL\n");
		}

		if (testResult_3)
		{
			printf("test 3: have baby: PASS\n");
		}
		else
		{
			printf("test 3: have baby: FAIL\n");
		}

		if (testResult_4)
		{
			printf("test 4: baby check: PASS\n");
		}
		else
		{
			printf("test 4: baby check: FAIL\n");
		}

		if (testResult_5)
		{
			printf("test 5: neural pathway: PASS\n");
		}
		else
		{
			printf("test 5: neural pathway: FAIL\n");
		}

		if (testResult_6)
		{
			printf("test 6: breathing: PASS\n");
		}
		else
		{
			printf("test 6: breathing: FAIL\n");
		}
	}
	return false;
}
