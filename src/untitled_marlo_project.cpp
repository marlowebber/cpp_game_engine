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
const bool taxIsByMass           = true;
const bool respawnLowSpecies     = true;
const bool setOrSteerAngle       = false;
const bool printLogs             = true;

const bool environmentScarcity = false;

const bool killLoiteringAdversary = false;
const bool erode = true;
const int prelimSize = worldSize / 10;
const int cameraPanSpeed = 10;
const float baseLungCapacity = 1.0f;
const unsigned int prelimSquareSize = prelimSize * prelimSize;
const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.
const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);
const float neuralNoise = 0.05f;
const float liverStorage = 20.0f;
const unsigned int baseLifespan = 2000;			// if the lifespan is long, the animal's strategy can have a greater effect on its success. If it's very short, the animal is compelled to be just a moving mouth.
const float musclePower = 180.0f;
const float playerSpeed = 5.0f;
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

// const float plantRate = 0.05f;

int plantIdentityCursor = 0;

// map updating
const int sectors = 4;
const int sectorSize = worldSquareSize / sectors;
std::string progressString = std::string("");
GameState game;



// struct Square world[worldSquareSize * 4];
Square *world = new Square[worldSquareSize];
Animal *animals = new Animal[numberOfAnimals];

// GameState * smoothBum = new GameState;


// these are variables which are only needed per session, and never need to be stored.
float prelimMap[prelimSquareSize];
float prelimWater[prelimSquareSize];

// tinyerode stuff
const float initWaterLevel = 1.0f;
auto getHeight = [](int x, int y) -> float {
	unsigned int address = (y * prelimSize) + x;
	return  prelimMap[address];  ///* return height value at (x, y) */
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

bool getPause()
{
	return game.paused;
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

bool playerGrab = false;
bool playerDrop = false;

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
	animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;
	animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = 0;//extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	animals[animalIndex].body[cellLocalPositionI].connections[i].weight = 0.0f;//RNG() - 0.5f;
}

void resetCell(int animalIndex, unsigned int cellLocalPositionI)
{
	animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
	animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
	animals[animalIndex].body[cellLocalPositionI].damage = 0.0f;
	animals[animalIndex].body[cellLocalPositionI].eyeLookX = 0;
	animals[animalIndex].body[cellLocalPositionI].eyeLookY = 0;
	animals[animalIndex].body[cellLocalPositionI].localPosX = 0;
	animals[animalIndex].body[cellLocalPositionI].localPosY = 0;
	animals[animalIndex].body[cellLocalPositionI].grabbedCreature = -1;
	animals[animalIndex].body[cellLocalPositionI].workingValue = 0.0f;

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
		animals[animalIndex].body[i].color = filterColor(  newAnimalColorA , multiplyColorByScalar( newAnimalColorB , RNG())  );
		if (animals[animalIndex].body[i].organ == ORGAN_SENSOR_EYE)
		{
			animals[animalIndex].body[i].color = color_green;
		}
	}
}

void resetAnimal(int animalIndex)
{
	ZoneScoped;
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		std::string gunDescription = std::string("An animal");
		strcpy( &animals[animalIndex].displayName[0] , gunDescription.c_str() );
		animals[animalIndex].cellsUsed = 0;
		animals[animalIndex].numberOfTimesReproduced = 0;
		animals[animalIndex].damageDone = 0;
		animals[animalIndex].damageReceived = 0;
		animals[animalIndex].birthLocation = 0;
		animals[animalIndex].age = 0;
		animals[animalIndex].lifespan = baseLifespan;
		animals[animalIndex].parentIdentity = -1;
		animals[animalIndex].offspringEnergy = 1.0f;
		animals[animalIndex].energy   = 0.0f;
		animals[animalIndex].energyDebt   = 0.0f;
		animals[animalIndex].maxEnergy   = 0.0f;
		animals[animalIndex].fPosX = 0.0f;
		animals[animalIndex].fPosY = 0.0f;
		animals[animalIndex].position = 0;
		animals[animalIndex].uPosX = 0;
		animals[animalIndex].uPosY = 0;
		animals[animalIndex].parentAmnesty = true;
		animals[animalIndex].retired = true;
		animals[animalIndex].isMachine = false;
		animals[animalIndex].machineCallback = MATERIAL_NOTHING;
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnectable(  animals[animalIndex].body[cellIndex].organ ))
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
				animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;//extremelyFastNumberFromZeroTo(1);
				animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = getRandomConnectableCell(animalIndex) ;//animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo;
				animals[animalIndex].body[cellLocalPositionI].connections[i].weight      = (RNG() - 0.5f ) * 2.0f; //animals[animalIndex].body[cellLocalPositionI].connections[i].weight;
			}
			else
			{
				animals[animalIndex].body[cellLocalPositionI].connections[i].used = false; // extremelyFastNumberFromZeroTo(1);
			}
		}
	}
}

void setupExampleAnimal3(int i)
{
	resetAnimal(i);


	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_SENSOR_RANDOM );
	animalAppendCell( i, ORGAN_MUSCLE );
	animalAppendCell( i, ORGAN_MUSCLE_TURN );
	animals[i].body[2].connections[0].used = true;
	animals[i].body[2].connections[0].connectedTo = 0;
	animals[i].body[2].connections[0].weight = 0.1f;
	animals[i].body[3].connections[0].used = true;
	animals[i].body[3].connections[0].connectedTo = 1;
	animals[i].body[3].connections[0].weight = 0.1f;
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_LIVER );
	animalAppendCell( i, ORGAN_GILL );
	animalAppendCell( i, ORGAN_LUNG );
	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );
	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );


	animals[i].generation = 0;
}

void setupExampleAnimal2(int i, bool underwater)
{

	ZoneScoped;
	// set the example back to the default state or it wont work properly.
	resetAnimal(i);


	animalAppendCell( i, ORGAN_GILL );
	animalAppendCell( i, ORGAN_LUNG );


	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );
	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );
	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );

	animalAppendCell( i, ORGAN_LIVER );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );

	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_RANDOM );
	animalAppendCell( i, ORGAN_SENSOR_RANDOM );


	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );


	animalAppendCell( i, ORGAN_MUSCLE );
	animalAppendCell( i, ORGAN_MUSCLE_STRAFE );
	animalAppendCell( i, ORGAN_MUSCLE_TURN );

	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );


	animals[i].generation = 0;
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
		setupExampleAnimal3(j);
		paintAnimal(j);
		game.champions[i] = animals[j];
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
		world[i].terrain = MATERIAL_VOIDMETAL;
		world[i].wall = MATERIAL_NOTHING;
		world[i].identity = -1;
		world[i].trail = 0.0f;
		world[i].height = 1.0f;
		world[i].light = color_black;
		world[i].downhillNeighbour = 0;
		world[i].pheromoneChannel = -1;
// #ifdef PLANTS
		world[i].grassColor =  color_green;
		memset(&(world[i].plantGenes[0]), PLANTGENE_END, sizeof(char) * plantGenomeSize);
		world[i].plantState = MATERIAL_NOTHING;
		world[i].geneCursor = 0;
		world[i].plantIdentity = -1;
		world[i].energy = 0.0f;
		world[i].sequenceReturn = 0;
		world[i].sequenceNumber = 0;
		world[i].grown = false;;
		memset(&(world[i].seedGenes[0]), PLANTGENE_END, sizeof(char) * plantGenomeSize);
		world[i].seedState = MATERIAL_NOTHING;
		world[i].seedIdentity =  -1;
		world[i].seedColor = color_yellow;
		world[i].branching = false;


// #endif
	}
}

void fastReset()
{
	memset( &game, 0x00, sizeof(GameState)  );
	for (int i = 0; i < numberOfAnimals; ++i)
	{
		animals[i].retired = true;
	}
	for (int i = 0; i < worldSquareSize; ++i)
	{
		world[i].plantState = MATERIAL_NOTHING;
		world[i].seedState  = MATERIAL_NOTHING;
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
	game.lockfps           = false;
	game.paused = false;
	game.mousePositionX =  -430;
	game.mousePositionY =  330;

	// these variables keep track of the main characters in the game
	game.playerCreature = -1;
	game.adversary = -1;
	game.adversaryRespawnPos;
	game.selectedAnimal = -1;
	game.selectedPlant = -1;
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

	game.ecoSettings[3]       = 0.0001f; // tax energy scale
	game.ecoSettings[2]     = 0.001f;      // movement energy scale
	game.ecoSettings[0]           = 0.95f; // food (meat) energy
	game.ecoSettings[1]          = 0.25f; // grass energy
	game.ecoSettings[4] = (worldSquareSize / 64) / sectors; ;//updateSize;
	game.ecoSettings[5] = 1.0f ;  //nutrient rate
	game.ecoSettings[6] = 1.0f / 20.0f ; // amount of energy a plant tile requires
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
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX - 1 , animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX     , animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX + 1 , animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX - 1 , animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX + 1 , animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX - 1 , animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX     , animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX + 1 , animals[animalIndex].body[cellIndex].localPosY + 1  ),
	};
	for (int potentialNeighbour = 0; potentialNeighbour < animals[animalIndex].cellsUsed; ++potentialNeighbour)
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			if (animals[animalIndex].body[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        animals[animalIndex].body[potentialNeighbour].localPosY == locations_to_check[i].y  )
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
		unsigned int i = extremelyFastNumberFromZeroTo(animals[animalIndex].cellsUsed);
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
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX - 1 , animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX     , animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX + 1 , animals[animalIndex].body[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX - 1 , animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX + 1 , animals[animalIndex].body[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX - 1 , animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX     , animals[animalIndex].body[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].body[cellIndex].localPosX + 1 , animals[animalIndex].body[cellIndex].localPosY + 1  ),
	};
	for (int i = 0; i < nNeighbours; ++i)
	{
		bool empty = true;
		for (int potentialNeighbour = 0; potentialNeighbour < animals[animalIndex].cellsUsed; ++potentialNeighbour)
		{
			if (animals[animalIndex].body[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        animals[animalIndex].body[potentialNeighbour].localPosY == locations_to_check[i].y  )
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
	unsigned int cellIndex = animals[animalIndex].cellsUsed;
	if (cellIndex < animalSquareSize)
	{
		animals[animalIndex].cellsUsed ++;
		animals[animalIndex].body[cellIndex].localPosX = newPosition.x;
		animals[animalIndex].body[cellIndex].localPosY = newPosition.y;
		animals[animalIndex].body[cellIndex].organ = organType;

		if (  isCellConnecting(organType)) // if the cell is supposed to have connections, go hook it up
		{
			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				unsigned int connectableCell = getRandomConnectableCell( animalIndex);// pick a random connectable cell to connect to.
				bool alreadyConnected =  false;	// check if you are already connected to it.
				for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
				{
					if (  animals[animalIndex].body[cellIndex].connections[j].connectedTo == connectableCell &&
					        animals[animalIndex].body[cellIndex].connections[j] .used)
					{
						alreadyConnected = true;
					}
				}
				if (!alreadyConnected)	// make the new connection if appropriate.
				{
					for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
					{
						if ( ! (animals[animalIndex].body[cellIndex].connections[j].used))
						{
							animals[animalIndex].body[cellIndex].connections[j].used = true;
							animals[animalIndex].body[cellIndex].connections[j].connectedTo = connectableCell;
							animals[animalIndex].body[cellIndex].connections[j].weight = (RNG() - 0.5f ) * 2;
							break;
						}
					}
				}
			}
		}


		if (organType == ORGAN_BIASNEURON)
		{
			animals[animalIndex].body[cellIndex].workingValue =  ((RNG() - 0.5f) * 2.0f);
		}
	}
}

// add a cell to an animal germline in a guided but random way. Used to messily construct new animals, for situations where lots of variation is desirable.
void animalAppendCell(int animalIndex, unsigned int organType)
{
	ZoneScoped;
	Vec_i2 newPosition   = getRandomEmptyEdgeLocation(animalIndex); 	// figure out a new position anywhere on the animal edge
	appendCell(animalIndex, organType,  newPosition);
}

void setupTestAnimal_eye(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_SENSOR_EYE, Vec_i2(0, 0) );
	appendCell( i, ORGAN_NEURON, Vec_i2(0, 1) );
	appendCell( i, ORGAN_MUSCLE_TURN, Vec_i2(0, 2) );
	animals[i].body[1].connections[0].used = true;
	animals[i].body[1].connections[0].connectedTo = 0;
	animals[i].body[1].connections[0].weight = 1.0f;
	animals[i].body[2].connections[0].used = true;
	animals[i].body[2].connections[0].connectedTo = 1;
	animals[i].body[2].connections[0].weight = 1.0f;
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 4) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 5) );
	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 6) );
}

void setupTestAnimal_reproducer(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 0) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 1) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 2) );
	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 3) );
}

void setupTestAnimal_straightline(int i)
{
	// test animal 2 is little more than a mouth that moves at a constant pace.
	resetAnimal(i);
	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 0) );
	appendCell( i, ORGAN_BIASNEURON, Vec_i2(0, 1) );
	appendCell( i, ORGAN_MUSCLE, Vec_i2(0, 2) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 4) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 5) );
	animals[i].body[2].connections[0].used = true;
	animals[i].body[2].connections[0].connectedTo = 1;
	animals[i].body[2].connections[0].weight = 1.0f;
	animals[i].body[1].workingValue = 0.1f;
}

void setupTestAnimal_amphibious(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 0) );
	appendCell( i, ORGAN_GILL, Vec_i2(0, 1) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, 2) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 4) );
	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 5) );
}

void setupTestAnimal_airbreathing(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_LUNG, Vec_i2(0, 0) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, 1) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 2) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );
	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 4) );
}

void setupTestAnimal_waterbreathing(int i)
{
	resetAnimal(i);
	appendCell( i, ORGAN_GILL, Vec_i2(0, 0) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, 1) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 2) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, 3) );

	appendCell( i, ORGAN_MOUTH_VEG, Vec_i2(0, 4) );
}

Vec_f2 getTerrainSlope(unsigned int worldPositionI)
{
	if ((worldPositionI + worldSize) < worldSquareSize)
	{
		float xSurfaceAngle = world[worldPositionI].height - world[worldPositionI + 1].height ;
		float ySurfaceAngle = world[worldPositionI].height - world[worldPositionI + worldSize].height ;
		return Vec_f2(xSurfaceAngle, ySurfaceAngle);
	}
	return Vec_f2(0.0f, 0.0f);

}


void swapEnergyWithNeighbour(unsigned int worldI, unsigned int neighbour)
{
	float amount = (world[worldI].energy - world[neighbour].energy);
	world[worldI].energy    -=  (amount / 2.0f) ;
	world[neighbour].energy +=  (amount / 2.0f) ;
}


void swapNootsWithNeighbour(unsigned int worldI, unsigned int neighbour)
{
	float amount = (world[worldI].nutrients - world[neighbour].nutrients);
	world[worldI].nutrients    -=  (amount / 2.0f) ;
	world[neighbour].nutrients +=  (amount / 2.0f) ;
}


const float seedCost = 3.0f;

// this function governs how plants propagate from square to square.
// The reason for the separation of plant and seed identities is that it allows seeds to move in front of plants in the game world.
// returns if any growth was made, or not.
// bool
void growInto( int to ,  int from,  unsigned int organ, bool fromSeed)
{

	// don't do it if the destination cell already has the same organ type and identity. stops wood from endlessly overgrowing itself.
	int originID  = world[from].seedIdentity ;
	if (!fromSeed)
	{
		originID = world[from].plantIdentity;
		if (world[to].plantState == organ && world[to].plantIdentity == originID) //the thing you are trying to grow is already grown- you should stop.
		{
			return ;//false;
		}
	}
	if (to < 0 || from < 0 || to >= worldSquareSize || from >= worldSquareSize)  // trying to grow over the world size. In this case, the function reports true but you grow nothing, as if the tissue disappeared.
	{
		return;// true;
	}


	if (( materialBlocksMovement (world[to].wall))) // again, return true but get nothing.
	{

		return;// true;
	}
	if (organ == MATERIAL_SEED || organ == MATERIAL_POLLEN) // production of plant gametes
	{

		if (fromSeed)
		{
			// return false;


			if (organ == MATERIAL_SEED)
			{
				world[to].seedIdentity = extremelyFastNumberFromZeroTo(65536);
			}
			else if (organ == MATERIAL_POLLEN)
			{
				world[to].seedIdentity = world[from].seedIdentity;
			}

			world[to].seedColorMoving = world[from].seedColorMoving;
			memcpy( world[to].seedGenes , world[from].seedGenes,  plantGenomeSize * sizeof(char)  );

		}

		else

		{


			if (organ == MATERIAL_SEED)
			{
				world[to].seedIdentity = extremelyFastNumberFromZeroTo(65536);
				world[from].nutrients -= seedCost;//1.0f;
				world[from].energy    -= seedCost;//1.0f;
			}
			else if (organ == MATERIAL_POLLEN)
			{
				world[to].seedIdentity = world[from].plantIdentity;
			}

			world[to].seedColorMoving = world[from].seedColor;
			memcpy( world[to].seedGenes , world[from].plantGenes,  plantGenomeSize * sizeof(char)  );


		}

		world[to].seedState = organ;

	}
	else                                          // production of functional tissues
	{
		if (fromSeed)										// when spawning from a seed
		{
			world[to].plantIdentity =  world[from].seedIdentity;
			world[to].grassColor = color_green;
			world[to].seedColor = color_yellow;
			memcpy( & (world[to].plantGenes[0]) , &(world[from].seedGenes[0]),  plantGenomeSize * sizeof(char)  );
			memset( & (world[to].growthMatrix), false, sizeof(bool) * nNeighbours);
			world[to].energy = seedCost;
			world[to].nutrients = seedCost;
			world[to].geneCursor = 0;
		}
		else                                                 // when propagating from existing tissues
		{
			world[to].plantIdentity = world[from].plantIdentity ;
			world[to].grassColor = world[from].grassColor;
			world[to].seedColor = world[from].seedColor;
			memcpy( & (world[to].plantGenes[0]) , &(world[from].plantGenes[0]),  plantGenomeSize * sizeof(char)  );
			memcpy( &(world[to].growthMatrix[0]), &(world[from].growthMatrix[0]), sizeof(bool) * nNeighbours   );

			world[from].nutrients -= 1.0f;

			world[to].nutrients = 0.0f;
			world[to].energy = 0.0f;

			swapNootsWithNeighbour(from, to);
			swapEnergyWithNeighbour(from, to);

			world[to].geneCursor = world[from].geneCursor + 1 ;
			world[to].sequenceNumber = world[from].sequenceNumber;
			world[to].sequenceReturn = world[from].sequenceReturn;
		}
		world[to].plantState = organ;
		world[to].grown = false;
	}
	return;// true;
}


// // this test plant spawns with a tiny leaf and root, and tests the germination ability.
// void setupTestPlant3(unsigned int worldPositionI)
// {
// 	memset(world[worldPositionI].seedGenes, 0x00, sizeof(char) * plantGenomeSize);

// 	world[worldPositionI].seedGenes[0] = 2;
// 	world[worldPositionI].seedGenes[1] = PLANTGENE_BRANCH;
// 	world[worldPositionI].seedGenes[2] = PLANTGENE_ROOT;
// 	world[worldPositionI].seedGenes[3] = PLANTGENE_END;
// 	world[worldPositionI].seedGenes[4] = PLANTGENE_BREAK;
// 	world[worldPositionI].seedGenes[5] = 2;
// 	world[worldPositionI].seedGenes[6] = 6;
// 	world[worldPositionI].seedGenes[7] = PLANTGENE_BRANCH;
// 	world[worldPositionI].seedGenes[8] = PLANTGENE_LEAF;
// 	world[worldPositionI].seedGenes[9] = PLANTGENE_END;
// 	world[worldPositionI].seedGenes[10] = PLANTGENE_BREAK;
// 	world[worldPositionI].seedGenes[11] = 6;
// 	world[worldPositionI].seedGenes[12] = 4;
// 	world[worldPositionI].seedGenes[13] = PLANTGENE_BRANCH;
// 	world[worldPositionI].seedGenes[14] = PLANTGENE_BUD_A;
// 	world[worldPositionI].seedGenes[15] = PLANTGENE_END;
// 	world[worldPositionI].seedGenes[16] = PLANTGENE_BREAK;
// 	world[worldPositionI].seedGenes[17] = 4;
// }

void setupTestPlant3(unsigned int worldPositionI)
{
	memset(world[worldPositionI].seedGenes, 255, sizeof(char) * plantGenomeSize);
	world[worldPositionI].seedGenes[0] = 2;
	world[worldPositionI].seedGenes[1] = PLANTGENE_WOOD;
	world[worldPositionI].seedGenes[2] = PLANTGENE_BRANCH;
	world[worldPositionI].seedGenes[3] = PLANTGENE_LEAF;
	world[worldPositionI].seedGenes[4] = PLANTGENE_BREAK;
	world[worldPositionI].seedGenes[5] = PLANTGENE_BUD_A;
	world[worldPositionI].seedGenes[6] = PLANTGENE_BUD_A;
	world[worldPositionI].seedGenes[7] = PLANTGENE_END;
	world[worldPositionI].seedColorMoving = color_yellow;
	growInto(worldPositionI, worldPositionI, MATERIAL_SEED, true);
}


void spawnRandomPlant(unsigned int worldI)
{
// spawn some plants
	if (world[worldI].seedState == MATERIAL_NOTHING)
	{
		for (int k = 0; k < plantGenomeSize; ++k)
		{
			world[worldI].plantGenes[k] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
		}

		// setupTestPlant3(worldI);

		growInto(worldI, worldI, MATERIAL_SEED, false);
	}
}

void detailTerrain()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)// place items and terrain
	{
		unsigned int x = worldPositionI % worldSize;
		unsigned int y = worldPositionI / worldSize;
		world[worldPositionI].terrain = MATERIAL_ROCK;
		if (x < wallThickness || x > worldSize - wallThickness || y < wallThickness  || y > worldSize - wallThickness)	// walls around the world edge
		{
			world[worldPositionI].wall = MATERIAL_VOIDMETAL;
		}

	}
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (  world[worldPositionI].terrain == MATERIAL_ROCK )
		{
			Vec_f2 slope = getTerrainSlope(worldPositionI);
			float grade = sqrt( (slope.x * slope.x) +  (slope.y * slope.y)  );
			float colorNoise = 1 + (((RNG() - 0.5f) * 0.35)) ; // map -1,1 to 0,0.8
			if (world[worldPositionI]. height < seaLevel)
			{
				if (grade < 5.0f)
				{

					if (extremelyFastNumberFromZeroTo(5) == 0)
					{
						spawnRandomPlant( worldPositionI );
					}
				}
			}

			if ( world[worldPositionI]. height < biome_marine)
			{
				if (grade < 5.0f)
				{
					world[worldPositionI].terrain = MATERIAL_SAND;
				}

				else
				{
					world[worldPositionI].terrain = MATERIAL_BASALT;
				}

				if (world[worldPositionI].height < seaLevel)
				{
					world[worldPositionI].wall = MATERIAL_WATER;
				}
			}

			else if (world[worldPositionI]. height  > biome_marine && world[worldPositionI]. height  < biome_coastal )
			{
				if (grade < 2.5f)
				{
					world[worldPositionI].terrain = MATERIAL_SOIL;
				}
				else  if (grade < 5.0f)
				{
					world[worldPositionI].terrain = MATERIAL_DIRT;
				}
				else
				{
					world[worldPositionI].terrain = MATERIAL_BASALT;
					world[worldPositionI].wall = MATERIAL_BASALT;
				}
			}

			else if (world[worldPositionI]. height  > biome_coastal)
			{
				if (grade < 2.5f)
				{
					world[worldPositionI].terrain = MATERIAL_GRAVEL;

				}
				else if (grade < 5.0f)
				{
					world[worldPositionI].terrain = MATERIAL_DUST;
				}
				else
				{
					world[worldPositionI].terrain = MATERIAL_BASALT;
					world[worldPositionI].wall = MATERIAL_BASALT;
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
			if (animals[animalIndex].retired)
			{
				return animalIndex;
			}
		}
	}
	game.speciesVacancies[speciesIndex] = false;
	return -1;
}

// this is the bare minimum requirements for any animal to be allowed to exist.
bool validateAnimal(unsigned int animalIndex)
{
	// nominee must be reproductively viable (1 gonad is not enough!) and have a breathing apparatus and a mouth
	unsigned int totalGonads = 0;
	unsigned int totalMouths = 0;
	unsigned int totalBreathing  = 0;
	for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
	{
		if ( animals[animalIndex].body[i].organ == ORGAN_GONAD)
		{
			totalGonads++;
		}

		if (       animals[animalIndex].body[i].organ == ORGAN_LUNG
		           || animals[animalIndex].body[i].organ == ORGAN_GILL )
		{
			totalBreathing++;
		}

		if (          animals[animalIndex].body[i].organ == ORGAN_MOUTH_CARNIVORE
		              || animals[animalIndex].body[i].organ == ORGAN_MOUTH_SCAVENGE
		              || animals[animalIndex].body[i].organ == ORGAN_MOUTH_PARASITE
		              || animals[animalIndex].body[i].organ == ORGAN_MOUTH_VEG
		   )
		{
			totalMouths++;
		}
	}
	if (
	    totalGonads >= 2 && totalMouths >= 1 && totalBreathing >= 1
	    && animals[animalIndex].cellsUsed > 0
	)
	{
		return true;
	}
	return false;
}


// // some genes have permanent effects, or effects that need to be known immediately at birth. Compute them here.
// this function studies the phenotype, not the genotype.
// returns whether the animal is fit to live.
void measureAnimalQualities( int animalIndex)
{
	animals[animalIndex].energyDebt = animals[animalIndex].cellsUsed;
	animals[animalIndex].totalMuscle = 0;
	animals[animalIndex].offspringEnergy = 1.0f;
	animals[animalIndex].lifespan = baseLifespan;
	unsigned int totalLiver = 0;

	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE ||
		        animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_TURN ||
		        animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_STRAFE)
		{
			animals[animalIndex].totalMuscle ++;
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_ADDOFFSPRINGENERGY)
		{
			animals[animalIndex].offspringEnergy += animals[animalIndex].offspringEnergy ;
			if (animals[animalIndex].offspringEnergy > animals[animalIndex].cellsUsed / 2)
			{
				animals[animalIndex].offspringEnergy = animals[animalIndex].cellsUsed / 2; // if its bigger than this, the animal will never be able to reproduce.
			}
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_ADDLIFESPAN)
		{
			animals[animalIndex].lifespan += baseLifespan;
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_LIVER)
		{
			totalLiver++;
		}
	}
	animals[animalIndex].maxEnergy = animals[animalIndex].cellsUsed + (totalLiver * liverStorage);
	animals[animalIndex].lifespan *= 0.85 + (RNG() * 0.3);
}

// choose a random cell of any type that can put forth a connection, which includes all neurons and actuators.
int getRandomConnectingCell(  int animalIndex)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnecting(  animals[animalIndex].body[cellIndex].organ ))
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
	for ( int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnecting(  animals[animalIndex].body[cellIndex].organ ))
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
			tries++; if (tries > animals[animalIndex].cellsUsed) {return -1; }
			std::list< int>::iterator iterator = cellsOfType.begin();
			std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				if (animals[animalIndex].body[(*iterator)].connections[i].used)
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
		if (animals[animalIndex].body[cellIndex].connections[i].used)
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
		if (!(animals[animalIndex].body[cellIndex].connections[i].used))
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
	for ( int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].body[cellIndex].organ == organType)
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
	for ( int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_GILL || animals[animalIndex].body[cellIndex].organ == ORGAN_LUNG )
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
			if (animals[animalIndex].body[ *iterator ].signalIntensity > best)
			{
				best = animals[animalIndex].body[ *iterator ].signalIntensity;
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
	for ( int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].body[cellIndex].organ == organType)
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list< int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return animals[animalIndex] .body[(*iterator)].speakerChannel;
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].body[cellIndex].speakerChannel == channel)
		{
			animals[animalIndex].body[cellIndex].speakerChannel = newChannel;
		}
	}
}

// choose any random populated cell.
int getRandomPopulatedCell( int animalIndex)
{
	std::list< int> cellsOfType;
	int found = 0;
	for ( int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
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
		signalIntensities[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity;
	}
	for (int cellIndex = cellToDelete + 1; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex) // shift array of cells down 1, overwriting the lowest modified cell (the cell to delete)
	{
		animals[animalIndex].body[cellIndex - 1] = animals[animalIndex].body[cellIndex];
	}

	if (animals[animalIndex].cellsUsed > 0)
	{

		animals[animalIndex].cellsUsed--; // clear the end cell which would have been duplicated
		for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)	// go through all cells and update connections
		{
			for (int connectionIndex = 0; connectionIndex < NUMBER_OF_CONNECTIONS; ++connectionIndex)
			{
				if (animals[animalIndex].body[cellIndex].connections[connectionIndex].connectedTo == cellToDelete	)
				{
					animals[animalIndex].body[cellIndex].connections[connectionIndex].used = false;
				}
				else if (animals[animalIndex].body[cellIndex].connections[connectionIndex].connectedTo > cellToDelete)
				{
					animals[animalIndex].body[cellIndex].connections[connectionIndex].connectedTo --;
				}
			}
		}
		for (int cellIndex = 0; cellIndex < animalSquareSize - 1; ++cellIndex)
		{
			animals[animalIndex].body[cellIndex].signalIntensity = signalIntensities[cellIndex + 1];
		}
	}
}

void mutateAnimal( int animalIndex)
{

	if (animals[animalIndex].cellsUsed <= 0) { return;}
	if (animalIndex < 0 || animalIndex >= numberOfAnimals) { return;}

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
		animals[animalIndex].body[mutantSkinCell].color = mutateColor(	animals[animalIndex].body[mutantSkinCell].color);
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
				animals[animalIndex].body[mutantCell].connections[mutantConnection].used = true;//!(animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
				// connect it to whatever
				int target = getRandomConnectableCell(animalIndex);
				if (target >= 0)
				{
					animals[animalIndex].body[mutantCell].connections[mutantConnection].connectedTo = target;
					animals[animalIndex].body[mutantCell].connections[mutantConnection].weight = ((RNG() - 0.5f) * 2.0f );
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
				animals[animalIndex].body[mutantCell].connections[mutantConnection].used = false;//!(animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
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
				animals[animalIndex].body[mutantCell].connections[mutantConnection].connectedTo = mutantPartner;
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
			animals[animalIndex].body[mutantCell].connections[mutantConnection].weight += ((RNG() - 0.5f) * neuralMutationStrength);
		}
		break;
	}
	case MUTATION_MULTIPLYWEIGHT:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			animals[animalIndex].body[mutantCell].connections[mutantConnection].weight *= ((RNG() - 0.5f) * neuralMutationStrength);
		}
		break;
	}
	case MUTATION_ALTERBIAS:// randomise a bias neuron's strength.
	{
		int mutantCell = getRandomCellOfType(animalIndex, ORGAN_BIASNEURON);
		if (mutantCell >= 0)
		{
			animals[animalIndex].body[mutantCell].workingValue *= ((RNG() - 0.5f ) * neuralMutationStrength);
			animals[animalIndex].body[mutantCell].workingValue += ((RNG() - 0.5f ) * neuralMutationStrength);

		}
		break;
	}
	case MUTATION_MOVECELL:// swap an existing cell location without messing up the connections.
	{
		int mutantCell = getRandomPopulatedCell(animalIndex);
		if (mutantCell >= 0)
		{
			Vec_i2 destination  = getRandomEmptyEdgeLocation(animalIndex);
			animals[animalIndex].body[mutantCell].localPosX = destination.x;
			animals[animalIndex].body[mutantCell].localPosY = destination.y;
		}
		break;
	}
	case MUTATION_SPEAKERCHANNEL:// mutate a speaker channel used by groups of cells; change the whole group
	{
		int occupiedChannel = -1;
		int typeOfChannel   = -1;
		unsigned int startingRandomCell = extremelyFastNumberFromZeroTo(animals[animalIndex].cellsUsed) - 1;
		for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
		{
			if (animals[animalIndex].cellsUsed > 0)
			{
				unsigned int cellIndex = (startingRandomCell + i) % animals[animalIndex].cellsUsed;
				if ( organUsesSpeakerChannel( animals[animalIndex].body[cellIndex].organ )  )
				{
					occupiedChannel =	 findOccupiedChannel( animalIndex, animals[animalIndex].body[cellIndex].organ);
				}
				else
				{
					return;
				}
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
				animals[animalIndex].body[mutantCell].eyeLookX += (extremelyFastNumberFromZeroTo(2) - 1);
			}
			else
			{
				animals[animalIndex].body[mutantCell].eyeLookY += (extremelyFastNumberFromZeroTo(2) - 1);
			}
		}
		break;
	}
	}
}

void spawnAnimalIntoSlot(  int animalIndex,
                           Animal parent,
                           unsigned int position, bool mutation) // copy genes from the parent and then copy body from own new genes.
{

	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	resetAnimal(animalIndex);
	for (int i = 0; i < animalSquareSize; ++i)
	{
		animals[animalIndex].body[i] = parent.body[i];
	}
	for (int i = 0; i < animalSquareSize; ++i)
	{
		animals[animalIndex].body[i].damage = 0.0f;
	}
	animals[animalIndex].cellsUsed = parent.cellsUsed;

	animals[animalIndex].isMachine = parent.isMachine;
	animals[animalIndex].machineCallback = parent.machineCallback;
	animals[animalIndex].position = position;
	animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = position / worldSize;
	animals[animalIndex].birthLocation = position;
	animals[animalIndex].fAngle = ( (RNG() - 0.5f) * 2 * const_pi );
	animals[animalIndex].generation ++;

	if (mutation)
	{
		mutateAnimal( animalIndex);
	}

	memcpy( &( animals[animalIndex].displayName[0]), &(parent.displayName[0]), sizeof(char) * displayNameSize  );
	measureAnimalQualities(animalIndex) ;
	if (speciesIndex == 0)
	{
		animals[animalIndex].retired = false;
	}
	else
	{
		if ( validateAnimal( animalIndex) )
		{
			animals[animalIndex].retired = false;
		}
		else
		{
			animals[animalIndex].retired = true;
		}
	}
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(int animalIndex, unsigned int cellWorldPositionI)
{
	ZoneScoped;
	if ( animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		if (cellWorldPositionI < worldSquareSize && world[cellWorldPositionI].identity >= 0)
		{

			if (!animals[animalIndex].retired)
			{
				unsigned int cellIndex = world[cellWorldPositionI].occupyingCell;

				if (animals[animalIndex].body[cellIndex].worldPositionI == cellWorldPositionI)
				{
					if (animals[animalIndex].body[cellIndex].damage < 1.0f)
					{
						return cellIndex;
					}
				}
			}
		}
	}
	return -1;
}

void selectCursorAnimal()
{
	if (game.selectedAnimal >= 0 || game.selectedPlant >= 0)
	{
		game.selectedAnimal = -1;
		game.selectedPlant = -1;

	}
	else
	{
		bool anAnimalWasSelected = false;

		int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
		int cursorPosY = game.cameraPositionY + game.mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
		if (game.cursorAnimal >= 0 && game.cursorAnimal < numberOfAnimals)
		{
			int occupyingCell = isAnimalInSquare(game.cursorAnimal, worldCursorPos);
			if ( occupyingCell >= 0)
			{
				game.selectedAnimal = game.cursorAnimal;
				anAnimalWasSelected = true;
			}
		}


		if (!anAnimalWasSelected)
		{


			if (world [worldCursorPos].plantState != MATERIAL_NOTHING)
			{
				game.selectedPlant = world [worldCursorPos].plantIdentity;
			}

		}

	}
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
	animals[animalIndex].retired = true;
	for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex) // process organs and signals and clear animalIndex on grid
	{
		unsigned int cellWorldPositionI  = animals[animalIndex].body[cellIndex].worldPositionI;
		if (cellWorldPositionI < worldSquareSize)
		{
			if (animals[animalIndex].body[cellIndex].organ != MATERIAL_NOTHING && animals[animalIndex].body[cellIndex].damage < 1.0f)
			{
				world[cellWorldPositionI].pheromoneChannel = 13;
				if (world[cellWorldPositionI].wall == MATERIAL_NOTHING)
				{
					world[cellWorldPositionI].wall = MATERIAL_FOOD;
				}
				if (animals[animalIndex].body[cellIndex].organ == ORGAN_BONE)
				{
					world[cellWorldPositionI].wall = MATERIAL_BONE;
				}
			}
		}
	}
}


void defeatAdversary()
{
	int i = 1;
	setupNeuroGlasses(i);
	spawnAnimalIntoSlot(8, animals[i], animals[game.adversary].position, false);

	game.adversaryDefeated = true;
	killAnimal(game.adversary);

	appendLog( std::string("the adversary was killed!") );
}

void spill(unsigned int material,  unsigned int worldPositionI)
{
	if (world[worldPositionI].wall == MATERIAL_NOTHING)
	{
		world[worldPositionI].wall = material;
	}
	else
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			unsigned int neighbour = worldPositionI += neighbourOffsets[i];
			if ( neighbour < worldSquareSize)
			{
				if (world[neighbour].wall == MATERIAL_NOTHING)
				{
					world[neighbour].wall = material;
					break;
				}
			}
		}
	}
	switch (material)
	{
	case MATERIAL_BLOOD:
	{
		world[worldPositionI].pheromoneChannel = PHEROMONE_BLOOD;
		break;
	}

	case MATERIAL_VOMIT:
	{
		world[worldPositionI].pheromoneChannel = PHEROMONE_PUKE;
		break;
	}

	case MATERIAL_FOOD:
	{
		world[worldPositionI].pheromoneChannel = PHEROMONE_ROTTINGMEAT;
		break;
	}

	case MATERIAL_WATER:
	{
		world[worldPositionI].pheromoneChannel = PHEROMONE_RAIN;
		break;
	}
	}
}

int defenseAtWorldPoint( int animalIndex, unsigned int cellWorldPositionI)
{
	int defense = 1;
	for (unsigned int n = 0; n < nNeighbours; ++n)
	{
		unsigned int worldNeighbour = cellWorldPositionI + neighbourOffsets[n];
		int occupyingCell = isAnimalInSquare(animalIndex, worldNeighbour) ;
		if ( occupyingCell >= 0)
		{
			if (animals[animalIndex].body[occupyingCell].organ == ORGAN_BONE)
			{
				defense++;
			}
		}
	}
	return defense;
}


// return true if you blow the limb off, false if its still attached.
void hurtAnimal( int animalIndex, unsigned int cellIndex, float amount, int shooterIndex)
{
	std::string damageLog = std::string("");
	bool limbLost = false;
	unsigned int cellWorldPositionI = animals[animalIndex].body[cellIndex].worldPositionI;
	float defense = defenseAtWorldPoint(world[cellWorldPositionI].identity, cellWorldPositionI);
	float finalAmount = amount / defense;
	animals[animalIndex].body[cellIndex].damage += amount;
	int painCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_PAIN);
	if (painCell >= 0)
	{
		animals[animalIndex].body[painCell].signalIntensity += amount;
	}
	if (animals[animalIndex].body[cellIndex].damage > 1.0f)
	{
		animals[animalIndex].damageReceived++;

		if (animalIndex == game.adversary && shooterIndex == game.playerCreature && shooterIndex != -1 && animalIndex != -1 )
		{
			if (animals[game.adversary].damageReceived > animals[game.adversary].cellsUsed / 2)
			{
				defeatAdversary();
			}

		}
		limbLost = true;
	}
	bool dropped = false;
	if (limbLost)
	{
		if (animals[animalIndex].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
		{
			spill(organProduces(animals[animalIndex].body[cellIndex].organ), cellWorldPositionI);
			dropped = true;
		}
	}
	if (!dropped)
	{
		spill(MATERIAL_BLOOD,  cellWorldPositionI);
	}
}


// updates the animal's position, and performs any actions resulting from that.
void place( int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	animals[animalIndex].fAngleCos = cos(animals[animalIndex].fAngle);
	animals[animalIndex].fAngleSin = sin(animals[animalIndex].fAngle);

	bool trailUpdate = false;
	float dAngle = 0.0f;
	if (animals[animalIndex].fPosX != animals[animalIndex].lastfposx || animals[animalIndex].fPosY != animals[animalIndex].lastfposy  )
	{
		float fdiffx =  animals[animalIndex].fPosX - animals[animalIndex].lastfposx;
		float fdiffy =  animals[animalIndex].fPosY - animals[animalIndex].lastfposy;
		dAngle = atan2(fdiffy, fdiffx);// use atan2 to turn the diff into an angle.
		dAngle -= 0.5 * const_pi;
		if (dAngle < const_pi)
		{
			dAngle += (2 * const_pi);
		}
		trailUpdate = true;
		animals[animalIndex].lastfposx = animals[animalIndex].fPosX;
		animals[animalIndex].lastfposy = animals[animalIndex].fPosY;
	}

	unsigned int newPosX  = animals[animalIndex].fPosX;
	unsigned int newPosY  = animals[animalIndex].fPosY;
	unsigned int newPosition  =  (newPosY * worldSize) + newPosX;

	if (newPosition < worldSquareSize)
	{
		if (animals[animalIndex].position < worldSquareSize)
		{
			if (  materialBlocksMovement( world[animals[animalIndex].position].wall ) ) // vibrate out of wall if stuck.
			{
				animals[animalIndex].fPosX += ( (RNG() - 0.5f) * 10.0f  );
				animals[animalIndex].fPosY += ( (RNG() - 0.5f) * 10.0f  );
			}
		}


		bool animalInTheWay = false;

		int donkedCreature = world[newPosition].identity;
		if (donkedCreature != animalIndex && donkedCreature >= 0 && donkedCreature < numberOfAnimals)
		{
			int targetLocalPositionI = isAnimalInSquare( donkedCreature, newPosition);
			if (targetLocalPositionI >= 0)
			{
				// don't run into creatures you're carrying.
				bool carrying = false;
				for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
				{
					if (animals[animalIndex].body[i].organ == ORGAN_GRABBER)
					{
						if (animals[animalIndex].body[i].grabbedCreature == donkedCreature)
						{
							carrying = true;
							break;
						}
					}
				}

				if (!carrying)
				{
					animalInTheWay = true;
				}
			}
		}
		if (  materialBlocksMovement( world[newPosition].wall ) || animalInTheWay ) // don't move into walls.
		{
			animals[animalIndex].fPosX  = animals[animalIndex].uPosX;
			animals[animalIndex].fPosY  = animals[animalIndex].uPosY;
		}
		else
		{
			animals[animalIndex].uPosX  = animals[animalIndex].fPosX;
			animals[animalIndex].uPosY  = animals[animalIndex].fPosY;
			animals[animalIndex].position = newPosition;
		}
	}

	for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)                                      // place animalIndex on grid and attack / eat. add captured energy
	{
		if (taxIsByMass)
		{
			animals[animalIndex].energy -= game.ecoSettings[3] ;
		}
		bool okToStep = true;
		int rotatedX = animals[animalIndex].body[cellIndex].localPosX * animals[animalIndex].fAngleCos - animals[animalIndex].body[cellIndex].localPosY * animals[animalIndex].fAngleSin;
		int rotatedY = animals[animalIndex].body[cellIndex].localPosX * animals[animalIndex].fAngleSin + animals[animalIndex].body[cellIndex].localPosY * animals[animalIndex].fAngleCos ;
		unsigned int cellWorldPositionX = animals[animalIndex].uPosX + rotatedX;
		unsigned int cellWorldPositionY = animals[animalIndex].uPosY + rotatedY;
		unsigned int cellWorldPositionI = ((cellWorldPositionY * worldSize) + (cellWorldPositionX)) % worldSquareSize;

		if (world[cellWorldPositionI].identity >= 0 && world[cellWorldPositionI].identity != animalIndex && world[cellWorldPositionI].identity < numberOfAnimals)
		{
			int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionI);
			if (targetLocalPositionI >= 0)
			{
				okToStep = false;
				if (!animals[   world[cellWorldPositionI].identity  ].isMachine)
				{
					unsigned int fellowSpeciesIndex = (world[cellWorldPositionI].identity) / numberOfAnimalsPerSpecies;
					if (fellowSpeciesIndex == speciesIndex)
					{
						animals[animalIndex].lastTouchedKin = world[cellWorldPositionI].identity;
					}
					else
					{
						animals[animalIndex].lastTouchedStranger = world[cellWorldPositionI].identity;
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
			if (world[cellWorldPositionI].plantState == MATERIAL_THORNS)
			{
				hurtAnimal(  animalIndex, cellIndex, 0.01f, -1 );
			}

			world[cellWorldPositionI].identity = animalIndex;
			world[cellWorldPositionI].occupyingCell = cellIndex;

			if (trailUpdate)
			{
				world[cellWorldPositionI].trail    = dAngle;
			}

			// move pollen along with animal
			unsigned int prevWorldPositionI = animals[animalIndex].body[cellIndex].worldPositionI;
			if (prevWorldPositionI < worldSquareSize)
			{
				if (world[  prevWorldPositionI].seedState != MATERIAL_NOTHING )
				{

					if (world[cellWorldPositionI].seedState == MATERIAL_NOTHING)
					{
						world[cellWorldPositionI].seedState = world[  prevWorldPositionI].seedState;
						memcpy( &(world[cellWorldPositionI].seedGenes[0]), &(world[prevWorldPositionI].seedGenes[0]) , sizeof(char)*plantGenomeSize);
						world[prevWorldPositionI].seedState = MATERIAL_NOTHING;
					}
				}
			}
			animals[animalIndex].body[cellIndex].worldPositionI = cellWorldPositionI;
		}
	}
}

void lookAtNextNonretiredAnimal()
{
	unsigned int choice = game.cameraTargetCreature ;
	for (int i = 0; i < numberOfAnimals; ++i)
	{
		choice = (choice + 1) % numberOfAnimals;
		if ( !(animals[choice].retired ))
		{
			break;
		}

	}

	game.cameraTargetCreature = choice;
	game.selectedAnimal = game.cameraTargetCreature;

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


Color whatColorIsThisSquare(  unsigned int worldI)
{
	ZoneScoped;
	Color displayColor = color_black;
	int viewedAnimal = -1;
	int animalIndex = world[worldI].identity;
	int occupyingCell = -1;
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		occupyingCell = isAnimalInSquare(  animalIndex , worldI    );
		if (occupyingCell >= 0)
		{
			viewedAnimal = animalIndex;
		}
	}
	if (viewedAnimal >= 0)
	{
		if (organVisible(animals[viewedAnimal].body[occupyingCell].organ ))
		{
			displayColor = organColors(animals[viewedAnimal].body[occupyingCell].organ );
		}

		if ( animals[viewedAnimal].body[occupyingCell].damage > 0.5f  )
		{
			displayColor = organColors(animals[viewedAnimal].body[occupyingCell].organ );
			displayColor = mixColor( displayColor, color_brightred , ((animals[viewedAnimal].body[occupyingCell].damage ) - 0.5f) * 2.0f );
		}
		else
		{
			displayColor = animals[viewedAnimal].body[occupyingCell].color;
			displayColor = mixColor( color_brightred, displayColor, (animals[viewedAnimal].body[occupyingCell].damage) * 2.0f );
		}


	}
	else
	{
		//1. terrain.
		displayColor = materialColors(world[worldI].terrain);
		if ( world[worldI].plantState != MATERIAL_NOTHING)
		{


			if (


			    world[worldI].plantState == MATERIAL_TUBER ||
			    world[worldI].plantState == MATERIAL_WOOD ||
			    world[worldI].plantState == MATERIAL_ROOT

			)
			{

				displayColor =   multiplyColorByScalar( world[worldI].grassColor , 0.5f);
			}
			else
			{

				displayColor = world[worldI].grassColor;
			}



		}

		//2. wall
		if (world[worldI].wall != MATERIAL_NOTHING)
		{
			if (materialIsTransparent(world[worldI].wall))
			{
				displayColor = filterColor( displayColor,  multiplyColorByScalar( materialColors(world[worldI].wall), 0.5f ) );
			}
			else
			{
				displayColor = materialColors(world[worldI].wall);
			}
		}
	}

	if (true)
	{
		if ( world[worldI].seedState != MATERIAL_NOTHING ) // pollen is visible over the top of animals, because it can cling to them.
		{
			displayColor =  filterColor( displayColor , multiplyColorByScalar( world[worldI].seedColorMoving , 0.5f)  ) ;
		}
	}

	displayColor = multiplyColor(displayColor, world[worldI].light);
	return displayColor;
}

float getNormalisedHeight(unsigned int worldPositionI)
{
	float answer =   world[worldPositionI].height / (worldSize);
	return answer;
}


// given two world vertices, find the direction from a to b, as expressed by what entry in neighbourOffsets is the closest.
// basically get the angle with atan, apply an angle offset so both 0's are in the same place, and then map 0..2pi to 0..8
unsigned int getDownhillNeighbour ( float x,  float y)
{
	unsigned int result = 0;
	float angle = atan2(y, x);
	angle += const_pi;
	angle = angle / (2.0 * const_pi);
	angle *= 8.0f;
	result = angle;
	unsigned int dhn = result % nNeighbours;
	return dhn;
}

void computeLight(unsigned int worldPositionI, float xLightAngle, float yLightAngle)
{
	if (worldPositionI + worldSize < worldSquareSize)
	{
		Vec_f2 slope = getTerrainSlope( worldPositionI);
		world[worldPositionI].downhillNeighbour = getDownhillNeighbour(slope.x, slope.y);
		float xSurfaceDifference = (xLightAngle - slope.x);
		float ySurfaceDifference = (yLightAngle - slope.y);

		float brightness = 1.0f - ((xSurfaceDifference + ySurfaceDifference) / (2.0f * const_pi));
		brightness *= 0.5;
		brightness += 0.5f;
		world[worldPositionI].light = multiplyColorByScalar(color_white, brightness);
	}
}

void smoothSquare(unsigned int worldPositionI, float strength)
{
	float avg = world[worldPositionI].height;
	unsigned int count = 1;
	for (int n = 0; n < nNeighbours; ++n)
	{
		unsigned int neighbour = worldPositionI + neighbourOffsets[n];
		if (neighbour < worldSquareSize)
		{
			avg += world[neighbour].height;
			count ++;
		}
	}
	avg = avg / count;
	for (int n = 0; n < nNeighbours; ++n)
	{
		unsigned int neighbour = worldPositionI + neighbourOffsets[n];
		if (neighbour < worldSquareSize)
		{
			world[neighbour].height += (avg - world[neighbour].height) * strength;
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


// operates on seed genes, not plant genes.
void mutatePlants(unsigned int worldI)
{
	if (extremelyFastNumberFromZeroTo(1) == 0) {return;}

	unsigned int mutationChoice = extremelyFastNumberFromZeroTo(2);
	unsigned int mutationIndex = extremelyFastNumberFromZeroTo(plantGenomeSize - 1);

	if (mutationChoice == 0)
	{	// swap a letter
		world[worldI].plantGenes[mutationIndex] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
	}
	else if (mutationChoice == 1)
	{	// insert a letter
		if (mutationIndex == 0) {mutationIndex = 1;}
		for (int i = plantGenomeSize - 1; i > mutationIndex; --i)
		{
			world[worldI].plantGenes[i] = world[worldI].plantGenes[i - 1] ;
		}
		world[worldI].plantGenes[mutationIndex] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);

	}
	else if (mutationChoice == 2)
	{	// remove a letter
		for (int i = mutationIndex ; i < plantGenomeSize - 1; ++i)
		{
			world[worldI].plantGenes[i] = world[worldI].plantGenes[i + 1] ;
		}
		world[worldI].plantGenes[plantGenomeSize - 1] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
	}
}




void propagateFlame(unsigned int worldI, unsigned int depth)
{
	const unsigned int firePropagationSpeed = 3;
	if (depth > firePropagationSpeed)
	{
		return;
	}
	for (int i = 0; i < nNeighbours; ++i)
	{
		unsigned int neighbour = worldI + neighbourOffsets[i];
		if (neighbour < worldSquareSize)
		{
			if (world[neighbour].plantState != MATERIAL_NOTHING)
			{
				world[neighbour].plantState = MATERIAL_FIRE;
				propagateFlame(neighbour, depth + 1);
			}
		}
	}
}


void clearGrowthMask(unsigned int worldI)
{
	world[worldI].growthMatrix[0] = false;
	world[worldI].growthMatrix[1] = false;
	world[worldI].growthMatrix[2] = false;
	world[worldI].growthMatrix[3] = false;
	world[worldI].growthMatrix[4] = false;
	world[worldI].growthMatrix[5] = false;
	world[worldI].growthMatrix[6] = false;
	world[worldI].growthMatrix[7] = false;
}


void rotateGrowthMask(unsigned int worldI, int k)
{
	int n = abs(k);
	bool ccw = false;
	if (k < 0) { ccw = true;}
	for (int j = 0; j < n; ++j)
	{
		if (ccw)
		{
			bool temp = world[worldI].growthMatrix[0];
			for (unsigned int i = 0; i < nNeighbours; ++i)
			{
				unsigned int n = (i + 1) % nNeighbours;
				world[worldI].growthMatrix[i] = world[worldI].growthMatrix[n]  ;
			}
			world[worldI].growthMatrix[nNeighbours - 1] = temp;
		}
		else
		{
			bool temp = world[worldI].growthMatrix[nNeighbours - 1];
			for (unsigned int i = nNeighbours - 1 ; i > 0; --i)
			{
				unsigned int n = (i - 1) % nNeighbours;
				world[worldI].growthMatrix[i] = world[worldI].growthMatrix[n]  ;
			}
			world[worldI].growthMatrix[0] = temp;
		}
	}
}


// attempt to grow into the nearby squares. return true if there is no potential for growth.
void growIntoNeighbours(unsigned int worldI, unsigned int material)
{
	// bool result = false;

	// if (world[worldI].branching)
	// {


	// 	// if branching, the main trunk continues from the next break at equivalent depth. the growth mask is also rotated.
	// 	int skipAhead = world[worldI].geneCursor;

	// 	// scan forward to find the break point associated with this
	// 	int presentDepth = 1;


	// 	for (int i =  world[worldI].geneCursor + 1; i < plantGenomeSize; ++i)
	// 	{
	// 		char geneAtThisLocation = world[worldI].plantGenes[  i];




	// 		if ( geneAtThisLocation == PLANTGENE_BRANCH)
	// 		{
	// 			presentDepth ++;
	// 		}
	// 		else if  ( geneAtThisLocation == PLANTGENE_SEQUENCE)
	// 		{
	// 			presentDepth++;
	// 		}
	// 		else if  ( geneAtThisLocation == PLANTGENE_BREAK)
	// 		{
	// 			presentDepth--;

	// 			if (presentDepth == 0) // this is your stop
	// 			{
	// 				skipAhead =   i ;
	// 				break;
	// 			}
	// 		}
	// 	}


	// 	// // branches grow perpendicular to the trunk, so just rotate the growth mask 90 degrees


	// 	for (int i = 0; i < nNeighbours; ++i)
	// 	{
	// 		if (world[worldI].growthMatrix[i])
	// 		{
	// 			unsigned int neighbour = worldI + neighbourOffsets[i];
	// 			if (neighbour < worldSquareSize)
	// 			{
	// 				growInto( neighbour, worldI, material, false);
	// 				clearGrowthMask(neighbour) ;
	// 				world[neighbour].growthMatrix[i] = true; // then after branching, the limb just grows straight, initially at least.
	// 			}
	// 		}
	// 	}

	// 	rotateGrowthMask(worldI, -2);

	// 	world[worldI].branching = false;
	// }
	// else
	// {

	for (int i = 0; i < nNeighbours; ++i)
	{
		if (world[worldI].growthMatrix[i])
		{
			unsigned int neighbour = worldI + neighbourOffsets[i];
			if (neighbour < worldSquareSize)
			{
				growInto( neighbour, worldI, material, false);
			}
		}
	}
	// }
}







// this is what seeds and runners spawn.
#define SPROUT MATERIAL_ROOT

void growPlants(unsigned int worldI)
{
	if (worldI >= worldSquareSize) {return;}
	if (world[worldI].grown) {return;}
	if (world[worldI].geneCursor >= plantGenomeSize)
	{
		world[worldI].grown = true;
		return;
	}

	bool done = false;
	int	skipAhead = world[worldI].geneCursor + 1;
	char c = world[worldI].plantGenes[world[worldI].geneCursor];
	if (c < nNeighbours)
	{
		world[worldI].growthMatrix[c] = !(world[worldI].growthMatrix[c]);
	}
	else
	{
		// physically growable stuff
		if (
		    growable(c)
		)
		{
			int numberToGrow = 0;
			for (int i = 0; i < nNeighbours; ++i)
			{
				if (world[worldI].growthMatrix[i])
				{
					numberToGrow++;
				}
			}
			// bool canAfford = false;
			float cost = 1.0f * numberToGrow;
			float energyCost = 0.0f;
			if (c == PLANTGENE_BUD_A || c == PLANTGENE_BUD_F || c == PLANTGENE_RUNNER)
			{
				energyCost = seedCost;
				cost  = (seedCost + 1.0f) * numberToGrow; // each seed carries enough nutrients to make a wood, a root, and a leaf, and to afford it you must also be able to make a bud.
			}
			else if (c == PLANTGENE_BUD_M)
			{
				cost  = 2.0f * numberToGrow;
			}

			if (world[worldI].nutrients > cost && world[worldI].energy > energyCost)
			{



				bool b = world[worldI].branching;


				if (	b )
				{


					rotateGrowthMask(worldI, 2);



					// 	// scan forward to find the break point associated with this
					int presentDepth = 1;


					for (int i =  world[worldI].geneCursor + 1; i < plantGenomeSize; ++i)
					{
						char geneAtThisLocation = world[worldI].plantGenes[  i];




						if ( geneAtThisLocation == PLANTGENE_BRANCH)
						{
							presentDepth ++;
						}
						else if  ( geneAtThisLocation == PLANTGENE_SEQUENCE)
						{
							presentDepth++;
						}
						else if  ( geneAtThisLocation == PLANTGENE_BREAK)
						{
							presentDepth--;

							if (presentDepth == 0) // this is your stop
							{
								skipAhead =   i + 1 ;
								break;
							}
						}
					}
				}




				switch (c)
				{
				case PLANTGENE_TUBER:
					growIntoNeighbours(worldI, MATERIAL_TUBER);
					break;

				case PLANTGENE_WOOD:
					growIntoNeighbours(worldI, MATERIAL_WOOD);
					break;

				case PLANTGENE_ROOT:
					growIntoNeighbours(worldI, MATERIAL_ROOT);
					break;

				case PLANTGENE_LEAF:
					growIntoNeighbours(worldI, MATERIAL_LEAF);
					break;

				case PLANTGENE_POLLENTRAP:
					growIntoNeighbours(worldI, MATERIAL_POLLENTRAP);
					break;

				case PLANTGENE_BUD_A:
					growIntoNeighbours(worldI, MATERIAL_BUD_A);
					growIntoNeighbours(worldI, MATERIAL_SEED);
					break;

				case PLANTGENE_BUD_M:
					growIntoNeighbours(worldI, MATERIAL_BUD_M);
					growIntoNeighbours(worldI, MATERIAL_POLLEN);
					break;

				case PLANTGENE_BUD_F:
					growIntoNeighbours(worldI, MATERIAL_BUD_F);
					break;

				case PLANTGENE_THORNS:
					growIntoNeighbours(worldI, MATERIAL_THORNS);
					break;

				case PLANTGENE_TRICHOME:
					growIntoNeighbours(worldI, MATERIAL_TRICHOME);
					break;


				case PLANTGENE_RUNNER: // start the plant over again
				{


					growIntoNeighbours(worldI, SPROUT);


					for (int i = 0; i < nNeighbours; ++i)
					{
						if (world[worldI].growthMatrix[i])
						{
							unsigned int neighbour = worldI + neighbourOffsets[i];
							if (neighbour < worldSquareSize)
							{
								// clearGrowthMask(neighbour) ;
								// world[neighbour].growthMatrix[i] = true; // branch grows perpendicular to the trunk.



								world[neighbour].identity = extremelyFastNumberFromZeroTo(65536);
								world[neighbour].grassColor = color_green;
								world[neighbour].seedColor = color_yellow;

								world[neighbour].nutrients = seedCost; //6.0f;
								world[neighbour].energy    = seedCost; //6.0f;
								world[worldI].nutrients    -= seedCost; //6.0f;
								world[worldI].energy       -= seedCost; //6.0f;

								world[neighbour].geneCursor = 0;
								world[neighbour].grown = false;
								mutatePlants(neighbour);


							}
						}
					}


					// growInto(  worldI, worldI, SPROUT, false );
					// skipAhead = 0;
					// done = true;
					break;
				}








					// }

				}


				if (b)
				{
// if (world[worldI].plantIdentity == game.selectedPlant)
// 				{
// 					printf("selected plant grew with branching!\n");
// 				}
					rotateGrowthMask(worldI, -2);

					world[worldI].branching = false;
				}

				else
				{
					done = true;
				}

				// }


			}
			else
			{
				// if (world[worldI].plantIdentity == game.selectedPlant)
				// {
				// 	printf("selected plant cannot afford!\n");
				// }
				return;
			}

		}
		else
		{



			switch (c)
			{

			case PLANTGENE_BRANCH:
			{

				world[worldI].branching = true;
				break;
			}


			case PLANTGENE_NECTAR:
			{





				if (world[worldI].nutrients < 1.0f)
				{
					return;
				}



				// {
				spill( MATERIAL_HONEY, worldI);
				// successfullyGrown= true;
				world[worldI].nutrients -= 1.0f;

				break;
			}



			case PLANTGENE_GROW_SYMM_H:
			{
				// mirror the left and right halves of the growth matrix.
				bool originalGrowthMatrix[nNeighbours];
				for (int i = 0; i < nNeighbours; ++i)
				{
					originalGrowthMatrix[i] = world[worldI].growthMatrix[i];
				}
				if (originalGrowthMatrix[7]) { world[worldI].growthMatrix[5] = true;}
				if (originalGrowthMatrix[5]) { world[worldI].growthMatrix[7] = true;}
				if (originalGrowthMatrix[0]) { world[worldI].growthMatrix[4] = true;}
				if (originalGrowthMatrix[4]) { world[worldI].growthMatrix[0] = true;}
				if (originalGrowthMatrix[1]) { world[worldI].growthMatrix[3] = true;}
				if (originalGrowthMatrix[3]) { world[worldI].growthMatrix[1] = true;}




				break;
			}
			case PLANTGENE_GROW_SYMM_V:
			{
				// mirror the top and bottom halves of the growth matrix.
				bool originalGrowthMatrix[nNeighbours];
				for (int i = 0; i < nNeighbours; ++i)
				{
					originalGrowthMatrix[i] = world[worldI].growthMatrix[i];
				}
				if (originalGrowthMatrix[1]) { world[worldI].growthMatrix[5] = true;}
				if (originalGrowthMatrix[2]) { world[worldI].growthMatrix[6] = true;}
				if (originalGrowthMatrix[3]) { world[worldI].growthMatrix[7] = true;}
				if (originalGrowthMatrix[5]) { world[worldI].growthMatrix[1] = true;}
				if (originalGrowthMatrix[6]) { world[worldI].growthMatrix[2] = true;}
				if (originalGrowthMatrix[7]) { world[worldI].growthMatrix[3] = true;}
				break;
			}

			case PLANTGENE_SEQUENCE:
			{
				if ((world[worldI].geneCursor + 1) < plantGenomeSize)
				{
					world[worldI].sequenceNumber = world[worldI].plantGenes[world[worldI].geneCursor + 1]; // take the value of the next gene- that is the number of times to repeat the sequence.
					world[worldI].sequenceReturn = world[worldI].geneCursor + 2;
				}

				skipAhead = world[worldI].geneCursor + 2;

				break;
			}
			case PLANTGENE_RANDOMIZEGROWTHMASK:
			{
				for (int i = 0; i < nNeighbours; ++i)
				{

					world[worldI].growthMatrix[i] = extremelyFastNumberFromZeroTo(1);
				}

				// unsigned int randomDirection = extremelyFastNumberFromZeroTo(nNeighbours - 1);
				// world[worldI].growthMatrix[randomDirection] = true;

				break;
			}



			case PLANTGENE_RANDOMDIRECTION:
			{
				for (int i = 0; i < nNeighbours; ++i)
				{
					world[worldI].growthMatrix[i] = false;
				}

				unsigned int randomDirection = extremelyFastNumberFromZeroTo(nNeighbours - 1);
				world[worldI].growthMatrix[randomDirection] = true;

				break;
			}


			case PLANTGENE_END:
			{
				// done =  true;
				skipAhead = plantGenomeSize;
				break;
			}


			case PLANTGENE_BREAK:
			{
				// scroll back and find what this break is for.
				bool sequenceBreak = false;
				bool branchBreak = false;
				int presentDepth = 1;

				for (int i =  world[worldI].geneCursor - 1; i > 0; --i)
				{
					char geneAtThisLocation = world[worldI].plantGenes[ i ];
					if ( geneAtThisLocation == PLANTGENE_BRANCH)
					{
						presentDepth --;
						if (presentDepth == 0) // this is your stop
						{
							branchBreak = true;
							break;
						}
					}
					else if  ( geneAtThisLocation == PLANTGENE_SEQUENCE)
					{
						presentDepth--;

						if (presentDepth == 0) // this is your stop
						{
							sequenceBreak = true;
							break;
						}


					}
					else if  ( geneAtThisLocation == PLANTGENE_BREAK)
					{
						presentDepth++;

					}
				}



				if (branchBreak)
				{
					// if this is a branch break, it means you have come to the end of the branch, you should just deactivate this cell for now.
					done = true;
				}


				if (sequenceBreak)
				{

					// if the sequence number is greater than zero, return to the sequence origin and decrement the sequence number and depth.
					if (world[worldI].sequenceNumber > 0)
					{
						// world[worldI].geneCursor = world[worldI].sequenceReturn;

						skipAhead = world[worldI].sequenceReturn;//world[worldI].geneCursor + i + 1;
						world[worldI].sequenceNumber--;
					}
					else
					{
						// if it is 0, you've completed doing the sequence n times, so you can exit it.
						// In this case, sequenceReturn of the next cells will be sampled from a cell before the sequence start, which allows nested sequences.
						int innerSequenceReturn = world[worldI].sequenceReturn;

						// if (innerSequenceReturn > 3) // impossible to have a complete sequence header earlier than 3
						// {
						// Why 3? Because sequence returns don't point at the sequence gene itself, they point at the first gene IN the sequence.
						// The header goes <last gene of outer sequence> <sequence gene> <length> <first gene> .. . you always return to the first gene inside the sequence
						// when breaking an inner sequence,  return to the last gene of outer sequence, which is 3 cells behind where the inner sequence returns to.
						// int lastGeneOfOuterSequence = innerSequenceReturn - 3;
						if (innerSequenceReturn > 3)
						{
							world[worldI].sequenceReturn = innerSequenceReturn - 3 ;
							world[worldI].sequenceNumber =  0;
						}
						// }
					}
				}


				break;
			}

			case PLANTGENE_GOTO:
			{
				int nextgene = world[worldI].geneCursor + 1 ;
				if (nextgene < plantGenomeSize)
				{
					int destination = world[worldI].plantGenes[  nextgene  ];
					if (destination < plantGenomeSize)
					{
						int newGeneCursor = nextgene % plantGenomeSize;
						skipAhead = nextgene;
					}
				}
				break;
			}





			case PLANTGENE_EXTRUDEMATRIX:
			{


				bool tempMatrix[nNeighbours];
				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					tempMatrix[i] = world[worldI].growthMatrix[i];
				}

				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					if (tempMatrix[i])
					{
						unsigned int prevNeighbour = (i - 1) % nNeighbours;
						unsigned int nextNeighbour = (i + 1) % nNeighbours;
						world[worldI].growthMatrix[prevNeighbour] = true;
						world[worldI].growthMatrix[nextNeighbour] = true;
					}
				}
				break;
			}

			case PLANTGENE_ROTATEMATRIXCCW:
			{
				// bool temp = world[worldI].growthMatrix[0];
				// for (unsigned int i = 0; i < nNeighbours; ++i)
				// {
				// 	unsigned int n = (i + 1) % nNeighbours;
				// 	world[worldI].growthMatrix[i] = world[worldI].growthMatrix[n]  ;
				// }
				// world[worldI].growthMatrix[nNeighbours - 1] = temp;

				rotateGrowthMask(worldI, 1);

				break;
			}

			case PLANTGENE_ROTATEMATRIXCW:
			{
				// bool temp = world[worldI].growthMatrix[0];
				// for (unsigned int i = 0; i < nNeighbours; ++i)
				// {
				// 	unsigned int n = (i + 1) % nNeighbours;
				// 	world[worldI].growthMatrix[i] = world[worldI].growthMatrix[n]  ;
				// }
				// world[worldI].growthMatrix[nNeighbours - 1] = temp;

				rotateGrowthMask(worldI, -1);

				break;
			}

			case PLANTGENE_UPHILL:
			{
				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					world[worldI].growthMatrix[i] = false;
				}
				unsigned int uphillNeighbour = 0;
				float uphillHeight = -1 * (worldSize) * 10.0f;
				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					unsigned int neighbour = worldI + neighbourOffsets[i];
					if (neighbour < worldSquareSize)
					{
						if (world[  neighbour  ].height > uphillHeight)
						{
							uphillNeighbour = i;
							uphillHeight = world[  neighbour  ].height ;
						}
					}
				}
				world[worldI].growthMatrix[uphillNeighbour] = true;
				break;
			}

			case PLANTGENE_TOWARDLIGHT:
			{
				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					world[worldI].growthMatrix[i] = false;
				}
				unsigned int uphillNeighbour = 0;
				float uphillHeight = -1 * (worldSize) * 10.0f;
				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					unsigned int neighbour = worldI + neighbourOffsets[i];
					if (neighbour < worldSquareSize)
					{
						float cuctus = colorAmplitude( world[  neighbour  ].light );
						if ( cuctus  > uphillHeight)
						{
							uphillNeighbour = i;
							uphillHeight = cuctus ;
						}
					}
				}
				world[worldI].growthMatrix[uphillNeighbour] = true;
				break;
			}


			case PLANTGENE_INVERTMATRIX:
			{
				for (unsigned int i = 0; i < nNeighbours; ++i)
				{
					world[worldI].growthMatrix[i] = !(world[worldI].growthMatrix[i]);
				}

				break;
			}

			case PLANTGENE_RED:
			{
				world[worldI].grassColor.r *= 1.35f;
				world[worldI].grassColor = normalizeColor(world[worldI].grassColor);
				break;
			}
			case PLANTGENE_GREEN:
			{
				world[worldI].grassColor.b *= 1.35f;
				world[worldI].grassColor = normalizeColor(world[worldI].grassColor);
				break;
			}
			case PLANTGENE_BLUE:
			{
				world[worldI].grassColor.b *= 1.35f;
				world[worldI].grassColor = normalizeColor(world[worldI].grassColor);
				break;
			}
			case PLANTGENE_LIGHT:
			{
				world[worldI].grassColor.r *= 1.35f;
				world[worldI].grassColor.g *= 1.35f;
				world[worldI].grassColor.b *= 1.35f;
				world[worldI].grassColor = normalizeColor(world[worldI].grassColor);
				break;
			}
			case PLANTGENE_DARK:
			{
				world[worldI].grassColor.r *= 0.75f;
				world[worldI].grassColor.g *= 0.75f;
				world[worldI].grassColor.b *= 0.75f;
				break;
			}
			case PLANTGENE_SEEDCOLOR_RED:
			{
				world[worldI].seedColor.r *= 1.35f;
				world[worldI].seedColor = normalizeColor(world[worldI].seedColor);
				break;
			}
			case PLANTGENE_SEEDCOLOR_GREEN:
			{
				world[worldI].seedColor.b *= 1.35f;
				world[worldI].seedColor = normalizeColor(world[worldI].seedColor);
				break;
			}
			case PLANTGENE_SEEDCOLOR_BLUE:
			{
				world[worldI].seedColor.b *= 1.35f;
				world[worldI].seedColor = normalizeColor(world[worldI].seedColor);
				break;
			}
			case PLANTGENE_SEEDCOLOR_LIGHT:
			{
				world[worldI].seedColor.r *= 1.35f;
				world[worldI].seedColor.g *= 1.35f;
				world[worldI].seedColor.b *= 1.35f;
				world[worldI].seedColor = normalizeColor(world[worldI].seedColor);
				break;
			}
			case PLANTGENE_SEEDCOLOR_DARK:
			{
				world[worldI].seedColor.r *= 0.75f;
				world[worldI].seedColor.g *= 0.75f;
				world[worldI].seedColor.b *= 0.75f;
				break;
			}







			}
		}


	}



	if (done) // if done, don't increment the gene cursor again- this cell will be what it is now permanently, until overgrown at least, but it will regrow into its neighbours if it can.
	{

		world[worldI].grown = true;
	}
	else
	{

		world[worldI].geneCursor = skipAhead;
	}


}



void damagePlants(unsigned int worldI)
{

	// regrow plants if they are damaged
	for (int i = 0; i < nNeighbours; ++i)
	{
		unsigned int neighbour = worldI + neighbourOffsets[i];
		if (neighbour < worldSquareSize)
		{
			world[worldI].grown = false;
		}
	}

	world[worldI].plantState = MATERIAL_NOTHING;

}



void moveSeed(unsigned int from, unsigned int to)
{
	world[to].seedState = world[from].seedState;
	world[to].seedIdentity = world[from].seedIdentity;
	memcpy( world[to].seedGenes , world[from].seedGenes,  plantGenomeSize * sizeof(char)  );
	world[to].seedColorMoving = world[from].seedColorMoving;
	world[from].seedState = MATERIAL_NOTHING;
	world[from].seedIdentity = -1;
}


// #define EXCHANGE_RATE_TUBER 10.0f
// #define EXCHANGE_RATE_NORMAL 1.0f



void equalizeWithNeighbours( unsigned int worldI )
{
	// transfer energy and nutrients between adjacent cells with same ID
	for (int i = 0; i < nNeighbours; ++i)
	{
		unsigned int neighbour = worldI + neighbourOffsets[  i ];
		if (neighbour < worldSquareSize)
		{
			if ( world[neighbour].identity == world[worldI].identity  )
			{
				if (world[worldI].plantState != MATERIAL_NOTHING && world[neighbour].plantState != MATERIAL_NOTHING)
				{

					// bool swap = false;
					// if ( world[worldI].plantState == world[neighbour].plantState)
					// {swap = true;}

					// if ( world[worldI].plantState == MATERIAL_WOOD || world[neighbour].plantState == MATERIAL_WOOD)
					// {swap = true;}
					// if (
					//     (world[worldI].plantState == MATERIAL_ROOT  && world[neighbour].plantState == MATERIAL_TUBER) ||
					//     (world[worldI].plantState == MATERIAL_TUBER && world[neighbour].plantState == MATERIAL_ROOT)
					// )
					// {swap = true;}
					// if (swap)
					// {
					swapNootsWithNeighbour(worldI, neighbour);
					swapEnergyWithNeighbour(worldI, neighbour);
					// }
				}
			}
		}
	}
}



void updatePlants(unsigned int worldI)
{
	if ( (worldI >= worldSquareSize)) {return;}

	if (world[worldI].seedState != MATERIAL_NOTHING)
	{
		switch (world[worldI].seedState)
		{
		case MATERIAL_POLLEN:
		{
			// only move the pollen if it's drifted from where it grew.
			if (world[worldI].seedIdentity != world[worldI].plantIdentity)
			{
				// move seeds randomly
				unsigned int neighbour = worldI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
				if (neighbour < worldSquareSize)
				{
					if ( !materialBlocksMovement( world[neighbour].wall ) && world[neighbour].seedState == MATERIAL_NOTHING )
					{
						moveSeed(worldI, neighbour);
					}


					// pollen degrades if not attached to an animal, to prevent it building up too much in the world.
					bool bonded = false;
					if (world[worldI].identity >= 0 && world[worldI].identity < numberOfAnimals)
					{
						int bond = isAnimalInSquare( world[worldI].identity , worldI )  ;
						if (  bond >= 0 )
						{
							bonded = true;
						}
					}
					if (!bonded)
					{
						if (extremelyFastNumberFromZeroTo(100) == 0)
						{
							world[neighbour].seedState = MATERIAL_NOTHING;
						}
					}
				}
			}
			break;
		}
		case MATERIAL_SEED:
		{

			// spawn plants from seeds if applicable
			// if (materialSupportsGrowth(world[worldI].terrain))
			// {
			if (extremelyFastNumberFromZeroTo(10) == 0)
			{


				growInto(  worldI, worldI, SPROUT, true );
				world[worldI].seedState = MATERIAL_NOTHING;

			}
			// }

			// move seeds randomly
			unsigned int neighbour = worldI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
			if (neighbour < worldSquareSize)
			{
				if ( !materialBlocksMovement( world[neighbour].wall ) && world[neighbour].seedState == MATERIAL_NOTHING )
				{
					moveSeed(worldI, neighbour);
				}

				bool bonded = false;
				if (world[worldI].identity >= 0 && world[worldI].identity < numberOfAnimals)
				{
					int bond = isAnimalInSquare( world[worldI].identity , worldI )  ;
					if (  bond >= 0 )
					{
						bonded = true;
					}
				}
				if (!bonded)
				{
					if (extremelyFastNumberFromZeroTo(100) == 0)
					{
						world[neighbour].seedState = MATERIAL_NOTHING;
					}
				}
			}
			break;
		}
		}
	}

	if (    world[worldI].plantState != MATERIAL_NOTHING)
	{



		equalizeWithNeighbours( worldI );

		if (world[worldI].energy >= 0.0f && world[worldI].nutrients >= 0.0f)
		{
			growPlants(worldI);
		}
		else if (world[worldI].energy <= -1.0f || world[worldI].nutrients <= -1.0f)
		{
			damagePlants(worldI);
		}


		world[worldI].energy -= game.ecoSettings[6];



		world[worldI].energy     = clamp(world[worldI].energy ,    -1.0f, nNeighbours * 3.0f);
		world[worldI].nutrients  = clamp(world[worldI].nutrients , -1.0f, nNeighbours * 3.0f);


		// grow plant into neighboring squares if applicable
		switch (world[worldI].plantState)
		{

		case MATERIAL_LEAF:
		{
			if (environmentScarcity)
			{

				world[worldI].energy += colorAmplitude(    multiplyColor(  world[worldI].light , world[worldI].grassColor)) ;
			}
			else
			{

				world[worldI].energy += 1.0f; // colorAmplitude(    multiplyColor(  world[worldI].light , world[worldI].grassColor)) ;
			}
			// equalizeWithNeighbours( worldI );
			break;
		}
		case MATERIAL_WOOD:
		{
			// equalizeWithNeighbours( worldI );


			// if a cut stem is sitting in water, it can survive for a little while.
			// if (world[worldI].wall == MATERIAL_WATER)
			// {
			// 	world[worldI].nutrients += materialFertility (world[worldI].terrain) * (game.ecoSettings[5] * 0.5f);
			// }

			break;
		}

		case MATERIAL_TUBER:
		{
			// stores energy with great capacity.
			break;
		}

		case MATERIAL_ROOT:
		{
			// get nutrients from the environment
			if (environmentScarcity)
			{

				world[worldI].nutrients += materialFertility (world[worldI].terrain) * (game.ecoSettings[5] );
			}
			else
			{

				world[worldI].nutrients +=  (game.ecoSettings[5]) ;
			}

			break;
		}


		case MATERIAL_BUD_F:
		{
			if (world[worldI].energy > 1.0f && world[worldI].nutrients > 1.0f)
			{
				for (int n = 0; n < nNeighbours; ++n)
				{
					unsigned int neighbour = worldI + neighbourOffsets[n];
					if (neighbour < worldSquareSize)
					{
						if (world[neighbour].seedState == MATERIAL_POLLEN)
						{
							if (world[neighbour].seedIdentity != world[worldI].plantIdentity)
							{

								// plant species is basically organized by the color of their seeds.
								const float plantSpeciesThreshold = 0.25f;

								float totalDifference = (
								                            abs(world[neighbour].seedColorMoving.r - world[worldI].seedColor.r)  +
								                            abs(world[neighbour].seedColorMoving.g - world[worldI].seedColor.g)  +
								                            abs(world[neighbour].seedColorMoving.b - world[worldI].seedColor.b))
								                        * 0.33f
								                        ;


								if ( totalDifference < plantSpeciesThreshold)
								{
									// sex between two plants
									// take half the pollen genes, put them in that bud's plantgenes.
									// mutate the bud before adding the new genes, the result is that this method offers 50% less mutation.
									mutatePlants(worldI);

									for (int i = 0; i < plantGenomeSize; ++i)
									{
										if (extremelyFastNumberFromZeroTo(1) == 0)
										{
											world[worldI].plantGenes[i] =  world[neighbour].seedGenes[i] ;
										}
									}
									world[neighbour].seedState = MATERIAL_NOTHING;
									world[neighbour].seedIdentity = -1;

									// then grow a seed from that mix.
									growInto( worldI, worldI, MATERIAL_SEED, false);

									world[worldI].plantState = MATERIAL_NOTHING;
									world[worldI].plantIdentity = -1;
									break;
								}
							}
						}
					}
				}
			}
			break;
		}

		case MATERIAL_POLLENTRAP:
		{
			for (int n = 0; n < nNeighbours; ++n)
			{
				unsigned int neighbour = worldI + neighbourOffsets[n];
				if (neighbour < worldSquareSize)
				{
					if (world[neighbour].seedState == MATERIAL_POLLEN)
					{
						if (world[neighbour].seedIdentity != world[worldI].plantIdentity)
						{
							// plant species is basically organized by the color of their seeds.
							const float plantSpeciesThreshold = 0.25f;
							float totalDifference = (
							                            abs(world[neighbour].seedColorMoving.r - world[worldI].seedColor.r)  +
							                            abs(world[neighbour].seedColorMoving.g - world[worldI].seedColor.g)  +
							                            abs(world[neighbour].seedColorMoving.b - world[worldI].seedColor.b))
							                        * 0.33f;
							if ( totalDifference > plantSpeciesThreshold)
							{
								// destroy the alien pollen.
								world[neighbour].seedState = MATERIAL_NOTHING;
								break;
							}
						}
					}
				}
			}
			break;
		}
		}
	}

	if ( world[worldI].plantState == MATERIAL_FIRE)
	{
		propagateFlame(worldI, 0);
	}
}

// #endif



bool raining = false;
void toggleRain()
{
	raining = !raining;
}

void updateMapI(unsigned int worldI)
{
	// slowly reduce pheromones over time.
	if (world[worldI].pheromoneChannel >= 0)
	{
		for (int n = 0; n < nNeighbours; ++n)
		{
			unsigned int neighbour = worldI + neighbourOffsets[n];
			if (neighbour < worldSquareSize)
			{
				if (world[neighbour].pheromoneChannel == MATERIAL_NOTHING)
				{
					world[neighbour].pheromoneChannel = world[worldI].pheromoneChannel;
					world[worldI].pheromoneChannel = MATERIAL_NOTHING;
				}
			}
		}
		const unsigned int pheromoneDecayRate = 10;
		if (extremelyFastNumberFromZeroTo(pheromoneDecayRate))
		{
			world[worldI].pheromoneChannel = MATERIAL_NOTHING;
		}
	}

	if (world[worldI].height > seaLevel)
	{
		if (isALiquid(world[worldI].wall))
		{
			unsigned int dhn =  worldI + neighbourOffsets[ world[worldI].downhillNeighbour ] ;
			if (dhn < worldSquareSize)
			{
				if ( !(  materialBlocksMovement(  world[   dhn  ].wall )  ))
				{
					unsigned int swapWall = world[  dhn ].wall;
					world[  dhn ].wall = world[  worldI ].wall;
					world[  worldI ].wall = swapWall;
				}
			}
		}
	}
	if (raining)
	{
		const unsigned int rainStrength = 1000;
		if (extremelyFastNumberFromZeroTo(rainStrength) == 0 )
		{
			spill(MATERIAL_WATER, worldI);
		}
	}

	if ( materialDegrades( world[worldI].wall) )
	{
		world[worldI].wall = MATERIAL_NOTHING;
	}
	updatePlants(worldI);
}


void updateMapSector( unsigned int sector )
{
	ZoneScoped;
	unsigned int from = sector * sectorSize;
	unsigned int to   = ((sector + 1 ) * sectorSize ) - 1;

	// generate a huge ass list of the map squares you're going to update. Then update them all at once. more than 2x faster than generating and visiting each one in turn.
	unsigned int updateSize = game.ecoSettings[4];
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


	for (unsigned int i = 0; i < numberOfSpeakerChannels; ++i)
	{
		game.speakerChannelsLastTurn [i] = game.speakerChannels[i];
		game.speakerChannels[i] = 0.0f;
	}
}



Vec_i2 getMousePositionRelativeToAnimal( int animalIndex)
{
	int newPosX = game.mousePositionX -   ( game.cameraPositionX  - animals[animalIndex].uPosX);
	int newPosY = game.mousePositionY -   ( game.cameraPositionY  - animals[animalIndex].uPosY);
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
		for (int i = 0; i < animals[game.selectedAnimal].cellsUsed; ++i)
		{
			if (animals[game.selectedAnimal].body[i].localPosX == newpos.x &&
			        animals[game.selectedAnimal].body[i].localPosY == newpos.y )
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
		for (int i = 0; i < animals[game.selectedAnimal].cellsUsed; ++i)
		{
			float startPosX = 500.0f;
			float startPosY = 500.0f;
			float bigSquareSize = 100.0f;

			drawRectangle( Vec_f2(startPosX + (animals[game.selectedAnimal].body[i].localPosX * bigSquareSize), startPosY + (animals[game.selectedAnimal].body[i].localPosY * bigSquareSize)) ,
			               organColors(animals[game.selectedAnimal].body[i].organ), bigSquareSize, bigSquareSize);

		}
	}
}




void healAnimal( int animalIndex)
{
	int pre = 0;
	for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
	{
		if (animals[animalIndex].body[i].damage > 1.0f)
		{
			pre++;
		}
	}

	for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
	{
		if (animals[animalIndex].body[i].damage > 0.0f)
		{
			animals[animalIndex].body[i].damage -= 0.05f;
		}
		else
		{
			animals[animalIndex].body[i].damage = 0.0f;
		}
	}

	int post = 0;
	for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
	{
		if (animals[animalIndex].body[i].damage > 1.0f)
		{
			post++;
		}
	}

	int diff = pre - post;
	if (diff <= animals[animalIndex].damageReceived)
	{
		animals[animalIndex].damageReceived -= diff;
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
			if (world[shootWorldPosition].identity >= 0 && world[shootWorldPosition].identity != gunIndex && world[shootWorldPosition].identity < numberOfAnimals && world[shootWorldPosition].identity != shooterIndex)
			{
				int shotOffNub = isAnimalInSquare(world[shootWorldPosition].identity, shootWorldPosition);
				if (shotOffNub >= 0 && shotOffNub < animalSquareSize)
				{


					float defense = defenseAtWorldPoint(world[shootWorldPosition].identity, shootWorldPosition);

					hurtAnimal(world[shootWorldPosition].identity, shotOffNub, (RNG()) / defense, shooterIndex);
				}
			}
			if (world[shootWorldPosition].wall == MATERIAL_NOTHING )
			{
				world[shootWorldPosition].wall = MATERIAL_SMOKE;
			}
			if ( materialBlocksMovement( world[shootWorldPosition].wall)  )
			{
				world[shootWorldPosition].wall = MATERIAL_NOTHING;
				break;
			}
		}
	}
}

void exampleGunCallback( int gunIndex, int shooterIndex)
{
	if (gunIndex >= 0)
	{
		shoot( gunIndex, shooterIndex,  animals[gunIndex].position, animals[gunIndex].fAngle);
	}
}

void lighterCallback( int gunIndex, int shooterIndex )
{
	if (gunIndex >= 0)
	{
		int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
		int cursorPosY = game.cameraPositionY + game.mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
		if ( world[worldCursorPos].wall == MATERIAL_NOTHING  )
		{
			world[worldCursorPos].wall = MATERIAL_FIRE;
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
			unsigned int neighbour = animals[animalIndex].body[cellIndex].worldPositionI + (y * worldSize) + x;
			if (neighbour < worldSquareSize)
			{
				if (world[neighbour].identity >= 0 && world[neighbour].identity != animalIndex && world[neighbour].identity < numberOfAnimals)
				{
					int targetLocalPositionI = isAnimalInSquare( world[neighbour].identity, neighbour);
					if (targetLocalPositionI >= 0)
					{
						bool grabbedByAnotherGrabber = false;// finally, make sure the item is not grabbed by another of your own grabbers.
						for (unsigned int cellIndexB = 0; cellIndexB < animals[animalIndex].cellsUsed; ++cellIndexB)                                      // place animalIndex on grid and attack / eat. add captured energy
						{
							if (animals[animalIndex].body[cellIndexB].organ == ORGAN_GRABBER)
							{
								if (animals[animalIndex].body[cellIndexB].grabbedCreature == world[neighbour].identity )
								{
									grabbedByAnotherGrabber = true;
									break;
								}
							}
						}
						if (!grabbedByAnotherGrabber)
						{
							gotSomething = true;
							result = world[neighbour].identity;
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
			if (animals [ game.playerCreature].body[game.playerActiveGrabber].grabbedCreature >= 0)
			{
				if (animals [   animals [ game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].isMachine)
				{
					switch (animals [   animals [ game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback )
					{
					case MACHINECALLBACK_PISTOL :
						exampleGunCallback(animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
						break;
					case MACHINECALLBACK_LIGHTER :
						lighterCallback(animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
						break;
					case MACHINECALLBACK_HOSPITAL :
						paletteCallback(animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
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
	if ( animals[a].energyDebt <= 0.0f && animals[b].energyDebt <= 0.0f  )
	{
		if (animals[a].energy > animals[a].offspringEnergy && animals[b].energy > animals[b].offspringEnergy )
		{
			float energyDonation = animals[a].offspringEnergy + animals[b].offspringEnergy ;
			animals[a].energy -= animals[a].offspringEnergy;
			animals[b].energy -= animals[b].offspringEnergy;
			unsigned int bSpecies = b % numberOfAnimalsPerSpecies;
			int newAnimal = spawnAnimal( bSpecies, animals[b], animals[b].position, true );
			if (newAnimal >= 0)
			{
				for (int i = 0; i < animalSquareSize; ++i)
				{
					if (extremelyFastNumberFromZeroTo(1) == 0)
					{
						animals[newAnimal].body[i] = animals[a].body[i];
					}
				}
				animals[newAnimal].energy += energyDonation;
			}
		}
	}
}



int getRandomCreature(unsigned int speciesIndex)
{

	// int result = -1;
	if (speciesIndex == 0 || game.speciesPopulationCounts[speciesIndex] == 0) {return -1;}

	int start = (speciesIndex * numberOfAnimalsPerSpecies) + extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies - 1);
	for (int i = 0; i < numberOfAnimalsPerSpecies; ++i)
	{



		int choice = (start + i);


		if (choice > (((speciesIndex + 1) * numberOfAnimalsPerSpecies) - 1 )   ) // wrap around this species
		{
			choice -= (numberOfAnimalsPerSpecies - 1);
		}


		if (  !( animals[choice].retired))
		{
			// result = choice;
			return choice;
		}
	}

	return -1;
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

float sumInputs( int animalIndex,  int cellIndex)
{

	float sum = 0.0f;
	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		if (animals[animalIndex].body[cellIndex].connections[i] .used)
		{
			unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
			if (connected_to_cell < animalSquareSize)
			{
				// animals[animalIndex].body[cellIndex].signalIntensity
				sum  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
			}
		}
	}
	return sum;

}


void animal_organs( int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	float totalLiver = 0;

	float sensorium[animals[animalIndex].cellsUsed];

	for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity;//0.0f;

		if (animals[animalIndex].body[cellIndex].damage >= 1.0f) { continue;}
		unsigned int cellWorldPositionI = animals[animalIndex].body[cellIndex].worldPositionI;
		if (cellWorldPositionI >= worldSquareSize) {continue;}
		unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
		unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;
		unsigned int organ = animals[animalIndex].body[cellIndex].organ;
		switch (organ)
		{

		case ORGAN_SWITCH:
		{
			// in a switch, the sum of inputs except 0 and 1 are taken. If the sum is greater than 0, the input 0 is copied to the output. else, the input 1 is copied to the output.
			float sum = 0.0f;
			for (int i = 2; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				if (animals[animalIndex].body[cellIndex].connections[i] .used)
				{
					unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
					if (connected_to_cell < animalSquareSize)
					{
						sum += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
					}
				}
			}
			unsigned int switchedChannel = 0;
			if (sum > 0.0f)
			{
				switchedChannel = 1;
			}
			unsigned int switchCell = animals[animalIndex].body[cellIndex].connections[switchedChannel] .connectedTo;
			sensorium[cellIndex] = animals[animalIndex].body[switchCell].signalIntensity * animals[animalIndex].body[cellIndex].connections[switchedChannel] .weight;
			break;
		}


		case ORGAN_TIMER:
		{


			animals[animalIndex].body[cellIndex].workingValue += (1.0f /
			        ((animals[animalIndex].body[cellIndex].speakerChannel * animals[animalIndex].body[cellIndex].speakerChannel )
			         + 1) );
			sensorium[cellIndex] = sin(animals[animalIndex].body[cellIndex].workingValue );

			break;
		}

		case ORGAN_MEMORY_TX:
		{
			// in this memory, the output is latched to -1 if the sum of inputs goes below -1, and latched to +1 if the sum of inputs goes above 1.
			// it can be thought of as a 'digital' memory encoding a state with two values.
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum > 1.0f)
			{
				sensorium[cellIndex]  = 1.0f;
			}
			else 	if (sum < -1.0f)
			{
				sensorium[cellIndex] = -1.0f;
			}
			break;
		}

		case ORGAN_MEMORY_RX:
		{
			// in this memory, the sum of inputs except 0 are taken.  input 0 is latched to the output if the sum is less than 0, and the output is preserved at its last value if the sum is greater than 0.
			// it can be thought of as an 'analog' memory that stores a continuous number.
			float sum = 0.0f;
			for (int i = 1; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				if (animals[animalIndex].body[cellIndex].connections[i] .used)
				{
					unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
					if (connected_to_cell < animalSquareSize)
					{
						sum += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
					}
				}
			}
			if (sum > 0.0f)
			{
				unsigned int switchCell = animals[animalIndex].body[cellIndex].connections[0] .connectedTo;
				sensorium[cellIndex] = animals[animalIndex].body[switchCell].signalIntensity * animals[animalIndex].body[cellIndex].connections[0] .weight;
			}
			break;
		}


		case ORGAN_COMPARATOR:
		{
			//The output is 1 if input 0 is greater than input 1. else the output is -1.
			unsigned int switchCell = animals[animalIndex].body[cellIndex].connections[0] .connectedTo;
			float inA = animals[animalIndex].body[switchCell].signalIntensity * animals[animalIndex].body[cellIndex].connections[0] .weight;

			switchCell = animals[animalIndex].body[cellIndex].connections[1] .connectedTo;
			float inB = animals[animalIndex].body[switchCell].signalIntensity * animals[animalIndex].body[cellIndex].connections[1] .weight;


			if (inA > inB)
			{
				sensorium[cellIndex] = 1;
			}
			else
			{
				sensorium[cellIndex] = -1;
			}

			break;
		}

		case ORGAN_DERIVATOR:
		{

			// this part is like iirlow. it allows speakerchannel to set the window over which the slope is calculated.
			float sum = sumInputs(  animalIndex,   cellIndex);
			animals[animalIndex].body[cellIndex].workingValue += sum;
			float feedback = animals[animalIndex].body[cellIndex].workingValue / animals[animalIndex].body[cellIndex].speakerChannel; // in this case, speakerchannel refers to the number of 'taps'.
			animals[animalIndex].body[cellIndex].workingValue -= feedback;

			//The output is the derivative of the input sum.
			sensorium[cellIndex] = sum - feedback;
			break;
		}


		case ORGAN_ABSOLUTE:
		{
			// the output is the absolute value of the input sum.
			float sum = sumInputs(  animalIndex,   cellIndex);
			sensorium[cellIndex]  = abs(sum);

			break;
		}
		case ORGAN_IIRLOW:
		{
			//The output is an accumulation of the previous n input sums.
			float sum = sumInputs(  animalIndex,   cellIndex);
			animals[animalIndex].body[cellIndex].workingValue += sum;
			float feedback = animals[animalIndex].body[cellIndex].workingValue / animals[animalIndex].body[cellIndex].speakerChannel; // in this case, speakerchannel refers to the number of 'taps'.
			animals[animalIndex].body[cellIndex].workingValue -= feedback;
			sensorium[cellIndex] = feedback;
			break;
		}
		case ORGAN_IIRHIGH:
		{
			//. The output is the input sum, from which is subtracted an accumulation of the previous n input sums.
			float sum = sumInputs(  animalIndex,   cellIndex);
			animals[animalIndex].body[cellIndex].workingValue += sum;
			float feedback = animals[animalIndex].body[cellIndex].workingValue / animals[animalIndex].body[cellIndex].speakerChannel; // in this case, speakerchannel refers to the number of 'taps'.
			animals[animalIndex].body[cellIndex].workingValue -= feedback;

			sensorium[cellIndex] = feedback - (sum / animals[animalIndex].body[cellIndex].speakerChannel);
			break;
		}




		case ORGAN_EMITTER_WAX:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum > 0.0f)
			{
				spill(MATERIAL_WAX, cellWorldPositionI);
				animals[animalIndex].energy -= 1.0f;
			}
			break;
		}
		case ORGAN_EMITTER_HONEY:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum > 0.0f)
			{
				spill(MATERIAL_HONEY, cellWorldPositionI);
				animals[animalIndex].energy -= 1.0f;
			}
			break;
		}

		case ORGAN_SENSOR_AGE:
		{
			if (animals[animalIndex].lifespan > 0.0f)
			{
				sensorium[cellIndex] = animals[animalIndex].age / animals[animalIndex].lifespan;
			}
			break;
		}
		case TILE_DESTROYER_EYE:
		{
			// pick a random tile within range, see if it contains an animal not of species 0, and shoot it if so.
			if (speciesIndex != 0) {continue;}
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
						if (world[targetPosI].identity >= 0 && world[targetPosI].identity < numberOfAnimals && world[targetPosI].identity != animalIndex)
						{
							unsigned int targetSpecies = world[targetPosI].identity / numberOfAnimalsPerSpecies;
							if ( targetSpecies != 0 )
							{
								int targetCell = isAnimalInSquare( world[targetPosI].identity , targetPosI );
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
				shoot( animalIndex, animalIndex,  animals[animalIndex].position, angleToTarget);
			}
			break;
		}

		case ORGAN_SENSOR_RANDOM:
		{
			sensorium[cellIndex] = ((extremelyFastNumberFromZeroTo(64) - 32.0f) / 32.0f ); //(RNG() - 0.5f) * 2.0f;
			break;
		}

		case ORGAN_GRABBER:
		{
			float sum = 0.0f;
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

				// if an item is selected, only it can be picked up. This is for precise grabbing.
				if (playerGrab)
				{
					bool ye = false;
					if (game.selectedAnimal >= 0)
					{
						if (game.selectedAnimal == potentialGrab)
						{
							ye = true;
						}
					}
					else
					{
						ye = true;
					}

					if (ye)
					{
						sum = 1.0f;
						playerGrab = false;
					}
				}
				if (playerDrop && cellIndex == game.playerActiveGrabber)
				{
					sum = -1.0f;
					playerDrop = false;
				}
			}
			else
			{
				sum = sumInputs(  animalIndex,   cellIndex);
			}

			// Grab stuff.
			if (sum >= 1.0f && animals[animalIndex].body[cellIndex].grabbedCreature  == -1)
			{
				int potentialGrab = getGrabbableItem (animalIndex, cellIndex);
				if (potentialGrab >= 0)
				{
					animals[animalIndex].body[cellIndex].grabbedCreature = potentialGrab;
				}
			}

			// Grabbed items behavior
			if (animals[animalIndex].body[cellIndex].grabbedCreature >= 0 )// if there is a grabbed creature, adjust its position to the grabber.
			{
				//Move grabbed items to the grabber position.
				animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosX = cellWorldPositionX;
				animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosY = cellWorldPositionY;
				animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosX = cellWorldPositionX;
				animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosY = cellWorldPositionY;
				animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].position = cellWorldPositionI;

				// also, if grabbed by the player, adjust the angle of the grabbed object so it points at the mouse cursor. for aiming weapons.
				float fposx = cellWorldPositionX;
				float fposy = cellWorldPositionY;
				float fmousePositionX = game.mousePositionX;
				float fmousePositionY = game.mousePositionY;
				float angleToCursor = atan2(   fmousePositionY - (  game.cameraPositionY - fposy)  ,  fmousePositionX - (game.cameraPositionX - fposx));
				animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fAngle = angleToCursor;

				// Dropping items.
				if ( sum <= -1.0f)
				{
					animals[animalIndex].body[cellIndex].grabbedCreature = -1;
				}
			}

			if (
			    animals[animalIndex].body[cellIndex].grabbedCreature >= 0)
			{
				sensorium[cellIndex]  = 1.0f;
			}
			else
			{
				sensorium[cellIndex]  = -1.0f;
			}

			break;
		}

		case ORGAN_SENSOR_PAIN:
		{
			sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity * 0.99f;
		}

		case ORGAN_SENSOR_HUNGER:
		{
			if (animals[animalIndex].maxEnergy > 0.0f)
			{
				sensorium[cellIndex] = animals[animalIndex].energy / animals[animalIndex].maxEnergy;
			}
			break;
		}

		case ORGAN_SENSOR_BIRTHPLACE:
		{
			if (animals[animalIndex].birthLocation > 0 && animals[animalIndex].birthLocation < worldSquareSize)
			{
				float targetWorldPositionX =   animals[animalIndex]. birthLocation % worldSize;  ;
				float targetWorldPositionY =   animals[animalIndex]. birthLocation / worldSize;  ;
				float fdiffx = targetWorldPositionX - animals[animalIndex].fPosX;
				float fdiffy = targetWorldPositionY - animals[animalIndex].fPosY;
				float targetAngle = atan2( fdiffy, fdiffx );
				sensorium[cellIndex] =  smallestAngleBetween( targetAngle, animals[animalIndex].fAngle);
			}
			break;
		}

		case ORGAN_SENSOR_PARENT:
		{
			if (animals[animalIndex].parentIdentity >= 0 && animals[animalIndex].parentIdentity < numberOfAnimals)
			{
				if (!( animals[  animals[animalIndex].parentIdentity   ]  .retired  )   )
				{
					float targetWorldPositionX = animals[  animals[animalIndex].parentIdentity   ]  .fPosX;
					float targetWorldPositionY = animals[  animals[animalIndex].parentIdentity   ]  .fPosY;
					float fdiffx = targetWorldPositionX - animals[animalIndex].fPosX;
					float fdiffy = targetWorldPositionY - animals[animalIndex].fPosY;
					float targetAngle = atan2( fdiffy, fdiffx );
					sensorium[cellIndex] =  smallestAngleBetween( targetAngle, animals[animalIndex].fAngle);
				}
			}
			break;
		}

		case ORGAN_SENSOR_LAST_STRANGER:
		{
			if (animals[animalIndex].lastTouchedStranger >= 0 && animals[animalIndex].lastTouchedStranger < numberOfAnimals)
			{
				if (!( animals[  animals[animalIndex].lastTouchedStranger   ]  .retired  )   )
				{
					float targetWorldPositionX = animals[  animals[animalIndex].lastTouchedStranger   ]  .fPosX;
					float targetWorldPositionY = animals[  animals[animalIndex].lastTouchedStranger   ]  .fPosY;
					float fdiffx = targetWorldPositionX - animals[animalIndex].fPosX;
					float fdiffy = targetWorldPositionY - animals[animalIndex].fPosY;
					float targetAngle = atan2( fdiffy, fdiffx );
					sensorium[cellIndex] =  smallestAngleBetween( targetAngle, animals[animalIndex].fAngle);
				}
			}
			break;
		}

		case ORGAN_SENSOR_LAST_KIN:
		{
			if (animals[animalIndex].lastTouchedKin >= 0 && animals[animalIndex].lastTouchedKin < numberOfAnimals)
			{
				if (!( animals[  animals[animalIndex].lastTouchedKin   ]  .retired  )   )
				{
					float targetWorldPositionX = animals[  animals[animalIndex].lastTouchedKin   ]  .fPosX;
					float targetWorldPositionY = animals[  animals[animalIndex].lastTouchedKin   ]  .fPosY;
					float fdiffx = targetWorldPositionX - animals[animalIndex].fPosX;
					float fdiffy = targetWorldPositionY - animals[animalIndex].fPosY;
					float targetAngle = atan2( fdiffy, fdiffx );
					sensorium[cellIndex] =  smallestAngleBetween( targetAngle, animals[animalIndex].fAngle);
				}
			}
			break;
		}

		case ORGAN_LUNG:
		{
			if (world[cellWorldPositionI].wall != MATERIAL_WATER)
			{
				sensorium[cellIndex] = baseLungCapacity;
			}
			else
			{
				bool hasGill = false;
				for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
				{
					if (animals[animalIndex].body[i].organ == ORGAN_GILL)
					{
						hasGill = true;
						break;
					}
				}
				if (!hasGill)
				{

					sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity -  aBreath;
					if (animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						animals[animalIndex].damageReceived++;
					}
				}
			}
			break;
		}
		case ORGAN_GILL:
		{
			if (world[cellWorldPositionI].wall == MATERIAL_WATER)
			{
				sensorium[cellIndex] = baseLungCapacity;
			}
			else
			{
				bool hasLung = false;
				for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
				{
					if (animals[animalIndex].body[i].organ == ORGAN_LUNG)
					{
						hasLung = true;
						break;
					}
				}
				if (!hasLung)
				{
					sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity -  aBreath;
					if (animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						animals[animalIndex].damageReceived++;
					}
				}
			}
			break;
		}



		case ORGAN_SENSOR_PHEROMONE:
		{
			animals[animalIndex].body[cellIndex].signalIntensity = 0;
			if (world[cellWorldPositionI].pheromoneChannel >= 0)
			{
				if (animals[animalIndex].body[cellIndex]. speakerChannel ==   world[cellWorldPositionI].pheromoneChannel)
				{
					sensorium[cellIndex]  = 1.0f;
				}
			}
			break;
		}

		case ORGAN_EMITTER_PHEROMONE:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);

			if (sum > 0.0f)
			{
				world[cellWorldPositionI].pheromoneChannel = animals[animalIndex].body[cellIndex]. speakerChannel ;
			}

			break;
		}

		case ORGAN_SPEAKER:
		{
			if ( animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
			{
				float sum = sumInputs(  animalIndex,   cellIndex);
				sum = clamp(sum, -1.0f, 1.0f);
				game.speakerChannels[  animals[animalIndex].body[cellIndex].speakerChannel ] += sum;//animals[animalIndex].body[cellIndex].signalIntensity ;
			}
			break;
		}

		case ORGAN_SENSOR_EAR:
		{
			if (animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
			{
				sensorium[cellIndex]  = game.speakerChannelsLastTurn[ animals[animalIndex].body[cellIndex].speakerChannel ];
			}
			break;
		}

		case ORGAN_SENSOR_TRACKER:
		{
			animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			if ( world [cellWorldPositionI].identity != animalIndex )
			{
				sensorium[cellIndex]  =  smallestAngleBetween(  world[cellWorldPositionI].trail, animals[animalIndex].fAngle);
			}
			break;
		}

		case ORGAN_SENSOR_BODYANGLE:
		{
			sensorium[cellIndex]  = animals[animalIndex].fAngle;
			break;
		}

		case ORGAN_SENSOR_EYE:
		{
			Vec_f2 eyeLook = Vec_f2(animals[animalIndex].body[cellIndex].eyeLookX , animals[animalIndex].body[cellIndex].eyeLookY);
			Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), animals[animalIndex].fAngleSin, animals[animalIndex].fAngleCos, eyeLook);
			unsigned int eyeLookWorldPositionX = cellWorldPositionX + rotatedEyeLook.x;
			unsigned int eyeLookWorldPositionY = cellWorldPositionY + rotatedEyeLook.y;
			unsigned int eyeLookWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

			Color receivedColor = whatColorIsThisSquare(eyeLookWorldPositionI);
			float perceivedColor = 0.0f;
			perceivedColor += (animals[animalIndex].body[cellIndex].color.r - receivedColor.r );
			perceivedColor += (animals[animalIndex].body[cellIndex].color.g - receivedColor.g );
			perceivedColor += (animals[animalIndex].body[cellIndex].color.b - receivedColor.b );
			perceivedColor = perceivedColor / 3.0f;
			sensorium[cellIndex] = 1.0f - perceivedColor;
			break;
		}

		case ORGAN_SENSOR_TOUCH:
		{
			sensorium[cellIndex] = 0.0f;
			for (int i = 0; i < nNeighbours; ++i)
			{
				unsigned int neighbour = cellWorldPositionI + neighbourOffsets[i];
				if (neighbour < worldSquareSize)
				{
					if (world[neighbour].identity >= 0 && world[neighbour].identity < numberOfAnimals)
					{
						if (isAnimalInSquare( world[neighbour].identity , neighbour ) >= 0)
						{
							sensorium[cellIndex]  += 0.5f;
						}
						else if (world[neighbour].wall != MATERIAL_NOTHING)
						{
							sensorium[cellIndex]  += 0.5f;
						}
					}
				}
			}
			unsigned int touchedAnimal = world[cellWorldPositionI].identity;
			if (touchedAnimal < numberOfAnimals)
			{
				if (touchedAnimal >= 0 && touchedAnimal < numberOfAnimals)
				{
					if (touchedAnimal != animalIndex)
					{
						if (isAnimalInSquare( touchedAnimal , cellWorldPositionI ) >= 0)
						{
							sensorium[cellIndex] += 0.5f;
						}
						else if (world[cellWorldPositionI].wall != MATERIAL_NOTHING)
						{
							sensorium[cellIndex] += 0.5f;
						}
					}
				}
			}
			break;
		}


		case ORGAN_BIASNEURON:
		{
			sensorium[cellIndex]  = animals[animalIndex].body[cellIndex].workingValue;
			break;
		}

		case ORGAN_NEURON:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			sensorium[cellIndex]  = fast_sigmoid(sum);
			break;
		}


		case ORGAN_ADDER:
		{
			sensorium[cellIndex] = sumInputs(  animalIndex,   cellIndex);
			break;
		}

		case ORGAN_GONAD:
		{
			bool bonked = false;
			if (doReproduction && animals[animalIndex].energyDebt <= 0.0f )
			{
				float reproducesAt = ((animals[animalIndex].cellsUsed / 2 ) + animals[animalIndex].offspringEnergy );
				if (animals[animalIndex].energy > reproducesAt)
				{
					if (cellWorldPositionI < worldSquareSize)
					{
						unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;

						bool mutate = false;
						if (extremelyFastNumberFromZeroTo(1) == 0)
						{
							mutate = true;
						}

						int result = spawnAnimal( speciesIndex, animals[animalIndex], animals[animalIndex].position, mutate );
						if (result >= 0)
						{
							animals[animalIndex].body[cellIndex].damage = 1.0f;
							animals[animalIndex].numberOfTimesReproduced++;
							animals[animalIndex].energy -= animals[animalIndex].offspringEnergy;
							animals[result].energy       =  animals[animalIndex].offspringEnergy;
							animals[result].parentIdentity       = animalIndex;

							// distribute pheromones
							for (int i = 0; i < nNeighbours; ++i)
							{
								unsigned int neighbour = cellWorldPositionI += neighbourOffsets[i];
								if ( neighbour < worldSquareSize)
								{
									world[neighbour].pheromoneChannel = PHEROMONE_MUSK;
								}
							}

							bonked = true;
						}
					}
				}
			}
			if (bonked)
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity * 0.95f;
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
			bool ate_plant = false;

			if (world[cellWorldPositionI].plantState == MATERIAL_BUD_A ||
			        world[cellWorldPositionI].plantState == MATERIAL_BUD_M ||
			        world[cellWorldPositionI].plantState == MATERIAL_BUD_F ||
			        world[cellWorldPositionI].plantState == MATERIAL_TUBER ||
			        world[cellWorldPositionI].plantState == MATERIAL_LEAF)
			{

				ate_plant = true;
			}
			bool ate = false;
			if (ate_plant)
			{
				ate = true;
				animals[animalIndex].energy += game.ecoSettings[1] ;
				damagePlants(cellWorldPositionI);
			}
			if (world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				animals[animalIndex].energy += 1.0f;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}

			break;
		}
		case ORGAN_MOUTH_WOOD:
		{
			bool ate_plant = false;

			if (world[cellWorldPositionI].plantState == MATERIAL_ROOT ||
			        world[cellWorldPositionI].plantState == MATERIAL_TUBER ||
			        world[cellWorldPositionI].plantState == MATERIAL_WOOD)
			{

				ate_plant = true;
			}
			bool ate = false;
			if (ate_plant)
			{
				ate = true;
				animals[animalIndex].energy += game.ecoSettings[1] ;
				damagePlants(cellWorldPositionI);

			}
			if (world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				animals[animalIndex].energy += 1.0f;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}

			break;
		}
		case ORGAN_MOUTH_SEEDS:
		{
			bool ate_plant = false;

			if (world[cellWorldPositionI].seedState     == MATERIAL_SEED )
			{

				ate_plant = true;
			}
			bool ate = false;
			if (ate_plant)
			{
				ate = true;
				animals[animalIndex].energy += game.ecoSettings[0] ;
				world[cellWorldPositionI].seedState = MATERIAL_NOTHING;
			}
			if (world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				animals[animalIndex].energy += 1.0f;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}
			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}

			break;
		}
		case ORGAN_MOUTH_SCAVENGE :
		{
			bool ate = false;
			if (world[cellWorldPositionI].wall == MATERIAL_FOOD)
			{
				animals[animalIndex].energy += game.ecoSettings[0] ;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
				ate  = true;
			}

			if (world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				animals[animalIndex].energy += 1.0f;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
				ate = true;
			}

			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}


			break;
		}
		case ORGAN_MOUTH_PARASITE:
		{
			bool ate = false;
			if (world[cellWorldPositionI].identity != animalIndex && world[cellWorldPositionI].identity >= 0 && world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
			{
				int leechAttackVictim = isAnimalInSquare(world[cellWorldPositionI].identity , cellWorldPositionI);
				if (leechAttackVictim >= 0)
				{
					if (animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
					{
						if (world[cellWorldPositionI].identity == animals[animalIndex].parentIdentity)
						{
							continue;
						}
					}
					float amount = (animals[world[cellWorldPositionI].identity].energy) / animalSquareSize;
					float defense = defenseAtWorldPoint(world[cellWorldPositionI].identity, cellWorldPositionI);
					amount = amount / defense;
					animals[animalIndex].energy += amount;
					animals[world[cellWorldPositionI].identity].energy -= amount;

					unsigned int victimSpecies =  (world[cellWorldPositionI].identity / numberOfAnimalsPerSpecies) ;
					if (victimSpecies < numberOfSpecies)
					{
						foodWeb[speciesIndex][  victimSpecies] += amount ;
						ate = true;
					}
				}
			}

			if (world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				animals[animalIndex].energy += 1.0f;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}


			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}

			break;
		}
		case ORGAN_MOUTH_CARNIVORE:
		{
			bool ate = false;
			if (world[cellWorldPositionI].identity != animalIndex && world[cellWorldPositionI].identity >= 0 && world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
			{
				int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity , cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					if (animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
					{
						if (world[cellWorldPositionI].identity == animals[animalIndex].parentIdentity)
						{
							continue;
						}
					}
					hurtAnimal(world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f, animalIndex );
					if (world[cellWorldPositionI].wall == MATERIAL_FOOD)
					{
						animals[animalIndex].energy += game.ecoSettings[0] ;
						unsigned int victimSpecies =  (world[cellWorldPositionI].identity / numberOfAnimalsPerSpecies) ;
						if (victimSpecies < numberOfSpecies)
						{
							ate = true;
							foodWeb[speciesIndex][  victimSpecies] += game.ecoSettings[0] ;
						}
					}
				}
			}

			if (world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				animals[animalIndex].energy += 1.0f;
				world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}

			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}
			break;

		}
		case ORGAN_CLAW:
		{

			float sum = 0.0f;
			if (animalIndex != game.playerCreature)
			{
				sum = sumInputs(  animalIndex,   cellIndex);
			}

			bool ate = false;
			if (sum > 0.0f)
			{

				if (world[cellWorldPositionI].identity != animalIndex && world[cellWorldPositionI].identity >= 0 && world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
				{
					int targetLocalPositionI = isAnimalInSquare(world[cellWorldPositionI].identity , cellWorldPositionI);
					if (targetLocalPositionI >= 0)
					{
						ate = true;
						hurtAnimal(world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f, animalIndex );
					}
				}

				if (world[cellWorldPositionI].wall == MATERIAL_WAX)
				{
					ate = true;
					world[cellWorldPositionI].wall = MATERIAL_NOTHING;
				}
			}

			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] += 1.0f;
			}

			break;
		}
		case ORGAN_MUSCLE :
		{
			float sum = 0.0f;
			if (animalIndex != game.playerCreature)
			{
				sum = sumInputs(  animalIndex,   cellIndex);
			}
			sum = clamp(sum, -1.0f, 1.0f);
			sensorium[cellIndex] = sum;

			float impulse = animals[animalIndex].body[cellIndex].signalIntensity  * musclePower;

			animals[animalIndex].fPosX += (impulse / animals[animalIndex].cellsUsed) * animals[animalIndex].fAngleSin;
			animals[animalIndex].fPosY += (impulse / animals[animalIndex].cellsUsed) * animals[animalIndex].fAngleCos;
			animals[animalIndex].energy -= game.ecoSettings[2] * abs(animals[animalIndex].body[cellIndex].signalIntensity ) ;
			break;
		}

		case ORGAN_MUSCLE_STRAFE :
		{
			float sum = 0.0f;
			if (animalIndex != game.playerCreature)
			{
				sum = sumInputs(  animalIndex,   cellIndex);
			}
			sum = clamp(sum, -1.0f, 1.0f);

			sensorium[cellIndex] = sum;

			float impulse = animals[animalIndex].body[cellIndex].signalIntensity  * musclePower;

			animals[animalIndex].fPosX += (impulse / animals[animalIndex].cellsUsed) * animals[animalIndex].fAngleCos;
			animals[animalIndex].fPosY += (impulse / animals[animalIndex].cellsUsed) * animals[animalIndex].fAngleSin;
			animals[animalIndex].energy -= game.ecoSettings[2] * abs(animals[animalIndex].body[cellIndex].signalIntensity ) ;
			break;
		}
		case ORGAN_MUSCLE_TURN:
		{
			float sum = 0.0f;
			if (animalIndex != game.playerCreature)
			{
				sum = sumInputs(  animalIndex,   cellIndex);
			}
			sensorium[cellIndex] = sum;
			if (setOrSteerAngle)
			{
				animals[animalIndex].fAngle = (animals[animalIndex].body[cellIndex].signalIntensity ) ;
			}
			else
			{
				animals[animalIndex].fAngle += sum * turnMusclePower;
			}






			break;
		}
		case ORGAN_GENITAL_A:
		{
			bool bonked = false;

			if ( world[cellWorldPositionI].identity >= 0 &&  world[cellWorldPositionI].identity < numberOfAnimals)
			{
				int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					if (animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].organ == ORGAN_GENITAL_B )
					{
						sexBetweenTwoCreatures( animalIndex, world[cellWorldPositionI].identity );

						bonked = true;
					}
				}
			}
			if (bonked)
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity * 0.95f;
			}

			break;
		}
		case ORGAN_GENITAL_B:
		{

			bool bonked = false;

			if ( world[cellWorldPositionI].identity >= 0 &&  world[cellWorldPositionI].identity < numberOfAnimals)
			{
				int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionI);
				if (targetLocalPositionI >= 0)
				{
					if (animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].organ == ORGAN_GENITAL_A )
					{
						sexBetweenTwoCreatures( world[cellWorldPositionI].identity , animalIndex);

						// distribute pheromones
						for (int i = 0; i < nNeighbours; ++i)
						{
							unsigned int neighbour = cellWorldPositionI += neighbourOffsets[i];
							if ( neighbour < worldSquareSize)
							{
								world[neighbour].pheromoneChannel = PHEROMONE_MUSK;
							}
						}

						bonked = true;
					}
				}
			}

			if (bonked)
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = animals[animalIndex].body[cellIndex].signalIntensity * 0.95f;
			}

			break;
		}

		case ORGAN_LOCATIONREMEMBERER:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum <  0.0f)
			{
				animals[animalIndex].body[cellIndex].speakerChannel = cellWorldPositionI; // remember current location
			}
			else
			{
				if (animals[animalIndex].body[cellIndex]. speakerChannel > 0 && animals[animalIndex].body[cellIndex]. speakerChannel < worldSquareSize)
				{
					float targetWorldPositionX =   animals[animalIndex].body[cellIndex]. speakerChannel % worldSize;  ;
					float targetWorldPositionY =   animals[animalIndex].body[cellIndex]. speakerChannel / worldSize;  ;
					float fdiffx = targetWorldPositionX - animals[animalIndex].fPosX;
					float fdiffy = targetWorldPositionY - animals[animalIndex].fPosY;
					float targetAngle = atan2( fdiffy, fdiffx );
					animals[animalIndex].body[cellIndex].signalIntensity =  smallestAngleBetween( targetAngle, animals[animalIndex].fAngle); // direction to remembered location
				}
			}
			break;
		}
		}
		if ( organIsANeuron(animals[animalIndex].body[cellIndex].organ) || organIsASensor(animals[animalIndex].body[cellIndex].organ) )
		{
			sensorium[cellIndex] += (RNG() - 0.5f) * neuralNoise;
		}
	}
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		animals[animalIndex]. body[cellIndex].signalIntensity = sensorium[cellIndex];
	}
	animals[animalIndex].maxEnergy = animals[animalIndex].cellsUsed + (totalLiver * liverStorage);
}

void animalEnergy(int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	animals[animalIndex].age++;

	animals[animalIndex].energy -= game.ecoSettings[3] * animals[animalIndex].cellsUsed;

	if (animals[animalIndex].energy > animals[animalIndex].maxEnergy)
	{
		animals[animalIndex].energy = animals[animalIndex].maxEnergy;
	}
	if (animals[animalIndex].energyDebt > 0.0f)
	{
		if (animals[animalIndex].energy > (animals[animalIndex].maxEnergy / 2))
		{
			float repayment = animals[animalIndex].energy  - (animals[animalIndex].maxEnergy / 2)  ;
			animals[animalIndex].energyDebt -= repayment;
			animals[animalIndex].energy -= repayment;
		}
	}
	else
	{
		if (animals[animalIndex].parentAmnesty)
		{
			animals[animalIndex].parentAmnesty = false;
		}
	}
	bool execute = false;
	if ( !animals[animalIndex].isMachine && animals[animalIndex].age > 0) // reasons an npc can die
	{
		if (speciesIndex != 0)
		{
			if (animals[animalIndex].energy < 0.0f)
			{
				// printf("died low energy\n");
				execute = true;
			}

			if (animals[animalIndex].age > animals[animalIndex].lifespan)
			{

				// printf("died old\n");
				execute = true;
			}
			// if (animals[animalIndex].totalGonads == 0)
			// {

			// 	printf("died no balls\n");
			// 	execute = true;
			// }
		}



		if (animals[animalIndex].damageReceived > animals[animalIndex].cellsUsed / 2)
		{

			// printf("died damaged\n");
			execute = true;
		}
		if (animals[animalIndex].cellsUsed <= 0)
		{

			// printf("died no mass\n");
			execute = true;
		}
	}
	if (execute)
	{
		killAnimal( animalIndex);

		if (animalIndex == game.selectedAnimal)
		{
			game.selectedAnimal = -1;
		}
	}

	if (speciesIndex > 0)
	{
		if (game.adversary >= 0 && game.adversary < numberOfAnimals)
		{
			// bool nominate = false;
			float animalScore = 0.0f;
			animalScore = animals[animalIndex].damageDone + animals[animalIndex].damageReceived  + (animals[animalIndex].numberOfTimesReproduced ) ;
			float distance = abs(animals[animalIndex].fPosX - animals[game.adversary].fPosX) + abs(animals[animalIndex].fPosY - animals[game.adversary].fPosY) ;
			if (distance > 100.0f) { distance = 100.0f;}
			distance  *= 0.5f;
			animalScore += distance ;
			if ( animalScore > game.championScores[speciesIndex])
			{
				game.championScores[speciesIndex] = animalScore;
				game.champions[speciesIndex] = animals[animalIndex];
			}
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
		if (!animals[animalIndex].retired && speciesIndex < numberOfSpecies)
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


void drawNeuroConnections( int animalIndex,  int animalCell, int vx, int vy)
{

	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		if (animals[game.selectedAnimal].body[animalCell].connections[i].used)
		{

			Vec_f2 start = Vec_f2(vx, vy);
			Vec_f2 end    = Vec_f2(vx, vy);

			int connected_to_cell = animals[game.selectedAnimal].body[animalCell].connections[i].connectedTo;
			if (connected_to_cell >= 0 && connected_to_cell < animals[game.selectedAnimal].cellsUsed)
			{
				unsigned int connectedPos = animals[game.selectedAnimal].body[connected_to_cell].worldPositionI;
				int connectedX = connectedPos % worldSize;
				int connectedY = connectedPos / worldSize;

				unsigned int x = animals[game.selectedAnimal].body[connected_to_cell].worldPositionI % worldSize;
				unsigned int y = animals[game.selectedAnimal].body[connected_to_cell].worldPositionI / worldSize;

				end.x -= (x - connectedX);
				end.y -= (y - connectedY);


				Color signalColor = color_white;
				float brightness = animals[game.selectedAnimal].body[connected_to_cell].signalIntensity * animals[game.selectedAnimal].body[animalCell].connections[i].weight ;
				signalColor = multiplyColorByScalar(signalColor, brightness);
				drawLine(  start, end, 0.1f, signalColor );
			}
		}
	}


	//draw eyelooks
	if (animals[animalIndex].body[animalCell].organ == ORGAN_SENSOR_EYE)
	{
		Vec_f2 eyeLook = Vec_f2(animals[animalIndex].body[animalCell].eyeLookX , animals[animalIndex].body[animalCell].eyeLookY);
		Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), animals[animalIndex].fAngleSin, animals[animalIndex].fAngleCos, eyeLook);
		Vec_f2 eyelookCameraPosition = Vec_f2( vx + rotatedEyeLook.x, vy + rotatedEyeLook.y );
		drawLine(  Vec_f2(vx, vy), eyelookCameraPosition, 0.1f, color_white );
		drawTile(eyelookCameraPosition , color_white);
	}
}

void camera()
{
	ZoneScoped;
	if (flagSave) { return;}

	if (game.cameraTargetCreature >= 0)
	{
		game.cameraPositionX = animals[game.cameraTargetCreature].position % worldSize;
		game.cameraPositionY = animals[game.cameraTargetCreature].position / worldSize;
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




					if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals )
					{



						bool squareIsSelectedAnimal = false;


						if (world[worldI].identity >= 0 && world[worldI].identity < numberOfAnimals)
						{
							if (world[worldI].identity == game.selectedAnimal)
							{
								int animalCell = isAnimalInSquare(world[worldI].identity, worldI );
								if (animalCell >= 0 && animalCell < animals[game.selectedAnimal].cellsUsed)
								{
									drawNeuroConnections(game.selectedAnimal, animalCell, vx, vy);
								}
								squareIsSelectedAnimal = true;
							}
						}
						if (!squareIsSelectedAnimal)
						{
							displayColor = filterColor(displayColor, tint_shadow);
						}



					}

					else if (game.selectedPlant >= 0  )
					{


						bool squareIsSelectedPlant = false;


						if (world[worldI].plantIdentity >= 0 )
						{
							if (world[worldI].plantIdentity == game.selectedPlant)
							{
								squareIsSelectedPlant = true;
							}
						}
						if (!squareIsSelectedPlant)
						{
							displayColor = filterColor(displayColor, tint_shadow);
						}

					}









					drawTile( Vec_f2( fx, fy ), displayColor);
				}
			}
		}
	}

	Color displayColor = color_white; // draw the mouse cursor.


	Vec_f2 worldMousePos = Vec_f2( game.mousePositionX, game.mousePositionY);


	Vec_f2 worldMousePosA = Vec_f2( game.mousePositionX + 1, game.mousePositionY);
	Vec_f2 worldMousePosB = Vec_f2( game.mousePositionX - 1, game.mousePositionY);
	Vec_f2 worldMousePosC = Vec_f2( game.mousePositionX, game.mousePositionY + 1);
	Vec_f2 worldMousePosD = Vec_f2( game.mousePositionX, game.mousePositionY - 1);

	drawTile( worldMousePosA, displayColor);
	drawTile( worldMousePosB, displayColor);
	drawTile( worldMousePosC, displayColor);
	drawTile( worldMousePosD, displayColor);

}

void displayComputerText()
{
	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;

	// bool displayedAComputer = false;


	if (game.palette)
	{
		menuY += spacing;
		drawPalette(menuX, menuY);
		// displayedAComputer = true;
	}


	else if (game.ecologyComputerDisplay)
	{
		// displayedAComputer = true;

		// draw the food web.
		int originalMenuX = menuX;
		for (int i = 0; i < numberOfSpecies; ++i)
		{
			printText2D( "species " + std::to_string(i) + " eats: " , menuX, menuY, textSize);

			for (int j = 0; j < numberOfSpecies; ++j)
			{
				printText2D(  std::to_string(foodWeb[i][j]) , menuX, menuY, textSize);
				menuX += spacing * 5;

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
				printText2D( selectString +  std::string("Energy in 1 square of meat: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 1:
			{
				printText2D( selectString +  std::string("Energy in 1 square of grass: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 2:
			{
				printText2D( selectString +  std::string("Energy required to move 1 square: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 3:
			{
				printText2D( selectString +  std::string("Energy required for each animal square each turn: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 4:
			{
				printText2D( selectString +  std::string("Number of map squares to update each turn: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 5:
			{
				printText2D( selectString +  std::string("Nutrition each plant can extract each turn: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			case 6:
			{
				printText2D( selectString +  std::string("Energy each plant requires each turn: ") + std::to_string(game.ecoSettings[j]) , menuX, menuY, textSize);
				break;
			}
			}
			menuY -= spacing;
		}
	}

	else if (game.computerdisplays[0])
	{

		// displayedAComputer = true;
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

		// displayedAComputer = true;
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

		// displayedAComputer = true;
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

		// displayedAComputer = true;
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

	else
	{

		if (  printLogs)
		{
			menuY += spacing;
			for (int i = 0; i < 8; ++i)
			{
				printText2D(   game.logs[i] , menuX, menuY, textSize);
				menuY += spacing;
			}
			menuY += spacing;
		}
	}


	menuY -= spacing;



	// return displayedAComputer;
}

void incrementSelectedGrabber()
{

	if (animals[game.playerCreature].cellsUsed <= 0) { return;}
	for (unsigned int i = 0; i < animals[game.playerCreature].cellsUsed ; ++i)
	{
		if (animals[game.playerCreature].cellsUsed > 0)
		{
			unsigned int neighbour = (game.playerActiveGrabber + i) % animals[game.playerCreature].cellsUsed;
			if (animals[game.playerCreature].body[neighbour].organ == ORGAN_GRABBER && animals[game.playerCreature].body[neighbour].grabbedCreature >= 0
			        && neighbour != game.playerActiveGrabber
			   )
			{
				game.playerActiveGrabber = neighbour;
				break;
			}
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
		printText2D(   std::string("Start by finding items in the world and picking them up.") , menuX, menuY, textSize);
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
	for (int i = 0; i < animals[game.playerCreature].cellsUsed; ++i)
	{
		if (animals[game.playerCreature].body[i].organ == ORGAN_GRABBER)
		{
			if (animals[game.playerCreature].body[i].grabbedCreature >= 0)
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
			stringToPrint += std::string("Holding ") + animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].displayName + std::string(" [f] drop ");

			if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].isMachine)
			{

				if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_HOSPITAL)
				{
					stringToPrint += std::string("[lmb, rmb] add, erase [y, h] next, prev");

					healAnimal(game.playerCreature);
					game.palette = true;
				}

				if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_ECOLOGYCOMPUTER)
				{
					stringToPrint += std::string("[lmb, rmb] +, - [y, h] next, last");
					game.ecologyComputerDisplay = true;
				}

				if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER1 ||
				        animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER2 ||
				        animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER3 ||
				        animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER4 ||
				        animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_MESSAGECOMPUTER5

				   )
				{
					stringToPrint += std::string("[lmb] read messages");
				}

				if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_LIGHTER)
				{
					stringToPrint += std::string("[lmb] start fire");
				}

				if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_KNIFE)
				{
					stringToPrint += std::string("[lmb] cut");

					if (game.playerLMBDown)
					{
						knifeCallback(animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature , game.playerCreature  );
					}
				}

				if (animals[  animals[game.playerCreature].body[game.playerActiveGrabber].grabbedCreature ].machineCallback == MACHINECALLBACK_PISTOL)
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
		printText2D(   std::string("[g] pick up ") + std::string(animals[game.playerCanPickupItem].displayName) , menuX, menuY, textSize);
		menuY += spacing;
	}



	if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals)
	{
		int selectedAnimalSpecies = game.selectedAnimal / numberOfAnimalsPerSpecies;
		printText2D(   std::string("Selected an animal of species ") + std::to_string(selectedAnimalSpecies ) +
		               std::string(". Energy ") + std::to_string(animals[game.cursorAnimal].energy ) +
		               std::string(", size ") + std::to_string(animals[game.cursorAnimal].maxEnergy ) +
		               std::string(", debt ") + std::to_string(animals[game.cursorAnimal].energyDebt ) +
		               std::string(", age ") + std::to_string(animals[game.cursorAnimal].age ) +
		               std::string(", gen. ") + std::to_string(animals[game.cursorAnimal].generation )


		               , menuX, menuY, textSize);
		menuY += spacing;




	}
	else if (game.selectedPlant >= 0)
	{


		std::string cursorDescription = std::string(".");

		if (worldCursorPos < worldSquareSize)
		{
			if (world[worldCursorPos].plantIdentity == game.selectedPlant)
			{

				cursorDescription += 	std::string(" This is ") + tileDescriptions(world[worldCursorPos].plantState );
				cursorDescription += std::string(". Energy ") + std::to_string(world[worldCursorPos].energy)
				                     + std::string(", noots ") + std::to_string(world[worldCursorPos].nutrients)
				                     + std::string(", genecursor ") + std::to_string(world[worldCursorPos].geneCursor)
				                     + std::string(", grown ") + std::to_string(world[worldCursorPos].grown)


				                     + std::string(", growthMatrix: ")

				                     ;
			}


			for (int i = 0; i < nNeighbours; ++i)
			{
				cursorDescription += std::to_string(world[worldCursorPos].growthMatrix[i]);
			}


		}

		printText2D(   std::string("Selected plant ") + std::to_string(game.selectedPlant) + cursorDescription  , menuX, menuY, textSize);
		menuY += spacing;


		std::string geneString = std::string("Plant genes: ");

		for (int i = 0; i < plantGenomeSize; ++i)
		{
			geneString += std::to_string(world[worldCursorPos].plantGenes[i]) + " ";
		}

		printText2D( geneString  , menuX, menuY, textSize);
		menuY += spacing;

		// std::string
		geneString = std::string("Seed genes: ");

		for (int i = 0; i < plantGenomeSize; ++i)
		{
			geneString += std::to_string(world[worldCursorPos].seedGenes[i]) + " ";
		}

		printText2D( geneString  , menuX, menuY, textSize);
		menuY += spacing;



	}

	// print what is at the cursor position.
	if (worldCursorPos < worldSquareSize)
	{
		int heightInt = world[worldCursorPos].height;
		menuY += spacing;


		std::string cursorDescription = std::string("");


		bool animalInSquare = false;
		if (world[worldCursorPos].identity >= 0 && world[worldCursorPos].identity < numberOfAnimals)
		{
			int occupyingCell = isAnimalInSquare(world[worldCursorPos].identity , worldCursorPos);
			if ( occupyingCell >= 0)
			{
				animalInSquare = true;
				unsigned int cursorAnimalSpecies = world[worldCursorPos].identity  / numberOfAnimalsPerSpecies;
				game.cursorAnimal = world[worldCursorPos].identity;
				if (cursorAnimalSpecies == 0)
				{
					if (game.cursorAnimal == game.playerCreature)
					{

						cursorDescription += std::string("It's you. ");
					}
					else
					{
						cursorDescription += std::string("A ") +  std::string(animals[game.cursorAnimal].displayName) + std::string(". ");
					}
				}
				else
				{
					cursorDescription += std::string("An animal of species ") + std::to_string(cursorAnimalSpecies ) + std::string(". ");

				}
				cursorDescription +=  std::string(" This is ") + tileDescriptions(  animals[  game.cursorAnimal].body[occupyingCell].organ );
			}
		}

		if (!animalInSquare)
		{
			if (world[worldCursorPos].seedState != MATERIAL_NOTHING)
			{
				cursorDescription += 	std::string(" This is ") + tileDescriptions(world[worldCursorPos].seedState );
			}

			else if (world[worldCursorPos].plantState != MATERIAL_NOTHING)
			{
				cursorDescription += 	std::string(" This is ") + tileDescriptions(world[worldCursorPos].plantState );
			}
			else if (world[worldCursorPos].wall != MATERIAL_NOTHING)
			{
				cursorDescription += std::string(" This is ") + tileDescriptions(world[worldCursorPos].wall);
			}
			cursorDescription += std::string(" Below is ") +  tileDescriptions(world[worldCursorPos].terrain);
		}

		printText2D(  cursorDescription, menuX, menuY, textSize);
		menuY += spacing;
	}
	if (game.playerCreature >= 0)
	{
		int playerPheromoneSensor = getRandomCellOfType( game.playerCreature, ORGAN_SENSOR_PHEROMONE ) ;// if the player has a nose, print what it smells like here.
		if (playerPheromoneSensor >= 0)
		{
			unsigned int playerPheromoneSensorWorldPos = animals[game.playerCreature].body[playerPheromoneSensor].worldPositionI;
			if (world[playerPheromoneSensorWorldPos].pheromoneChannel >= 0 &&  world[playerPheromoneSensorWorldPos].pheromoneChannel < numberOfSpeakerChannels)
			{
				printText2D(   pheromoneDescriptions( world[playerPheromoneSensorWorldPos].pheromoneChannel ) , menuX, menuY, textSize);
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
			if (animals[game.playerCreature].body[playerGill].signalIntensity < 0.0f)
			{
				printText2D(   std::string("You have no oxygen left.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (animals[game.playerCreature].body[playerGill].signalIntensity < baseLungCapacity / 2)
			{
				printText2D(   std::string("You're half out of oxygen.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}

		playerGill = getRandomCellOfType( game.playerCreature, ORGAN_SENSOR_PAIN ) ;
		if (playerGill >= 0)
		{
			if (animals[game.playerCreature].damageReceived > (animals[game.playerCreature].cellsUsed) * 0.25 &&
			        animals[game.playerCreature].damageReceived < (animals[game.playerCreature].cellsUsed) * 0.375
			   )
			{
				printText2D(   std::string("You're badly hurt.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (animals[game.playerCreature].damageReceived > (animals[game.playerCreature].cellsUsed) * 0.375)
			{
				printText2D(   std::string("You are mortally wounded.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
	}


	// bool pute =
	displayComputerText();



}

void paintCreatureFromCharArray( int animalIndex,  char * start, unsigned int len, unsigned int width )
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
		for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
		{
			if (animals[animalIndex].body[i].localPosX == p.x && animals[animalIndex].body[i].localPosY == p.y)
			{
				animals[animalIndex].body[i].color = newColor;
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
	printf( "%s\n",   animals[animalIndex].displayName  );

	for (int i = 0; i < animalSquareSize; ++i)
	{
		printf( "%s\n",   tileShortNames(animals[animalIndex].body[i].organ).c_str()  );
	}
}

void setupCreatureFromCharArray( int animalIndex, char * start, unsigned int len, unsigned int width, std::string newName, int newMachineCallback )
{
	resetAnimal(animalIndex);
	animals[animalIndex].generation = 0;
	strcpy( &animals[animalIndex].displayName[0] , newName.c_str() );
	if (newMachineCallback >= 0)
	{
		animals[animalIndex].isMachine = true;
		animals[animalIndex].machineCallback = newMachineCallback;
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
			if (animals[animalIndex].cellsUsed >= (animalSquareSize - 1)) { break;}
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
	for (int i = 0; i < animals[animalIndex].cellsUsed; ++i)
	{
		animals[animalIndex].body[i].localPosX -= width / 2;
		animals[animalIndex].body[i].localPosY -= p.y / 2;
	}
}

void spawnAdversary(unsigned int targetWorldPositionI)
{
	game.adversary = numberOfAnimalsPerSpecies + 1; // game.adversary animal is a low number index in the 1th species. 0th is for players and machines.
	spawnAnimalIntoSlot(game.adversary, game.champions[1], targetWorldPositionI, true);
	animals[game.adversary].position = targetWorldPositionI;
	animals[game.adversary].uPosX = targetWorldPositionI % worldSize;
	animals[game.adversary].uPosY = targetWorldPositionI / worldSize;
	animals[game.adversary].fPosX = animals[game.adversary].uPosX;
	animals[game.adversary].fPosY = animals[game.adversary].uPosY;
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
	spawnAnimalIntoSlot(game.playerCreature, animals[i], targetWorldPositionI, false);
	game.cameraTargetCreature = game.playerCreature;
	appendLog( std::string("Spawned the player.") );

	for (int animalIndex = 0; animalIndex < 13; ++animalIndex)
	{
		animals[animalIndex].retired = false;
		animals[animalIndex].damageReceived = 0;


		animals[animalIndex].position = animals[animalIndex].birthLocation;

		unsigned int bex = animals[animalIndex].birthLocation % worldSize;
		unsigned int bey = animals[animalIndex].birthLocation / worldSize;

		animals[animalIndex].uPosX = bex;
		animals[animalIndex].uPosY = bey;
		animals[animalIndex].fPosX = bex;
		animals[animalIndex].fPosY = bey;

		for (int k = 0; k < animals[animalIndex].cellsUsed; ++k)
		{
			animals[animalIndex].body[k].damage = 0.0f;
		}
	}

	appendLog( std::string("Restored game items.") );
}

void adjustPlayerPos(Vec_f2 pos)
{
	if (game.playerCreature >= 0)
	{
		animals[game.playerCreature].fAngle = 0.0f;
		int strafeMuscle = getRandomCellOfType(game.playerCreature, ORGAN_MUSCLE_STRAFE);
		int muscle = getRandomCellOfType(game.playerCreature, ORGAN_MUSCLE);
		if (strafeMuscle >= 0)
		{
			animals[game.playerCreature].body[strafeMuscle].signalIntensity = pos.y;
		}
		if (muscle >= 0)
		{
			animals[game.playerCreature].body[muscle].signalIntensity = pos.x;
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
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{


		std::ofstream out7( filename .c_str());
		out7.write( (char*)(&animals[game.selectedAnimal]), sizeof(Animal));
		out7.close();
	}
}

void loadParticlarAnimal(int animalIndex, std::string filename)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{

		std::ifstream in7(filename.c_str());
		in7.read( (char*)(&animals[game.selectedAnimal]), sizeof(Animal));
		in7.close();
	}
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
		if (world[worldPositionI].height > maxHeight)
		{
			maxHeight = world[worldPositionI].height;
		}
		if (world[worldPositionI].height < minHeight)
		{
			minHeight = world[worldPositionI].height;
		}
	}
	float heightRange =  maxHeight - minHeight ;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		world [ worldPositionI] .height =  ((world [ worldPositionI] .height - minHeight) / (  heightRange )  ) * (worldSize);
	}
	float postMaxHeight = 0.0f;
	float postMinHeight = 0.0f;
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (world[worldPositionI].height > postMaxHeight)
		{
			postMaxHeight = world[worldPositionI].height;
		}
		if (world[worldPositionI].height < postMinHeight)
		{
			postMinHeight = world[worldPositionI].height;
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
		world[worldPositionI].height = prelimMap[prelimSampleIndex] ;
	}
}

void recomputeTerrainLighting()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize - 1; worldPositionI++)
	{
		computeLight( worldPositionI, sunXangle, sunYangle);
		if (world[worldPositionI].height < seaLevel)
		{
			float depth = (seaLevel - world[worldPositionI].height);
			float brightness = (1 / (1 + (depth / (worldSize / 8))) );
			if (brightness < 0.2f) { brightness = 0.2f;}

			world[worldPositionI].light = multiplyColorByScalar(world[worldPositionI].light, brightness   );
		}
		float steps = 8;
		float b = world[worldPositionI].light.a * steps;//100.0f;
		int ib = b;
		float betoot =  (ib / steps);
		world[worldPositionI].light.a = betoot;
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
				if (world[randomI].height > seaLevel)
				{
					continue;
				}
			}
			else
			{
				if (world[randomI].height < biome_coastal)
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
					if (world[scan].wall == MATERIAL_NOTHING) { hasAir = true; }
					if (world[scan].wall == MATERIAL_WATER) { hasWater = true; }
					if (world[scan].wall == MATERIAL_VOIDMETAL) { unsuitable = true;}

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
			avgHeight += world[i].height;
			tally++;
			world[i].terrain = MATERIAL_VOIDMETAL;
			if (  !(world[i].wall == MATERIAL_NOTHING || world[i].wall == MATERIAL_WATER) )
			{
				world[i].wall = MATERIAL_NOTHING;
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
			world[i].wall = MATERIAL_VOIDMETAL;
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
			world[i].height = avgHeight;
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
	spawnAnimalIntoSlot(2, animals[i], building1, false);

	building1 += 25;
	setupMessageComputer( i, 0);
	spawnAnimalIntoSlot(3, animals[i], building1, false);

	building1 += 25 * worldSize;

	game.playerRespawnPos = building1; // game.adversaryRespawnPos;
	spawnPlayer();


	// BUILDING 2
	// contains hospital and message computer 2
	int building2 =  getRandomPosition(false);
	setupBuilding_playerBase(building2);
	setupHospitalComputer(i);
	spawnAnimalIntoSlot(4, animals[i], building2, false);

	building2 += 25 * worldSize;
	setupMessageComputer( i, 1);
	spawnAnimalIntoSlot(5, animals[i], building2, false);


	// BUILDING 3
	//contains tracker glasses, pistol, and message computer 3
	int building3 =  getRandomPosition(true);
	setupBuilding_playerBase(building3);
	setupTrackerGlasses(i);
	spawnAnimalIntoSlot(6, animals[i], building3, false);

	building3 += 25;
	setupExampleGun(i);
	spawnAnimalIntoSlot(7, animals[i], building3, false);

	building3 += 25 * worldSize;
	setupMessageComputer( i, 2);
	spawnAnimalIntoSlot(8, animals[i], building3, false);


	// adversary is outside, under water
	game.adversaryRespawnPos =  getRandomPosition(true);
	spawnAdversary(game.adversaryRespawnPos);


	// BUILDING 4
	// contains knife, lighter, and message computer 4
	int building4 =  getRandomPosition(true);
	setupBuilding_playerBase(building4);
	setupExampleKnife(i);
	spawnAnimalIntoSlot(9, animals[i], building4, false);

	building4 += 25;
	setupExampleLighter(i);
	spawnAnimalIntoSlot(10, animals[i], building4, false);

	building4 += 25 * worldSize;
	setupMessageComputer( i, 3);
	spawnAnimalIntoSlot(11, animals[i], building4, false);

	building4 -= 25;
	setupDestroyer( i);
	spawnAnimalIntoSlot(12, animals[i], building4, false);
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


const int emergencyPopulationLimit = 10;

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
			if (animals[game.adversary].retired)
			{
				spawnAdversary(game.adversaryRespawnPos);
			}
			else
			{
				if (animals[game.adversary].position >= 0 && animals[game.adversary].position < worldSquareSize)
				{
					for (int i = 0; i < nNeighbours; ++i)
					{
						unsigned int neighbour = animals[game.adversary].position + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							if (materialSupportsGrowth(world[animals[game.adversary].position].terrain ))
							{
								if (extremelyFastNumberFromZeroTo(100) == 0)
								{
									spawnRandomPlant( animals[game.adversary].position  );
								}

							}
						}
					}


					game.adversaryRespawnPos = animals[game.adversary].position;
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
					if (animals[game.adversary].position != adversaryLoiterPos)
					{
						adversaryLoiterPos = animals[game.adversary].position ;
						adversaryLoiter = 0;
					}
					else
					{
						adversaryLoiter++;
					}
					if (adversaryLoiter > 1000)
					{
						killAnimal(game.adversary);
						animals[game.adversary].retired = true;
					}
				}

				if (respawnLowSpecies)
				{
					census();


					int totalPopulation = 0;

					for (int k = 1; k < numberOfSpecies; ++k)// spawn lots of the example animal
					{
						totalPopulation  += game.speciesPopulationCounts[k];
					}






					if (totalPopulation < emergencyPopulationLimit)
					{


						// the entire ecosystem has crashed, respawn some champions and example animals.


						for (int k = 1; k < numberOfSpecies; ++k)// spawn lots of the example animal
						{



							if (game.speciesPopulationCounts[k] < 1)
							{





								int j = 1;
								int domingo = -1;
								unsigned int randomPos = animals[game.adversary].position + (-5 + extremelyFastNumberFromZeroTo(10)  + ( (-5 * worldSize) + (extremelyFastNumberFromZeroTo(10) * worldSize)  )   );

								int whatToSpawn = extremelyFastNumberFromZeroTo(1);

								if (whatToSpawn == 0)  // spawn example animals
								{
									setupExampleAnimal3(j);
									domingo = spawnAnimal( k,  animals[j], randomPos, true);
									animals[domingo].energy = animals[domingo].maxEnergy / 2.0f;
								}
								else if (whatToSpawn == 1)  // spawn a species champion
								{
									domingo = spawnAnimal( k,  game.champions[k], randomPos, true);

								}
								// else if (whatToSpawn == 2)  // spawn an individual from another successful species (horizontal gene transfer)
								// {
								// 	// get the most populous species.
								// 	unsigned int mostPopulousSpecies = 1;
								// 	for (int i = 1; i < numberOfSpecies; ++i)
								// 	{
								// 		if (game.speciesPopulationCounts[i] > mostPopulousSpecies)
								// 		{
								// 			mostPopulousSpecies = i;
								// 		}
								// 	}
								// 	int  bromelich = getRandomCreature(mostPopulousSpecies);
								// 	if (bromelich >= 0 )
								// 	{
								// 		domingo = spawnAnimal( k,  animals[bromelich], animals[bromelich].position, true);
								// 	}
								// }
								if (domingo >= 0)
								{
									animals[domingo].fPosX += ((RNG() - 0.5) * 10.0f);
									animals[domingo].fPosY += ((RNG() - 0.5) * 10.0f);
									animals[domingo].fAngle += ((RNG() - 0.5) );
									animals[domingo].fAngleCos = cos(animals[domingo].fAngle);
									animals[domingo].fAngleSin = sin(animals[domingo].fAngle);
									animals[domingo].energy = animals[domingo].maxEnergy / 2.0f;
									animals[domingo].parentIdentity = game.adversary;

									if (world[animals[game.adversary].position].wall == MATERIAL_WATER)
									{
										bool hasGill = false;
										for (int i = 0; i < animals[domingo].cellsUsed; ++i)
										{
											if (animals[domingo].body[i].organ == ORGAN_GILL)
											{
												hasGill = true;
												break;
											}
										}
										if (!hasGill)
										{
											animalAppendCell(domingo, ORGAN_GILL);
										}
									}
									else
									{
										bool hasLung = false;
										for (int i = 0; i < animals[domingo].cellsUsed; ++i)
										{
											if (animals[domingo].body[i].organ == ORGAN_LUNG)
											{
												hasLung = true;
												break;
											}
										}
										if (!hasLung)
										{
											animalAppendCell(domingo, ORGAN_LUNG);
										}
									}
								}
								// game.championScores[k] = 0.0f;
							}
						}
					}

					else

					{


						for (int k = 1; k < numberOfSpecies; ++k)// spawn lots of the example animal
						{



							if (game.speciesPopulationCounts[k] < emergencyPopulationLimit)
							{
								// printf("LO SPECIES!\n");

// get the most populous species.
								int mostPopulousSpecies = 1;
								unsigned int mostPopulousCount = 0;
								for (int i = 1; i < numberOfSpecies; ++i)
								{
									if (game.speciesPopulationCounts[i] > mostPopulousCount)
									{
										mostPopulousSpecies = i;
										mostPopulousCount = game.speciesPopulationCounts[i];
									}
								}

								if (mostPopulousCount > emergencyPopulationLimit * 2) {

									// for (int i = 0; i < emergencyPopulationLimit; ++i)
									// {
									/* code */


									int  bromelich = getRandomCreature(mostPopulousSpecies);
									if (bromelich >= 0 )
									{
										// printf("LO SPECIES RESPEENDED!\n");
										int domingo = spawnAnimal( k,  animals[bromelich], animals[bromelich].position, false);

										// int newCreature = (k * numberOfAnimalsPerSpecies) + i;// extremelyFastNumberFromZeroTo((numberOfAnimalsPerSpecies - 1));
										// animals[newCreature] = animals[bromelich];
										// animals[bromelich].retired = true;
										if (domingo >= 0)
										{

											animals[bromelich].retired = true;
										}

									}
									// }
								}

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
	int j = i;
	ZoneScoped;
	if (j >= 0 && j < numberOfAnimals)
	{
		if (! (animals[j].retired))
		{
			place(j);
			animal_organs( j);
			animalEnergy( j);
		}
	}
}

void model_sector( int from,  int to)
{
	ZoneScoped;
	// includes 'from' but ends just before 'to'
	for ( int i = from; i < to; ++i)
	{
		animalTurn(i);
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
	for (int i = 0; i < numberOfSectors; ++i)
	{
		int from = (i ) * numberOfAnimalsPerSector ;
		int to = (i + 1) * numberOfAnimalsPerSector;
		sectors[i] =	boost::thread { model_sector ,   from  , to    };
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
		printText2D(   std::string("loading animals from file "), menuX, menuY, textSize);
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


	std::ofstream out7(std::string("save/world").c_str());
	out7.write( (char*)(&world), sizeof(Square) * worldSquareSize );
	out7.close();

	std::ofstream out8(std::string("save/animals").c_str());
	out8.write( (char*)(&animals), sizeof(Animal) * numberOfAnimals );
	out8.close();



	worldCreationStage = 10;
	flagSave = false;
}

void load()
{
	worldCreationStage = 12;


	std::ifstream in6(std::string("save/game").c_str());
	in6.read( (char *)(&game), sizeof(GameState));
	in6.close();

	std::ifstream in7(std::string("save/world").c_str());
	in7.read( (char *)(&world), sizeof(Square) * worldSquareSize );
	in7.close();


	std::ifstream in8(std::string("save/animals").c_str());
	in8.read( (char *)(&animals), sizeof(Animal) * numberOfAnimals );
	in8.close();


	worldCreationStage = 10;
	setFlagReady();
}



// this test plant makes a beautiful pattern of blue, and tests nested branches and sequences.
void setupTestPlant2(unsigned int worldPositionI)
{
	memset(world[worldPositionI].seedGenes, 0x00, sizeof(char) * plantGenomeSize);

	world[worldPositionI].seedGenes[0] = PLANTGENE_BLUE;
	world[worldPositionI].seedGenes[1] = 2;
	world[worldPositionI].seedGenes[2] = PLANTGENE_WOOD;
	world[worldPositionI].seedGenes[3] = 0;
	world[worldPositionI].seedGenes[4] = PLANTGENE_GROW_SYMM_H;
	world[worldPositionI].seedGenes[5] = PLANTGENE_BRANCH;
	world[worldPositionI].seedGenes[6] = 2;
	world[worldPositionI].seedGenes[7] = PLANTGENE_LEAF;
	world[worldPositionI].seedGenes[8] = PLANTGENE_SEQUENCE;
	world[worldPositionI].seedGenes[9] = 5;
	world[worldPositionI].seedGenes[10] = PLANTGENE_WOOD;
	world[worldPositionI].seedGenes[11] = PLANTGENE_BLUE;
	world[worldPositionI].seedGenes[12] = PLANTGENE_WOOD;
	world[worldPositionI].seedGenes[13] = 0;
	world[worldPositionI].seedGenes[14] = PLANTGENE_BRANCH;
	world[worldPositionI].seedGenes[15] = PLANTGENE_LEAF;
	world[worldPositionI].seedGenes[16] = PLANTGENE_BREAK;
	world[worldPositionI].seedGenes[17] = 2;
	world[worldPositionI].seedGenes[18] = PLANTGENE_BREAK;
	world[worldPositionI].seedGenes[19] = PLANTGENE_BREAK;
	world[worldPositionI].seedIdentity = extremelyFastNumberFromZeroTo(256);
	world[worldPositionI].seedState = MATERIAL_SEED;
}


void simpleTestPlant(unsigned int worldPositionI)
{
	world[worldPositionI].seedGenes[0] = PLANTGENE_BLUE;
	world[worldPositionI].seedGenes[1] = PLANTGENE_BLUE;
	world[worldPositionI].seedGenes[2] = PLANTGENE_BLUE;
	world[worldPositionI].seedGenes[3] = 2;
	world[worldPositionI].seedGenes[4] = PLANTGENE_LEAF;
}

void setupTestPlant(unsigned int worldPositionI)
{
	memset(world[worldPositionI].seedGenes, 0x00, sizeof(char) * plantGenomeSize);
	// the test plant should demonstrate the basic structural features and reproductive capability of the plant.
	// a short preamble makes the plant a red color.
	world[worldPositionI].seedGenes[0] = PLANTGENE_RED;
	world[worldPositionI].seedGenes[1] = PLANTGENE_RED;
	world[worldPositionI].seedGenes[2] = PLANTGENE_RED;
	world[worldPositionI].seedGenes[3] = PLANTGENE_SEQUENCE;
	world[worldPositionI].seedGenes[4] = 2;
	world[worldPositionI].seedGenes[4] = 4;
	world[worldPositionI].seedGenes[5] = PLANTGENE_WOOD;
	world[worldPositionI].seedGenes[6] = 4;
	world[worldPositionI].seedGenes[7] = PLANTGENE_BRANCH;
	world[worldPositionI].seedGenes[8] = 3;
	world[worldPositionI].seedGenes[8] = 2;
	world[worldPositionI].seedGenes[9] = PLANTGENE_LEAF;
	world[worldPositionI].seedGenes[11] = 4;
	world[worldPositionI].seedGenes[12] = PLANTGENE_WOOD;
	world[worldPositionI].seedGenes[13] = 4;
	world[worldPositionI].seedGenes[14] = PLANTGENE_BRANCH;
	world[worldPositionI].seedGenes[15] = 3;
	world[worldPositionI].seedGenes[16] = 3;
	world[worldPositionI].seedGenes[17] = PLANTGENE_BUD_A;
	world[worldPositionI].seedGenes[18] = PLANTGENE_BREAK;
	world[worldPositionI].seedGenes[19] = PLANTGENE_BREAK;
	world[worldPositionI].seedGenes[20] = PLANTGENE_BREAK;
	world[worldPositionI].seedIdentity = extremelyFastNumberFromZeroTo(256);
	world[worldPositionI].seedState = MATERIAL_SEED;
}

void setupPlantAtCursor()
{
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	setupTestPlant3(worldCursorPos);
}

bool test_animals()
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
	int testAnimal = spawnAnimal(  testSpecies, animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].fAngle = 0.0f;
	place(testAnimal); // apply the changes just made.
	animals[testAnimal].energy = animals[testAnimal].energy = 1.0f;
	const unsigned int howManyPlantsToEat = 10;
	float initialEnergy = animals[testAnimal].energy;
	for (int i = 0; i < howManyPlantsToEat; ++i)
	{
		setupTestPlant(testPos + (i * worldSize));
	}
	for (int i = 0; i < howManyPlantsToEat; ++i)
	{
		animalTurn(testAnimal);
	}

	if (animals[testAnimal].energy != initialEnergy)
	{
		testResult_2 = true;
	}
	killAnimal(testAnimal);

	// 3. animals reproduce when they have enough energy and their debt is 0
	const int how_long_it_takes_to_make_sure = (baseLungCapacity / aBreath) * 1;
	setupTestAnimal_reproducer(j);
	testAnimal = spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = animals[testAnimal].maxEnergy;
	animals[testAnimal].energyDebt = 0.0f;
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


	// 4. reproduction copies the animal wholly and exactly,
	// except that lifetime stats are reset to 0 in the new generation, cell damage is healed
	// and some mutation may be carried along
	setupTestAnimal_reproducer(j);
	testAnimal =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = animals[testAnimal].maxEnergy / 2;
	animals[testAnimal].energyDebt = 0.0f;
	animals[testAnimal].body[0].damage = 0.05f;
	animalTurn(testAnimal);

	int diffs = 0;
	int child = testAnimal;
	for (int i = 0; i < numberOfAnimalsPerSpecies; ++i)
	{
		child = (testSpecies * numberOfAnimalsPerSpecies) + i;
		if ( !animals[child].retired  )
		{
			if (animals[child].parentIdentity == testAnimal  )
			{
				Cell * childBody = animals[child].body;
				Cell * parentBody = animals[testAnimal].body;
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

	if (diffs == 0 && animals[child].body[0].damage == 0.0f)
	{
		testResult_4 = true;
	}
	killAnimal(testAnimal);

	// 5, the animal can sense stimulus from the environment, it can propagate through the brain and trigger an actuator.
	// note that this test uses a different test animal.
	setupTestAnimal_eye(j);
	testAnimal =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = animals[testAnimal].maxEnergy;
	animals[testAnimal].energyDebt = 0.0f;
	animals[testAnimal].energy = (animals[testAnimal].maxEnergy / 2) - 1.0f; // give it enough energy for the test

	place(testAnimal);

	int testEye = getRandomCellOfType(testAnimal, ORGAN_SENSOR_EYE);
	float originalAngle = animals[testAnimal].fAngle;
	unsigned int testEyePosition ;
	if (testEye >= 0)
	{
		testEyePosition = animals[testAnimal].body[testEye].worldPositionI;
		animals[testAnimal].body[testEye].color = color_white;
		world[testEyePosition].light = color_white;
	}

	// it takes a few turns for the neural signal to propagate through the network.
	animalTurn(testAnimal);
	animalTurn(testAnimal);
	animalTurn(testAnimal);
	animalTurn(testAnimal);
	animalTurn(testAnimal);

	if (animals[testAnimal].fAngle != originalAngle)
	{
		testResult_5 = true;
	}
	killAnimal(testAnimal);

// 6. lungs
	setupTestAnimal_airbreathing(j);
	testPos += 10;
	int testAnimal_air_in_air =	spawnAnimal( testSpecies , animals[j], testPos, false);
	float amount =  (animals[testAnimal].maxEnergy / 2.0f);

	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = amount;

	setupTestAnimal_airbreathing(j);
	testPos += 10;
	int testAnimal_air_in_water =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = amount;
	world[testPos].wall = MATERIAL_WATER;

	setupTestAnimal_waterbreathing(j);
	testPos += 10;
	int testAnimal_water_in_air =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = amount;

	setupTestAnimal_waterbreathing(j);
	testPos += 10;
	int testAnimal_water_in_water =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = amount;
	world[testPos].wall = MATERIAL_WATER;

	setupTestAnimal_amphibious(j);
	int testAnimal_amphi_in_air =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = amount;

	setupTestAnimal_amphibious(j);
	testPos += 10;
	int testAnimal_amphi_in_water =	spawnAnimal( testSpecies , animals[j], testPos, false);
	animals[testAnimal].position = testPos;
	animals[testAnimal].uPosX = testPos % worldSize;
	animals[testAnimal].uPosY = testPos / worldSize;
	animals[testAnimal].fPosX = animals[testAnimal].uPosX;
	animals[testAnimal].fPosY = animals[testAnimal].uPosY;
	animals[testAnimal].energy = amount;

	world[testPos].wall = MATERIAL_WATER;
	testPos += worldSize;
	world[testPos].wall = MATERIAL_WATER;

	for (int i = 0; i < how_long_it_takes_to_make_sure; ++i)
	{
		animals[testAnimal_air_in_air].energy     = amount;
		animals[testAnimal_air_in_water].energy   = amount;
		animals[testAnimal_water_in_air].energy   = amount;
		animals[testAnimal_water_in_water].energy = amount;
		animals[testAnimal_amphi_in_air].energy   = amount;
		animals[testAnimal_amphi_in_water].energy = amount;

		animalTurn(testAnimal_air_in_air);
		animalTurn(testAnimal_air_in_water);
		animalTurn(testAnimal_water_in_air);
		animalTurn(testAnimal_water_in_water);
		animalTurn(testAnimal_amphi_in_air);
		animalTurn(testAnimal_amphi_in_water);
		// printf("%i of %i \n", i , how_long_it_takes_to_make_sure);
		// printf("testAnimal_air_in_air %i \n",animals[testAnimal_air_in_air].retired    );
		// printf("testAnimal_air_in_water %i \n",animals[testAnimal_air_in_water].retired    );
		// printf("testAnimal_water_in_air %i \n",animals[testAnimal_water_in_air].retired    );
		// printf("testAnimal_water_in_water %i \n",animals[testAnimal_water_in_water].retired    );
		// printf("testAnimal_amphi_in_air %i \n",animals[testAnimal_air_in_air].retired    );
		// printf("testAnimal_amphi_in_water %i \n",animals[testAnimal_amphi_in_water].retired    );

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (
	    !(animals[testAnimal_air_in_air].retired     ) &&
	    (animals[testAnimal_air_in_water].retired   ) &&
	    (animals[testAnimal_water_in_air].retired   ) &&
	    !(animals[testAnimal_water_in_water].retired ) &&
	    !(animals[testAnimal_amphi_in_air].retired   ) &&
	    !(animals[testAnimal_amphi_in_water].retired )
	)
	{
		testResult_6 = true;
	}




	if (
	    testResult_2 &&
	    testResult_3 &&
	    testResult_4 &&
	    testResult_5 &&
	    testResult_6
	)
	{
		return true;
	}
	else
	{

		if (!testResult_2)
		{
			printf("test 2: eat grass: FAIL\n");
		}

		if (!testResult_3)
		{
			printf("test 3: have baby: FAIL\n");
		}

		if (!testResult_4)
		{
			printf("test 4: baby check: FAIL\n");
		}

		if (!testResult_5)
		{
			printf("test 5: neural pathway: FAIL\n");
		}

		if (!testResult_6)
		{
			printf("test 6: breathing: FAIL\n");
		}
	}
	return false;
}



bool test_plants()
{









	// setup the test plant, which has a very distinctive shape
	resetGameState();
	bool testResult_1 = false;
	unsigned int testPos = (worldSize * 10) +  extremelyFastNumberFromZeroTo(worldSquareSize -  ((worldSize * 10) + 1) );
	world[testPos].terrain = MATERIAL_SOIL;
	setupTestPlant2(testPos);
	growInto(  testPos, testPos, MATERIAL_ROOT, true );
	int testPlantPatch = 10;
	unsigned int testPosX = testPos % worldSize;
	unsigned int testPosY = testPos / worldSize;

	// grow the plant
	for (int i = 0; i < 100; ++i)
	{
		for (int vy = -testPlantPatch; vy < +testPlantPatch; ++vy)
		{
			for (int vx = -testPlantPatch; vx < +testPlantPatch; ++vx)
			{

				unsigned int actualX = testPosX + vx;
				unsigned int actualY = testPosY + vy;

				unsigned int updateAddress = ( (actualY) * worldSize ) + (actualX);


				world[updateAddress].terrain = MATERIAL_SOIL;
				world[updateAddress].light = color_white;
				updatePlants(updateAddress);


			}
		}
	}


	// inspecc the plant
	unsigned int woodProbe1 = testPos ;
	woodProbe1 -= worldSize * 4;
	if (world[woodProbe1].plantState == MATERIAL_WOOD)
	{
		testResult_1 = true;
	}

	if (testResult_1)
	{
		return true;
	}
	return false;
}



bool test_all()
{

	bool testResult_all = true;

	bool testResult_animals = test_animals();
	bool testResult_plants  = test_plants();


	if ( !testResult_animals )
	{
		printf("animals: FAIL\n");
		testResult_all = false;
	}

	if ( !testResult_plants )
	{
		printf("plants: FAIL\n");
		// testResult_all = false;
	}

	return testResult_all;
}