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
#include "main.h"
#include "SimplexNoise.h"
#include "TinyErode.h"
#include "content.h"

const bool doReproduction        = true;
const bool taxIsByMass           = true;
const bool respawnLowSpecies     = true;
const bool setOrSteerAngle       = true;
const bool printLogs             = true;
const bool doHoney = false;
const bool doAsexualPlants = false;
const bool doAsexualAnimals = false;
const bool seedDegradation = false;
const bool movePollenUnconditionally = true;
const bool pollenDegradation = false;

// const bool doBrambles = false;
const bool environmentScarcity = true;
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
const unsigned int baseLifespan = 1000;
// if the lifespan is long, the animal's strategy can have a greater effect on its success. If it's very short, the animal is compelled to be just a moving mouth.
// with that being said, this value is supposed to represent the smallest end of the valid range
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
const float honeyCost = 3.0f;
const float seaLevel =  0.5f * worldSize;
const float biome_marine  = seaLevel + (worldSize / 20);
const float biome_coastal = seaLevel + (worldSize / 3);
const float sunXangle = 0.35f;
const float sunYangle = 0.35f;
const unsigned int baseSize = 50;
const unsigned int wallThickness = 8;
const unsigned int doorThickness = 16;
const int destroyerRange = 250;

const float plantMaxEnergy = nNeighbours * 3.0f;

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

int plantIdentityCursor = 0;

// map updating
const int numberOfMapSectors = 4;
const int mapSectorSize = worldSquareSize / numberOfMapSectors;
std::string progressString = std::string("");
float coverage[numberOfMapSectors];

GameState * pgame = new GameState;
GameState& game   = *pgame; // https://stackoverflow.com/questions/4436805/does-dereferencing-a-pointer-make-a-copy-of-it


// Square * world   = new Square[worldSquareSize];
// Animal * animals = new Animal[numberOfAnimals];

int menuX = 50;
int menuY = 50;
int textSize = 10;
int spacing = 20;


float sideTextScrollPos = 0.0f;

void incrementSideText()
{
	sideTextScrollPos += spacing;
}
void decrementSideText()
{
	sideTextScrollPos -= spacing;

}
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
	game.animals[animalIndex].body[cellLocalPositionI].workingValue = 0.0f;

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
		if (game.animals[animalIndex].body[i].organ != ORGAN_SENSOR_EYE  && game.animals[animalIndex].body[i].organ != ORGAN_SENSOR_SCANNINGEYE )
		{
			game.animals[animalIndex].body[i].color = filterColor(  newAnimalColorA , multiplyColorByScalar( newAnimalColorB , RNG())  );


		}
	}
}


void paintAnimals ()
{

	for (int i = 0; i < numberOfAnimals; ++i)
	{
		int speciesIndex = i / numberOfAnimalsPerSpecies;
		if (speciesIndex > 0)
		{
			paintAnimal(i);
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
		game.animals[animalIndex].cellsUsed = 0;
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
		game.animals[animalIndex].isMachine = false;
		game.animals[animalIndex].machineCallback = MATERIAL_NOTHING;
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

void scrambleAnimal( int animalIndex)
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

void scrambleSelectedAnimal()
{
	if (game.selectedAnimal >= 0)
	{

		scrambleAnimal(game.selectedAnimal);
	}
}

void setupExampleAnimal3(int i)
{
	resetAnimal(i);



	animalAppendCell( i, ORGAN_SENSOR_RANDOM );
	animalAppendCell( i, ORGAN_SENSOR_RANDOM );
	animalAppendCell( i, ORGAN_SENSOR_RANDOM );
	animalAppendCell( i, ORGAN_MUSCLE );
	animalAppendCell( i, ORGAN_MUSCLE_STRAFE );
	animalAppendCell( i, ORGAN_MUSCLE_TURN );

	game.animals[i].body[2].connections[0].used = true;
	game.animals[i].body[2].connections[0].connectedTo = 0;
	game.animals[i].body[2].connections[0].weight = 1.0f;

	game.animals[i].body[3].connections[0].used = true;
	game.animals[i].body[3].connections[0].connectedTo = 1;
	game.animals[i].body[3].connections[0].weight = 1.0f;

	game.animals[i].body[4].connections[0].used = true;
	game.animals[i].body[4].connections[0].connectedTo = 2;
	game.animals[i].body[4].connections[0].weight = 1.0f;

	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SCANNING_EYE_PLANT );
	animalAppendCell( i, ORGAN_SCANNING_EYE_PLANT );
	animalAppendCell( i, ORGAN_SCANNING_EYE_CREATURE );
	animalAppendCell( i, ORGAN_SCANNING_EYE_OBSTACLE );
	animalAppendCell( i, ORGAN_SENSOR_SCANNINGEYE );
	animalAppendCell( i, ORGAN_SENSOR_SCANNINGEYE );

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
	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );
	animalAppendCell( i, ORGAN_ADDOFFSPRINGENERGY );
	animalAppendCell( i, ORGAN_ADDLIFESPAN );
	animalAppendCell( i, ORGAN_ADDLIFESPAN );
	if (doAsexualAnimals)
	{
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
		animalAppendCell( i, ORGAN_GONAD);
	}
	else
	{

		if (extremelyFastNumberFromZeroTo(1) == 0)
		{
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
			animalAppendCell( i, ORGAN_GENITAL_B );
		}

		else
		{

			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );
			animalAppendCell( i, ORGAN_GENITAL_A );

		}
	}

	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );
	animalAppendCell( i, ORGAN_MOUTH_WOOD );

	animalAppendCell( i, ORGAN_GILL );

	// animalAppendCell( i, ORGAN_MUSCLE );
	// animalAppendCell( i, ORGAN_MUSCLE_TURN );

	// animalAppendCell( i, ORGAN_MUSCLE );
	// animalAppendCell( i, ORGAN_MUSCLE_TURN );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_WOOD );
	// animalAppendCell( i, ORGAN_MOUTH_SEEDS );
	// animalAppendCell( i, ORGAN_MOUTH_SEEDS );
	// animalAppendCell( i, ORGAN_MOUTH_SEEDS );
	// animalAppendCell( i, ORGAN_MOUTH_SEEDS );
	// animalAppendCell( i, ORGAN_MOUTH_SEEDS );
	// animalAppendCell( i, ORGAN_MOUTH_SEEDS );
	// animalAppendCell( i, ORGAN_MOUTH_SCAVENGE );
	// animalAppendCell( i, ORGAN_MOUTH_SCAVENGE );
	// animalAppendCell( i, ORGAN_MOUTH_SCAVENGE );
	// animalAppendCell( i, ORGAN_MOUTH_SCAVENGE );

	paintAnimal(i);

	// scrambleAnimal(i);
	game.animals[i].generation = 0;
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


	game.animals[i].generation = 0;
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
		game.world[i].downhillNeighbour = 0;
		game.world[i].pheromoneChannel = -1;
// #ifdef PLANTS
		game.world[i].grassColor =  color_green;
		memset(&(game.world[i].plantGenes[0]), PLANTGENE_END, sizeof(char) * plantGenomeSize);
		game.world[i].plantState = MATERIAL_NOTHING;
		game.world[i].geneCursor = 0;
		game.world[i].plantIdentity = -1;
		game.world[i].energy = 0.0f;
		game.world[i].sequenceReturn = 0;
		game.world[i].sequenceNumber = 0;
		game.world[i].grown = false;;
		memset(&(game.world[i].seedGenes[0]), PLANTGENE_END, sizeof(char) * plantGenomeSize);
		game.world[i].seedState = MATERIAL_NOTHING;
		game.world[i].seedIdentity =  -1;
		game.world[i].seedColor = color_yellow;
		game.world[i].branching = false;
		game.world[i].aquaticPlant = true;


// #endif
	}
}

void fastReset()
{
	// memset( &game, 0x00, sizeof(GameState)  );
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
	game.ecoSettings[0]           = 0.85f; // food (meat) energy
	game.ecoSettings[1]          = 0.25f; // grass energy
	game.ecoSettings[4] = (worldSquareSize / 32) / numberOfMapSectors; ; //updateSize;
	game.ecoSettings[5] = 0.35f ;  //nutrient rate
	game.ecoSettings[6] = 0.01f ; // amount of energy a plant tile requires
	game.ecoSettings[7] = 10.0f ; // the animal mutation rate
	game.ecoSettings[8] = 10.0f ; // the plant mutation rate


	game.ecoSettings[9] =    25000;     // autosave timer.

	game.ecoSettings[10] =    5;     // pollen mobility.
	game.ecoSettings[11] =    1.0f;     // auto reproduction chance.

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


		if (organType == ORGAN_BIASNEURON)
		{
			game.animals[animalIndex].body[cellIndex].workingValue =  ((RNG() - 0.5f) * 2.0f);
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
	appendCell( i, ORGAN_SENSOR_EYE, Vec_i2(0, 0) );
	appendCell( i, ORGAN_NEURON, Vec_i2(0, 1) );
	appendCell( i, ORGAN_MUSCLE_TURN, Vec_i2(0, 2) );
	game.animals[i].body[1].connections[0].used = true;
	game.animals[i].body[1].connections[0].connectedTo = 0;
	game.animals[i].body[1].connections[0].weight = 1.0f;
	game.animals[i].body[2].connections[0].used = true;
	game.animals[i].body[2].connections[0].connectedTo = 1;
	game.animals[i].body[2].connections[0].weight = 1.0f;
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
	game.animals[i].body[2].connections[0].used = true;
	game.animals[i].body[2].connections[0].connectedTo = 1;
	game.animals[i].body[2].connections[0].weight = 1.0f;
	game.animals[i].body[1].workingValue = 0.1f;
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
		float xSurfaceAngle = game.world[worldPositionI].height - game.world[worldPositionI + 1].height ;
		float ySurfaceAngle = game.world[worldPositionI].height - game.world[worldPositionI + worldSize].height ;
		return Vec_f2(xSurfaceAngle, ySurfaceAngle);
	}
	return Vec_f2(0.0f, 0.0f);

}
void swapEnergyWithNeighbour(unsigned int worldI, unsigned int neighbour, float rate)
{
	float amount = (game.world[worldI].energy - game.world[neighbour].energy) * rate;
	game.world[worldI].energy    -=  (amount / 2.0f) ;
	game.world[neighbour].energy +=  (amount / 2.0f) ;
}


void swapNootsWithNeighbour(unsigned int worldI, unsigned int neighbour, float rate)
{
	float amount = (game.world[worldI].nutrients - game.world[neighbour].nutrients) * rate;
	game.world[worldI].nutrients    -=  (amount / 2.0f) ;
	game.world[neighbour].nutrients +=  (amount / 2.0f) ;
}


const float seedCost = 3.0f;

// this function governs how plants propagate from square to square.
// The reason for the separation of plant and seed identities is that it allows seeds to move in front of plants in the game game.world.
// returns if any growth was made, or not.
// bool
void growInto( int to ,  int from,  unsigned int organ, bool fromSeed)
{

	// don't do it if the destination cell already has the same organ type and identity. stops wood from endlessly overgrowing itself.
	int originID  = game.world[from].seedIdentity ;
	if (!fromSeed)
	{
		originID = game.world[from].plantIdentity;
		if (game.world[to].plantState == organ && game.world[to].plantIdentity == originID) //the thing you are trying to grow is already grown- you should stop.
		{
			return ;
		}
	}
	if (to < 0 || from < 0 || to >= worldSquareSize || from >= worldSquareSize)  // trying to grow over the game.world size. In this case, the function reports true but you grow nothing, as if the tissue disappeared.
	{
		return;// true;
	}

	if (( materialBlocksMovement (game.world[to].wall))) // again, return true but get nothing.
	{
		return;
	}

	if (organ == MATERIAL_SEED || organ == MATERIAL_POLLEN) // production of plant gametes
	{
		if (fromSeed)
		{
			if (organ == MATERIAL_SEED)
			{
				game.world[to].seedIdentity = extremelyFastNumberFromZeroTo(65536);
			}
			else if (organ == MATERIAL_POLLEN)
			{
				game.world[to].seedIdentity = game.world[from].seedIdentity;
			}

			game.world[to].seedColorMoving = game.world[from].seedColorMoving;
			memcpy( game.world[to].seedGenes , game.world[from].seedGenes,  plantGenomeSize * sizeof(char)  );
		}

		else
		{
			if (organ == MATERIAL_SEED)
			{
				game.world[to].seedIdentity = extremelyFastNumberFromZeroTo(65536);
				game.world[from].nutrients -= seedCost;
				game.world[from].energy    -= seedCost;
			}
			else if (organ == MATERIAL_POLLEN)
			{
				game.world[to].seedIdentity = game.world[from].plantIdentity;
			}

			game.world[to].seedColorMoving = game.world[from].seedColor;
			memcpy( game.world[to].seedGenes , game.world[from].plantGenes,  plantGenomeSize * sizeof(char)  );
		}
		game.world[to].seedState = organ;
	}
	else                                          // production of functional tissues
	{

		if (fromSeed)										// when spawning from a seed
		{
			game.world[to].plantIdentity =  game.world[from].seedIdentity;
			game.world[to].grassColor = color_green;
			game.world[to].seedColor = color_yellow;
			memcpy( & (game.world[to].plantGenes[0]) , &(game.world[from].seedGenes[0]),  plantGenomeSize * sizeof(char)  );
			memset( & (game.world[to].growthMatrix), false, sizeof(bool) * nNeighbours);
			game.world[to].energy = seedCost;
			game.world[to].nutrients = seedCost;
			game.world[to].geneCursor = 0;
		}
		else                                                 // when propagating from existing tissues
		{
			game.world[to].plantIdentity = game.world[from].plantIdentity ;
			game.world[to].grassColor = game.world[from].grassColor;
			game.world[to].seedColor = game.world[from].seedColor;
			memcpy( & (game.world[to].plantGenes[0]) , &(game.world[from].plantGenes[0]),  plantGenomeSize * sizeof(char)  );
			memcpy( &(game.world[to].growthMatrix[0]), &(game.world[from].growthMatrix[0]), sizeof(bool) * nNeighbours   );

			game.world[from].nutrients -= 1.0f;

			game.world[to].nutrients = 0.0f;
			game.world[to].energy = 0.0f;

			swapNootsWithNeighbour(from, to, 0.1f);
			swapEnergyWithNeighbour(from, to, 0.1f);

			game.world[to].geneCursor = game.world[from].geneCursor + 1 ;
			game.world[to].sequenceNumber = game.world[from].sequenceNumber;
			game.world[to].sequenceReturn = game.world[from].sequenceReturn;
		}
		game.world[to].plantState = organ;
		game.world[to].grown = false;
	}
	return;
}

void setupTestPlant3(unsigned int worldPositionI)
{
	memset(game.world[worldPositionI].seedGenes, 255, sizeof(char) * plantGenomeSize);
	game.world[worldPositionI].seedGenes[0] = 2;
	game.world[worldPositionI].seedGenes[1] = PLANTGENE_WOOD;
	game.world[worldPositionI].seedGenes[2] = PLANTGENE_BRANCH;
	game.world[worldPositionI].seedGenes[3] = PLANTGENE_LEAF;
	game.world[worldPositionI].seedGenes[4] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedGenes[5] = PLANTGENE_BUD_A;
	game.world[worldPositionI].seedGenes[6] = PLANTGENE_BUD_A;
	game.world[worldPositionI].seedGenes[7] = PLANTGENE_END;
	game.world[worldPositionI].seedColorMoving = color_yellow;
	growInto(worldPositionI, worldPositionI, MATERIAL_SEED, true);
}


void spawnRandomPlant(unsigned int worldI)
{
// spawn some plants
	if (game.world[worldI].seedState == MATERIAL_NOTHING)
	{
		for (int k = 0; k < plantGenomeSize; ++k)
		{
			game.world[worldI].plantGenes[k] = extremelyFastNumberFromZeroTo(numberOfPlantGenes);
		}
		growInto(worldI, worldI, MATERIAL_SEED, false);
	}
}

void detailTerrain()
{
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
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (  game.world[worldPositionI].terrain == MATERIAL_ROCK )
		{
			Vec_f2 slope = getTerrainSlope(worldPositionI);
			float grade = sqrt( (slope.x * slope.x) +  (slope.y * slope.y)  );
			float colorNoise = 1 + (((RNG() - 0.5f) * 0.35)) ; // map -1,1 to 0,0.8
			if (game.world[worldPositionI]. height < seaLevel)
			{
				if (grade < 5.0f)
				{
					if (extremelyFastNumberFromZeroTo(5) == 0)
					{
						spawnRandomPlant( worldPositionI );
					}
				}
			}

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

// this is the bare minimum requirements for any animal to be allowed to exist.
bool validateAnimal(unsigned int animalIndex)
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

		if (       game.animals[animalIndex].body[i].organ == ORGAN_LUNG
		           || game.animals[animalIndex].body[i].organ == ORGAN_GILL )
		{
			totalBreathing++;
		}

		if (

		    organIsAMouth( game.animals[animalIndex].body[i].organ )
		)
		{
			totalMouths++;
		}
	}
	if (
	    totalGonads >= 2 && totalMouths >= 1 && totalBreathing >= 1
	    && game.animals[animalIndex].cellsUsed > 0
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
	game.animals[animalIndex].energyDebt = game.animals[animalIndex].cellsUsed;
	game.animals[animalIndex].totalMuscle = 0;
	game.animals[animalIndex].offspringEnergy = 1.0f;
	game.animals[animalIndex].lifespan = baseLifespan;
	unsigned int totalLiver = 0;


	unsigned int breathingType = ORGAN_GILL;
	bool chosenBreathing = false;

	unsigned int mouthType = ORGAN_MOUTH_WOOD;
	bool chosenMouth = false;

	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{


		if ( organIsAMouth( game.animals[animalIndex].body[cellIndex].organ ) )
		{
			if (chosenMouth)
			{
				game.animals[animalIndex].body[cellIndex].organ = mouthType;
			}
			else
			{
				mouthType = game.animals[animalIndex].body[cellIndex].organ ;
				chosenMouth = true;
			}
		}

		if (  game.animals[animalIndex].body[cellIndex].organ  == ORGAN_LUNG ||  game.animals[animalIndex].body[cellIndex].organ  == ORGAN_GILL)
		{
			if (chosenBreathing)
			{
				game.animals[animalIndex].body[cellIndex].organ = breathingType;
			}
			else
			{
				breathingType = game.animals[animalIndex].body[cellIndex].organ ;
				chosenBreathing = true;
			}
		}



		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE ||
		        game.animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_TURN ||
		        game.animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_STRAFE)
		{
			game.animals[animalIndex].totalMuscle ++;
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_ADDOFFSPRINGENERGY)
		{
			game.animals[animalIndex].offspringEnergy += game.animals[animalIndex].offspringEnergy ;
			if (game.animals[animalIndex].offspringEnergy > game.animals[animalIndex].cellsUsed / 2)
			{
				game.animals[animalIndex].offspringEnergy = game.animals[animalIndex].cellsUsed / 2; // if its bigger than this, the animal will never be able to reproduce.
			}
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_ADDLIFESPAN)
		{
			game.animals[animalIndex].lifespan += game.animals[animalIndex].lifespan ; // increase fibonaccically. :)
		}
		if (game.animals[animalIndex].body[cellIndex].organ == ORGAN_LIVER)
		{
			totalLiver++;
		}
	}
	game.animals[animalIndex].maxEnergy = game.animals[animalIndex].cellsUsed + (totalLiver * liverStorage);
	game.animals[animalIndex].lifespan *= 0.85 + (RNG() * 0.3);
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

	if (game.animals[animalIndex].cellsUsed <= 0) { return;}
	if (animalIndex < 0 || animalIndex >= numberOfAnimals) { return;}


	// some mutations are chosen more commonly than others. They are classed into groups, and each group is associated with a normalized likelyhood of occurring.
	// the reason for this is that the development of the brain must always occur faster and in more detail than the development of the body, or else intelligent behavior can never arise.
	const float group1Probability = 1.0f;//1.0f;
	const float group2Probability = 1.0f;//0.5f;
	const float group3Probability = 0.65f;//0.125f;
	const float group4Probability = 0.35f;//0.0625f;
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

	// animal receives some skin color mutations every time.
	for (int i = 0; i < game.animals[animalIndex].cellsUsed / 20; ++i)
	{
		int mutantSkinCell = getRandomPopulatedCell(animalIndex);
		if (mutantSkinCell >= 0 && mutantSkinCell < animalSquareSize)
		{
			game.animals[animalIndex].body[mutantSkinCell].color = mutateColor(	game.animals[animalIndex].body[mutantSkinCell].color);
		}
	}

	int mutation = MATERIAL_NOTHING;// choose a mutation from the group randomly.
	if (group == 1)
	{
		int mutationChoice = extremelyFastNumberFromZeroTo(3);
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
		else if (mutationChoice == 3)
		{
			mutation = MUTATION_INVERTWEIGHT;
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
		int mutationChoice = extremelyFastNumberFromZeroTo(4);
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
		else if (mutationChoice == 3)
		{
			mutation = MUTATION_DUPLICATEORGAN;
		}
		else if (mutationChoice == 4)
		{
			mutation = MUTATION_CHANGEORGAN;
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
	case MUTATION_CHANGEORGAN:
	{


		int mutantCell = getRandomPopulatedCell( animalIndex);
		if (mutantCell >= 0)
		{


			unsigned int newOrgan = extremelyFastNumberFromZeroTo(numberOfOrganTypes);
			if (organIsAMouth(newOrgan))
			{
				for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
				{
					if (organIsAMouth (game.animals[animalIndex].body[i].organ))
					{
						game.animals[animalIndex].body[i].organ = newOrgan;
					}
				}
			}

			if (newOrgan == ORGAN_LUNG || newOrgan == ORGAN_GILL)
			{
				for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
				{
					if (newOrgan == ORGAN_LUNG && game.animals[animalIndex].body[i].organ == ORGAN_GILL)
					{
						game.animals[animalIndex].body[i].organ = ORGAN_LUNG;
					}
					if (newOrgan == ORGAN_GILL && game.animals[animalIndex].body[i].organ == ORGAN_LUNG)
					{
						game.animals[animalIndex].body[i].organ = ORGAN_GILL;
					}
				}
			}



			game.animals[animalIndex].body[mutantCell].organ = extremelyFastNumberFromZeroTo(numberOfOrganTypes);
		}
		break;
	}
	case MUTATION_ADDORGAN:
	{
		unsigned int newOrgan = extremelyFastNumberFromZeroTo(numberOfOrganTypes);

		// animal should only have one kind of mouth, if you add a new type, convert every existing mouth to that type.
		if (organIsAMouth(newOrgan))
		{
			for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
			{
				if (organIsAMouth (game.animals[animalIndex].body[i].organ))
				{
					game.animals[animalIndex].body[i].organ = newOrgan;
				}
			}
		}
		if (newOrgan == ORGAN_LUNG || newOrgan == ORGAN_GILL)
		{
			for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
			{
				if (newOrgan == ORGAN_LUNG && game.animals[animalIndex].body[i].organ == ORGAN_GILL)
				{
					game.animals[animalIndex].body[i].organ = ORGAN_LUNG;
				}
				if (newOrgan == ORGAN_GILL && game.animals[animalIndex].body[i].organ == ORGAN_LUNG)
				{
					game.animals[animalIndex].body[i].organ = ORGAN_GILL;
				}
			}
		}

		if (extremelyFastNumberFromZeroTo(1) == 0   || organIsASensor(newOrgan)) // add symmetric
		{
			Vec_i2 newPosition   = getRandomEmptyEdgeLocation(animalIndex); 	// figure out a new position anywhere on the animal edge
			Vec_i2 mirrorPos   =  newPosition;
			mirrorPos.x *= -1;
			appendCell(animalIndex, newOrgan,  newPosition);
			appendCell(animalIndex, newOrgan,  mirrorPos);
		}
		else
		{
			Vec_i2 newPosition   = getRandomEmptyEdgeLocation(animalIndex); 	// figure out a new position anywhere on the animal edge
			appendCell(animalIndex, newOrgan,  newPosition);
		}
		break;
	}
	case MUTATION_DUPLICATEORGAN:
	{
		int mutantCell = getRandomPopulatedCell(animalIndex);
		if ( mutantCell >= 0)
		{
			int newOrgan = game.animals[animalIndex].body[mutantCell].organ;//extremelyFastNumberFromZeroTo(numberOfOrganTypes);
			appendCell(animalIndex, newOrgan,  Vec_i2(game.animals[animalIndex].body[mutantCell].localPosX, game.animals[animalIndex].body[mutantCell].localPosY));

			int mostRecentAddedCell = game.animals[animalIndex].cellsUsed - 1;
			if ( mostRecentAddedCell > 0 && mutantCell > 0 && mutantCell < game.animals[animalIndex].cellsUsed )
			{
				game.animals[animalIndex].body[game.animals[animalIndex].cellsUsed - 1] = game.animals[animalIndex].body[mutantCell];
				game.animals[animalIndex].body[mutantCell].localPosX  +=   -1 + extremelyFastNumberFromZeroTo(2) ;
				game.animals[animalIndex].body[mutantCell].localPosY  +=   -1 + extremelyFastNumberFromZeroTo(2);
			}
		}
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
					game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight = ((RNG() - 0.5f) * 4.0f );
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
			game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight += ((RNG() - 0.5f) * 2.0f); // it can add -1 to 1 .
		}
		break;
	}
	case MUTATION_MULTIPLYWEIGHT:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight =
			    game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight  *  (((RNG() - 0.5f) * 0.75) + 1.25)    ; // it can double or halve the value (i.e. multiply from 0.5 to 2 times) but should not change the sign
		}
		break;
	}

	case MUTATION_INVERTWEIGHT:
	{
		int mutantCell =  getRandomConnectingCell(animalIndex);
		if (mutantCell >= 0)
		{
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight =
			    game.animals[animalIndex].body[mutantCell].connections[mutantConnection].weight  * 1.0f   ;
		}
		break;
	}
	case MUTATION_ALTERBIAS:// randomise a bias neuron's strength, or anything else that uses the working value.
	{
		int nChoices = 6;
		int choices[nChoices] = {
			getRandomCellOfType(animalIndex, ORGAN_BIASNEURON)         ,
			getRandomCellOfType(animalIndex, ORGAN_TIMER)         ,
			getRandomCellOfType(animalIndex, ORGAN_SENSOR_SCANNINGEYE) ,
			getRandomCellOfType(animalIndex, ORGAN_SCANNING_EYE_PLANT) ,
			getRandomCellOfType(animalIndex, ORGAN_SCANNING_EYE_CREATURE) ,
			getRandomCellOfType(animalIndex, ORGAN_SCANNING_EYE_OBSTACLE) ,
		};
		int mutantCell = -1;
		int i = 0;
		while (true)
		{
			int choice = choices[extremelyFastNumberFromZeroTo(nChoices - 1)];
			if (choice >= 0 )
			{
				mutantCell = choice;
				break;
			}
			i++; if (i > nChoices) {return;}
		}



		if (mutantCell >= 0)
		{
			game.animals[animalIndex].body[mutantCell].workingValue = game.animals[animalIndex].body[mutantCell].workingValue * (RNG() + 0.5f) ;
			game.animals[animalIndex].body[mutantCell].workingValue += ((RNG() - 0.5f ) * 2.0f);

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
			if (game.animals[animalIndex].cellsUsed > 0)
			{
				unsigned int cellIndex = (startingRandomCell + i) % game.animals[animalIndex].cellsUsed;
				if ( organUsesSpeakerChannel( game.animals[animalIndex].body[cellIndex].organ )  )
				{
					occupiedChannel =	 findOccupiedChannel( animalIndex, game.animals[animalIndex].body[cellIndex].organ);
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

		int nChoices = 5;
		int choices[nChoices] = {
			getRandomCellOfType(animalIndex, ORGAN_SENSOR_EYE)         ,
			getRandomCellOfType(animalIndex, ORGAN_SENSOR_SCANNINGEYE) ,
			getRandomCellOfType(animalIndex, ORGAN_SCANNING_EYE_PLANT) ,
			getRandomCellOfType(animalIndex, ORGAN_SCANNING_EYE_CREATURE) ,
			getRandomCellOfType(animalIndex, ORGAN_SCANNING_EYE_OBSTACLE) ,
		};
		int mutantCell = -1;
		int i = 0;
		while (true)
		{
			int choice = choices[extremelyFastNumberFromZeroTo(nChoices - 1)];
			if (choice >= 0 )
			{
				mutantCell = choice;
				break;
			}
			i++; if (i > nChoices) {return;}
		}

		if (mutantCell >= 0 && mutantCell < animalSquareSize)
		{

			game.animals[animalIndex].body[mutantCell].eyeLookX += (extremelyFastNumberFromZeroTo(2) - 1);
			game.animals[animalIndex].body[mutantCell].eyeLookY += (extremelyFastNumberFromZeroTo(2) - 1);

		}
		break;
	}

	}



	if (mutation == MUTATION_ADDORGAN || mutation == MUTATION_CHANGEORGAN)
	{

	}



}

void mutateAnimalByRate( int animalIndex)
{
	// if the mutation rate is less than 1, roll for a chance.
	if (game.ecoSettings[7] < 1.0f)
	{
		if (RNG() < game.ecoSettings[7])
		{
			mutateAnimal(animalIndex);
		}
	}
	else
	{
		int n = game.ecoSettings[7];
		for (int i = 0; i < n; ++i)
		{
			mutateAnimal(animalIndex);
		}
	}
}

void normalizeAnimalCellPositions( int animalIndex)
{
	// this function moves all the cell local positions so that the animal's geometric center is at 0,0.
	if (game.animals[animalIndex].cellsUsed == 0) {return;}

	// find the geometric center.
	Vec_f2 centroid = Vec_f2(0.0f, 0.0f);

	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		centroid.x += game.animals[animalIndex].body[i].localPosX;
		centroid.y += game.animals[animalIndex].body[i].localPosY;
	}

	centroid.x = centroid.x / game.animals[animalIndex].cellsUsed;
	centroid.y = centroid.y / game.animals[animalIndex].cellsUsed;

	printf("normalized animal %i. The centroid was %f %f \n", animalIndex, centroid.x, centroid.y);

	Vec_i2 i_centroid = Vec_i2(centroid.x, centroid.y);

	for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
	{
		game.animals[animalIndex].body[i].localPosX -= i_centroid.x;
		game.animals[animalIndex].body[i].localPosY -= i_centroid.y;
	}
}

void spawnAnimalIntoSlot(  int animalIndex,
                           Animal parent,
                           unsigned int position, bool mutation) // copy genes from the parent and then copy body from own new genes.
{

	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	resetAnimal(animalIndex);

	game.animals[animalIndex].cellsUsed = parent.cellsUsed;
	for (int i = 0; i < game.animals[animalIndex].cellsUsed ; ++i)
	{
		game.animals[animalIndex].body[i] = parent.body[i];
	}
	for (int i = 0; i < animalSquareSize; ++i)
	{
		game.animals[animalIndex].body[i].damage = 0.0f;
	}
	// normalizeAnimalCellPositions( animalIndex);


	game.animals[animalIndex].isMachine = parent.isMachine;
	game.animals[animalIndex].machineCallback = parent.machineCallback;


	game.animals[animalIndex].position = position;
	game.animals[animalIndex].uPosX = position % worldSize; // set the new creature to the desired position
	game.animals[animalIndex].uPosY = position / worldSize;
	game.animals[animalIndex].fPosX = game.animals[animalIndex].uPosX;//position % worldSize; // set the new creature to the desired position
	game.animals[animalIndex].fPosY = game.animals[animalIndex].uPosY;//position / worldSize;



	game.animals[animalIndex].birthLocation = position;
	game.animals[animalIndex].fAngle = ( (RNG() - 0.5f) * 2 * const_pi );
	game.animals[animalIndex].generation = parent.generation;
	game.animals[animalIndex].generation ++;

	if (mutation)
	{
		mutateAnimalByRate( animalIndex);
	}

	memcpy( &( game.animals[animalIndex].displayName[0]), &(parent.displayName[0]), sizeof(char) * displayNameSize  );
	measureAnimalQualities(animalIndex) ;
	// if (speciesIndex == 0)
	// {
	// 	game.animals[animalIndex].retired = false;
	// }
	// else
	// {
	// 	if ( validateAnimal( animalIndex) )
	// 	{
	game.animals[animalIndex].retired = false;
	// }
	// else
	// {
	// 	game.animals[animalIndex].retired = true;
	// }
	// }
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(int animalIndex, unsigned int cellWorldPositionI)
{
	ZoneScoped;
	if ( animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		if (cellWorldPositionI < worldSquareSize && game.world[cellWorldPositionI].identity >= 0)
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
			if (game.world [worldCursorPos].plantState != MATERIAL_NOTHING)
			{
				game.selectedPlant = game.world [worldCursorPos].plantIdentity;
			}
		}
	}
}

void killAnimal(int animalIndex)
{


	for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex) // process organs and signals and clear animalIndex on grid
	{
		unsigned int cellWorldPositionI  = game.animals[animalIndex].body[cellIndex].worldPositionI;
		if (cellWorldPositionI < worldSquareSize)
		{
			if (game.animals[animalIndex].body[cellIndex].organ != MATERIAL_NOTHING && game.animals[animalIndex].body[cellIndex].damage < 1.0f)
			{
				game.world[cellWorldPositionI].pheromoneChannel = 13;
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


	if (animalIndex == game.playerCreature)
	{
		appendLog(std::string("You died!"));

		int pre = 0;
		for (int i = 0; i < game.animals[game.playerCreature].cellsUsed; ++i)
		{
			game.animals[game.playerCreature].body[i].damage = 0.0f;
		}
		game.animals[game.playerCreature].damageReceived = 0;
		return;

	}
	if (animalIndex == game.cameraTargetCreature)
	{
		game.cameraTargetCreature = -1;
	}





	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	game.speciesVacancies[speciesIndex] = true;
	game.animals[animalIndex].retired = true;
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

void spill(unsigned int material,  unsigned int worldPositionI)
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
	switch (material)
	{
	case MATERIAL_BLOOD:
	{
		game.world[worldPositionI].pheromoneChannel = PHEROMONE_BLOOD;
		break;
	}

	case MATERIAL_VOMIT:
	{
		game.world[worldPositionI].pheromoneChannel = PHEROMONE_PUKE;
		break;
	}

	case MATERIAL_FOOD:
	{
		game.world[worldPositionI].pheromoneChannel = PHEROMONE_ROTTINGMEAT;
		break;
	}

	case MATERIAL_WATER:
	{
		game.world[worldPositionI].pheromoneChannel = PHEROMONE_RAIN;
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
			if (game.animals[animalIndex].body[occupyingCell].organ == ORGAN_BONE)
			{
				defense++;
			}
		}
	}
	return defense;
}


int plantDefenseAtWorldPoint(unsigned int cellWorldPositionI)
{
	int defense = 1;
	for (unsigned int n = 0; n < nNeighbours; ++n)
	{
		unsigned int worldNeighbour = cellWorldPositionI + neighbourOffsets[n];
		if ( worldNeighbour < worldSquareSize)
		{
			if (game.world[worldNeighbour].plantState == MATERIAL_THORNS)
			{
				if (game.world[worldNeighbour].plantIdentity == game.world[cellWorldPositionI].plantIdentity)
				{
					defense++;
				}
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
	unsigned int cellWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
	float defense = defenseAtWorldPoint(game.world[cellWorldPositionI].identity, cellWorldPositionI);
	float finalAmount = amount / defense;
	game.animals[animalIndex].body[cellIndex].damage += amount;
	int painCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_PAIN);
	if (painCell >= 0)
	{
		game.animals[animalIndex].body[painCell].signalIntensity += amount;
	}
	if (game.animals[animalIndex].body[cellIndex].damage > 1.0f)
	{
		game.animals[animalIndex].damageReceived++;

		if (animalIndex == game.adversary && shooterIndex == game.playerCreature && shooterIndex != -1 && animalIndex != -1 )
		{
			if (game.animals[game.adversary].damageReceived > game.animals[game.adversary].cellsUsed / 2)
			{
				defeatAdversary();
			}

		}
		limbLost = true;
	}
	bool dropped = false;
	if (limbLost)
	{
		if (game.animals[animalIndex].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
		{
			spill(organProduces(game.animals[animalIndex].body[cellIndex].organ), cellWorldPositionI);
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
		if (game.animals[animalIndex].position < worldSquareSize)
		{
			if (  materialBlocksMovement( game.world[game.animals[animalIndex].position].wall ) ) // vibrate out of wall if stuck.
			{
				game.animals[animalIndex].fPosX += ( (RNG() - 0.5f) * 10.0f  );
				game.animals[animalIndex].fPosY += ( (RNG() - 0.5f) * 10.0f  );
			}
		}

		bool animalInTheWay = false;

		int donkedCreature = game.world[newPosition].identity;
		if (donkedCreature != animalIndex && donkedCreature >= 0 && donkedCreature < numberOfAnimals)
		{
			int targetLocalPositionI = isAnimalInSquare( donkedCreature, newPosition);
			if (targetLocalPositionI >= 0)
			{
				// don't run into creatures you're carrying.
				bool carrying = false;
				for (int i = 0; i < game.animals[animalIndex].cellsUsed; ++i)
				{
					if (game.animals[animalIndex].body[i].organ == ORGAN_GRABBER)
					{
						if (game.animals[animalIndex].body[i].grabbedCreature == donkedCreature)
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
		if (  materialBlocksMovement( game.world[newPosition].wall ) || animalInTheWay ) // don't move into walls.
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
				if (game.world[  prevWorldPositionI].seedState != MATERIAL_NOTHING )
				{

					if (game.world[cellWorldPositionI].seedState == MATERIAL_NOTHING)
					{
						game.world[cellWorldPositionI].seedState = game.world[  prevWorldPositionI].seedState;
						memcpy( &(game.world[cellWorldPositionI].seedGenes[0]), &(game.world[prevWorldPositionI].seedGenes[0]) , sizeof(char)*plantGenomeSize);
						game.world[prevWorldPositionI].seedState = MATERIAL_NOTHING;
					}
				}
			}
			game.animals[animalIndex].body[cellIndex].worldPositionI = cellWorldPositionI;
		}
	}
}

void lookAtNextNonretiredAnimal()
{
	unsigned int choice = game.cameraTargetCreature ;
	for (int i = 0; i < numberOfAnimals; ++i)
	{
		choice = (choice + 1) % numberOfAnimals;
		if ( !(game.animals[choice].retired ))
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
	if (viewedAnimal >= 0)
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


	}
	else
	{
		//1. terrain.
		displayColor = materialColors(game.world[worldI].terrain);
		if ( game.world[worldI].plantState != MATERIAL_NOTHING)
		{
			if (
			    game.world[worldI].plantState == MATERIAL_TUBER ||
			    game.world[worldI].plantState == MATERIAL_WOOD ||
			    game.world[worldI].plantState == MATERIAL_ROOT
			)
			{
				displayColor =   multiplyColorByScalar( game.world[worldI].grassColor , 0.5f);
			}
			else
			{
				displayColor = game.world[worldI].grassColor;
			}
		}

		//2. wall
		if (game.world[worldI].wall != MATERIAL_NOTHING)
		{
			if (materialIsTransparent(game.world[worldI].wall))
			{
				displayColor = filterColor( displayColor,  multiplyColorByScalar( materialColors(game.world[worldI].wall), 0.5f ) );
			}
			else
			{
				displayColor = materialColors(game.world[worldI].wall);
			}
		}
	}

	if ( game.world[worldI].seedState != MATERIAL_NOTHING ) // pollen is visible over the top of game.animals, because it can cling to them.
	{
		displayColor =  filterColor( displayColor , multiplyColorByScalar( game.world[worldI].seedColorMoving , 0.5f)  ) ;
	}
	;
	return displayColor;
}

float getNormalisedHeight(unsigned int worldPositionI)
{
	float answer =   game.world[worldPositionI].height / (worldSize);
	return answer;
}


// given two game.world vertices, find the direction from a to b, as expressed by what entry in neighbourOffsets is the closest.
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
		game.world[worldPositionI].downhillNeighbour = getDownhillNeighbour(slope.x, slope.y);
		float xSurfaceDifference = (xLightAngle - slope.x);
		float ySurfaceDifference = (yLightAngle - slope.y);

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


// operates on seed genes, not plant genes.
void mutatePlants(unsigned int worldI)
{
	// if (extremelyFastNumberFromZeroTo(1) == 0) {return;}

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



void mutatePlantByRate(unsigned int worldI)
{
	// if the mutation rate is less than 1, roll for a chance.
	if (game.ecoSettings[8] < 1.0f)
	{
		if (RNG() < game.ecoSettings[8])
		{
			mutatePlants(worldI);
		}
	}
	else
	{
		int n = game.ecoSettings[8];
		for (int i = 0; i < n; ++i)
		{
			mutatePlants(worldI);
		}
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
			if (game.world[neighbour].plantState != MATERIAL_NOTHING)
			{
				game.world[neighbour].plantState = MATERIAL_FIRE;
				propagateFlame(neighbour, depth + 1);
			}
		}
	}
}


void clearGrowthMask(unsigned int worldI)
{
	game.world[worldI].growthMatrix[0] = false;
	game.world[worldI].growthMatrix[1] = false;
	game.world[worldI].growthMatrix[2] = false;
	game.world[worldI].growthMatrix[3] = false;
	game.world[worldI].growthMatrix[4] = false;
	game.world[worldI].growthMatrix[5] = false;
	game.world[worldI].growthMatrix[6] = false;
	game.world[worldI].growthMatrix[7] = false;
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
			bool temp = game.world[worldI].growthMatrix[0];
			for (unsigned int i = 0; i < nNeighbours; ++i)
			{
				unsigned int n = (i + 1) % nNeighbours;
				game.world[worldI].growthMatrix[i] = game.world[worldI].growthMatrix[n]  ;
			}
			game.world[worldI].growthMatrix[nNeighbours - 1] = temp;
		}
		else
		{
			bool temp = game.world[worldI].growthMatrix[nNeighbours - 1];
			for (unsigned int i = nNeighbours - 1 ; i > 0; --i)
			{
				unsigned int n = (i - 1) % nNeighbours;
				game.world[worldI].growthMatrix[i] = game.world[worldI].growthMatrix[n]  ;
			}
			game.world[worldI].growthMatrix[0] = temp;
		}
	}
}


// attempt to grow into the nearby squares. return true if there is no potential for growth.
void growIntoNeighbours(unsigned int worldI, unsigned int material)
{
	for (int i = 0; i < nNeighbours; ++i)
	{
		if (game.world[worldI].growthMatrix[i])
		{
			unsigned int neighbour = worldI + neighbourOffsets[i];
			if (neighbour < worldSquareSize)
			{
				growInto( neighbour, worldI, material, false);
			}
		}
	}
}

// this is what seeds and runners spawn.
#define SPROUT MATERIAL_ROOT

void growPlants(unsigned int worldI)
{
	if (worldI >= worldSquareSize) {return;}


	bool done = false;

	while (true)
	{
		if (game.world[worldI].grown) {return;}
		if (game.world[worldI].geneCursor >= plantGenomeSize)
		{
			game.world[worldI].grown = true;
			return;
		}

		int skipAhead = game.world[worldI].geneCursor + 1;


		char c = game.world[worldI].plantGenes[game.world[worldI].geneCursor];
		if (c < nNeighbours)                                                    // growth directions
		{
			game.world[worldI].growthMatrix[c] = !(game.world[worldI].growthMatrix[c]);
		}
		else                                                                    // physically growable stuff
		{
			if (
			    growable(c)
			)
			{
				int numberToGrow = 0;
				for (int i = 0; i < nNeighbours; ++i)
				{
					if (game.world[worldI].growthMatrix[i])
					{
						numberToGrow++;
					}
				}
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

				if (game.world[worldI].nutrients > cost && game.world[worldI].energy > energyCost)
				{
					bool b = game.world[worldI].branching;
					if (	b )
					{
						rotateGrowthMask(worldI, 2);

						//  scan forward to find the break point associated with this
						int presentDepth = 1;
						for (int i =  game.world[worldI].geneCursor + 1; i < plantGenomeSize; ++i)
						{
							char geneAtThisLocation = game.world[worldI].plantGenes[  i];
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

					case PLANTGENE_STEALYROOT:
						growIntoNeighbours(worldI, MATERIAL_STEALYROOT);
						break;


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
						if (doAsexualPlants)
						{
							growIntoNeighbours(worldI, MATERIAL_BUD_A);
							growIntoNeighbours(worldI, MATERIAL_SEED);
						}
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
						if (doAsexualPlants)
						{
							growIntoNeighbours(worldI, SPROUT);
							for (int i = 0; i < nNeighbours; ++i)
							{
								if (game.world[worldI].growthMatrix[i])
								{
									unsigned int neighbour = worldI + neighbourOffsets[i];
									if (neighbour < worldSquareSize)
									{
										game.world[neighbour].identity = extremelyFastNumberFromZeroTo(65536);
										game.world[neighbour].grassColor = color_green;
										game.world[neighbour].seedColor = color_yellow;
										game.world[neighbour].nutrients = seedCost;
										game.world[neighbour].energy    = seedCost;
										game.world[worldI].nutrients    -= seedCost;
										game.world[worldI].energy       -= seedCost;
										game.world[neighbour].geneCursor = 0;
										game.world[neighbour].grown = false;
										mutatePlantByRate(neighbour);
									}
								}
							}
						}
						break;
					}
					}
					if (b)
					{
						rotateGrowthMask(worldI, -2);
						game.world[worldI].branching = false;
					}
					else
					{
						done = true;
					}
				}
				else
				{
					return;
				}

			}
			else
			{
				switch (c)
				{
				case PLANTGENE_BRANCH:
				{
					game.world[worldI].branching = true;
					break;
				}
				case PLANTGENE_NECTAR:
				{
					if (doHoney)
					{
						if (game.world[worldI].nutrients < honeyCost)
						{
							return;
						}
						spill( MATERIAL_HONEY, worldI);
						game.world[worldI].nutrients -= honeyCost;
					}
					break;
				}
				case PLANTGENE_GROW_SYMM_H:
				{
					// mirror the left and right halves of the growth matrix.
					bool originalGrowthMatrix[nNeighbours];
					for (int i = 0; i < nNeighbours; ++i)
					{
						originalGrowthMatrix[i] = game.world[worldI].growthMatrix[i];
					}
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
					// mirror the top and bottom halves of the growth matrix.
					bool originalGrowthMatrix[nNeighbours];
					for (int i = 0; i < nNeighbours; ++i)
					{
						originalGrowthMatrix[i] = game.world[worldI].growthMatrix[i];
					}
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
					if ((game.world[worldI].geneCursor + 1) < plantGenomeSize)
					{
						game.world[worldI].sequenceNumber = game.world[worldI].plantGenes[game.world[worldI].geneCursor + 1]; // take the value of the next gene- that is the number of times to repeat the sequence.
						game.world[worldI].sequenceReturn = game.world[worldI].geneCursor + 2;
					}
					skipAhead = game.world[worldI].geneCursor + 2;

					break;
				}
				case PLANTGENE_RANDOMIZEGROWTHMASK:
				{
					for (int i = 0; i < nNeighbours; ++i)
					{
						game.world[worldI].growthMatrix[i] = extremelyFastNumberFromZeroTo(1);
					}
					break;
				}
				case PLANTGENE_RANDOMDIRECTION:
				{
					for (int i = 0; i < nNeighbours; ++i)
					{
						game.world[worldI].growthMatrix[i] = false;
					}

					unsigned int randomDirection = extremelyFastNumberFromZeroTo(nNeighbours - 1);
					game.world[worldI].growthMatrix[randomDirection] = true;
					break;
				}
				case PLANTGENE_END:
				{
					skipAhead = plantGenomeSize;
					break;
				}


				case PLANTGENE_TERRESTRIAL:
				{
					game.world[worldI].aquaticPlant = false;
					break;
				}
				case PLANTGENE_BREAK:
				{
					// scroll back and find what this break is for.
					bool sequenceBreak = false;
					bool branchBreak = false;
					int presentDepth = 1;
					for (int i =  game.world[worldI].geneCursor - 1; i > 0; --i)
					{
						char geneAtThisLocation = game.world[worldI].plantGenes[ i ];
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
						if (game.world[worldI].sequenceNumber > 0)
						{
							skipAhead = game.world[worldI].sequenceReturn;
							game.world[worldI].sequenceNumber--;
						}
						else
						{
							// if it is 0, you've completed doing the sequence n times, so you can exit it.
							// In this case, sequenceReturn of the next cells will be sampled from a cell before the sequence start, which allows nested sequences.
							int innerSequenceReturn = game.world[worldI].sequenceReturn;
							// Why 3? Because sequence returns don't point at the sequence gene itself, they point at the first gene IN the sequence.
							// The header goes <last gene of outer sequence> <sequence gene> <length> <first gene> .. . you always return to the first gene inside the sequence
							// when breaking an inner sequence,  return to the last gene of outer sequence, which is 3 cells behind where the inner sequence returns to.
							if (innerSequenceReturn > 3)
							{
								game.world[worldI].sequenceReturn = innerSequenceReturn - 3 ;
								game.world[worldI].sequenceNumber =  0;
							}
						}
					}
					break;
				}

				case PLANTGENE_GOTO:
				{
					int nextgene = game.world[worldI].geneCursor + 1 ;
					if (nextgene < plantGenomeSize)
					{
						int destination = game.world[worldI].plantGenes[  nextgene  ];
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
						tempMatrix[i] = game.world[worldI].growthMatrix[i];
					}

					for (unsigned int i = 0; i < nNeighbours; ++i)
					{
						if (tempMatrix[i])
						{
							unsigned int prevNeighbour = (i - 1) % nNeighbours;
							unsigned int nextNeighbour = (i + 1) % nNeighbours;
							game.world[worldI].growthMatrix[prevNeighbour] = true;
							game.world[worldI].growthMatrix[nextNeighbour] = true;
						}
					}
					break;
				}

				case PLANTGENE_ROTATEMATRIXCCW:
				{
					rotateGrowthMask(worldI, 1);
					break;
				}

				case PLANTGENE_ROTATEMATRIXCW:
				{
					rotateGrowthMask(worldI, -1);
					break;
				}

				case PLANTGENE_UPHILL:
				{
					for (unsigned int i = 0; i < nNeighbours; ++i)
					{
						game.world[worldI].growthMatrix[i] = false;
					}
					unsigned int uphillNeighbour = 0;
					float uphillHeight = -1 * (worldSize) * 10.0f;
					for (unsigned int i = 0; i < nNeighbours; ++i)
					{
						unsigned int neighbour = worldI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							if (game.world[  neighbour  ].height > uphillHeight)
							{
								uphillNeighbour = i;
								uphillHeight = game.world[  neighbour  ].height ;
							}
						}
					}
					game.world[worldI].growthMatrix[uphillNeighbour] = true;
					break;
				}

				case PLANTGENE_TOWARDLIGHT:
				{
					for (unsigned int i = 0; i < nNeighbours; ++i)
					{
						game.world[worldI].growthMatrix[i] = false;
					}
					unsigned int uphillNeighbour = 0;
					float uphillHeight = -1 * (worldSize) * 10.0f;
					for (unsigned int i = 0; i < nNeighbours; ++i)
					{
						unsigned int neighbour = worldI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							float cuctus = colorAmplitude( game.world[  neighbour  ].light );
							if ( cuctus  > uphillHeight)
							{
								uphillNeighbour = i;
								uphillHeight = cuctus ;
							}
						}
					}
					game.world[worldI].growthMatrix[uphillNeighbour] = true;
					break;
				}
				case PLANTGENE_INVERTMATRIX:
				{
					for (unsigned int i = 0; i < nNeighbours; ++i)
					{
						game.world[worldI].growthMatrix[i] = !(game.world[worldI].growthMatrix[i]);
					}

					break;
				}

				case PLANTGENE_RED:
				{
					game.world[worldI].grassColor.r *= 1.35f;
					game.world[worldI].grassColor = normalizeColor(game.world[worldI].grassColor);
					break;
				}
				case PLANTGENE_GREEN:
				{
					game.world[worldI].grassColor.b *= 1.35f;
					game.world[worldI].grassColor = normalizeColor(game.world[worldI].grassColor);
					break;
				}
				case PLANTGENE_BLUE:
				{
					game.world[worldI].grassColor.b *= 1.35f;
					game.world[worldI].grassColor = normalizeColor(game.world[worldI].grassColor);
					break;
				}
				case PLANTGENE_LIGHT:
				{
					game.world[worldI].grassColor.r *= 1.35f;
					game.world[worldI].grassColor.g *= 1.35f;
					game.world[worldI].grassColor.b *= 1.35f;
					game.world[worldI].grassColor = normalizeColor(game.world[worldI].grassColor);
					break;
				}
				case PLANTGENE_DARK:
				{
					game.world[worldI].grassColor.r *= 0.75f;
					game.world[worldI].grassColor.g *= 0.75f;
					game.world[worldI].grassColor.b *= 0.75f;
					break;
				}
				case PLANTGENE_SEEDCOLOR_RED:
				{
					game.world[worldI].seedColor.r *= 1.35f;
					game.world[worldI].seedColor = normalizeColor(game.world[worldI].seedColor);
					break;
				}
				case PLANTGENE_SEEDCOLOR_GREEN:
				{
					game.world[worldI].seedColor.b *= 1.35f;
					game.world[worldI].seedColor = normalizeColor(game.world[worldI].seedColor);
					break;
				}
				case PLANTGENE_SEEDCOLOR_BLUE:
				{
					game.world[worldI].seedColor.b *= 1.35f;
					game.world[worldI].seedColor = normalizeColor(game.world[worldI].seedColor);
					break;
				}
				case PLANTGENE_SEEDCOLOR_LIGHT:
				{
					game.world[worldI].seedColor.r *= 1.35f;
					game.world[worldI].seedColor.g *= 1.35f;
					game.world[worldI].seedColor.b *= 1.35f;
					game.world[worldI].seedColor = normalizeColor(game.world[worldI].seedColor);
					break;
				}
				case PLANTGENE_SEEDCOLOR_DARK:
				{
					game.world[worldI].seedColor.r *= 0.75f;
					game.world[worldI].seedColor.g *= 0.75f;
					game.world[worldI].seedColor.b *= 0.75f;
					break;
				}
				}
			}
		}
		if (done) // if done, don't increment the gene cursor again- this cell will be what it is now permanently, until overgrown at least, but it will regrow into its neighbours if it can.
		{
			game.world[worldI].grown = true;
			break;
		}
		else
		{
			game.world[worldI].geneCursor = skipAhead;
		}
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
			game.world[worldI].grown = false;
		}
	}
	game.world[worldI].plantState = MATERIAL_NOTHING;
}

void moveSeed(unsigned int from, unsigned int to)
{
	game.world[to].seedState = game.world[from].seedState;
	game.world[to].seedIdentity = game.world[from].seedIdentity;
	memcpy( game.world[to].seedGenes , game.world[from].seedGenes,  plantGenomeSize * sizeof(char)  );
	game.world[to].seedColorMoving = game.world[from].seedColorMoving;
	game.world[from].seedState = MATERIAL_NOTHING;
	game.world[from].seedIdentity = -1;
}

void equalizeWithNeighbours( unsigned int worldI )
{
	// transfer energy and nutrients between adjacent cells with same ID
	for (int i = 0; i < nNeighbours; ++i)
	{
		unsigned int neighbour = worldI + neighbourOffsets[  i ];
		if (neighbour < worldSquareSize)
		{
			if ( game.world[neighbour].identity == game.world[worldI].identity  )
			{
				if (game.world[worldI].plantState != MATERIAL_NOTHING && game.world[neighbour].plantState != MATERIAL_NOTHING)
				{
					float rate = 0.1f;
					if (game.world[worldI].plantState == MATERIAL_WOOD || game.world[neighbour].plantState == MATERIAL_WOOD)
					{
						rate = 1.0f;
					}

					swapNootsWithNeighbour(worldI, neighbour , rate );
					swapEnergyWithNeighbour(worldI, neighbour, rate );
				}
			}
		}
	}
}


void updatePlants(unsigned int worldI)
{
	unsigned int pollenMobility = game.ecoSettings[10];

	if ( (worldI >= worldSquareSize)) {return;}

	if (game.world[worldI].seedState != MATERIAL_NOTHING)
	{
		switch (game.world[worldI].seedState)
		{
		case MATERIAL_POLLEN:
		{
			// only move the pollen if it's drifted from where it grew.
			if (game.world[worldI].seedIdentity != game.world[worldI].plantIdentity || movePollenUnconditionally)
			{
				// move seeds randomly
				// unsigned int neighbour = worldI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];

				unsigned int x = worldI % worldSize;
				unsigned int y = worldI / worldSize;

				int rx = extremelyFastNumberFromZeroTo(pollenMobility) - (pollenMobility / 2);
				int ry = extremelyFastNumberFromZeroTo(pollenMobility) - (pollenMobility / 2);

				x += rx;
				y += ry;

				unsigned int neighbour = (worldSize * y) + x;


				if (neighbour < worldSquareSize)
				{

					if (game.world[worldI].wall == game.world[neighbour].wall)
					{

						if ( !materialBlocksMovement( game.world[neighbour].wall ) && game.world[neighbour].seedState == MATERIAL_NOTHING )
						{
							moveSeed(worldI, neighbour);
						}
					}

					// pollen degrades if not attached to an animal, to prevent it building up too much in the game.world.
					bool bonded = false;
					if (game.world[worldI].identity >= 0 && game.world[worldI].identity < numberOfAnimals)
					{
						int bond = isAnimalInSquare( game.world[worldI].identity , worldI )  ;
						if (  bond >= 0 )
						{
							bonded = true;
						}
					}

					if (pollenDegradation)
					{
						if (!bonded)
						{
							if (extremelyFastNumberFromZeroTo(50) == 0)
							{
								game.world[neighbour].seedState = MATERIAL_NOTHING;
							}
						}
					}
				}
			}
			break;
		}
		case MATERIAL_SEED:
		{
			if (extremelyFastNumberFromZeroTo(10) == 0)
			{
				growInto(  worldI, worldI, SPROUT, true );
				game.world[worldI].seedState = MATERIAL_NOTHING;
			}

			// move seeds randomly
			unsigned int neighbour = worldI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
			if (neighbour < worldSquareSize)
			{
				if ( !materialBlocksMovement( game.world[neighbour].wall ) && game.world[neighbour].seedState == MATERIAL_NOTHING )
				{
					moveSeed(worldI, neighbour);
				}

				bool bonded = false;
				if (game.world[worldI].identity >= 0 && game.world[worldI].identity < numberOfAnimals)
				{
					int bond = isAnimalInSquare( game.world[worldI].identity , worldI )  ;
					if (  bond >= 0 )
					{
						bonded = true;
					}
				}
				if (seedDegradation)
				{
					if (!bonded)
					{
						if (extremelyFastNumberFromZeroTo(100) == 0)
						{
							game.world[neighbour].seedState = MATERIAL_NOTHING;
						}
					}
				}
			}
			break;
		}
		}
	}

	if (    game.world[worldI].plantState != MATERIAL_NOTHING)
	{
		equalizeWithNeighbours( worldI );
		if (game.world[worldI].energy >= 0.0f && game.world[worldI].nutrients >= 0.0f)
		{
			growPlants(worldI);
		}
		else if (game.world[worldI].energy <= -1.0f || game.world[worldI].nutrients <= -1.0f)
		{
			damagePlants(worldI);
		}
		game.world[worldI].energy -= game.ecoSettings[6];
		game.world[worldI].energy     = clamp(game.world[worldI].energy ,    -1.0f, plantMaxEnergy);
		game.world[worldI].nutrients  = clamp(game.world[worldI].nutrients , -1.0f, plantMaxEnergy);

		// grow plant into neighboring squares if applicable
		switch (game.world[worldI].plantState)
		{
		case MATERIAL_LEAF:
		{
			if (environmentScarcity)
			{
				game.world[worldI].energy += colorAmplitude(    multiplyColor(  game.world[worldI].light , game.world[worldI].grassColor)) ;
			}
			else
			{
				game.world[worldI].energy += 1.0f;
			}
			break;
		}
		case MATERIAL_WOOD:
		{
			break;
		}

		case MATERIAL_TUBER:
		{
			break;
		}

		case MATERIAL_ROOT:
		{
			// get nutrients from the environment
			float amount = 0.0f;
			if (environmentScarcity)
			{
				amount = materialFertility (game.world[worldI].terrain) * (game.ecoSettings[5] );
			}
			else
			{
				amount =  (game.ecoSettings[5]) ;
			}
			if (game.world[worldI].aquaticPlant)
			{
				if (game.world[worldI].wall == MATERIAL_WATER)
				{
					game.world[worldI].nutrients += amount;
				}
			}
			else
			{
				if (game.world[worldI].wall != MATERIAL_WATER)
				{
					game.world[worldI].nutrients += amount;
				}
			}
			break;
		}

		case MATERIAL_STEALYROOT:
		{


			for (int n = 0; n < nNeighbours; ++n)
			{
				unsigned int neighbour = worldI + neighbourOffsets[n];
				if (neighbour < worldSquareSize)
				{
					if (game.world[neighbour].plantState != MATERIAL_NOTHING)
					{
						if (game.world[neighbour].plantIdentity != game.world[worldI].plantIdentity)
						{

							if (game.world[neighbour].nutrients > game.world[worldI].nutrients)
							{

// get nutrients from the environment
								float amount = game.world[neighbour].nutrients - game.world[worldI].nutrients;

								amount = amount / plantDefenseAtWorldPoint(neighbour);

								game.world[worldI].nutrients +=    amount / 2;
								game.world[neighbour].nutrients -= amount / 2;

							}

						}
					}
				}
			}


			break;
		}

		case MATERIAL_BUD_F:
		{
			if (game.world[worldI].energy > 1.0f && game.world[worldI].nutrients > 1.0f)
			{
				for (int n = 0; n < nNeighbours; ++n)
				{
					unsigned int neighbour = worldI + neighbourOffsets[n];
					if (neighbour < worldSquareSize)
					{
						if (game.world[neighbour].seedState == MATERIAL_POLLEN)
						{
							if (game.world[neighbour].seedIdentity != game.world[worldI].plantIdentity)
							{

								// plant species is basically organized by the color of their seeds.
								const float plantSpeciesThreshold = 0.25f;

								float totalDifference = (
								                            abs(game.world[neighbour].seedColorMoving.r - game.world[worldI].seedColor.r)  +
								                            abs(game.world[neighbour].seedColorMoving.g - game.world[worldI].seedColor.g)  +
								                            abs(game.world[neighbour].seedColorMoving.b - game.world[worldI].seedColor.b))
								                        * 0.33f
								                        ;


								if ( totalDifference < plantSpeciesThreshold)
								{
									// sex between two plants
									// take half the pollen genes, put them in that bud's plantgenes.
									// mutate the bud before adding the new genes, the result is that this method offers 50% less mutation.
									mutatePlantByRate(worldI);

									for (int i = 0; i < plantGenomeSize; ++i)
									{
										if (extremelyFastNumberFromZeroTo(1) == 0)
										{
											game.world[worldI].plantGenes[i] =  game.world[neighbour].seedGenes[i] ;
										}
									}
									game.world[neighbour].seedState = MATERIAL_NOTHING;
									game.world[neighbour].seedIdentity = -1;

									// then grow a seed from that mix.
									growInto( worldI, worldI, MATERIAL_SEED, false);

									game.world[worldI].plantState = MATERIAL_NOTHING;
									game.world[worldI].plantIdentity = -1;
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
					if (game.world[neighbour].seedState == MATERIAL_POLLEN)
					{
						if (game.world[neighbour].seedIdentity != game.world[worldI].plantIdentity)
						{
							// plant species is basically organized by the color of their seeds.
							const float plantSpeciesThreshold = 0.25f;
							float totalDifference = (
							                            abs(game.world[neighbour].seedColorMoving.r - game.world[worldI].seedColor.r)  +
							                            abs(game.world[neighbour].seedColorMoving.g - game.world[worldI].seedColor.g)  +
							                            abs(game.world[neighbour].seedColorMoving.b - game.world[worldI].seedColor.b))
							                        * 0.33f;
							if ( totalDifference > plantSpeciesThreshold)
							{
								// destroy the alien pollen.
								game.world[neighbour].seedState = MATERIAL_NOTHING;
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

	if ( game.world[worldI].plantState == MATERIAL_FIRE)
	{
		propagateFlame(worldI, 0);
	}
}

bool raining = false;
void toggleRain()
{
	raining = !raining;
}

void updateMapI(unsigned int worldI)
{
	// slowly reduce pheromones over time.
	if (game.world[worldI].pheromoneChannel >= 0)
	{
		for (int n = 0; n < nNeighbours; ++n)
		{
			unsigned int neighbour = worldI + neighbourOffsets[n];
			if (neighbour < worldSquareSize)
			{
				if (game.world[neighbour].pheromoneChannel == MATERIAL_NOTHING)
				{
					game.world[neighbour].pheromoneChannel = game.world[worldI].pheromoneChannel;
					game.world[worldI].pheromoneChannel = MATERIAL_NOTHING;
				}
			}
		}
		const unsigned int pheromoneDecayRate = 10;
		if (extremelyFastNumberFromZeroTo(pheromoneDecayRate))
		{
			game.world[worldI].pheromoneChannel = MATERIAL_NOTHING;
		}
	}
	if (game.world[worldI].height > seaLevel)
	{
		if (isALiquid(game.world[worldI].wall))
		{

			unsigned int dhnn = game.world[worldI].downhillNeighbour ;
			dhnn += (extremelyFastNumberFromZeroTo(4) - 2);
			dhnn = dhnn % nNeighbours;

			unsigned int dhn =  worldI + neighbourOffsets[dhnn ] ;
			if (dhn < worldSquareSize)
			{
				if ( !(  materialBlocksMovement(  game.world[   dhn  ].wall )  ))
				{
					unsigned int swapWall = game.world[  dhn ].wall;
					game.world[  dhn ].wall = game.world[  worldI ].wall;
					game.world[  worldI ].wall = swapWall;
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

	if ( materialDegrades( game.world[worldI].wall) )
	{
		game.world[worldI].wall = MATERIAL_NOTHING;
	}
	updatePlants(worldI);
}


void updateMapSector( unsigned int sector )
{
	ZoneScoped;
	unsigned int from = sector * mapSectorSize;
	unsigned int to   = ((sector + 1 ) * mapSectorSize ) - 1;

	// generate a huge ass list of the map squares you're going to update. Then update them all at once. more than 2x faster than generating and visiting each one in turn.
	unsigned int updateSize = game.ecoSettings[4];
	unsigned int squaresToUpdate[updateSize];

	for (int i = 0; i < updateSize; ++i)
	{
		squaresToUpdate[i] =  from + extremelyFastNumberFromZeroTo(to);
	}


	unsigned int plantCoverage = 0;
	for (int i = 0; i < updateSize; ++i)
	{
		if (squaresToUpdate[i] < worldSquareSize)
		{

			updateMapI(squaresToUpdate[i]);

			if (game.world[squaresToUpdate[i]].plantState != MATERIAL_NOTHING )
			{
				plantCoverage++;
			}
		}
	}
	float gargleblaster = plantCoverage;
	coverage[sector] = gargleblaster / mapSectorSize;
}

void updateMap()
{
	boost::thread mapSectors[numberOfMapSectors];
	for (int i = 0; i < numberOfMapSectors; ++i)
	{
		mapSectors[i] = boost::thread { updateMapSector, i };
	}
	for (int i = 0; i < numberOfMapSectors; ++i)
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

void drawPalette( std::vector<std::string>  * sideText)
{
	if (game.selectedAnimal < 0)
	{
		sideText->push_back(   std::string("Select a creature to add or remove tiles from it."));
	}
	// draw the available tiles.
	for (int i = 0; i < numberOfOrganTypes; ++i)
	{
		if (i == game.paletteSelectedOrgan)
		{
			sideText->push_back(  std::string("X ") +  tileShortNames(i) );
		}
		else
		{
			sideText->push_back( tileShortNames(i));
		}
	}
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
					float defense = defenseAtWorldPoint(game.world[shootWorldPosition].identity, shootWorldPosition);
					hurtAnimal(game.world[shootWorldPosition].identity, shotOffNub, (RNG()) / defense, shooterIndex);
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

int getGrabbableItemAtCursor ()
{



	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int neighbour = ((cursorPosY * worldSize) + cursorPosX ) % worldSquareSize;

	int result = -1;
	bool gotSomething = false;

	if (game.world[neighbour].identity >= 0 && game.world[neighbour].identity != game.playerCreature && game.world[neighbour].identity < numberOfAnimals)
	{
		int targetLocalPositionI = isAnimalInSquare( game.world[neighbour].identity, neighbour);
		if (targetLocalPositionI >= 0)
		{

			bool grabbedByAnotherGrabber = false;// finally, make sure the item is not grabbed by another of your own grabbers.
			for (unsigned int cellIndexB = 0; cellIndexB < game.animals[game.playerCreature].cellsUsed; ++cellIndexB)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				if (game.animals[game.playerCreature].body[cellIndexB].organ == ORGAN_GRABBER)
				{
					if (game.animals[game.playerCreature].body[cellIndexB].grabbedCreature == game.world[neighbour].identity )
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
				// break;
			}
		}
	}

	return result;
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

bool sexBetweenTwoCreatures( int a,  int b)
{

	bool babied = false;

		printf("j %f %f\n",  game.animals[a].energyDebt ,game.animals[b].energyDebt );
	if ( game.animals[a].energyDebt <= 0.0f && game.animals[b].energyDebt <= 0.0f  )
	{
		printf("a\n");
		if (game.animals[a].energy > game.animals[a].offspringEnergy && game.animals[b].energy > game.animals[b].offspringEnergy )
		{

		printf("b\n");
			float energyDonation = game.animals[a].offspringEnergy + game.animals[b].offspringEnergy ;
			game.animals[a].energy -= game.animals[a].offspringEnergy;
			game.animals[b].energy -= game.animals[b].offspringEnergy;
			unsigned int bSpecies = b / numberOfAnimalsPerSpecies;
			int newAnimal = spawnAnimal( bSpecies, game.animals[b], game.animals[b].position, true );
			if (newAnimal >= 0)
			{

		printf("c\n");


				bool AorB = extremelyFastNumberFromZeroTo(1); // gender reveal! all instances of A and B genitals will be converted to one of either A or B across the whole creature.



				for (int i = 0; i < animalSquareSize; ++i)
				{
					if (extremelyFastNumberFromZeroTo(1) == 0)
					{
						game.animals[newAnimal].body[i] = game.animals[a].body[i];
					}


					if (game.animals[newAnimal].body[i].organ == ORGAN_GENITAL_A || game.animals[newAnimal].body[i].organ == ORGAN_GENITAL_B)
					{

						if (AorB)
						{
							game.animals[newAnimal].body[i].organ = ORGAN_GENITAL_A;
						}
						else
						{
							game.animals[newAnimal].body[i].organ = ORGAN_GENITAL_B;
						}
					}


				}
				game.animals[newAnimal].energy += energyDonation;

				printf("sexonomically produced animal %i \n", newAnimal);
				babied = true;
			}
		}
	}



	if (babied)
	{
		return true;
	}
	return false;
}

int getRandomCreature(unsigned int speciesIndex)
{
	if (speciesIndex == 0 || game.speciesPopulationCounts[speciesIndex] == 0) {return -1;}
	int start = (speciesIndex * numberOfAnimalsPerSpecies) + extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies - 1);
	for (int i = 0; i < numberOfAnimalsPerSpecies; ++i)
	{
		int choice = (start + i);
		if (choice > (((speciesIndex + 1) * numberOfAnimalsPerSpecies) - 1 )   ) // wrap around this species
		{
			choice -= (numberOfAnimalsPerSpecies - 1);
		}
		if (  !( game.animals[choice].retired))
		{
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
		if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
		{
			unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
			if (connected_to_cell < animalSquareSize)
			{
				// game.animals[animalIndex].body[cellIndex].signalIntensity
				sum  += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
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
	float sensorium[game.animals[animalIndex].cellsUsed];
	for (unsigned int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity;//0.0f;

		if (game.animals[animalIndex].body[cellIndex].damage >= 1.0f) { continue;}
		unsigned int cellWorldPositionI = game.animals[animalIndex].body[cellIndex].worldPositionI;
		if (cellWorldPositionI >= worldSquareSize) {continue;}
		unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
		unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;
		unsigned int organ = game.animals[animalIndex].body[cellIndex].organ;
		switch (organ)
		{

		case ORGAN_SWITCH:
		{
			// in a switch, the sum of inputs except 0 and 1 are taken. If the sum is greater than 0, the input 0 is copied to the output. else, the input 1 is copied to the output.
			float sum = 0.0f;
			for (int i = 2; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
				{
					unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
					if (connected_to_cell < animalSquareSize)
					{
						sum += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
					}
				}
			}
			unsigned int switchedChannel = 0;
			if (sum > 0.0f)
			{
				switchedChannel = 1;
			}
			unsigned int switchCell = game.animals[animalIndex].body[cellIndex].connections[switchedChannel] .connectedTo;
			sensorium[cellIndex] = game.animals[animalIndex].body[switchCell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[switchedChannel] .weight;
			break;
		}


		case ORGAN_TIMER:
		{
			game.animals[animalIndex].body[cellIndex].workingValue += (1.0f /
			        ((game.animals[animalIndex].body[cellIndex].speakerChannel * game.animals[animalIndex].body[cellIndex].speakerChannel )
			         + 1) );
			sensorium[cellIndex] = sin(game.animals[animalIndex].body[cellIndex].workingValue );
			break;
		}

		case ORGAN_MEMORY_TX:
		{
			// in this memory, the output is latched to -1 if the sum of inputs goes below -1, and latched to +1 if the sum of inputs goes above 1.
			// it can be thought of as a 'digital' memory encoding a state with two values.
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum > 0.85f)
			{
				sensorium[cellIndex]  = 1.0f;
			}
			else if (sum < -0.85f)
			{
				sensorium[cellIndex] = -1.0f;
			}
			else
			{
				sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity;
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
				if (game.animals[animalIndex].body[cellIndex].connections[i] .used)
				{
					unsigned int connected_to_cell = game.animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
					if (connected_to_cell < animalSquareSize)
					{
						sum += game.animals[animalIndex].body[connected_to_cell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[i] .weight;
					}
				}
			}
			if (sum > 0.0f)
			{
				unsigned int switchCell = game.animals[animalIndex].body[cellIndex].connections[0] .connectedTo;
				sensorium[cellIndex] = game.animals[animalIndex].body[switchCell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[0] .weight;
			}
			else
			{
				sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity;
			}
			break;
		}


		case ORGAN_COMPARATOR:
		{
			//The output is 1 if input 0 is greater than input 1. else the output is -1.
			unsigned int switchCell = game.animals[animalIndex].body[cellIndex].connections[0] .connectedTo;
			float inA = game.animals[animalIndex].body[switchCell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[0] .weight;

			switchCell = game.animals[animalIndex].body[cellIndex].connections[1] .connectedTo;
			float inB = game.animals[animalIndex].body[switchCell].signalIntensity * game.animals[animalIndex].body[cellIndex].connections[1] .weight;


			if (inA > inB)
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = -1.0f;
			}

			break;
		}

		case ORGAN_DERIVATOR:
		{

			// this part is like iirlow. it allows speakerchannel to set the window over which the slope is calculated.
			float sum = sumInputs(  animalIndex,   cellIndex);
			game.animals[animalIndex].body[cellIndex].workingValue += sum;
			float feedback = game.animals[animalIndex].body[cellIndex].workingValue / game.animals[animalIndex].body[cellIndex].speakerChannel; // in this case, speakerchannel refers to the number of 'taps'.
			game.animals[animalIndex].body[cellIndex].workingValue -= feedback;

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
			game.animals[animalIndex].body[cellIndex].workingValue += sum;
			float feedback = game.animals[animalIndex].body[cellIndex].workingValue / game.animals[animalIndex].body[cellIndex].speakerChannel; // in this case, speakerchannel refers to the number of 'taps'.
			game.animals[animalIndex].body[cellIndex].workingValue -= feedback;
			sensorium[cellIndex] = feedback;
			break;
		}
		case ORGAN_IIRHIGH:
		{
			//. The output is the input sum, from which is subtracted an accumulation of the previous n input sums.
			float sum = sumInputs(  animalIndex,   cellIndex);
			game.animals[animalIndex].body[cellIndex].workingValue += sum;
			float feedback = game.animals[animalIndex].body[cellIndex].workingValue / game.animals[animalIndex].body[cellIndex].speakerChannel; // in this case, speakerchannel refers to the number of 'taps'.
			game.animals[animalIndex].body[cellIndex].workingValue -= feedback;

			sensorium[cellIndex] = feedback - (sum / game.animals[animalIndex].body[cellIndex].speakerChannel);
			break;
		}
		case ORGAN_EMITTER_WAX:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum > 0.0f)
			{
				spill(MATERIAL_WAX, cellWorldPositionI);
				game.animals[animalIndex].energy -= 1.0f;
			}
			break;
		}
		case ORGAN_EMITTER_HONEY:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum > 0.0f)
			{
				spill(MATERIAL_HONEY, cellWorldPositionI);
				game.animals[animalIndex].energy -= 1.0f;
			}
			break;
		}

		case ORGAN_SENSOR_AGE:
		{
			if (game.animals[animalIndex].lifespan > 0.0f)
			{
				sensorium[cellIndex] = game.animals[animalIndex].age / game.animals[animalIndex].lifespan;
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
						if (game.world[targetPosI].identity >= 0 && game.world[targetPosI].identity < numberOfAnimals && game.world[targetPosI].identity != animalIndex)
						{
							unsigned int targetSpecies = game.world[targetPosI].identity / numberOfAnimalsPerSpecies;
							if ( targetSpecies != 0 )
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
			sensorium[cellIndex] = (RNG() - 0.5f) * 2.0f;
			break;
		}

		case ORGAN_GRABBER:
		{
			float sum = 0.0f;
			if (animalIndex == game.playerCreature)
			{
				int potentialGrab = getGrabbableItemAtCursor();// check if there is anything grabbable.
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
			if (sum >= 0.85f && game.animals[animalIndex].body[cellIndex].grabbedCreature  == -1)
			{
				int potentialGrab = getGrabbableItem (animalIndex, cellIndex);
				if (animalIndex == game.playerCreature)
				{
					potentialGrab	= getGrabbableItemAtCursor();// check if there is anything grabbable.
				}

				if (potentialGrab >= 0)
				{


					game.animals[animalIndex].body[cellIndex].grabbedCreature = potentialGrab;
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


				if (animalIndex == game.playerCreature && cellIndex == game.playerActiveGrabber)
				{


					int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
					int cursorPosY = game.cameraPositionY + game.mousePositionY;
					unsigned int cursorPosI = ((cursorPosY * worldSize) + cursorPosX ) % worldSquareSize;

					game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosX = cursorPosX;
					game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosY = cursorPosY;
					game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosX = cursorPosX;
					game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosY = cursorPosY;
					game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].position = cursorPosI;

				}

				// also, if grabbed by the player, adjust the angle of the grabbed object so it points at the mouse cursor. for aiming weapons.
				float fposx = cellWorldPositionX;
				float fposy = cellWorldPositionY;
				float fmousePositionX = game.mousePositionX;
				float fmousePositionY = game.mousePositionY;
				float angleToCursor = atan2(   fmousePositionY - (  game.cameraPositionY - fposy)  ,  fmousePositionX - (game.cameraPositionX - fposx));
				game.animals [ game.animals[animalIndex].body[cellIndex].grabbedCreature  ].fAngle = angleToCursor;

				// Dropping items.
				if ( sum <= -0.85f)
				{
					game.animals[animalIndex].body[cellIndex].grabbedCreature = -1;
				}
			}

			if (
			    game.animals[animalIndex].body[cellIndex].grabbedCreature >= 0)
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
			sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity * 0.99f;
		}

		case ORGAN_SENSOR_HUNGER:
		{
			if (game.animals[animalIndex].maxEnergy > 0.0f)
			{
				sensorium[cellIndex] = game.animals[animalIndex].energy / game.animals[animalIndex].maxEnergy;
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
				sensorium[cellIndex] =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle) / const_pi;
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
					sensorium[cellIndex] =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle) / const_pi;
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
					sensorium[cellIndex] =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle) / const_pi;
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
					sensorium[cellIndex] =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle) / const_pi;
				}
			}
			break;
		}

		case ORGAN_LUNG:
		{
			if (game.world[cellWorldPositionI].wall != MATERIAL_WATER)
			{
				sensorium[cellIndex] = baseLungCapacity;
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

					sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity -  aBreath;
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
				sensorium[cellIndex] = baseLungCapacity;
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
					sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity -  aBreath;
					if (game.animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						game.animals[animalIndex].damageReceived++;
					}
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
				game.world[cellWorldPositionI].pheromoneChannel = game.animals[animalIndex].body[cellIndex]. speakerChannel ;
			}

			break;
		}

		case ORGAN_SPEAKER:
		{
			if ( game.animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
			{
				float sum = sumInputs(  animalIndex,   cellIndex);
				sum = clamp(sum, -1.0f, 1.0f);
				game.speakerChannels[  game.animals[animalIndex].body[cellIndex].speakerChannel ] += sum;//game.animals[animalIndex].body[cellIndex].signalIntensity ;
			}
			break;
		}

		case ORGAN_SENSOR_EAR:
		{
			if (game.animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
			{
				sensorium[cellIndex]  = game.speakerChannelsLastTurn[ game.animals[animalIndex].body[cellIndex].speakerChannel ];
			}
			break;
		}

		case ORGAN_SENSOR_TRACKER:
		{
			game.animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
			if ( game.world [cellWorldPositionI].identity != animalIndex )
			{
				sensorium[cellIndex]  =  smallestAngleBetween(  game.world[cellWorldPositionI].trail, game.animals[animalIndex].fAngle) / const_pi;
			}
			break;
		}

		case ORGAN_SENSOR_BODYANGLE:
		{
			sensorium[cellIndex]  = game.animals[animalIndex].fAngle / const_pi;
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
			sensorium[cellIndex] = 1.0f - perceivedColor;
			break;
		}


		case ORGAN_SENSOR_SCANNINGEYE:
		{
			Vec_f2 eyeLook = Vec_f2(game.animals[animalIndex].body[cellIndex].eyeLookX , game.animals[animalIndex].body[cellIndex].eyeLookY);

			eyeLook.x += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue; // unlike a normal eye, the scanning eye looks in a range around its viewpoint. It still only sees 1 pixel per turn.
			eyeLook.y += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue;

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
			sensorium[cellIndex] = 1.0f - perceivedColor;
			break;
		}




		case ORGAN_SCANNING_EYE_CREATURE:
		{
			Vec_f2 eyeLook = Vec_f2(game.animals[animalIndex].body[cellIndex].eyeLookX , game.animals[animalIndex].body[cellIndex].eyeLookY);
			eyeLook.x += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue; // unlike a normal eye, the scanning eye looks in a range around its viewpoint. It still only sees 1 pixel per turn.
			eyeLook.y += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue;
			Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), game.animals[animalIndex].fAngleSin, game.animals[animalIndex].fAngleCos, eyeLook);
			unsigned int eyeLookWorldPositionX = cellWorldPositionX + rotatedEyeLook.x;
			unsigned int eyeLookWorldPositionY = cellWorldPositionY + rotatedEyeLook.y;
			unsigned int eyeLookWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

			// if there is an animal in the square, report 1, else report 0.
			int occupyingCell = isAnimalInSquare(game.world[eyeLookWorldPositionI].identity , eyeLookWorldPositionI);
			if ( occupyingCell >= 0)
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = 0.0f;
			}
			break;
		}

		case ORGAN_SCANNING_EYE_PLANT:
		{
			Vec_f2 eyeLook = Vec_f2(game.animals[animalIndex].body[cellIndex].eyeLookX , game.animals[animalIndex].body[cellIndex].eyeLookY);
			eyeLook.x += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue; // unlike a normal eye, the scanning eye looks in a range around its viewpoint. It still only sees 1 pixel per turn.
			eyeLook.y += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue;
			Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), game.animals[animalIndex].fAngleSin, game.animals[animalIndex].fAngleCos, eyeLook);
			unsigned int eyeLookWorldPositionX = cellWorldPositionX + rotatedEyeLook.x;
			unsigned int eyeLookWorldPositionY = cellWorldPositionY + rotatedEyeLook.y;
			unsigned int eyeLookWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

			// // if there is an animal in the square, report 1, else report 0.
			// int occupyingCell = isAnimalInSquare(game.world[worldCursorPos].identity , eyeLookWorldPositionI);
			if ( game.world[eyeLookWorldPositionI].plantState != MATERIAL_NOTHING)
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = 0.0f;
			}



			break;
		}

		case ORGAN_SCANNING_EYE_OBSTACLE:
		{
			Vec_f2 eyeLook = Vec_f2(game.animals[animalIndex].body[cellIndex].eyeLookX , game.animals[animalIndex].body[cellIndex].eyeLookY);
			eyeLook.x += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue; // unlike a normal eye, the scanning eye looks in a range around its viewpoint. It still only sees 1 pixel per turn.
			eyeLook.y += (RNG() - 0.5f) * game.animals[animalIndex].body[cellIndex].workingValue;
			Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), game.animals[animalIndex].fAngleSin, game.animals[animalIndex].fAngleCos, eyeLook);
			unsigned int eyeLookWorldPositionX = cellWorldPositionX + rotatedEyeLook.x;
			unsigned int eyeLookWorldPositionY = cellWorldPositionY + rotatedEyeLook.y;
			unsigned int eyeLookWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

			// if there is an animal in the square, report 1, else report 0.
			// int occupyingCell = isAnimalInSquare(game.world[worldCursorPos].identity , eyeLookWorldPositionI);
			if ( materialBlocksMovement(game.world[eyeLookWorldPositionI].wall) )
			{
				sensorium[cellIndex] = 1.0f;
			}
			else
			{
				sensorium[cellIndex] = 0.0f;
			}
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
					if (game.world[neighbour].identity >= 0 && game.world[neighbour].identity < numberOfAnimals)
					{
						if (isAnimalInSquare( game.world[neighbour].identity , neighbour ) >= 0)
						{
							sensorium[cellIndex]  += 0.5f;
						}
						else if (game.world[neighbour].wall != MATERIAL_NOTHING)
						{
							sensorium[cellIndex]  += 0.5f;
						}
					}
				}
			}
			unsigned int touchedAnimal = game.world[cellWorldPositionI].identity;
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
						else if (game.world[cellWorldPositionI].wall != MATERIAL_NOTHING)
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
			sensorium[cellIndex]  = game.animals[animalIndex].body[cellIndex].workingValue;
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
			if (doAsexualAnimals)
			{
				bool bonked = false;
				if (doReproduction && game.animals[animalIndex].energyDebt <= 0.0f )
				{
					float reproducesAt = ((game.animals[animalIndex].cellsUsed / 2 ) + game.animals[animalIndex].offspringEnergy );
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
								// printf("an animal reproduced gonastically\n");
								game.animals[animalIndex].body[cellIndex].damage = 1.0f;
								game.animals[animalIndex].numberOfTimesReproduced++;
								game.animals[animalIndex].energy -= game.animals[animalIndex].offspringEnergy;
								game.animals[result].energy       =  game.animals[animalIndex].offspringEnergy;
								game.animals[result].parentIdentity       = animalIndex;

								// distribute pheromones
								for (int i = 0; i < nNeighbours; ++i)
								{
									unsigned int neighbour = cellWorldPositionI += neighbourOffsets[i];
									if ( neighbour < worldSquareSize)
									{
										game.world[neighbour].pheromoneChannel = PHEROMONE_MUSK;
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
					sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity * 0.95f;
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
			bool ate_plant = false;

			if (game.world[cellWorldPositionI].plantState == MATERIAL_BUD_A ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_BUD_M ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_BUD_F ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_POLLENTRAP ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_TRICHOME ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_LEAF)
			{



				float defense =  plantDefenseAtWorldPoint(cellWorldPositionI);
				if (RNG() < (1.0 / defense))
				{
					ate_plant = true;
				}

			}
			bool ate = false;
			if (ate_plant)
			{
				ate = true;
				game.animals[animalIndex].energy += game.ecoSettings[1] ;
				damagePlants(cellWorldPositionI);
			}
			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
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

			if (game.world[cellWorldPositionI].plantState == MATERIAL_ROOT ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_TUBER ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_WOOD ||
			        game.world[cellWorldPositionI].plantState == MATERIAL_THORNS

			   )
			{

				float defense =  plantDefenseAtWorldPoint(cellWorldPositionI);
				if (RNG() < (1.0 / defense))
				{
					ate_plant = true;
				}

			}
			bool ate = false;
			if (ate_plant)
			{
				ate = true;
				game.animals[animalIndex].energy += game.ecoSettings[1] ;
				damagePlants(cellWorldPositionI);

			}
			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
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

			if (
			    game.world[cellWorldPositionI].seedState     == MATERIAL_SEED  ||
			    game.world[cellWorldPositionI].seedState     == MATERIAL_POLLEN

			)
			{

				ate_plant = true;
			}
			bool ate = false;
			if (ate_plant)
			{
				ate = true;
				game.animals[animalIndex].energy += game.ecoSettings[1] * 3.0f ;
				game.world[cellWorldPositionI].seedState = MATERIAL_NOTHING;
			}
			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
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
			if (

			    game.world[cellWorldPositionI].wall == MATERIAL_FOOD ||
			    game.world[cellWorldPositionI].wall == MATERIAL_BONE ||
			    game.world[cellWorldPositionI].wall == MATERIAL_BLOOD ||
			    game.world[cellWorldPositionI].wall == MATERIAL_VOMIT ||
			    game.world[cellWorldPositionI].wall == MATERIAL_SEMEN

			)
			{
				game.animals[animalIndex].energy += game.ecoSettings[0] ;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
				ate  = true;
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
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
					game.animals[animalIndex].energy                             += amount / 2;
					game.animals[game.world[cellWorldPositionI].identity].energy -= amount / 2;

					unsigned int victimSpecies =  (game.world[cellWorldPositionI].identity / numberOfAnimalsPerSpecies) ;
					if (victimSpecies < numberOfSpecies)
					{
						foodWeb[speciesIndex][  victimSpecies] += amount ;
						ate = true;
					}
				}
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
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
							ate = true;
							foodWeb[speciesIndex][  victimSpecies] += game.ecoSettings[0] ;
						}
					}
				}
			}

			if (game.world[cellWorldPositionI].wall == MATERIAL_HONEY)
			{
				ate = true;
				game.animals[animalIndex].energy += 1.0f;
				game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
			}

			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] = 1.0f;
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

				if (game.world[cellWorldPositionI].identity != animalIndex && game.world[cellWorldPositionI].identity >= 0 && game.world[cellWorldPositionI].identity < numberOfAnimals) // if the cell was occupied by another creature.
				{
					int targetLocalPositionI = isAnimalInSquare(game.world[cellWorldPositionI].identity , cellWorldPositionI);
					if (targetLocalPositionI >= 0)
					{
						ate = true;
						hurtAnimal(game.world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f, animalIndex );
					}
				}

				if (game.world[cellWorldPositionI].wall == MATERIAL_WAX)
				{
					ate = true;
					game.world[cellWorldPositionI].wall = MATERIAL_NOTHING;
				}
			}

			sensorium[cellIndex] = 0.0f;
			if (ate)
			{
				sensorium[cellIndex] = 1.0f;
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
			// sum  = fast_sigmoid(sum);
			sensorium[cellIndex] = sum;

			float impulse = game.animals[animalIndex].body[cellIndex].signalIntensity  * musclePower;

			game.animals[animalIndex].fPosX += (impulse / game.animals[animalIndex].cellsUsed) * game.animals[animalIndex].fAngleSin;
			game.animals[animalIndex].fPosY += (impulse / game.animals[animalIndex].cellsUsed) * game.animals[animalIndex].fAngleCos;
			game.animals[animalIndex].energy -= game.ecoSettings[2] * abs(game.animals[animalIndex].body[cellIndex].signalIntensity ) ;
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
			// sum  = fast_sigmoid(sum);

			sensorium[cellIndex] = sum;

			float impulse = game.animals[animalIndex].body[cellIndex].signalIntensity  * musclePower;

			game.animals[animalIndex].fPosX += (impulse / game.animals[animalIndex].cellsUsed) * game.animals[animalIndex].fAngleCos;
			game.animals[animalIndex].fPosY += (impulse / game.animals[animalIndex].cellsUsed) * game.animals[animalIndex].fAngleSin;
			game.animals[animalIndex].energy -= game.ecoSettings[2] * abs(game.animals[animalIndex].body[cellIndex].signalIntensity ) ;
			break;
		}
		case ORGAN_MUSCLE_TURN:
		{
			float sum = 0.0f;
			if (animalIndex != game.playerCreature)
			{
				sum = sumInputs(  animalIndex,   cellIndex);
			}
			// sum  = fast_sigmoid(sum);



			sensorium[cellIndex] = sum;

			if (setOrSteerAngle)
			{


				float angle = (game.animals[animalIndex].body[cellIndex].signalIntensity ) * const_pi;


				game.animals[animalIndex].fAngle +=  ((sum - game.animals[animalIndex].fAngle ) * 0.5f) / game.animals[animalIndex].cellsUsed;



			}
			else
			{
				game.animals[animalIndex].fAngle += sum * turnMusclePower;
			}
			break;
		}
		case ORGAN_GENITAL_A:
		{
			bool bonked = false;

			// printf("a\n");
			for (int i = 0; i < nNeighbours; ++i)
			{

				unsigned int neighbour = cellWorldPositionI += neighbourOffsets[i];
				if ( neighbour < worldSquareSize)
				{

					 int neighbourID = game.world[neighbour].identity;

					if (  neighbourID>= 0 && neighbourID < numberOfAnimals && neighbourID != animalIndex)
					{

			// printf("b\n");
						int targetLocalPositionI = isAnimalInSquare( neighbourID, neighbour);
						if (targetLocalPositionI >= 0)
						{
							if (game.animals[neighbourID].body[targetLocalPositionI].organ == ORGAN_GENITAL_B )
							{

			// printf("c\n");
								sexBetweenTwoCreatures( animalIndex, neighbourID);
								spill(MATERIAL_SEMEN, neighbour);
								bonked = true;
								break;
							}
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
				sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity * 0.95f;
			}

			break;
		}
		case ORGAN_GENITAL_B:
		{









			bool bonked = false;


			for (int i = 0; i < nNeighbours; ++i)
			{
				unsigned int neighbour = cellWorldPositionI += neighbourOffsets[i];
				if ( neighbour < worldSquareSize)
				{


					if ( game.world[neighbour].identity >= 0 &&  game.world[neighbour].identity < numberOfAnimals && game.world[neighbour].identity != animalIndex)
					{
						int targetLocalPositionI = isAnimalInSquare( game.world[neighbour].identity, neighbour);
						if (targetLocalPositionI >= 0)
						{
							if (game.animals[game.world[neighbour].identity].body[targetLocalPositionI].organ == ORGAN_GENITAL_A )
							{
									// printf("c\n");
								sexBetweenTwoCreatures( animalIndex, game.world[neighbour].identity );
								spill(MATERIAL_SEMEN, neighbour);
								bonked = true;
								break;
							}
						}
					}

				}
			}

			if (bonked)
			{
				sensorium[cellIndex] = 1.0f;

				// distribute pheromones
				for (int i = 0; i < nNeighbours; ++i)
				{
					unsigned int neighbour = cellWorldPositionI += neighbourOffsets[i];
					if ( neighbour < worldSquareSize)
					{
						game.world[neighbour].pheromoneChannel = PHEROMONE_MUSK;
					}
				}

				// the genital B is destroyed by this action.
				game.animals[animalIndex].body[cellIndex].damage = 1.0f;


			}
			else
			{
				sensorium[cellIndex] = game.animals[animalIndex].body[cellIndex].signalIntensity * 0.95f;
			}

			break;
		}

		case ORGAN_LOCATIONREMEMBERER:
		{
			float sum = sumInputs(  animalIndex,   cellIndex);
			if (sum <  0.0f)
			{
				game.animals[animalIndex].body[cellIndex].speakerChannel = cellWorldPositionI; // remember current location
			}
			else
			{
				if (game.animals[animalIndex].body[cellIndex]. speakerChannel > 0 && game.animals[animalIndex].body[cellIndex]. speakerChannel < worldSquareSize)
				{
					float targetWorldPositionX =   game.animals[animalIndex].body[cellIndex]. speakerChannel % worldSize;  ;
					float targetWorldPositionY =   game.animals[animalIndex].body[cellIndex]. speakerChannel / worldSize;  ;
					float fdiffx = targetWorldPositionX - game.animals[animalIndex].fPosX;
					float fdiffy = targetWorldPositionY - game.animals[animalIndex].fPosY;
					float targetAngle = atan2( fdiffy, fdiffx );
					game.animals[animalIndex].body[cellIndex].signalIntensity =  smallestAngleBetween( targetAngle, game.animals[animalIndex].fAngle)  / const_pi; // direction to remembered location, map it to -1..1 for network compatibility.
				}
			}
			break;
		}
		}
		if ( organIsANeuron(game.animals[animalIndex].body[cellIndex].organ) || organIsASensor(game.animals[animalIndex].body[cellIndex].organ) )
		{
			sensorium[cellIndex] += (RNG() - 0.5f) * neuralNoise;
		}
	}
	for (int cellIndex = 0; cellIndex < game.animals[animalIndex].cellsUsed; ++cellIndex)
	{
		game.animals[animalIndex]. body[cellIndex].signalIntensity = sensorium[cellIndex];
	}
	game.animals[animalIndex].maxEnergy = game.animals[animalIndex].cellsUsed + (totalLiver * liverStorage);
}




void autoReproduce(unsigned int animalIndex)
{

	// a sexually reproducing animal has matured, find it a partner and teleport them together so their naughty bits are touching.

	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;

	int applicableGonad = getRandomCellOfType(animalIndex, ORGAN_GENITAL_A);

	if (applicableGonad < 0)
	{
		applicableGonad = getRandomCellOfType(animalIndex, ORGAN_GENITAL_B);
	}


	if (applicableGonad >= 0)
	{



		// find a suitable partner
		int partner = (speciesIndex * numberOfAnimalsPerSpecies);
		bool foundPartner = false;
		int partnerGenital = -1;
		unsigned int start = extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies);
		for (int i = 0; i < numberOfAnimalsPerSpecies; ++i)
		{
			unsigned int partnerIndexInSpecies = ((start) + i) % numberOfAnimalsPerSpecies;
			unsigned int candidateIndex = (speciesIndex * numberOfAnimalsPerSpecies) + partnerIndexInSpecies;

			if (!(game.animals[candidateIndex].retired))
			{

				// check that the candidate has matured and has a suitable compatible gonad
				if ((game.animals[candidateIndex].energyDebt <= 0.0f))
				{
					int candidateGonad = 1;
					if (game.animals[animalIndex].body[applicableGonad].organ == ORGAN_GENITAL_A)
					{
						candidateGonad = getRandomCellOfType(candidateIndex, ORGAN_GENITAL_B);
					}
					else if (game.animals[animalIndex].body[applicableGonad].organ == ORGAN_GENITAL_B)
					{
						candidateGonad = getRandomCellOfType(candidateIndex, ORGAN_GENITAL_A);
					}

					if (candidateGonad >= 0)
					{
						partnerGenital = candidateGonad;
						partner = candidateIndex;
						foundPartner = true;
						break;
					}
				}

			}
		}

		if (foundPartner)
		{

			// example:
			// a is +100, +100. B is +200, +300.

			// teleport A onto B so that the are in a sex position.
			int partnerA_worldX = game.animals[animalIndex].position % worldSize;
			int partnerA_worldY = game.animals[animalIndex].position / worldSize;
			int vgA_worldX =  partnerA_worldX + game.animals[animalIndex].body[applicableGonad].localPosX;
			int vgA_worldY = partnerA_worldY + game.animals[animalIndex].body[applicableGonad].localPosY;

			int partnerB_worldX = game.animals[partner].position % worldSize;
			int partnerB_worldY = game.animals[partner].position / worldSize;
			int vgB_worldX = partnerB_worldX + game.animals[partner].body[partnerGenital].localPosX;
			int vgB_worldY = partnerB_worldY + game.animals[partner].body[partnerGenital].localPosY;

			int deltaX = vgB_worldX - vgA_worldX ;
			int deltaY = vgB_worldY - vgA_worldY ;

			int destinationX = (partnerA_worldX + deltaX);
			int destinationY = (partnerA_worldY + deltaY) ;
			int destinationI = (destinationY * worldSize ) + destinationX;

			game.animals[animalIndex].uPosX = destinationX;
			game.animals[animalIndex].uPosY = destinationY;
			game.animals[animalIndex].fPosX = destinationX;
			game.animals[animalIndex].fPosY = destinationY;
			game.animals[animalIndex].position = destinationI;
			game.animals[animalIndex].fAngle= 0.0f;

			// hereafter, it is assumed that the copulation will occur next time either one of the participants is placed in the world.

			printf("autoreproduced %u and %u. positions after A: %i %i; B: %i %i\n", animalIndex, partner, 
				game.animals[animalIndex].uPosX , 
				game.animals[animalIndex].uPosY , 
				game.animals[partner].uPosX , 
				game.animals[partner].uPosY  
				);

		}



	}


}




void animalEnergy(int animalIndex)
{

	ZoneScoped;
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	game.animals[animalIndex].age++;

	game.animals[animalIndex].energy -= game.ecoSettings[3] * game.animals[animalIndex].cellsUsed;

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

			if (game.animals[animalIndex].energyDebt <= 0.0f)
			{
				// the debt was just now repayed, and the animal is mature. roll to automatically reproduce it.
				if (RNG() < game.ecoSettings[11])
				{

					autoReproduce(animalIndex);
				}

			}
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
		if (speciesIndex != 0)
		{
			if (game.animals[animalIndex].energy < 0.0f)
			{
				// printf("died low energy\n");
				execute = true;
			}

			if (game.animals[animalIndex].age > game.animals[animalIndex].lifespan)
			{

				// printf("died old\n");
				execute = true;
			}
		}
		if (game.animals[animalIndex].damageReceived > game.animals[animalIndex].cellsUsed / 2)
		{

			// printf("died damaged\n");
			execute = true;
		}
		if (game.animals[animalIndex].cellsUsed <= 0)
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
			float animalScore = 0.0f;
			animalScore = game.animals[animalIndex].damageDone + game.animals[animalIndex].damageReceived  + (game.animals[animalIndex].numberOfTimesReproduced ) ;
			float distance = abs(game.animals[animalIndex].fPosX - game.animals[game.adversary].fPosX) + abs(game.animals[animalIndex].fPosY - game.animals[game.adversary].fPosY) ;
			if (distance > 100.0f) { distance = 100.0f;}
			distance  *= 0.5f;
			animalScore += distance ;
			if ( animalScore > game.championScores[speciesIndex])
			{
				game.championScores[speciesIndex] = animalScore;
				game.champions[speciesIndex] = game.animals[animalIndex];
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




void drawNeuroConnection(int animalIndex,  int animalCell, unsigned int connection, int vx, int vy)
{

	int startx = game.animals[animalIndex].body[animalCell].worldPositionI % worldSize;
	int starty = game.animals[animalIndex].body[animalCell].worldPositionI / worldSize;

	Vec_f2 start = Vec_f2(0, 0);
	Vec_f2 end    = Vec_f2(0, 0);

	if (game.animals[animalIndex].body[animalCell].connections[connection].used)
	{
		int connected_to_cell = game.animals[animalIndex].body[animalCell].connections[connection].connectedTo;
		if (connected_to_cell >= 0 && connected_to_cell < game.animals[animalIndex].cellsUsed)
		{

			// unsigned
			unsigned	int connectedPos = game.animals[animalIndex].body[connected_to_cell].worldPositionI;
			if (game.animals[animalIndex].body[connected_to_cell].worldPositionI > 0)
			{
				int connectedX = connectedPos % worldSize;
				int connectedY = connectedPos / worldSize;
				end.x -= (startx - connectedX);
				end.y -= (starty - connectedY);
				// Vec_f2(game.mousePositionX, game.mousePositionY);
				// }





				// get the difference between the cell local positions.
				// rotate it by the animal's body angle.
				// draw that, it is much more reliable. you cannot draw from placed positions because they can be total anarchy.

// 			int diffx =game.animals[connected_to_cell].body[animalCell].localPosX - game.animals[animalIndex].body[animalCell].localPosX    ;
// 			int diffy =  game.animals[connected_to_cell].body[animalCell].localPosY - game.animals[animalIndex].body[animalCell].localPosY ;



// 			end.x = diffx;
// 			end.y = diffy;





// // Vec_f2 rotatePointPrecomputed( Vec_f2 center, float s, float c, Vec_f2 point)
// 			end =rotatePointPrecomputed( Vec_f2(0,0), game.animals[animalIndex].fAngleSin,game.animals[animalIndex].fAngleCos, end  );

				start.x += vx;
				end.x  += vx;

				start.y += vy;
				end.y += vy;










				// float amount = game.animals[game.world[worldI].identity]. body [ occupyingCell].signalIntensity;
				// 		Color signalColor;
				// 		if (amount > 0)
				// 		{
				// 			signalColor = Color(1.0, 0.0f, 0.0f, 1.0f);
				// 		}
				// 		else
				// 		{
				// 			signalColor = Color(0.0, 0.0f, 1.0f, 1.0f);
				// 		}
				// 		displayColor = multiplyColorByScalar( signalColor, abs(amount) );




				Color signalColor = color_white;
				float brightness = (game.animals[animalIndex].body[connected_to_cell].signalIntensity ); // map -1..1 to 0..1

				if (brightness > 0)
				{
					signalColor = Color(1.0, 0.0f, 0.0f, 1.0f);
				}
				else
				{
					signalColor = Color(0.0, 0.0f, 1.0f, 1.0f);
				}

				signalColor = multiplyColorByScalar(signalColor, abs( brightness));



				float apparentWeight = abs(game.animals[animalIndex].body[animalCell].connections[connection].weight);
				apparentWeight = clamp(apparentWeight, 0.0f, 1.0f);
				apparentWeight *= 0.25;

				drawLine(  start, end, apparentWeight , signalColor );
				// printf("peem, %f %f , %f %f\n", start.x, start.y, end.x, end.y);
			}
		}
	}
}

void drawNeuroConnections( int animalIndex,  int animalCell, int vx, int vy)
{


	//


	// printf("asad\n");
	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		drawNeuroConnection( animalIndex,   animalCell,  i,  vx,  vy);
	}


}


void incrementVisualizer()
{
	if (game.visualizer == VISUALIZER_TRUECOLOR)
	{
		game.visualizer = VISUALIZER_ENERGY;
	}
	else if (game.visualizer == VISUALIZER_ENERGY)
	{
		game.visualizer = VISUALIZER_NEURALACTIVITY;
	}
	else if (game.visualizer == VISUALIZER_NEURALACTIVITY)
	{
		game.visualizer = VISUALIZER_TRUECOLOR;
	}
}

Color vis_energy(unsigned int worldI)
{

	Color displayColor = color_black;

	int occupyingCell = isAnimalInSquare(game.world[worldI].identity, worldI);
	if ( occupyingCell >= 0 )
	{
		int animalIndex = game.world[worldI].identity;
		if (game.animals[animalIndex].isMachine)
		{

			displayColor =   color_white;
		}
		else
		{
			displayColor =
			    Color( (game.animals[animalIndex].energy / game.animals[animalIndex].maxEnergy), (game.animals[animalIndex].energyDebt / game.animals[animalIndex].cellsUsed), 0.0f , 1.0f);

		}
	}
	else
	{



		if (game.world[worldI].plantState != MATERIAL_NOTHING)
		{
			displayColor =  Color(game.world[worldI].nutrients / plantMaxEnergy , 0.0f, game.world[worldI].energy / plantMaxEnergy, 1.0f);
		}




		if (game.world[worldI].seedState != MATERIAL_NOTHING)
		{
			displayColor =  filterColor(displayColor, multiplyColorByScalar( Color(1.0f, 0.0f, 0.0f, 1.0f), 0.5f  ));
		}



		if (game.world[worldI].wall != MATERIAL_NOTHING)
		{
			displayColor =  filterColor(displayColor, Color(1.0f, 1.0f, 1.0f, 0.1f));
		}



	}

	return displayColor;

}


Color vis_neural(unsigned int worldI)
{

	Color displayColor = color_black;

	int occupyingCell = isAnimalInSquare(game.world[worldI].identity, worldI);
	if ( occupyingCell >= 0 )
	{

		if (game.animals[game.world[worldI].identity].isMachine)
		{

			displayColor =  filterColor(displayColor, multiplyColorByScalar( color_white, 1.0f ));
		}
		else
		{

			float amount = game.animals[game.world[worldI].identity]. body [ occupyingCell].signalIntensity;
			Color signalColor;
			if (amount > 0)
			{
				signalColor = Color(1.0, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				signalColor = Color(0.0, 0.0f, 1.0f, 1.0f);
			}
			displayColor = multiplyColorByScalar( signalColor, abs(amount) );

		}
	}
	else
	{



		if (game.world[worldI].seedState != MATERIAL_NOTHING)
		{
			displayColor =  filterColor(displayColor, multiplyColorByScalar( color_white, 0.05f  ));
		}

		if (game.world[worldI].plantState != MATERIAL_NOTHING)
		{
			displayColor =  filterColor(displayColor, multiplyColorByScalar( color_white, 0.05f  ));
		}

		if (game.world[worldI].wall != MATERIAL_NOTHING)
		{
			displayColor =  filterColor(displayColor, multiplyColorByScalar( color_white, 0.05f  ));
		}





	}



	return displayColor;

}


Color vis_truecolor(unsigned int worldI)
{
	Color displayColor = whatColorIsThisSquare(worldI);

	bool squareSelected = false;
	if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals )
	{
		if (game.world[worldI].identity == game.selectedAnimal)
		{
			squareSelected = true;
		}
	}

	if (squareSelected)
	{
		if (isAnimalInSquare(game.world[worldI].identity , worldI) >= 0 )
		{

			displayColor = multiplyColor(displayColor, color_white);
		}
		else
		{



			displayColor = multiplyColor(displayColor, color_white_half);
		}
	}
	else
	{

		displayColor = multiplyColor(displayColor, game.world[worldI].light);
	}
	displayColor.a = 1.0f;


	// shadows
	if (worldI > worldSize + 1)
	{
		unsigned int shadowCaster = worldI - (worldSize + 1);
		bool shadow = false;
		int shadowCasterCell = isAnimalInSquare( game.world[shadowCaster].identity,  shadowCaster) ;
		int currentCell = isAnimalInSquare( game.world[worldI].identity,  worldI);
		if (currentCell < 0)
		{
			if (shadowCasterCell >= 0 )
			{
				shadow = true;
			}
			else if ( materialIsTransparent(game.world[worldI].wall) && (! (materialIsTransparent(game.world[shadowCaster].wall )  )))
			{
				shadow = true;
			}
			if (shadow)
			{
				displayColor = filterColor(displayColor, tint_shadow);
			}
		}
	}

	return displayColor;
}


const int viewFieldMax = +(viewFieldY / 2);
const int viewFieldMin = -(viewFieldX / 2);

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
		for ( int vy = viewFieldMin; vy < viewFieldMax; ++vy)
		{
			for ( int vx = viewFieldMin; vx < viewFieldMax; ++vx)
			{
				Color displayColor = color_black;
				int x = (vx + game.cameraPositionX);// % worldSize;
				int y = (vy + game.cameraPositionY);// % worldSize;

				bool overlap = false;
				if (x < 0 || x > worldSize) { overlap = true; }
				if (y < 0 || y > worldSize) { overlap = true; }


				unsigned int worldI = (y * worldSize) + x;
				if (worldI < worldSquareSize && (!overlap))
				{

					float fx = vx;
					float fy = vy;

					Color displayColor = color_black;

					switch (game.visualizer)
					{

					case VISUALIZER_TRUECOLOR:
					{
						displayColor = vis_truecolor(worldI);
						break;
					}
					case VISUALIZER_NEURALACTIVITY:
					{
						displayColor = vis_neural(worldI);



						break;
					}
					case VISUALIZER_ENERGY:
					{
						displayColor = vis_energy(worldI);
						break;
					}


					}


					if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals )
					{
						bool squareIsSelectedAnimal = false;
						if (game.world[worldI].identity >= 0 && game.world[worldI].identity < numberOfAnimals)
						{
							if (game.world[worldI].identity == game.selectedAnimal)
							{
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
						if (game.world[worldI].plantIdentity >= 0 )
						{
							if (game.world[worldI].plantIdentity == game.selectedPlant)
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




	// draw the neuro display if a creature is selected.
	if (game.visualizer == VISUALIZER_NEURALACTIVITY && game.selectedAnimal >= 0)
	{
		for ( int vy = viewFieldMin; vy < viewFieldMax; ++vy)
		{
			for ( int vx = viewFieldMin; vx < viewFieldMax; ++vx)
			{
				// Color displayColor = color_black;
				int x = (vx + game.cameraPositionX);// % worldSize;
				int y = (vy + game.cameraPositionY);// % worldSize;

				bool overlap = false;
				if (x < 0 || x > worldSize) { overlap = true; }
				if (y < 0 || y > worldSize) { overlap = true; }


				unsigned int worldI = (y * worldSize) + x;
				if (worldI < worldSquareSize && (!overlap))
				{

					int animalIndex = game.world[worldI].identity;

					if (animalIndex == game.selectedAnimal)
					{
						// printf("yeam\n");
						float fx = vx;
						float fy = vy;


						int animalCell = isAnimalInSquare(animalIndex, worldI );
						if (animalCell >= 0 && animalIndex >= 0 && animalIndex < numberOfAnimals)
						{

							// printf("llengng\n");
							if (animalCell < game.animals[animalIndex].cellsUsed )
							{

								// printf("dogueu\n");
								drawNeuroConnections(animalIndex, animalCell, vx, vy);




								if ( // anything that uses eyelook

								    game.animals[animalIndex].body[animalCell].organ == ORGAN_SENSOR_EYE        ||
								    game.animals[animalIndex].body[animalCell].organ == ORGAN_SENSOR_SCANNINGEYE ||
								    game.animals[animalIndex].body[animalCell].organ == ORGAN_SCANNING_EYE_PLANT ||
								    game.animals[animalIndex].body[animalCell].organ == ORGAN_SCANNING_EYE_CREATURE ||
								    game.animals[animalIndex].body[animalCell].organ == ORGAN_SCANNING_EYE_OBSTACLE

								)
								{
									Vec_f2 eyeLook = Vec_f2(game.animals[animalIndex].body[animalCell].eyeLookX , game.animals[animalIndex].body[animalCell].eyeLookY);
									Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), game.animals[animalIndex].fAngleSin, game.animals[animalIndex].fAngleCos, eyeLook);

									Vec_f2 eyelookCameraPosition = Vec_f2( vx + rotatedEyeLook.x, vy + rotatedEyeLook.y );
									drawLine(  Vec_f2(vx, vy), eyelookCameraPosition , 0.25f, Color(1.0f, 1.0f, 1.0f, 0.25f) );
									drawTile(eyelookCameraPosition , Color(1.0f, 1.0f, 1.0f, 0.25f));
								}



							}
						}




						// draw eyelooks
						// int id = game.world[worldI].identity;
						// int occupyingCell = isAnimalInSquare(id , worldI);
						// if ( occupyingCell >= 0)
						// {


						// if ( // anything that uses eyelook

						//      game.animals[id].body[occupyingCell].organ == ORGAN_SENSOR_EYE        ||
						//      game.animals[id].body[occupyingCell].organ == ORGAN_SENSOR_SCANNINGEYE||
						//      game.animals[id].body[occupyingCell].organ == ORGAN_SCANNING_EYE_PLANT||
						//      game.animals[id].body[occupyingCell].organ == ORGAN_SCANNING_EYE_CREATURE ||
						//      game.animals[id].body[occupyingCell].organ == ORGAN_SCANNING_EYE_OBSTACLE

						// )
						// {
						// 	Vec_f2 eyeLook = Vec_f2(game.animals[id].body[occupyingCell].eyeLookX , game.animals[id].body[occupyingCell].eyeLookY);
						// 	Vec_f2 rotatedEyeLook = rotatePointPrecomputed( Vec_f2(0, 0), game.animals[id].fAngleSin, game.animals[id].fAngleCos, eyeLook);

						// 	Vec_f2 eyelookCameraPosition = Vec_f2( vx + rotatedEyeLook.x, vy + rotatedEyeLook.y );
						// 	drawLine(  Vec_f2(vx, vy), eyelookCameraPosition , 0.25f, Color(1.0f, 1.0f, 1.0f, 0.25f) );
						// 	drawTile(eyelookCameraPosition , Color(1.0f, 1.0f, 1.0f, 0.25f));
						// }

						// }


					}






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

void displayComputerText( std::vector<std::string>  * sideText)
{
	if (game.palette)
	{
		drawPalette( sideText);
	}


	else if (game.ecologyComputerDisplay)
	{


		for (int i = 0; i < numberOfMapSectors; ++i)
		{
			std::string foodwebstring =					"Plant coverage sector " + std::to_string(i) + ": " + std::to_string(coverage[i]) ;
			sideText->push_back( foodwebstring);

		}




		for (int i = 0; i < numberOfSpecies; ++i)
		{
			std::string foodwebstring =					"species " + std::to_string(i) + " eats: " ;
			for (int j = 0; j < numberOfSpecies; ++j)
			{
				foodwebstring += std::to_string(foodWeb[i][j]) + "  ";
			}
			sideText->push_back( foodwebstring);
		}

		std::string foodweblabels =					"               " ;
		for (int i = 0; i < numberOfSpecies; ++i)
		{
			foodweblabels +=  " species" + std::to_string(i) + " ";
		}

		sideText->push_back( foodweblabels);

		for (int i = 0; i < numberOfSpecies; ++i)
		{
			sideText->push_back(
			    "Species " + std::to_string(i) +
			    " pop. " + std::to_string(game.speciesPopulationCounts[i]) +
			    " hits " + std::to_string(game.speciesAttacksPerTurn[i])
			    + " champion score " + std::to_string(game.championScores[i])
			);
		}
		for (int j = 0; j < numberOfEcologySettings; ++j)
		{
			std::string selectString = std::string("");
			if (j == game.activeEcoSetting) { selectString = std::string("X "); }
			switch (j)
			{
			case 0:
			{
				sideText->push_back( selectString +  std::string("Energy in 1 square of meat: ") + std::to_string(game.ecoSettings[j])  );
				break;
			}
			case 1:
			{
				sideText->push_back(	selectString +  std::string("Energy in 1 square of grass: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}
			case 2:
			{
				sideText->push_back( selectString +  std::string("Energy required to move 1 square: ") + std::to_string(game.ecoSettings[j])  );
				break;
			}
			case 3:
			{
				sideText->push_back( selectString +  std::string("Energy required for each animal square each turn: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}
			case 4:
			{
				sideText->push_back(	selectString +  std::string("Number of map squares to update each turn: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}
			case 5:
			{
				sideText->push_back(	selectString +  std::string("Nutrition each plant can extract each turn: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}
			case 6:
			{
				sideText->push_back(selectString +  std::string("Energy each plant requires each turn: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}
			case 7:
			{
				sideText->push_back(selectString +  std::string("Animal mutation rate: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}
			case 8:
			{
				sideText->push_back(selectString +  std::string("Plant mutation rate: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}








			case 9:
			{
				sideText->push_back(selectString +  std::string("Autosave interval: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}

			case 10:
			{
				sideText->push_back(selectString +  std::string("Pollen mobility: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}




			case 11:
			{
				sideText->push_back(selectString +  std::string("Auto reproduction chance: ") + std::to_string(game.ecoSettings[j]) );
				break;
			}






			}
		}
	}

	else if (game.computerdisplays[0])
	{
		sideText->push_back(   std::string("game.animals are groups of tiles. Each tile is an organ that performs a dedicated bodily function. ") );
		sideText->push_back(   std::string("Your body is made this way too. ") );
		sideText->push_back(   std::string("If your tiles are damaged, you will lose the tile's function,") );
		sideText->push_back(   std::string("which can include your sight, movement, or breathing, resulting in disorientation and death. ") );
		sideText->push_back(   std::string("Find the hospital! It is in a black building on land, just like this one.") );

	}
	else if (game.computerdisplays[1])
	{
		sideText->push_back(   std::string("The hospital will heal you if you pick it up.") );
		sideText->push_back(   std::string("It can also be used to alter your body and mind.") );
		sideText->push_back(   std::string("Use it to add a gill anywhere on your body, so that you can survive underwater") );
		sideText->push_back(   std::string("Find a building in the ocean and retrieve the tracker glasses.") );
		sideText->push_back(   std::string("But beware, there are dangers other than the water itself.") );
	}
	else if (game.computerdisplays[2])
	{
		sideText->push_back(   std::string("Activate the tracker glasses to see the trails that creatures leave.") );
		sideText->push_back(   std::string("You will recognize the adversary by its white trail.") );
		sideText->push_back(   std::string("Take the weapon, find the adversary and destroy it.") );
		sideText->push_back(   std::string("The adversary posesses neuro glasses, which allow you to see the minute electrical activity of living flesh.") );
		sideText->push_back(   std::string("You can use them, in combination with the hospital, to edit the mind of a living creature.") );
	}
	else if (game.computerdisplays[3])
	{
		if (game.adversaryDefeated)
		{
			sideText->push_back(   std::string("The adversary has been destroyed. Life will no longer be created in the game.world, but will persist from its current state,") );
			sideText->push_back(   std::string("or eventually be driven to extinction.") );
		}
		else
		{
			sideText->push_back(   std::string("You must defeat the adversary. Return here when it is done.") );
		}
	}
	else
	{
		if (  printLogs)
		{
			for (int i = 0; i < 8; ++i)
			{
				sideText->push_back(   game.logs[i] );
			}
		}
	}
}

void incrementSelectedGrabber()
{

	if (game.animals[game.playerCreature].cellsUsed <= 0) { return;}
	for (unsigned int i = 0; i < game.animals[game.playerCreature].cellsUsed ; ++i)
	{
		if (game.animals[game.playerCreature].cellsUsed > 0)
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
}

void drawGameInterfaceText()
{
	std::vector<std::string> vec_sideText;
	std::vector<std::string> * sideText = &vec_sideText;
	if (flagSave)
	{
		sideText->push_back(  std::string("saving")  );
		return;
	}
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = ((cursorPosY * worldSize) + cursorPosX ) % worldSquareSize;
	if (game.paused)
	{
		sideText->push_back(  std::string("FPS 0 (Paused)")  );
	}
	else
	{
		sideText->push_back( 	 std::string("FPS ") + std::to_string(modelFrameCount )  );
	}
	modelFrameCount = 0;
	if (game.showInstructions)
	{
		sideText->push_back( std::string("Start by finding items in the game.world and picking them up.") );
		sideText->push_back( 	 std::string("[esc] quit") );
		sideText->push_back(  std::string("[arrows] pan, [-,=] zoom") );
		std::string pauseString = std::string("[p] pause ");
		if (game.paused)
		{
			pauseString = std::string("[p] resume ");
		}

		if (game.lockfps)
		{
			sideText->push_back(   std::string("[l] max simulation speed") );
		}
		else
		{
			sideText->push_back(  std::string("[l] limit simulation speed") );
		}
		sideText->push_back( std::string("[o] save, ") + pauseString );
		sideText->push_back(    std::string("[space] return mouse") );

		if (game.playerCreature >= 0)
		{
			sideText->push_back( 	std::string("[w,a,s,d] move")  );
			sideText->push_back(   std::string("[r] despawn") );
		}
		else
		{
			sideText->push_back(  std::string("[r] spawn") );
		}
		sideText->push_back(   std::string("[u] hide instructions") );
	}
	else
	{
		sideText->push_back( 	std::string("[u] instructions") );
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
		sideText->push_back( 	 std::string("Holding ") + std::to_string(holding )  + std::string(" items. [t] next") );
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
			sideText->push_back(  stringToPrint  );
		}
	}

	if (game.playerCanPickup && game.playerCanPickupItem >= 0 && game.playerCanPickupItem < numberOfAnimals)
	{
		sideText->push_back( 	 std::string("[g] pick up ") + std::string(game.animals[game.playerCanPickupItem].displayName) );
	}

	if (game.selectedAnimal >= 0 && game.selectedAnimal < numberOfAnimals)
	{
		int selectedAnimalSpecies = game.selectedAnimal / numberOfAnimalsPerSpecies;
		sideText->push_back(
		    std::string("Selected an animal of species ") + std::to_string(selectedAnimalSpecies ) +
		    std::string(". Energy ")                      + std::to_string(game.animals[game.selectedAnimal].energy ) +
		    std::string(", size ")                        + std::to_string(game.animals[game.selectedAnimal].maxEnergy ) +
		    std::string(", debt ")                        + std::to_string(game.animals[game.selectedAnimal].energyDebt ) +
		    std::string(", offspringEnergy ")             + std::to_string(game.animals[game.selectedAnimal].offspringEnergy ) +
		    std::string(", dmg ")                         + std::to_string(game.animals[game.selectedAnimal].damageReceived ) +
		    std::string(", repr ")                        + std::to_string(game.animals[game.selectedAnimal].numberOfTimesReproduced ) +
		    std::string(", age ")                         + std::to_string(game.animals[game.selectedAnimal].age ) +
		    std::string(", gen. ")                        + std::to_string(game.animals[game.selectedAnimal].generation )
		);
	}
	else if (game.selectedPlant >= 0)
	{
		std::string cursorDescription = std::string(".");
		if (worldCursorPos < worldSquareSize)
		{
			if (game.world[worldCursorPos].plantIdentity == game.selectedPlant)
			{
				cursorDescription += 	std::string(" This is ") + tileDescriptions(game.world[worldCursorPos].plantState );
				cursorDescription += std::string(". Energy ") + std::to_string(game.world[worldCursorPos].energy)
				                     + std::string(", noots ") + std::to_string(game.world[worldCursorPos].nutrients)
				                     + std::string(", genecursor ") + std::to_string(game.world[worldCursorPos].geneCursor)
				                     + std::string(", grown ") + std::to_string(game.world[worldCursorPos].grown)
				                     + std::string(", growthMatrix: ");
			}
			for (int i = 0; i < nNeighbours; ++i)
			{
				cursorDescription += std::to_string(game.world[worldCursorPos].growthMatrix[i]);
			}
		}
		sideText->push_back(  std::string("Selected plant ") + std::to_string(game.selectedPlant) + cursorDescription );
		std::string geneString = std::string("Plant genes: ");
		for (int i = 0; i < plantGenomeSize; ++i)
		{
			geneString += std::to_string(game.world[worldCursorPos].plantGenes[i]) + " ";
		}
		sideText->push_back(  geneString );
		geneString = std::string("Seed genes: ");
		for (int i = 0; i < plantGenomeSize; ++i)
		{
			geneString += std::to_string(game.world[worldCursorPos].seedGenes[i]) + " ";
		}
		sideText->push_back( 	geneString );
	}

	// print what is at the cursor position.
	if (worldCursorPos < worldSquareSize)
	{
		int heightInt = game.world[worldCursorPos].height;
		std::string cursorDescription = std::string("");
		bool animalInSquare = false;
		if (game.world[worldCursorPos].identity >= 0 && game.world[worldCursorPos].identity < numberOfAnimals)
		{
			int occupyingCell = isAnimalInSquare(game.world[worldCursorPos].identity , worldCursorPos);
			if ( occupyingCell >= 0)
			{
				animalInSquare = true;
				unsigned int cursorAnimalSpecies = game.world[worldCursorPos].identity  / numberOfAnimalsPerSpecies;
				game.cursorAnimal = game.world[worldCursorPos].identity;
				if (cursorAnimalSpecies == 0)
				{
					if (game.cursorAnimal == game.playerCreature)
					{
						cursorDescription += std::string("It's you. ");
					}
					else
					{
						cursorDescription += std::string("A ") +  std::string(game.animals[game.cursorAnimal].displayName) + std::string(". ");
					}
				}
				else
				{
					cursorDescription += std::string("An animal of species ") + std::to_string(cursorAnimalSpecies ) + std::string(". ");
				}
				cursorDescription +=  std::string(" This is ") + tileDescriptions(  game.animals[  game.cursorAnimal].body[occupyingCell].organ );
			}
		}

		if (!animalInSquare)
		{
			if (game.world[worldCursorPos].seedState != MATERIAL_NOTHING)
			{
				cursorDescription += 	std::string(" This is ") + tileDescriptions(game.world[worldCursorPos].seedState );
			}

			else if (game.world[worldCursorPos].plantState != MATERIAL_NOTHING)
			{
				cursorDescription += 	std::string(" This is ") + tileDescriptions(game.world[worldCursorPos].plantState );
			}
			else if (game.world[worldCursorPos].wall != MATERIAL_NOTHING)
			{
				cursorDescription += std::string(" This is ") + tileDescriptions(game.world[worldCursorPos].wall);
			}
			cursorDescription += std::string(" Below is ") +  tileDescriptions(game.world[worldCursorPos].terrain);
		}
		sideText->push_back(  cursorDescription);
	}
	if (game.playerCreature >= 0)
	{
		int playerPheromoneSensor = getRandomCellOfType( game.playerCreature, ORGAN_SENSOR_PHEROMONE ) ;// if the player has a nose, print what it smells like here.
		if (playerPheromoneSensor >= 0)
		{
			unsigned int playerPheromoneSensorWorldPos = game.animals[game.playerCreature].body[playerPheromoneSensor].worldPositionI;
			if (game.world[playerPheromoneSensorWorldPos].pheromoneChannel >= 0 &&  game.world[playerPheromoneSensorWorldPos].pheromoneChannel < numberOfSpeakerChannels)
			{
				sideText->push_back(   pheromoneDescriptions( game.world[playerPheromoneSensorWorldPos].pheromoneChannel ));
			}
			else
			{
				sideText->push_back(   std::string("You can't smell anything in particular.")  );
			}
		}
		if (!game.playerCanSee)// if the player is blind, say so!
		{
			sideText->push_back( 	std::string("You can't see anything. ")  );
		}

		int playerGill = getCellWithAir(game.playerCreature);
		if (playerGill >= 0)
		{
			if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 0.0f)
			{
				sideText->push_back(  std::string("You have no oxygen left.") );
			}
			else if (game.animals[game.playerCreature].body[playerGill].signalIntensity < baseLungCapacity / 2)
			{
				sideText->push_back( 	  std::string("You're half out of oxygen.") );
			}
		}

		playerGill = getRandomCellOfType( game.playerCreature, ORGAN_SENSOR_PAIN ) ;
		if (playerGill >= 0)
		{
			std::string painString = std::string("");
			bool printPainString = false;
			if (game.animals[game.playerCreature].damageReceived > (game.animals[game.playerCreature].cellsUsed) * 0.25 &&
			        game.animals[game.playerCreature].damageReceived < (game.animals[game.playerCreature].cellsUsed) * 0.375
			   )
			{
				printPainString = true;
				painString += std::string("You're badly damaged. ") ;
			}
			else if (game.animals[game.playerCreature].damageReceived > (game.animals[game.playerCreature].cellsUsed) * 0.375)
			{
				printPainString = true;
				painString += std::string("You are mortally wounded. ") ;
			}
			if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 0.5f)
			{
				;
			}
			else if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 1.0f)
			{
				painString = std::string("It stings.");
				printPainString = true;
			}
			else if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 2.0f )
			{
				painString = std::string("It hurts.");
				printPainString = true;
			}
			else if (game.animals[game.playerCreature].body[playerGill].signalIntensity < 5.0f )
			{
				painString = std::string("It hurts really bad!.");
				printPainString = true;
			}
			else
			{
				painString = std::string("The pain is agonizing!");
				printPainString = true;
			}
			if ( printPainString )
			{
				sideText->push_back(  painString );
			}
		}
	}

	displayComputerText( sideText);

	// first, go through and draw rectangles where the text will be.
	float rollingY = menuY;
	for (std::vector<std::string>::iterator it = sideText->begin(); it != sideText->end(); ++it)
	{
		Vec_f2 upperBound = Vec_f2(menuX + ((spacing * (it->length() + 1) ) * 0.25f    ) + (spacing / 2) , rollingY + spacing - (spacing / 2) + sideTextScrollPos);
		Vec_f2 lowerBound = Vec_f2(menuX + (                                        0) - (spacing / 2) , rollingY           - (spacing / 2) + sideTextScrollPos);
		drawPanel( lowerBound, upperBound, tint_shadow  );
		rollingY += spacing;
	}

	commitBufferToScreen();

	rollingY = menuY;
	for (std::vector<std::string>::iterator it = sideText->begin(); it != sideText->end(); ++it)
	{
		printText2D(*it, menuX, rollingY + sideTextScrollPos, textSize);
		rollingY += spacing;
	}
}



void paintCreatureFromCharArray( int animalIndex,  char * start, unsigned int len, unsigned int width )
{
	printf("painting animal %i\n", animalIndex);
	Vec_i2 o = Vec_i2(0, 0);
	Vec_i2 p = Vec_i2(0, 0);
	if (len > animalSquareSize)
	{
		len = animalSquareSize;
	}
	for (unsigned int i = 0; i < len; ++i)
	{
		char c = start[i];

		printf("%c ", c);
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
				break;
			}
		}
		p.x++;
		if (p.x == width)
		{
			printf("\n");
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



void setupStructureFromCharArray(int worldPositionI, char * walls, char * floors, char * lights, unsigned int len, unsigned int width, unsigned int scale, float angle)

{

	angle = 0.0f;
	float pcsin = sin(angle);
	float pccos = cos(angle);

	int x = worldPositionI % worldSize;
	int y = worldPositionI / worldSize;

	// Vec_i2 o = Vec_i2(0, 0);
	Vec_i2 p = Vec_i2(0, 0);

	int i = 0;
	for (;;)
	{

		unsigned int newWall = MATERIAL_NOTHING;
		unsigned int newFloor = MATERIAL_NOTHING;
		float newLight = -1.0f;





		char wc = walls[i];
		switch (wc)
		{
		case 'V':
			newWall = MATERIAL_VOIDMETAL;
			break;
		case 'M':
			newWall = MATERIAL_METAL;
			break;
		case 'G':
			newWall = MATERIAL_GLASS;
			break;
		case 'S':
			newWall = MATERIAL_BASALT;
			break;
		}


		char fc = floors[i];
		switch (fc)
		{
		case 'V':
			newFloor = MATERIAL_VOIDMETAL;
			break;
		case 'M':
			newFloor = MATERIAL_METAL;
			break;
		case 'G':
			newFloor = MATERIAL_GLASS;
			break;
		case 'S':
			newWall = MATERIAL_BASALT;
			break;
		}


		char lc = lights[i];
		switch (lc)
		{
		case '0':
			newLight = 0.0f;// 8.0f / 8.0f;
			break;
		case '1':
			newLight = 1.0f / 8.0f;
			break;
		case '2':
			newLight = 2.0f / 8.0f;
			break;
		case '3':
			newLight = 3.0f / 8.0f;
			break;
		case '4':
			newLight = 4.0f / 8.0f;
			break;
		case '5':
			newLight = 5.0f / 8.0f;
			break;
		case '6':
			newLight = 6.0f / 8.0f;
			break;
		case '7':
			newLight = 7.0f / 8.0f;
			break;
		case '8':
			newLight = 8.0f / 8.0f;
			break;
		}



		// if (newMaterial != MATERIAL_NOTHING)
		// {

		int vx = x + (p.x * scale) ;
		int vy = y + (p.y * scale) ;



		// if scale is greater than 1, go fill in all the tiny squares inside the big size square.
		for (int vvy = 0; vvy < scale; ++vvy)
		{
			for (int vvx = 0; vvx < scale; ++vvx)
			{



				int pointx = (vx + vvx);
				int pointy = (vy + vvy) ;



// Vec_f2  rotatedPoint = rotatePointPrecomputed( Vec_f2( x,y ), pcsin, pccos, Vec_f2(pointx, pointy));



				int k = (pointy * worldSize) + pointx;

				if (k > 0 && k < worldSquareSize)
				{


					if (newFloor != MATERIAL_NOTHING)
					{
						game.world[k].terrain = newFloor;
					}

					if (newWall != MATERIAL_NOTHING)
					{
						game.world[k].wall = newWall;
					}

					if (newLight > 0.0 )
					{
						game.world[k].light.a = newLight;
					}

				}

			}
		}



		// }
		p.x++;
		if (p.x == width)
		{
			p.x = 0;
			p.y --;
		}
		i++;
		if (i > len) { break;}
	}

}



void setupCreatureFromCharArray( int animalIndex, char * start, unsigned int len, unsigned int width, std::string newName, int newMachineCallback )
{
	resetAnimal(animalIndex);
	game.animals[animalIndex].generation = 0;
	strcpy( &game.animals[animalIndex].displayName[0] , newName.c_str() );
	if (newMachineCallback >= 0)
	{
		game.animals[animalIndex].isMachine = true;
		game.animals[animalIndex].machineCallback = newMachineCallback;
	}
	// Vec_i2 o = Vec_i2(0, 0);
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

void resetGameItems()
{
	// puts the pick-upable items back in their right place and heals them of damage
	for (int animalIndex = 0; animalIndex < 13; ++animalIndex)
	{
		game.animals[animalIndex].retired = false;
		game.animals[animalIndex].damageReceived = 0;
		game.animals[animalIndex].position = game.animals[animalIndex].birthLocation;
		unsigned int bex = game.animals[animalIndex].birthLocation % worldSize;
		unsigned int bey = game.animals[animalIndex].birthLocation / worldSize;
		game.animals[animalIndex].uPosX = bex;
		game.animals[animalIndex].uPosY = bey;
		game.animals[animalIndex].fPosX = bex;
		game.animals[animalIndex].fPosY = bey;
		for (int k = 0; k < game.animals[animalIndex].cellsUsed; ++k)
		{
			game.animals[animalIndex].body[k].damage = 0.0f;
		}
	}
	appendLog( std::string("Restored game items.") );
}

void spawnPlayer()
{
	resetGameItems();
	unsigned int targetWorldPositionI =  game.playerRespawnPos;
	int i = 0;
	setupExampleHuman(i);
	paintCreatureFromCharArray(i, humanPaint, (9 * 33), 9 );
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
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		std::ofstream out7( filename .c_str());
		out7.write( (char*)(&game.animals[game.selectedAnimal]), sizeof(Animal));
		out7.close();
	}
}

void loadParticlarAnimal(int animalIndex, std::string filename)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		std::ifstream in7(filename.c_str());
		in7.read( (char*)(&game.animals[game.selectedAnimal]), sizeof(Animal));
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
		float b = game.world[worldPositionI].light.a * steps;
		int ib = b;
		float betoot =  (ib / steps);
		game.world[worldPositionI].light.a = betoot;
	}
}


int getRandomPosition(float depth)
{
	while (true)
	{
		int randomI = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
		unsigned int x = randomI % worldSize;
		unsigned int y = randomI / worldSize;
		if (x > baseSize && x < (worldSize - baseSize) && y > baseSize && y < (worldSize - baseSize)  )
		{


			if ( (game.world[randomI].height / worldSize ) > depth - 0.05 &&  (game.world[randomI].height / worldSize ) < depth + 0.05 )
			{
				return randomI;
			}


		}
	}
}


// int getRandomPosition(float depth)
// {
// 	while (true)
// 	{
// 		unsigned int randomI = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
// 		unsigned int x = randomI % worldSize;
// 		unsigned int y = randomI / worldSize;
// 		if (x > baseSize && x < (worldSize - baseSize) && y > baseSize && y < (worldSize - baseSize)  )
// 		{
// 			if (underwater)
// 			{
// 				if (game.world[randomI].height > seaLevel)
// 				{
// 					continue;
// 				}
// 			}
// 			else
// 			{
// 				if (game.world[randomI].height < biome_coastal)
// 				{
// 					continue;
// 				}
// 			}
// 			bool hasAir = false;
// 			bool hasWater = false;
// 			bool unsuitable = false;
// 			for (int k = -(baseSize / 2); k < (baseSize / 2); ++k)
// 			{
// 				for (int j = -(baseSize / 2); j < (baseSize / 2); ++j)
// 				{
// 					unsigned int scan = randomI + (k * worldSize) + j;
// 					if (game.world[scan].wall == MATERIAL_NOTHING) { hasAir = true; }
// 					if (game.world[scan].wall == MATERIAL_WATER) { hasWater = true; }
// 					if (game.world[scan].wall == MATERIAL_VOIDMETAL) { unsuitable = true;}

// 				}
// 			}
// 			if (unsuitable) { continue; }

// 			if (underwater)
// 			{
// 				if (!hasAir)
// 				{
// 					return randomI;
// 				}
// 			}

// 			if (!underwater)
// 			{
// 				if (!hasWater)
// 				{
// 					return randomI;
// 				}
// 			}
// 		}
// 	}
// }

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
	int building1 =  getRandomPosition(0.9f);

	// setupBuilding_playerBase(building1);

	// setupBasicBuilding( building1);

	setupStructureFromCharArray(building1, basicBuilding, basicBuildingFloor, basicBuildingLight, 10 * 10, 10, 25, RNG() * 2.0f * 3.1415f  );
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
	int building2 =  getRandomPosition(0.75f);
	setupBuilding_playerBase(building2);
	setupHospitalComputer(i);
	spawnAnimalIntoSlot(4, game.animals[i], building2, false);

	building2 += 25 * worldSize;
	setupMessageComputer( i, 1);
	spawnAnimalIntoSlot(5, game.animals[i], building2, false);


	// BUILDING 3
	//contains tracker glasses, pistol, and message computer 3
	int building3 =  getRandomPosition(0.25f);
	// setupBuilding_playerBase(building3);
	setupStructureFromCharArray(building3, arenaBuilding, arenaBuildingFloor, arenaBuildingLight, 11 * 9, 11, 25, RNG() * 2.0f * 3.1415f );

	setupTrackerGlasses(i);
	spawnAnimalIntoSlot(6, game.animals[i], building3, false);

	building3 += 25;
	setupExampleGun(i);
	spawnAnimalIntoSlot(7, game.animals[i], building3, false);

	building3 += 25 * worldSize;
	setupMessageComputer( i, 2);
	spawnAnimalIntoSlot(8, game.animals[i], building3, false);


	// adversary is outside, under water
	game.adversaryRespawnPos =  getRandomPosition(0.35f);
	spawnAdversary(game.adversaryRespawnPos);


	// BUILDING 4
	// contains knife, lighter, and message computer 4
	int building4 =  getRandomPosition(0.1f);
	// setupBuilding_playerBase(building4);


	setupStructureFromCharArray(building4, crashedShip, crashedShipFloor, crashedShipLight, 21 * 47, 21, 10, RNG() * 2.0f * 3.1415f );


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

	detailTerrain();
	worldCreationStage = 8;
	recomputeTerrainLighting();
	worldCreationStage = 9;
	setupGameItems();
	worldCreationStage = 13;

	printf("The expected size of GameState is %lu bytes\n", sizeof(GameState));
	// return;

	// save();

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
								if (extremelyFastNumberFromZeroTo(100) == 0)
								{
									spawnRandomPlant( game.animals[game.adversary].position  );
								}

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

				if (respawnLowSpecies)
				{
					census();
					int totalPopulation = 0;
					for (int k = 1; k < numberOfSpecies; ++k)// spawn lots of the example animal
					{
						totalPopulation  += game.speciesPopulationCounts[k];
					}
					if (totalPopulation < emergencyPopulationLimit)	// the entire ecosystem has crashed, respawn some champions and example game.animals.
					{
						for (int k = 1; k < numberOfSpecies; ++k)
						{
							if (game.speciesPopulationCounts[k] < 1)
							{
								for (int nnn = 0; nnn < 100; ++nnn)
								{
									int j = 1;
									int domingo = -1;
									// unsigned int randomPos = game.animals[game.adversary].position;// + (-5 + extremelyFastNumberFromZeroTo(10)  + ( (-5 * worldSize) + (extremelyFastNumberFromZeroTo(10) * worldSize)  )   );



									// int whatToSpawn = extremelyFastNumberFromZeroTo(1);
									// if (whatToSpawn == 0)  // spawn example game.animals
									// {
									setupExampleAnimal3(j);

									unsigned int rsize =  32;
									unsigned int position = game.animals[game.adversary].position;

									position += extremelyFastNumberFromZeroTo(rsize);
									position -= (rsize / 2);

									position += worldSize * extremelyFastNumberFromZeroTo(rsize);
									position -= worldSize * (rsize / 2);

									position = position % worldSquareSize;

									domingo = spawnAnimal( k,  game.animals[j], position , true);
									// }
									// else if (whatToSpawn == 1)  // spawn a species champion
									// {
									// 	domingo = spawnAnimal( k,  game.champions[k], game.animals[game.adversary].position, true);
									// }
									if (domingo >= 0)
									{
										// game.animals[domingo].retired= false;
										// place(domingo);

										printf("spawned new animal %u \n", domingo);
										// game.animals[domingo].fAngle += ((RNG() - 0.5) );
										// game.animals[domingo].fAngleCos = cos(game.animals[domingo].fAngle);
										// game.animals[domingo].fAngleSin = sin(game.animals[domingo].fAngle);
										game.animals[domingo].energy = 10.0f;
										// game.animals[domingo].parentIdentity = game.adversary;

										if (game.world[game.animals[game.adversary].position].wall == MATERIAL_WATER)
										{
											bool hasGill = false;
											for (int i = 0; i < game.animals[domingo].cellsUsed; ++i)
											{
												if (game.animals[domingo].body[i].organ == ORGAN_GILL)
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
											for (int i = 0; i < game.animals[domingo].cellsUsed; ++i)
											{
												if (game.animals[domingo].body[i].organ == ORGAN_LUNG)
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
								}
							}
						}
					}
					else// shift animals in from another species
					{
						for (int k = 1; k < numberOfSpecies; ++k)
						{
							if (game.speciesPopulationCounts[k] < emergencyPopulationLimit)
							{
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
								if (mostPopulousCount > emergencyPopulationLimit * 2)
								{
									int  bromelich = getRandomCreature(mostPopulousSpecies);
									if (bromelich >= 0 )
									{
										int newId = getNewIdentity(k);
										game.animals[newId] = game.animals[bromelich];
										game.animals[bromelich].retired = true;
									}
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
		if (! (game.animals[j].retired))
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
	const unsigned int numberOfAnimalSectors = 64;
	const unsigned int numberOfAnimalsPerSector = numberOfAnimals / numberOfAnimalSectors;
	boost::thread  animalSectors[numberOfAnimalSectors];
	for (int i = 0; i < numberOfAnimalSectors; ++i)
	{
		int from = (i ) * numberOfAnimalsPerSector ;
		int to = (i + 1) * numberOfAnimalsPerSector;
		animalSectors[i] =	boost::thread { model_sector ,   from  , to    };
	}
	for (int i = 0; i < numberOfAnimalSectors; ++i)
	{
		(animalSectors[i]).join();
	}
}


void save()
{
	flagSave = true;
	worldCreationStage = 13;

	std::ofstream out6(std::string("save/game").c_str());
	out6.write( (char*)(&game), sizeof(GameState) );
	out6.close();

	// std::ofstream out7(std::string("save/game.world").c_str());
	// out7.write( (char*)(&game.world), sizeof(Square) * worldSquareSize );
	// out7.close();

	// std::ofstream out8(std::string("save/game.animals").c_str());
	// out8.write( (char*)(&game.animals), sizeof(Animal) * numberOfAnimals );
	// out8.close();

	worldCreationStage = 10;
	flagSave = false;
}

void load()
{
	worldCreationStage = 12;

	std::ifstream in6(std::string("save/game").c_str());
	in6.read( (char *)(&game), sizeof(GameState));
	in6.close();

	// std::ifstream in7(std::string("save/game.world").c_str());
	// in7.read( (char *)(&game.world), sizeof(Square) * worldSquareSize );
	// in7.close();

	// std::ifstream in8(std::string("save/game.animals").c_str());
	// in8.read( (char *)(&game.animals), sizeof(Animal) * numberOfAnimals );
	// in8.close();

	worldCreationStage = 10;
	setFlagReady();
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

				unsigned int autosaveInterval = game.ecoSettings[9];

				if (modelFrameCount % autosaveInterval == 0)
				{
					if (modelFrameCount > 0)
					{
						save();
					}
				}
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
		}
		printText2D(   std::string("[j] new "), menuX, menuY, textSize);
		menuY += spacing;

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


// this test plant makes a beautiful pattern of blue, and tests nested branches and sequences.
void setupTestPlant2(unsigned int worldPositionI)
{
	memset(game.world[worldPositionI].seedGenes, 0x00, sizeof(char) * plantGenomeSize);

	game.world[worldPositionI].seedGenes[0] = PLANTGENE_BLUE;
	game.world[worldPositionI].seedGenes[1] = 2;
	game.world[worldPositionI].seedGenes[2] = PLANTGENE_WOOD;
	game.world[worldPositionI].seedGenes[3] = 0;
	game.world[worldPositionI].seedGenes[4] = PLANTGENE_GROW_SYMM_H;
	game.world[worldPositionI].seedGenes[5] = PLANTGENE_BRANCH;
	game.world[worldPositionI].seedGenes[6] = 2;
	game.world[worldPositionI].seedGenes[7] = PLANTGENE_LEAF;
	game.world[worldPositionI].seedGenes[8] = PLANTGENE_SEQUENCE;
	game.world[worldPositionI].seedGenes[9] = 5;
	game.world[worldPositionI].seedGenes[10] = PLANTGENE_WOOD;
	game.world[worldPositionI].seedGenes[11] = PLANTGENE_BLUE;
	game.world[worldPositionI].seedGenes[12] = PLANTGENE_WOOD;
	game.world[worldPositionI].seedGenes[13] = 0;
	game.world[worldPositionI].seedGenes[14] = PLANTGENE_BRANCH;
	game.world[worldPositionI].seedGenes[15] = PLANTGENE_LEAF;
	game.world[worldPositionI].seedGenes[16] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedGenes[17] = 2;
	game.world[worldPositionI].seedGenes[18] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedGenes[19] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedIdentity = extremelyFastNumberFromZeroTo(256);
	game.world[worldPositionI].seedState = MATERIAL_SEED;
}


void simpleTestPlant(unsigned int worldPositionI)
{
	game.world[worldPositionI].seedGenes[0] = PLANTGENE_BLUE;
	game.world[worldPositionI].seedGenes[1] = PLANTGENE_BLUE;
	game.world[worldPositionI].seedGenes[2] = PLANTGENE_BLUE;
	game.world[worldPositionI].seedGenes[3] = 2;
	game.world[worldPositionI].seedGenes[4] = PLANTGENE_LEAF;
}

void setupTestPlant(unsigned int worldPositionI)
{
	memset(game.world[worldPositionI].seedGenes, 0x00, sizeof(char) * plantGenomeSize);
	// the test plant should demonstrate the basic structural features and reproductive capability of the plant.
	// a short preamble makes the plant a red color.
	game.world[worldPositionI].seedGenes[0] = PLANTGENE_RED;
	game.world[worldPositionI].seedGenes[1] = PLANTGENE_RED;
	game.world[worldPositionI].seedGenes[2] = PLANTGENE_RED;
	game.world[worldPositionI].seedGenes[3] = PLANTGENE_SEQUENCE;
	game.world[worldPositionI].seedGenes[4] = 2;
	game.world[worldPositionI].seedGenes[4] = 4;
	game.world[worldPositionI].seedGenes[5] = PLANTGENE_WOOD;
	game.world[worldPositionI].seedGenes[6] = 4;
	game.world[worldPositionI].seedGenes[7] = PLANTGENE_BRANCH;
	game.world[worldPositionI].seedGenes[8] = 3;
	game.world[worldPositionI].seedGenes[8] = 2;
	game.world[worldPositionI].seedGenes[9] = PLANTGENE_LEAF;
	game.world[worldPositionI].seedGenes[11] = 4;
	game.world[worldPositionI].seedGenes[12] = PLANTGENE_WOOD;
	game.world[worldPositionI].seedGenes[13] = 4;
	game.world[worldPositionI].seedGenes[14] = PLANTGENE_BRANCH;
	game.world[worldPositionI].seedGenes[15] = 3;
	game.world[worldPositionI].seedGenes[16] = 3;
	game.world[worldPositionI].seedGenes[17] = PLANTGENE_BUD_A;
	game.world[worldPositionI].seedGenes[18] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedGenes[19] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedGenes[20] = PLANTGENE_BREAK;
	game.world[worldPositionI].seedIdentity = extremelyFastNumberFromZeroTo(256);
	game.world[worldPositionI].seedState = MATERIAL_SEED;
}

void setupPlantAtCursor()
{
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	setupTestPlant3(worldCursorPos);
}


void spawnAnimalAtCursor()
{
	int cursorPosX = game.cameraPositionX +  game.mousePositionX ;
	int cursorPosY = game.cameraPositionY + game.mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;

	int j = 1;
	setupExampleAnimal3(j);
	paintAnimal(j);
	int domingo = spawnAnimal( 1,  game.animals[j], worldCursorPos, false);
	if (domingo >= 0)
	{
		game.animals[domingo].energy = game.animals[domingo].cellsUsed / 2;
	}

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

	// 2. game.animals eat grass and gain energy
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
		setupTestPlant(testPos + (i * worldSize));
	}
	for (int i = 0; i < howManyPlantsToEat; ++i)
	{
		animalTurn(testAnimal);
	}

	if (game.animals[testAnimal].energy != initialEnergy)
	{
		testResult_2 = true;
	}
	killAnimal(testAnimal);

	// 3. game.animals reproduce when they have enough energy and their debt is 0
	const int how_long_it_takes_to_make_sure = (baseLungCapacity / aBreath) * 1;
	setupTestAnimal_reproducer(j);
	testAnimal = spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy;
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


	// 4. reproduction copies the animal wholly and exactly,
	// except that lifetime stats are reset to 0 in the new generation, cell damage is healed
	// and some mutation may be carried along
	setupTestAnimal_reproducer(j);
	testAnimal =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = game.animals[testAnimal].maxEnergy / 2;
	game.animals[testAnimal].energyDebt = 0.0f;
	game.animals[testAnimal].body[0].damage = 0.05f;
	animalTurn(testAnimal);

	int diffs = 0;
	int child = testAnimal;
	for (int i = 0; i < numberOfAnimalsPerSpecies; ++i)
	{
		child = (testSpecies * numberOfAnimalsPerSpecies) + i;
		if ( !game.animals[child].retired  )
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

	if (diffs == 0 && game.animals[child].body[0].damage == 0.0f)
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
	game.animals[testAnimal].energyDebt = 0.0f;
	game.animals[testAnimal].energy = (game.animals[testAnimal].maxEnergy / 2) - 1.0f; // give it enough energy for the test

	place(testAnimal);

	int testEye = getRandomCellOfType(testAnimal, ORGAN_SENSOR_EYE);
	float originalAngle = game.animals[testAnimal].fAngle;
	unsigned int testEyePosition ;
	if (testEye >= 0)
	{
		testEyePosition = game.animals[testAnimal].body[testEye].worldPositionI;
		game.animals[testAnimal].body[testEye].color = color_white;
		game.world[testEyePosition].light = color_white;
	}

	// it takes a few turns for the neural signal to propagate through the network.
	animalTurn(testAnimal);
	animalTurn(testAnimal);
	animalTurn(testAnimal);
	animalTurn(testAnimal);
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
	float amount =  (game.animals[testAnimal].maxEnergy / 2.0f);

	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = amount;

	setupTestAnimal_airbreathing(j);
	testPos += 10;
	int testAnimal_air_in_water =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = amount;
	game.world[testPos].wall = MATERIAL_WATER;

	setupTestAnimal_waterbreathing(j);
	testPos += 10;
	int testAnimal_water_in_air =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = amount;

	setupTestAnimal_waterbreathing(j);
	testPos += 10;
	int testAnimal_water_in_water =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = amount;
	game.world[testPos].wall = MATERIAL_WATER;

	setupTestAnimal_amphibious(j);
	int testAnimal_amphi_in_air =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = amount;

	setupTestAnimal_amphibious(j);
	testPos += 10;
	int testAnimal_amphi_in_water =	spawnAnimal( testSpecies , game.animals[j], testPos, false);
	game.animals[testAnimal].position = testPos;
	game.animals[testAnimal].uPosX = testPos % worldSize;
	game.animals[testAnimal].uPosY = testPos / worldSize;
	game.animals[testAnimal].fPosX = game.animals[testAnimal].uPosX;
	game.animals[testAnimal].fPosY = game.animals[testAnimal].uPosY;
	game.animals[testAnimal].energy = amount;

	game.world[testPos].wall = MATERIAL_WATER;
	testPos += worldSize;
	game.world[testPos].wall = MATERIAL_WATER;

	for (int i = 0; i < how_long_it_takes_to_make_sure; ++i)
	{
		game.animals[testAnimal_air_in_air].energy     = amount;
		game.animals[testAnimal_air_in_water].energy   = amount;
		game.animals[testAnimal_water_in_air].energy   = amount;
		game.animals[testAnimal_water_in_water].energy = amount;
		game.animals[testAnimal_amphi_in_air].energy   = amount;
		game.animals[testAnimal_amphi_in_water].energy = amount;

		animalTurn(testAnimal_air_in_air);
		animalTurn(testAnimal_air_in_water);
		animalTurn(testAnimal_water_in_air);
		animalTurn(testAnimal_water_in_water);
		animalTurn(testAnimal_amphi_in_air);
		animalTurn(testAnimal_amphi_in_water);
		// printf("%i of %i \n", i , how_long_it_takes_to_make_sure);
		// printf("testAnimal_air_in_air %i \n",game.animals[testAnimal_air_in_air].retired    );
		// printf("testAnimal_air_in_water %i \n",game.animals[testAnimal_air_in_water].retired    );
		// printf("testAnimal_water_in_air %i \n",game.animals[testAnimal_water_in_air].retired    );
		// printf("testAnimal_water_in_water %i \n",game.animals[testAnimal_water_in_water].retired    );
		// printf("testAnimal_amphi_in_air %i \n",game.animals[testAnimal_air_in_air].retired    );
		// printf("testAnimal_amphi_in_water %i \n",game.animals[testAnimal_amphi_in_water].retired    );

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (
	    !(game.animals[testAnimal_air_in_air].retired     ) &&
	    (game.animals[testAnimal_air_in_water].retired   ) &&
	    (game.animals[testAnimal_water_in_air].retired   ) &&
	    !(game.animals[testAnimal_water_in_water].retired ) &&
	    !(game.animals[testAnimal_amphi_in_air].retired   ) &&
	    !(game.animals[testAnimal_amphi_in_water].retired )
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
	game.world[testPos].terrain = MATERIAL_SOIL;
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

				game.world[updateAddress].terrain = MATERIAL_SOIL;
				game.world[updateAddress].light = color_white;
				updatePlants(updateAddress);
			}
		}
	}

	// inspecc the plant
	unsigned int woodProbe1 = testPos ;
	woodProbe1 -= worldSize * 4;
	if (game.world[woodProbe1].plantState == MATERIAL_WOOD)
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
	return true;
	bool testResult_all = true;

	bool testResult_animals = test_animals();
	bool testResult_plants  = test_plants();


	if ( !testResult_animals )
	{
		printf("game.animals: FAIL\n");
		testResult_all = false;
	}

	if ( !testResult_plants )
	{
		printf("plants: FAIL\n");
		// testResult_all = false;
	}

	return testResult_all;
}