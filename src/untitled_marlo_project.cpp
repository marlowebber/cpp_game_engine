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
const int worldSize = 4096;
const int cameraPanSpeed = 10;

const float baseLungCapacity = 1.0f;
const unsigned int prelimSquareSize = prelimSize * prelimSize;
const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.
const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const unsigned int animalSquareSize      = 128;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfSpecies = 8;
const unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);

const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.00002f;        // a multiplier for how much it costs animals just to exist.
const float movementEnergyScale    = 0.00002f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.9f;         // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float grassEnergy            = 0.3f;         // how much you get from eating a square of grass
const float neuralNoise = 0.1f;
const float liverStorage = 20.0f;
const unsigned int baseLifespan = 50000;			// if the lifespan is long, the animal's strategy can have a greater effect on its success. If it's very short, the animal is compelled to be just a moving mouth.
const float signalPropagationConstant = 0.1f;      // how strongly sensor organs compel the animal.
const float musclePower = 40.0f;
const float thresholdOfBoredom = 0.1f;
const unsigned int displayNameSize = 32;
const unsigned int numberOfSpeakerChannels = 16;
const float const_pi = 3.1415f;

const int paletteMenuX = 200;
const int paletteMenuY = 50;
const int paletteTextSize = 10;
const int paletteSpacing = 20;
const unsigned int paletteWidth = 3;
const unsigned int nLogs = 32;
const unsigned int logLength = 64;




// these are the parameters that set up the physical geography of the world.
const float seaLevel =  0.5f * worldSize;;;
const float biome_marine  = seaLevel + (worldSize / 20);
const float biome_coastal = seaLevel + (worldSize / 3);
const float sunXangle = 0.35f;
const float sunYangle = 0.35f;
const unsigned int baseSize = 100;
const unsigned int wallThickness = 8;
const unsigned int doorThickness = 16;

float prelimMap[prelimSquareSize];
float prelimWater[prelimSquareSize];
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

unsigned int worldCreationStage = 0;


int menuX = 50;
int menuY = 50;
int textSize = 10;
int spacing = 20;


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




bool mainMenu = true;





// telemetry of the game's performance
unsigned int modelFrameCount = 0;
unsigned int usPerFrame = 0;
float fps = 1.0f;



bool flagQuit = false;
bool flagCreate = false;
bool flagLoad = false;
bool flagReady = false;
bool flagReturn = false;

int mouseX;
int mouseY;

const float panSpeed = 0.1f;
const float playerSpeed = 0.3f;






// these parts need to be recorded in between sessions.
struct GameState()
{

// these variables are how the player drives their character, and what they get back from it.
	bool playerGrabState ;
	bool playerCanSee ;
	bool playerCanHear ;
	bool playerCanSmell ;
	bool palette ;
	bool playerCanPickup ;
	int playerCanPickupItem;
	bool lockfps;
	bool paused ;
	int mousePositionX ;
	int mousePositionY ;

	// these variables keep track of the main characters in the game world.
	int playerCreature ;
	int championScore ;
	float championEnergyScore ;
	int adversary = -1;
	unsigned int adversaryRespawnPos;
	int selectedAnimal ;
	int cursorAnimal ;
	unsigned int playerRespawnPos;
	Animal champion;

	// camera view
	unsigned int cameraPositionX;
	unsigned int cameraPositionY;
	int cameraTargetCreature ;

	// these variables govern the display of menus and other texts.
	bool showInstructions ;
	bool ecologyComputerDisplay ;
	int visualizer ;
	bool computerdisplays[5];
	char logs[logLength][nLogs];
	unsigned int paletteSelectedOrgan ;

	bool speciesVacancies [numberOfSpecies];
	unsigned int speciesPopulationCounts [numberOfSpecies];
	unsigned int populationCountUpdates  [numberOfSpecies];
	unsigned int speciesAttacksPerTurn   [numberOfSpecies];

	float speakerChannels[numberOfSpeakerChannels];
	float speakerChannelsLastTurn[numberOfSpeakerChannels];

	Animal animals[numberOfAnimals];

	struct Square world[worldSquareSize];


} ;






class DeepSea
{

	GameState game;

public:



	void activateGrabbedMachine()// occurs whenever a left click is received.
	{
		if (playerCreature >= 0)
		{
			for (int i = 0; i < animals[playerCreature].cellsUsed; ++i)
			{
				if (animals[playerCreature].body[i].organ == ORGAN_GRABBER)
				{
					if (animals[playerCreature].body[i].grabbedCreature >= 0 && animals[playerCreature].body[i].grabbedCreature < numberOfAnimals)
					{
						if (animals [   animals[playerCreature].body[i].grabbedCreature  ].isMachine)
						{
							if (animals [   animals[playerCreature].body[i].grabbedCreature  ].machineCallback !=  MATERIAL_NOTHING)
							{
								switch (animals [   animals[playerCreature].body[i].grabbedCreature  ].machineCallback )
								{
								case MACHINECALLBACK_KNIFE :
									knifeCallback(animals[playerCreature].body[i].grabbedCreature , playerCreature  );
									break;
								case MACHINECALLBACK_PISTOL :
									exampleGunCallback(animals[playerCreature].body[i].grabbedCreature , playerCreature  );
									break;
								case MACHINECALLBACK_LIGHTER :
									lighterCallback(animals[playerCreature].body[i].grabbedCreature , playerCreature  );
									break;
								case MACHINECALLBACK_HOSPITAL :
									paletteCallback(animals[playerCreature].body[i].grabbedCreature , playerCreature  );
									break;
								case MACHINECALLBACK_MESSAGECOMPUTER :
									communicationComputerCallback(animals[playerCreature].body[i].grabbedCreature , playerCreature  );
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
		if (selectedAnimal >= 0)
		{
			selectedAnimal = -1;
		}
		else
		{
			int cursorPosX = cameraPositionX +  mousePositionX ;
			int cursorPosY = cameraPositionY + mousePositionY;
			unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
			if (cursorAnimal >= 0 && cursorAnimal < numberOfAnimals)
			{
				int occupyingCell = isAnimalInSquare(cursorAnimal, worldCursorPos);
				if ( occupyingCell >= 0)
				{
					selectedAnimal = cursorAnimal;
				}
			}
		}
	}

	void rightClickCallback ()
	{
		if (palette)
		{
			paletteEraseAtMouse();
		}
	}


	void viewAdversary()
	{
		if (cameraTargetCreature == adversary)
		{
			cameraTargetCreature = -1;
			if (playerCreature >= 0)
			{
				cameraTargetCreature = playerCreature;
			}
		}
		else
		{
			if (adversary >= 0)
			{
				cameraTargetCreature = adversary;
			}
		}
	}

	void toggleInstructions()
	{
		showInstructions = !showInstructions;
	}


	void resetMouseCursor(  )
	{
		mousePositionX = 0;
		mousePositionY = 0;
	}

	void togglePause ()
	{
		paused = !paused;
	}

	void incrementSelectedOrgan()
	{
		paletteSelectedOrgan++;
		paletteSelectedOrgan = paletteSelectedOrgan % numberOfOrganTypes;
	}
	void decrementSelectedOrgan()
	{
		paletteSelectedOrgan--;
		paletteSelectedOrgan = paletteSelectedOrgan % numberOfOrganTypes;
	}

	void playerGrab()
	{
		if (playerCreature >= 0)
		{
			for (int i = 0; i < animals[playerCreature].cellsUsed; ++i)
			{
				if (animals[playerCreature].body[i].organ == ORGAN_GRABBER)
				{
					if (animals[playerCreature].body[i].grabbedCreature == -1)
					{
						animals[playerCreature].body[i].signalIntensity = 1;
					}
				}
			}
		}
	}

	void playerDrop()
	{
		if (playerCreature >= 0)
		{
			for (int i = 0; i < animals[playerCreature].cellsUsed; ++i)
			{
				if (animals[playerCreature].body[i].organ == ORGAN_GRABBER)
				{
					if (animals[playerCreature].body[i].grabbedCreature >= 0)
					{
						animals[playerCreature].body[i].signalIntensity = -1;
					}
				}
			}
		}
	}

	void adjustPlayerPos(Vec_f2 pos)
	{
		if (playerCreature >= 0)
		{
			animals[playerCreature].fAngle = 0.0f;
			int strafeMuscle = getRandomCellOfType(playerCreature, ORGAN_MUSCLE_STRAFE);
			int muscle = getRandomCellOfType(playerCreature, ORGAN_MUSCLE);
			if (strafeMuscle >= 0)
			{
				animals[playerCreature].body[strafeMuscle].signalIntensity = pos.y;
			}
			if (muscle >= 0)
			{
				animals[playerCreature].body[muscle].signalIntensity = pos.x;
			}
		}
	}


private:


};





void resetGameState(GameState game)
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


	// these variables keep track of the main characters in the game world.
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
		memcpy(  &logs[i] , &logs[i - 1], sizeof(char) * logLength   );
	}
	strcpy( &logs[0][0] , input.c_str() );
}




void resetConnection(unsigned int animalIndex, unsigned int cellLocalPositionI, unsigned int i)
{
	animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;
	animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	animals[animalIndex].body[cellLocalPositionI].connections[i].weight = RNG() - 0.5f;
}



void resetCell(unsigned int animalIndex, unsigned int cellLocalPositionI)
{
	animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
	animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
	animals[animalIndex].body[cellLocalPositionI].color  = color_darkgrey;
	animals[animalIndex].body[cellLocalPositionI].damage = 0.0f;
	animals[animalIndex].body[cellLocalPositionI].eyeLookX = 0;
	animals[animalIndex].body[cellLocalPositionI].eyeLookY = 0;
	animals[animalIndex].body[cellLocalPositionI].localPosX = 0;
	animals[animalIndex].body[cellLocalPositionI].localPosY = 0;
	animals[animalIndex].body[cellLocalPositionI].grabbedCreature = -1;

	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		resetConnection(animalIndex, cellLocalPositionI, i);
	}
	animals[animalIndex].genes[cellLocalPositionI] = animals[animalIndex].body[cellLocalPositionI];
}

void paintAnimal(unsigned int animalIndex)
{
	Color newAnimalColorA = Color(RNG(), RNG(), RNG(), 1.0f);
	Color newAnimalColorB = Color(RNG(), RNG(), RNG(), 1.0f);
	for (int i = 0; i < animalSquareSize; ++i)
	{
		animals[animalIndex].body[i].color = filterColor(  newAnimalColorA , multiplyColorByScalar( newAnimalColorB , RNG())  );
		animals[animalIndex].genes[i].color = animals[animalIndex].body[i].color ;
	}
}

void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		std::string gunDescription = std::string("An animal");
		strcpy( &animals[animalIndex].displayName[0] , gunDescription.c_str() );
		animals[animalIndex].mass = 0;
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
		animals[animalIndex].cellsUsed = 0;
		animals[animalIndex].identityColor = Color(RNG(), RNG(), RNG(), 1.0f);
		if (animalIndex == adversary)
		{
			animals[animalIndex].identityColor = color_white;
		}
		animals[animalIndex].isMachine = false;
		animals[animalIndex].machineCallback = MATERIAL_NOTHING;
		animals[animalIndex].temp_limit_low = 273.0f;
		animals[animalIndex].temp_limit_high = 323.0f;
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
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX - 1 , animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX     , animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX + 1 , animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX - 1 , animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX + 1 , animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX - 1 , animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX     , animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX + 1 , animals[animalIndex].genes[cellIndex].localPosY + 1  ),
	};
	for (int potentialNeighbour = 0; potentialNeighbour < animals[animalIndex].cellsUsed; ++potentialNeighbour)
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			if (animals[animalIndex].genes[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        animals[animalIndex].genes[potentialNeighbour].localPosY == locations_to_check[i].y  )
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

Vec_i2 getRandomEmptyEdgeLocation(unsigned int animalIndex)
{
	unsigned int cellIndex = getRandomEdgeCell(animalIndex);
	Vec_i2 result = Vec_i2(0, 0);
	// get an edge cell at random then search its neighbours to find the empty one. return the position of the empty neighbour.
	Vec_i2 locations_to_check[nNeighbours] =
	{
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX - 1 , animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX     , animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX + 1 , animals[animalIndex].genes[cellIndex].localPosY - 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX - 1 , animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX + 1 , animals[animalIndex].genes[cellIndex].localPosY   ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX - 1 , animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX     , animals[animalIndex].genes[cellIndex].localPosY + 1  ),
		Vec_i2(  animals[animalIndex].genes[cellIndex].localPosX + 1 , animals[animalIndex].genes[cellIndex].localPosY + 1  ),
	};
	for (int i = 0; i < nNeighbours; ++i)
	{
		bool empty = true;
		for (int potentialNeighbour = 0; potentialNeighbour < animals[animalIndex].cellsUsed; ++potentialNeighbour)
		{
			if (animals[animalIndex].genes[potentialNeighbour].localPosX == locations_to_check[i].x  &&
			        animals[animalIndex].genes[potentialNeighbour].localPosY == locations_to_check[i].y  )
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnectable(  animals[animalIndex].genes[cellIndex].organ ))
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
	unsigned int cellIndex = animals[animalIndex].cellsUsed;
	if (cellIndex < animalSquareSize)
	{
		animals[animalIndex].cellsUsed ++;
		animals[animalIndex].genes[cellIndex].localPosX = newPosition.x;
		animals[animalIndex].genes[cellIndex].localPosY = newPosition.y;
		animals[animalIndex].genes[cellIndex].organ = organType;

		if (  isCellConnecting(organType)) // if the cell is supposed to have connections, go hook it up
		{
			unsigned int randomNumberOfConnections = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS);
			for (int i = 0; i < randomNumberOfConnections; ++i)
			{

				unsigned int connectableCell = getRandomConnectableCell( animalIndex);// pick a random connectable cell to connect to.
				bool alreadyConnected =  false;	// check if you are already connected to it.
				for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
				{
					if (  animals[animalIndex].genes[cellIndex].connections[j].connectedTo == connectableCell &&
					        animals[animalIndex].genes[cellIndex].connections[j] .used)
					{
						alreadyConnected = true;
					}
				}
				if (!alreadyConnected)	// make the new connection if appropriate.
				{
					for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
					{
						if ( ! (animals[animalIndex].genes[cellIndex].connections[j].used))
						{
							animals[animalIndex].genes[cellIndex].connections[j].used = true;
							animals[animalIndex].genes[cellIndex].connections[j].connectedTo = connectableCell;
							animals[animalIndex].genes[cellIndex].connections[j].weight = (RNG() - 0.5f ) * 2;
							break;
						}
					}
				}
			}
		}
	}
	animals[animalIndex].body[cellIndex] = animals[animalIndex].genes[cellIndex] ;
}

// add a cell to an animal germline in a guided but random way. Used to messily construct new animals, for situations where lots of variation is desirable.
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
	champion = animals[j];
	championScore = 0;
}

Vec_f2 getTerrainSlope(unsigned int worldPositionI)
{
	float xSurfaceAngle = world[worldPositionI].height - world[worldPositionI + 1].height ;
	float ySurfaceAngle = world[worldPositionI].height - world[worldPositionI + worldSize].height ;
	return Vec_f2(xSurfaceAngle, ySurfaceAngle);
}


void detailTerrain(unsigned int worldPositionI)
{
	Vec_f2 slope = getTerrainSlope(worldPositionI);
	float grade = sqrt( (slope.x * slope.x) +  (slope.y * slope.y)  );
	float colorNoise = 1 + (((RNG() - 0.5f) * 0.35)) ; // map -1,1 to 0,0.8
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

void resetGrid()
{
	for (int i = 0; i < worldSquareSize; ++i)
	{
		world[i].terrain = MATERIAL_NOTHING;
		world[i].material = MATERIAL_NOTHING;
		world[i].identity = -1;
		world[i].trail = 0.0f;
		world[i].height = 0.0f;
		world[i].light = color_white;
		world[i].pheromoneIntensity = 0.0f;
		world[i].pheromoneChannel = -1;
		world[i].grassColor =  color_green;
	}
}

int getNewIdentity(unsigned int speciesIndex)
{
	if (!speciesVacancies[speciesIndex])
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
	speciesVacancies[speciesIndex] = false;
	return -1;
}

// // some genes have permanent effects, or effects that need to be known immediately at birth. Compute them here.
// this function studies the phenotype, not the genotype.
// returns whether the animal is fit to live.
bool measureAnimalQualities(unsigned int animalIndex)
{
	animals[animalIndex].mass = 0;
	animals[animalIndex].energyDebt = 0.0f;
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		animals[animalIndex].mass++;
		animals[animalIndex].energyDebt += 1.0f;
	}
	animals[animalIndex].totalMuscle = 0;
	animals[animalIndex].offspringEnergy = 1.0f;
	animals[animalIndex].lifespan = baseLifespan;
	unsigned int totalGonads = 0;
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
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_ADDLIFESPAN)
		{
			animals[animalIndex].lifespan += baseLifespan;
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_GONAD)
		{
			totalGonads ++;
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_COLDADAPT)
		{
			animals[animalIndex].temp_limit_low -= 35.0f;
			animals[animalIndex].temp_limit_high -= 35.0f;
		}
		if (animals[animalIndex].body[cellIndex].organ == ORGAN_HEATADAPT)
		{
			animals[animalIndex].temp_limit_high += 35.0f;
			animals[animalIndex].temp_limit_low  += 35.0f;
		}
	}
	animals[animalIndex].lifespan *= 0.75 + (RNG() * 0.5);
	if (animals[animalIndex].mass > 0 && totalGonads > 0 )
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (isCellConnecting(  animals[animalIndex].genes[cellIndex].organ ))
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].genes[cellIndex].organ == organType)
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].genes[cellIndex].organ == organType)
		{
			cellsOfType.push_back(cellIndex);
			found++;
		}
	}
	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
		return animals[animalIndex] .body[(*iterator)].speakerChannel;
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
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		if (animals[animalIndex].body[cellIndex].speakerChannel == channel)
		{
			animals[animalIndex].body[cellIndex].speakerChannel = newChannel;
		}
	}
}

// choose any random populated cell.
int getRandomPopulatedCell(unsigned int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
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
	for (int cellIndex = cellToDelete + 1; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex) // shift array of cells down 1, overwriting the lowest modified cell (the cell to delete)
	{
		animals[animalIndex].genes[cellIndex - 1] = animals[animalIndex].genes[cellIndex];
	}
	animals[animalIndex].cellsUsed--; // clear the end cell which would have been duplicated
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)	// go through all cells and update connections
	{
		for (int connectionIndex = 0; connectionIndex < NUMBER_OF_CONNECTIONS; ++connectionIndex)
		{
			if (animals[animalIndex].genes[cellIndex].connections[connectionIndex].connectedTo == cellToDelete	)
			{
				animals[animalIndex].genes[cellIndex].connections[connectionIndex].used = false;
			}
			else if (animals[animalIndex].genes[cellIndex].connections[connectionIndex].connectedTo > cellToDelete)
			{
				animals[animalIndex].genes[cellIndex].connections[connectionIndex].connectedTo --;
			}
		}
		animals[animalIndex].body[cellIndex] = animals[animalIndex].genes[cellIndex] ;
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
			animals[animalIndex].genes[mutantCell].connections[mutantConnection].used = !(animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
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
				animals[animalIndex].genes[mutantCell].connections[mutantConnection].connectedTo = mutantPartner;
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
				animals[animalIndex].genes[mutantCell].connections[mutantConnection].weight *= ((RNG() - 0.5) * 4);
			}
			else // add to it
			{
				animals[animalIndex].genes[mutantCell].connections[mutantConnection].weight += ((RNG() - 0.5 ) * 2);
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
				animals[animalIndex].genes[mutantCell].signalIntensity *= ((RNG() - 0.5 ) * 4);;
			}
			else // add to it
			{
				animals[animalIndex].genes[mutantCell].signalIntensity += ((RNG() - 0.5 ) * 2);;
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
			unsigned int cellIndex = (startingRandomCell + i) % animals[animalIndex].cellsUsed;
			if ( organUsesSpeakerChannel( animals[animalIndex].body[cellIndex].organ )  )
			{
				occupiedChannel =	 findOccupiedChannel( animalIndex, animals[animalIndex].body[cellIndex].organ);
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
				animals[animalIndex].genes[mutantCell].eyeLookX += (extremelyFastNumberFromZeroTo(2) - 1);
			}
			else
			{
				animals[animalIndex].genes[mutantCell].eyeLookY += (extremelyFastNumberFromZeroTo(2) - 1);
			}
		}
		break;
	}
	case MUTATION_SKINCOLOR:// mutate a cell's skin color
	{
		int mutantCell = getRandomPopulatedCell(animalIndex);
		if (mutantCell >= 0 && mutantCell < animalSquareSize)
		{
			animals[animalIndex].genes[mutantCell].color = mutateColor(	animals[animalIndex].genes[mutantCell].color);
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
		animals[animalIndex].genes[i] = parent.genes[i];
		animals[animalIndex].body[i] = parent.genes[i];
	}
	animals[animalIndex].isMachine = parent.isMachine;
	animals[animalIndex].machineCallback = parent.machineCallback;
	animals[animalIndex].cellsUsed = parent.cellsUsed;
	animals[animalIndex].retired = false;
	animals[animalIndex].position = position;
	animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = position / worldSize;
	animals[animalIndex].birthLocation = position;
	animals[animalIndex].fAngle = ( (RNG() - 0.5f) * 2 * 3.141f  );
	mutateAnimal( animalIndex);
	measureAnimalQualities(animalIndex);
	memcpy( &( animals[animalIndex].displayName[0]), &(parent.displayName[0]), sizeof(char) * displayNameSize  );
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
	if (animalIndex == playerCreature)
	{
		playerCreature = -1;
	}
	if (animalIndex == cameraTargetCreature)
	{
		cameraTargetCreature = -1;
	}
	unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
	speciesVacancies[speciesIndex] = true;
	animals[animalIndex].retired = true;
	unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
	for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex) // process organs and signals and clear animalIndex on grid
	{
		unsigned int cellLocalPositionX  = animals[animalIndex].body[cellIndex].localPosX;
		unsigned int cellLocalPositionY  = animals[animalIndex].body[cellIndex].localPosY;
		unsigned int cellWorldPositionX  = cellLocalPositionX + animalWorldPositionX;
		unsigned int cellWorldPositionY  = cellLocalPositionY + animalWorldPositionY;
		unsigned int cellWorldPositionI  = (cellWorldPositionY * worldSize) + cellWorldPositionX;
		if (cellWorldPositionI < worldSquareSize)
		{
			if (animals[animalIndex].body[cellIndex].organ != MATERIAL_NOTHING)
			{
				world[cellWorldPositionI].pheromoneChannel = 13;
				world[cellWorldPositionI].pheromoneIntensity = 1.0f;
				if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
				{
					world[cellWorldPositionI].material = MATERIAL_FOOD;
				}
				if (animals[animalIndex].body[cellIndex].organ == ORGAN_BONE)
				{
					world[cellWorldPositionI].material = MATERIAL_BONE;
				}
			}
		}
	}
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionI)
{
	if (cellWorldPositionI < worldSquareSize && world[cellWorldPositionI].identity >= 0 )
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
	return -1;
}


Color whatColorIsThisSquare(  unsigned int worldI)
{
	Color displayColor = color_black;
	int viewedAnimal = -1;
	unsigned int animalIndex = world[worldI].identity;
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
		if ( animals[viewedAnimal].body[occupyingCell].damage > 0.5f || organVisible(animals[viewedAnimal].body[occupyingCell].organ ) )
		{
			displayColor = organColors(animals[viewedAnimal].body[occupyingCell].organ );
		}
		else
		{
			displayColor = animals[viewedAnimal].body[occupyingCell].color;
		}
		if (viewedAnimal == selectedAnimal) // highlight selected animal.
		{
			displayColor =  addColor(displayColor, tint_selected);
		}
	}
	else
	{
		Color materialColor ;
		if (world[worldI].material == MATERIAL_GRASS )
		{
			materialColor = world[worldI].grassColor;
		}
		else
		{
			materialColor = materialColors(world[worldI].material);
		}
		displayColor = filterColor( materialColors(world[worldI].terrain) ,  materialColor);		// you can see the three material layers in order, wall then material then floor.
		Color wallColor = addColor(materialColors(world[worldI].wall), tint_wall);
		displayColor = filterColor( displayColor, wallColor  );
	}
	displayColor = multiplyColor(displayColor, world[worldI].light);
	return displayColor;
}

float getNormalisedHeight(unsigned int worldPositionI)
{
	float answer =   world[worldPositionI].height / (worldSize);
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
			if (world[randomI].pheromoneIntensity > 0.2f)
			{
				world[randomI].pheromoneIntensity -= 0.2f;
			}
			else
			{
				world[randomI].pheromoneChannel = -1;
			}
			if (world[randomI].height < seaLevel)
			{
				if (world[randomI].wall == MATERIAL_NOTHING)
				{
					world[randomI].wall = MATERIAL_WATER;
				}
			}
			else
			{
				if (world[randomI].wall == MATERIAL_WATER)
				{
					world[randomI].wall = MATERIAL_NOTHING;
				}
			}
			if (world[randomI].material == MATERIAL_FIRE)
			{
				for (int i = 0; i < nNeighbours; ++i)
				{
					unsigned int neighbour = randomI + neighbourOffsets[i];
					if (neighbour < worldSquareSize)
					{
						if (world[neighbour].material == MATERIAL_GRASS)
						{
							world[randomI].material = MATERIAL_FIRE;
						}
					}
				}
				if (extremelyFastNumberFromZeroTo(1) == 0)
				{
					world[randomI].material = MATERIAL_NOTHING;
				}
				else
				{
					unsigned int neighbour = randomI + neighbourOffsets[extremelyFastNumberFromZeroTo(nNeighbours - 1)];
					if (neighbour < worldSquareSize)
					{
						world[neighbour].material = MATERIAL_FIRE;
					}
				}
			}
			if (world[randomI].material == MATERIAL_GRASS)
			{
				for (int n = 0; n < nNeighbours; ++n)
				{
					unsigned int neighbour = randomI + neighbourOffsets[n];
					if (neighbour < worldSquareSize)
					{
						if (world[neighbour].material == MATERIAL_NOTHING && !materialBlocksMovement(world[neighbour].wall ) &&  materialSupportsGrowth(world[neighbour].terrain ) )
						{
							float growthChance =  0.0f;// grow speed proportional to light brightness
							if (RNG() < world[neighbour].light.a)
							{
								world[neighbour].material = MATERIAL_GRASS;
								world[neighbour].grassColor = world[randomI].grassColor;
								world[neighbour].grassColor.r += (RNG() - 0.5f) * 0.1f;
								world[neighbour].grassColor.g += (RNG() - 0.5f) * 0.1f;
								world[neighbour].grassColor.b += (RNG() - 0.5f) * 0.1f;
								world[neighbour].grassColor = clampColor(world[neighbour].grassColor);
							}
						}
					}
				}
			}
			if ( materialDegrades( world[randomI].material) )
			{
				world[randomI].material = MATERIAL_NOTHING;
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
			if (animals[animalIndex].body[occupyingCell].organ == ORGAN_BONE)
			{
				nBones++;
			}
		}
	}
	return nBones;
}

Vec_i2 getMousePositionRelativeToAnimal(unsigned int animalIndex)
{
	int newPosX = mousePositionX -   ( cameraPositionX  - animals[animalIndex].uPosX);
	int newPosY = mousePositionY -   ( cameraPositionY  - animals[animalIndex].uPosY);
	return Vec_i2(newPosX, newPosY);
}

void paletteCallback( int gunIndex, int shooterIndex )	// add the selected organ to the selected animal
{
	if (selectedAnimal >= 0 && selectedAnimal < numberOfAnimals)
	{
		if (paletteSelectedOrgan >= 0 && paletteSelectedOrgan < numberOfOrganTypes)
		{
			Vec_i2 newpos = getMousePositionRelativeToAnimal(selectedAnimal);
			appendCell(selectedAnimal, paletteSelectedOrgan, newpos);
		}
	}
}

void paletteEraseAtMouse()
{
	if (selectedAnimal >= 0 && selectedAnimal < numberOfAnimals)
	{
		Vec_i2 newpos = getMousePositionRelativeToAnimal(selectedAnimal);
		for (int i = 0; i < animals[selectedAnimal].cellsUsed; ++i)
		{
			if (animals[selectedAnimal].body[i].localPosX == newpos.x &&
			        animals[selectedAnimal].body[i].localPosY == newpos.y )
			{
				eliminateCell(selectedAnimal, i);
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
		if (i == paletteSelectedOrgan)
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
	if (gunIndex == 9) { computerdisplays[0] = !computerdisplays[0] ;}
	if (gunIndex == 10) { computerdisplays[1] = !computerdisplays[1] ;}
	if (gunIndex == 11) { computerdisplays[2] = !computerdisplays[2] ;}
	if (gunIndex == 12 ) { computerdisplays[3] = !computerdisplays[3] ;}
	if (gunIndex == 13) { computerdisplays[4] = !computerdisplays[4] ;}
}

void spillBlood(unsigned int worldPositionI)
{
	if (world[worldPositionI].material == MATERIAL_NOTHING)
	{
		world[worldPositionI].material = MATERIAL_BLOOD;
	}
	else
	{
		for (int i = 0; i < nNeighbours; ++i)
		{
			unsigned int neighbour = worldPositionI += neighbourOffsets[i];
			if ( neighbour < worldSquareSize)
			{
				if (world[neighbour].material == MATERIAL_NOTHING)
				{
					world[neighbour].material = MATERIAL_BLOOD;
					break;
				}
			}
		}
	}
}

// return true if you blow the limb off, false if its still attached.
bool hurtAnimal(unsigned int animalIndex, unsigned int cellIndex, float amount)
{
	unsigned int cellWorldPositionI = animals[animalIndex].body[cellIndex].worldPositionI;
	float defense = defenseAtWorldPoint(world[cellWorldPositionI].identity, cellWorldPositionI);
	if (defense > 0)
	{
		amount = amount / defense;
	}
	animals[animalIndex].body[cellIndex].damage += amount;
	spillBlood(cellWorldPositionI);
	int painCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_PAIN);
	if (painCell >= 0)
	{
		animals[animalIndex].body[painCell].signalIntensity += amount;
	}
	if (animals[animalIndex].body[cellIndex].damage > 1.0f)
	{
		animals[animalIndex].damageReceived++;
		animals[animalIndex].mass--;
		return true;
	}
	return false;
}

void knifeCallback( int gunIndex, int shooterIndex )
{
	int cursorPosX = cameraPositionX +  mousePositionX ;
	int cursorPosY = cameraPositionY + mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	if (cursorAnimal >= 0 && cursorAnimal < numberOfAnimals)
	{
		int occupyingCell = isAnimalInSquare(cursorAnimal, worldCursorPos);
		if ( occupyingCell >= 0)
		{
			hurtAnimal(cursorAnimal, occupyingCell, 0.3f);
		}
	}
}

void exampleGunCallback( int gunIndex, int shooterIndex)
{
	if (gunIndex >= 0)
	{
		unsigned int range = 1000;
		float bulletPosX = animals[gunIndex].fPosX;// trace a line from the gun and destroy any tissue found on the way.
		float bulletPosY = animals[gunIndex].fPosY;
		float angle      =  animals[gunIndex].fAngle;
		for (int i = 0; i < range; ++i)
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
					unsigned int shotOffNub = isAnimalInSquare(world[shootWorldPosition].identity, shootWorldPosition);
					if (shotOffNub >= 0 && shotOffNub < animalSquareSize)
					{
						hurtAnimal(world[shootWorldPosition].identity, shotOffNub, 0.35f + RNG());
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
}

void lighterCallback( int gunIndex, int shooterIndex )
{
	if (gunIndex >= 0)
	{
		int cursorPosX = cameraPositionX +  mousePositionX ;
		int cursorPosY = cameraPositionY + mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;

		if ( world[worldCursorPos].material == MATERIAL_NOTHING ||  world[worldCursorPos].material == MATERIAL_GRASS )
		{
			world[worldCursorPos].material = MATERIAL_FIRE;
		}
	}
}

void healAllDamage(unsigned int animalIndex)
{
	if (animalIndex < numberOfAnimals)
	{
		animals[animalIndex].damageReceived = 0;
		for (int i = 0; i < animalSquareSize; ++i)
		{
			animals[animalIndex].body[i].damage = 0.0f;
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
			unsigned int neighbour = animals[animalIndex].body[cellIndex].worldPositionI + (y * worldSize) + x;
			if (neighbour < worldSquareSize)
			{
				if (world[neighbour].identity >= 0 && world[neighbour].identity != animalIndex && world[neighbour].identity < numberOfAnimals)
				{
					int targetLocalPositionI = isAnimalInSquare( world[neighbour].identity, neighbour);
					if (targetLocalPositionI >= 0)
					{
						bool grabbedByAnotherGrabber = false;// finally, make sure the item is not grabbed by another of your own grabbers.
						for (unsigned int cellIndexB = 0; cellIndexB < animals[animalIndex].cellsUsed; cellIndexB++)                                      // place animalIndex on grid and attack / eat. add captured energy
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

// the animal is a grid of living cells that do different things. this function describes what they do each turn.
void organs_all()
{
	ZoneScoped;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			float totalLiver = 0;
			unsigned int totalGonads = 0;
			float highestIntensity = 0.0f;
			for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; cellIndex++)         
			{
				unsigned int cellWorldPositionI = animals[animalIndex].body[cellIndex].worldPositionI;
				unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
				unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;
				if (cellWorldPositionI >= worldSquareSize) {continue;}
				if (animals[animalIndex].body[cellIndex].damage > 1.0f) { continue;}
				unsigned int organ = animals[animalIndex].body[cellIndex].organ;
				switch (organ)
				{

				case ORGAN_SENSOR_AGE:
				{
					if (animals[animalIndex].lifespan > 0.0f)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = animals[animalIndex].age / animals[animalIndex].lifespan;
					}
					break;

				}

				case ORGAN_GRABBER:
				{
					if (animalIndex != playerCreature)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (animals[animalIndex].body[cellIndex].connections[i] .used)
							{
								unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
								}
							}
						}
					}
					else
					{
						int potentialGrab = getGrabbableItem(playerCreature, cellIndex);// check if there is anything grabbable.
						if (potentialGrab >= 0)
						{
							playerCanPickup = true;
							playerCanPickupItem = potentialGrab;
						}
						else
						{
							playerCanPickup = false;
							playerCanPickupItem = -1;
						}
					}

					if (animals[animalIndex].body[cellIndex].signalIntensity  >= 1.0f && animals[animalIndex].body[cellIndex].grabbedCreature  == -1)// if greater than 0, grab.
					{
						int potentialGrab = getGrabbableItem (animalIndex, cellIndex);
						if (potentialGrab >= 0)
						{
							animals[animalIndex].body[cellIndex].grabbedCreature = potentialGrab;//world[neighbour].identity;
							animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
							if (  animals[animals[playerCreature].body[cellIndex].grabbedCreature].isMachine  )// some machines have stuff that activates on pickup
							{
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) == (MACHINECALLBACK_HOSPITAL))
								{
									palette = true;
									healAllDamage(playerCreature);
								}
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_ECOLOGYCOMPUTER)
								{
									ecologyComputerDisplay = true;
								}
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_NEUROGLASSES )
								{
									visualizer = VISUALIZER_NEURALACTIVITY;
								}
								if ( animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback ==   MACHINECALLBACK_TRACKERGLASSES  )
								{
									visualizer = VISUALIZER_TRACKS;
								}
							}
						}
					}
					if (animals[animalIndex].body[cellIndex].grabbedCreature >= 0 )// if there is a grabbed creature, adjust its position to the grabber.
					{
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosX = cellWorldPositionX;
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosY = cellWorldPositionY;
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosX = cellWorldPositionX;
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosY = cellWorldPositionY;
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].position = cellWorldPositionI;

						// also, if grabbed by the player, adjust the angle of the grabbed object so it points at the mouse cursor. for aiming weapons.
						float fposx = cellWorldPositionX;
						float fposy = cellWorldPositionY;
						float fmousePositionX = mousePositionX;
						float fmousePositionY = mousePositionY;
						float angleToCursor = atan2(   fmousePositionY - (  cameraPositionY - fposy)  ,  fmousePositionX - (cameraPositionX - fposx));
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fAngle = angleToCursor;

						if (  animals[animalIndex].body[cellIndex].signalIntensity  <= -1.0f)
						{
							if (  animals[animals[playerCreature].body[cellIndex].grabbedCreature].isMachine  )
							{
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_HOSPITAL)
								{
									palette = false;
								}
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_MESSAGECOMPUTER)
								{
									for (int i = 0; i < 5; ++i)
									{
										computerdisplays[i] = false;
									}
								}
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_ECOLOGYCOMPUTER)
								{
									ecologyComputerDisplay = false;
								}
								if ((animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback) ==   MACHINECALLBACK_NEUROGLASSES ||
								        animals[animals[playerCreature].body[cellIndex].grabbedCreature].machineCallback ==   MACHINECALLBACK_TRACKERGLASSES  )
								{
									visualizer = VISUALIZER_TRUECOLOR;
								}
							}
							animals[animalIndex].body[cellIndex].grabbedCreature = -1;
							animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						}
					}
					break;
				}

				case ORGAN_SENSOR_PAIN:
				{
					animals[animalIndex].body[cellIndex].signalIntensity *= 0.95f;
				}

				case ORGAN_SENSOR_HUNGER:
				{
					if (animals[animalIndex].maxEnergy > 0.0f)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = animals[animalIndex].energy / animals[animalIndex].maxEnergy;
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
						animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
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
							animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
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
							animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
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
							animals[animalIndex].body[cellIndex].signalIntensity = targetAngle;
						}
					}
					break;
				}

				case ORGAN_LUNG:
				{
					if (world[cellWorldPositionI].wall != MATERIAL_WATER)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = baseLungCapacity;
					}
					else
					{
						animals[animalIndex].body[cellIndex].signalIntensity -= 0.01f;
					}
					if (animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						int otherLung = getRandomCellOfType(animalIndex, ORGAN_LUNG);
						if (otherLung < 0)
						{
							otherLung = getRandomCellOfType(animalIndex, ORGAN_GILL); // compatible with gills
						}
						if (otherLung >= 0)// if there is another reservoir of air, equalize them.
						{
							animals[animalIndex].body[cellIndex].signalIntensity +=	animals[animalIndex].body[otherLung].signalIntensity;
							animals[animalIndex].body[cellIndex].signalIntensity *= 0.5f;
							animals[animalIndex].body[otherLung].signalIntensity = animals[animalIndex].body[cellIndex].signalIntensity;
						}
						if (animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
						{
							animals[animalIndex].damageReceived++;
						}
					}
					break;
				}
				case ORGAN_GILL:
				{
					if (world[cellWorldPositionI].wall == MATERIAL_WATER)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = baseLungCapacity;
					}
					else
					{
						animals[animalIndex].body[cellIndex].signalIntensity -= 0.01f;
					}
					if (animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
					{
						int otherLung = getRandomCellOfType(animalIndex, ORGAN_GILL);
						if (otherLung < 0)
						{
							otherLung = getRandomCellOfType(animalIndex, ORGAN_LUNG); // compatible with lungs
						}
						if (otherLung >= 0)	// if there is another reservoir of air, equalize them.
						{
							animals[animalIndex].body[cellIndex].signalIntensity +=	animals[animalIndex].body[otherLung].signalIntensity;
							animals[animalIndex].body[cellIndex].signalIntensity *= 0.5f;
							animals[animalIndex].body[otherLung].signalIntensity = animals[animalIndex].body[cellIndex].signalIntensity;
						}
						if (animals[animalIndex].body[cellIndex].signalIntensity < 0.0f)
						{
							animals[animalIndex].damageReceived++;
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
					animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (animals[animalIndex].body[cellIndex].connections[i] .used)
						{
							unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
							}
						}
					}
					if (animals[animalIndex].body[cellIndex].signalIntensity > 1.0f || animals[animalIndex].body[cellIndex].signalIntensity  < -1.0f)
					{
						std::list<unsigned int> cellsOfType;
						unsigned int found = 0;
						for (int i = 0; i < animalSquareSize; ++i)
						{
							if (animals[animalIndex].body[i].organ == ORGAN_MEMORY_RX)
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
								if ( animals[animalIndex].body[(*iterator)].speakerChannel == animals[animalIndex].body[cellIndex].speakerChannel  )
								{
									correspondingCellRX = (*iterator);
								}
							}
						}
						if (correspondingCellRX >= 0 && correspondingCellRX < animalSquareSize)
						{
							animals[animalIndex].body[correspondingCellRX].signalIntensity = animals[animalIndex].body[cellIndex].signalIntensity ;
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
							animals[animalIndex].body[cellIndex].signalIntensity  = world[cellWorldPositionI].pheromoneIntensity;
						}
					}
					break;
				}

				case ORGAN_EMITTER_PHEROMONE:
				{
					animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (animals[animalIndex].body[cellIndex].connections[i] .used)
						{
							unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
							}
						}
					}
					world[cellWorldPositionI].pheromoneChannel = animals[animalIndex].body[cellIndex]. speakerChannel ;
					world[cellWorldPositionI].pheromoneIntensity = animals[animalIndex].body[cellIndex].signalIntensity;
					break;
				}

				case ORGAN_SPEAKER:
				{
					if ( animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (animals[animalIndex].body[cellIndex].connections[i] .used)
							{
								unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
								}
							}
						}
						if (animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
						}
						else if (animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
						}
						speakerChannels[  animals[animalIndex].body[cellIndex].speakerChannel ] += animals[animalIndex].body[cellIndex].signalIntensity ;
					}
					else
					{
						animals[animalIndex].body[cellIndex].speakerChannel = 0;
					}
					break;
				}

				case ORGAN_SENSOR_EAR:
				{
					if (animals[animalIndex].body[cellIndex].speakerChannel < numberOfSpeakerChannels)
					{
						animals[animalIndex].body[cellIndex].signalIntensity = speakerChannelsLastTurn[ animals[animalIndex].body[cellIndex].speakerChannel ];
					}
					else
					{
						animals[animalIndex].body[cellIndex].speakerChannel = 0;
					}
					break;
				}

				case ORGAN_SENSOR_TRACKER:
				{
					animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					if ( world [cellWorldPositionI].identity != animalIndex )
					{
						animals[animalIndex].body[cellIndex].signalIntensity = world[cellWorldPositionI].trail;
					}
					break;
				}

				case ORGAN_SENSOR_BODYANGLE:
				{
					animals[animalIndex].body[cellIndex].signalIntensity = animals[animalIndex].fAngle;
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
					Color perceivedColor = multiplyColor( receivedColor, animals[animalIndex].body[cellIndex].color  );
					animals[animalIndex].body[cellIndex].signalIntensity = colorAmplitude(perceivedColor );
					break;
				}

				case ORGAN_SENSOR_TOUCH:
				{
					animals[animalIndex].body[cellIndex].signalIntensity = 0;
					for (int i = 0; i < nNeighbours; ++i)
					{
						unsigned int neighbour = cellWorldPositionI + neighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							if (world[neighbour].identity >= 0)
							{
								if (isAnimalInSquare( world[neighbour].identity , neighbour ))
								{
									animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
								else if (world[neighbour].material != MATERIAL_NOTHING)
								{
									animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
							}
						}
					}
					unsigned int touchedAnimal = world[cellWorldPositionI].identity;
					if (touchedAnimal < numberOfAnimals)
					{
						if (touchedAnimal >= 0)
						{
							if (touchedAnimal != animalIndex)
							{
								if (isAnimalInSquare( touchedAnimal , cellWorldPositionI ))
								{
									animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
								}
								else if (world[cellWorldPositionI].material != MATERIAL_NOTHING)
								{
									animals[animalIndex].body[cellIndex].signalIntensity += 0.5f;
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
						if (animals[animalIndex].body[cellIndex].connections[i] .used)
						{
							unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								float connected_signal = animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
								sum += connected_signal;
							}
						}
					}
					animals[animalIndex].body[cellIndex].signalIntensity = fast_sigmoid(sum);
					break;
				}

				case ORGAN_GONAD:
				{
					totalGonads++;
					if (doReproduction && animals[animalIndex].energyDebt <= 0.0f )
					{
						if (animals[animalIndex].energy > ((animals[animalIndex].mass / 2 ) + animals[animalIndex].offspringEnergy ))
						{
							if (cellWorldPositionI < worldSquareSize)
							{
								unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;
								int result = spawnAnimal( speciesIndex, animals[animalIndex], animals[animalIndex].position, true );
								if (result >= 0)
								{
									animals[animalIndex].body[cellIndex].organ = MATERIAL_NOTHING;
									animals[animalIndex].numberOfTimesReproduced++;
									animals[animalIndex].energy -= animals[animalIndex].offspringEnergy;
									animals[result].energy       =  animals[animalIndex].offspringEnergy;
									animals[result].parentIdentity       = animalIndex;
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
					if (world[cellWorldPositionI].material == MATERIAL_GRASS)
					{
						animals[animalIndex].energy += grassEnergy ;
						world[cellWorldPositionI].material = MATERIAL_NOTHING;
					}
					break;
				}

				case ORGAN_MOUTH_SCAVENGE :
				{
					if (world[cellWorldPositionI].material == MATERIAL_FOOD)
					{
						animals[animalIndex].energy += foodEnergy ;
						world[cellWorldPositionI].material = MATERIAL_NOTHING;
					}
					break;
				}

				case ORGAN_MUSCLE :
				{
					if (doMuscles)
					{
						if (animalIndex != playerCreature)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;

							for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
							{
								if (animals[animalIndex].body[cellIndex].connections[i] .used)
								{
									unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
									if (connected_to_cell < animalSquareSize)
									{
										animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
									}
								}
							}
						}
						if (animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
						}
						else if (animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
						}
						animals[animalIndex].fPosX += animals[animalIndex].body[cellIndex].signalIntensity * 10 * cos(animals[animalIndex].fAngle);
						animals[animalIndex].fPosY += animals[animalIndex].body[cellIndex].signalIntensity * 10 * sin(animals[animalIndex].fAngle);
						animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					}
					break;
				}

				case ORGAN_MUSCLE_STRAFE :
				{
					if (doMuscles)
					{
						if (animalIndex != playerCreature)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;	// go through the list of connections and sum their values.
							for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
							{
								if (animals[animalIndex].body[cellIndex].connections[i] .used)
								{
									unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
									if (connected_to_cell < animalSquareSize)
									{
										animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
									}
								}
							}
						}
						if (animals[animalIndex].body[cellIndex].signalIntensity > 1.0f)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = 1.0f;
						}
						else if (animals[animalIndex].body[cellIndex].signalIntensity < -1.0f)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = -1.0f;
						}
						animals[animalIndex].fPosX += animals[animalIndex].body[cellIndex].signalIntensity * 10 * sin(animals[animalIndex].fAngle);// on the strafe muscle the sin and cos are reversed, that's all.
						animals[animalIndex].fPosY += animals[animalIndex].body[cellIndex].signalIntensity * 10 * cos(animals[animalIndex].fAngle);

						animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					}
					break;
				}

				case ORGAN_MUSCLE_TURN:
				{
					if (doMuscles)
					{
						if (animalIndex != playerCreature)
						{
							animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;// go through the list of connections and sum their values.
							for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
							{
								if (animals[animalIndex].body[cellIndex].connections[i] .used)
								{
									unsigned int connected_to_cell = animals[animalIndex].body[cellIndex].connections[i] .connectedTo;
									if (connected_to_cell < animalSquareSize)
									{
										animals[animalIndex].body[cellIndex].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellIndex].connections[i] .weight;
									}
								}
							}
						}
						if (setOrSteerAngle)
						{
							animals[animalIndex].fAngle = (animals[animalIndex].body[cellIndex].signalIntensity ) ;
						}
						else
						{
							animals[animalIndex].fAngle += (animals[animalIndex].body[cellIndex].signalIntensity ) * 0.1f;
						}
						if (animals[animalIndex].fAngle > const_pi)
						{
							animals[animalIndex].fAngle -= 2 * const_pi;
						}
						if (animals[animalIndex].fAngle < -const_pi)
						{
							animals[animalIndex].fAngle += 2 * const_pi;
						}
						animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
					}
					break;
				}
				}
			}
			animals[animalIndex].totalGonads = totalGonads;
			animals[animalIndex].maxEnergy = animals[animalIndex].mass + (totalLiver * liverStorage);
		}
	}

	for (unsigned int i = 0; i < numberOfSpeakerChannels; ++i)
	{
		speakerChannelsLastTurn [i] = speakerChannels[i];
		speakerChannels[i] = 0.0f;
	}
}

void move_all()
{
	ZoneScoped;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			bool trailUpdate = false;// calculate direction of movement.
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
			animals[animalIndex].fAngleCos = cos(animals[animalIndex].fAngle);
			animals[animalIndex].fAngleSin = sin(animals[animalIndex].fAngle);
			unsigned int newPosX  = animals[animalIndex].fPosX;
			unsigned int newPosY  = animals[animalIndex].fPosY;
			unsigned int newPosition  =  (newPosY * worldSize) + newPosX;
			if (newPosition < worldSquareSize)
			{
				if (  materialBlocksMovement( world[newPosition].wall ) )
				{
					animals[animalIndex].fPosX  = animals[animalIndex].uPosX;
					animals[animalIndex].fPosY  = animals[animalIndex].uPosY;
				}
				else
				{
					animals[animalIndex].uPosX  = animals[animalIndex].fPosX;
					animals[animalIndex].uPosY  = animals[animalIndex].fPosY;
				}
				animals[animalIndex].position = newPosition;
				if (false)
				{
					if (world[newPosition].temperature > animals[animalIndex].temp_limit_high)
					{
						animals[animalIndex].damageReceived += abs(world[newPosition].temperature  - animals[animalIndex].temp_limit_high);
					}
					if (world[newPosition].temperature < animals[animalIndex].temp_limit_low)
					{
						animals[animalIndex].damageReceived += abs(world[newPosition].temperature  - animals[animalIndex].temp_limit_low);
					}
				}
			}

			for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				if (taxIsByMass)
				{
					animals[animalIndex].energy -= taxEnergyScale *  organUpkeepCost(animals[animalIndex].body[cellIndex].organ); // * speciesEnergyOuts[speciesIndex] ;
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
							if (animals[animalIndex].body[cellIndex].organ == ORGAN_WEAPON || animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_CARNIVORE )
							{
								if (animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
								{
									if (world[cellWorldPositionI].identity == animals[animalIndex].parentIdentity)
									{
										continue;
									}
								}
								bool meatAvailable = hurtAnimal(world[cellWorldPositionI].identity , targetLocalPositionI, 1.0f );
								if (meatAvailable)
								{
									okToStep = true;
									animals[animalIndex].damageDone++;
									speciesAttacksPerTurn[speciesIndex] ++;
									if (animals[world[cellWorldPositionI].identity].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
									{
										if (animals[animalIndex].body[cellIndex].organ == ORGAN_WEAPON)
										{
											if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
											{
												world[cellWorldPositionI].material = MATERIAL_FOOD;
											}
										}
										if (animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_CARNIVORE)
										{
											animals[animalIndex].energy += foodEnergy ;
										}
									}
								}
							}
							else if (animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_PARASITE )
							{
								float amount = (animals[world[cellWorldPositionI].identity].energy) / animalSquareSize;
								float defense = defenseAtWorldPoint(world[cellWorldPositionI].identity, cellWorldPositionI);
								amount = amount / defense;
								animals[animalIndex].energy += amount;
								animals[world[cellWorldPositionI].identity].energy -= amount;
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
					if (world[cellWorldPositionI].identity == -1 && speciesIndex != 0)
					{
						if (world[cellWorldPositionI].material == MATERIAL_NOTHING || world[cellWorldPositionI].material == MATERIAL_GRASS)
						{
							world[cellWorldPositionI].material = MATERIAL_GRASS;
							world[cellWorldPositionI].grassColor =  addColor( color_green , multiplyColorByScalar(animals[animalIndex].identityColor, 0.5f ));//
						}
					}
					world[cellWorldPositionI].identity = animalIndex;
					world[cellWorldPositionI].occupyingCell = cellIndex;
					if (trailUpdate)
					{
						world[cellWorldPositionI].trail    = dAngle;
					}
					animals[animalIndex].body[cellIndex].worldPositionI = cellWorldPositionI;
				}
			}
		}
		else
		{
			animals[animalIndex].position =  animals[animalIndex].position % worldSquareSize;
		}
	}
}

void energy_all() // perform energies.
{
	ZoneScoped;
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		populationCountUpdates[i] = 0;
	}
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired && speciesIndex < numberOfSpecies)
		{
			populationCountUpdates[speciesIndex]++;
			animals[animalIndex].age++;
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
			if (!immortality && !animals[animalIndex].isMachine) // reasons an npc can die
			{
				if (speciesPopulationCounts[speciesIndex] > (( numberOfAnimals / numberOfSpecies) / 4) && animalIndex != playerCreature) // only kill off weak animals if there is some population.
					if (animals[animalIndex].energy < 0.0f)
					{
						execute = true;
					}
				if (animals[animalIndex].age > animals[animalIndex].lifespan && animalIndex != playerCreature)
				{
					execute = true;
				}
				if (animals[animalIndex].totalGonads == 0)
				{
					execute = true;
				}
				if (animals[animalIndex].damageReceived > animals[animalIndex].mass)
				{
					execute = true;
				}
				if (animals[animalIndex].mass <= 0)
				{
					execute = true;
				}
			}
			if (execute)
			{
				if (animalIndex != adversary)
				{
					killAnimal( animalIndex);
				}
			}
			bool nominate = false;
			int animalScore = animals[animalIndex].damageDone + animals[animalIndex].damageReceived  + animals[animalIndex].numberOfTimesReproduced ;
			if (animalIndex != playerCreature && speciesIndex > 0) // player & player species cannot be nominated
			{
				if ( animalScore > championScore)
				{
					nominate = true;
				}
				else if (animalScore == championScore)
				{
					if (animals[animalIndex].energy > championEnergyScore)
					{
						nominate = true;
					}
				}
			}
			if (nominate)
			{
				championScore = animalScore;
				championEnergyScore = animals[animalIndex].energy;
				champion = animals[animalIndex];
			}
		}
	}
	for (int i = 0; i < numberOfSpecies; ++i)
	{
		speciesPopulationCounts[i] = populationCountUpdates[i];
		speciesAttacksPerTurn[i] = 0;
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
	if (cameraTargetCreature >= 0)
	{
		cameraPositionX = animals[cameraTargetCreature].position % worldSize;
		cameraPositionY = animals[cameraTargetCreature].position / worldSize;
	}
	if (playerCreature >= 0 && cameraTargetCreature == playerCreature)	// if the player doesn't have any eyes, don't draw anything!
	{
		int playerEye = getRandomCellOfType(playerCreature, ORGAN_SENSOR_EYE);
		if (playerEye >= 0)
		{
			playerCanSee = true;
		}
		else
		{
			playerCanSee = false;
		}
	}
	else
	{
		playerCanSee = true; // if you're in the spectator view, basically not 'the player' or any other creature, you stil want to be able to see.
	}
	if (playerCanSee)
	{
		int viewFieldMax = +(viewFieldY / 2);
		int viewFieldMin = -(viewFieldX / 2);
		for ( int vy = viewFieldMin; vy < viewFieldMax; ++vy)
		{
			for ( int vx = viewFieldMin; vx < viewFieldMax; ++vx)
			{
				unsigned int x = (vx + cameraPositionX) % worldSize;
				unsigned int y = (vy + cameraPositionY) % worldSize;
				Color displayColor = color_black;
				unsigned int worldI = (y * worldSize) + x;
				if (worldI < worldSquareSize)
				{
					float fx = vx;
					float fy = vy;
					switch (visualizer)
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
						if (world[worldI].wall != MATERIAL_NOTHING)
						{
							amount -= 0.25f;
						}
						if (world[worldI].material != MATERIAL_NOTHING)
						{
							amount -= 0.0625f;
						}
						if (world[worldI].identity < numberOfAnimals && world[worldI].identity >= 0)
						{
							if (animals[ world[worldI].identity].isMachine  )
							{
								displayColor = color_white;
								drawTile( Vec_f2( fx, fy ), displayColor);
							}
							displayColor = animals[ world[worldI].identity ].identityColor;
							drawPointerTriangle( Vec_f2( fx, fy ), displayColor, world[worldI].trail );
						}
						break;
					}
					case VISUALIZER_NEURALACTIVITY:
					{
						displayColor = color_grey;
						float amount = 0.5f;
						if (world[worldI].wall != MATERIAL_NOTHING)
						{
							amount -= 0.25f;
						}
						if (world[worldI].material != MATERIAL_NOTHING)
						{
							amount -= 0.0625f;
						}
						if (world[worldI].identity < numberOfAnimals && world[worldI].identity >= 0)
						{
							int occupyingCell = isAnimalInSquare(world[worldI].identity, worldI);
							if ( occupyingCell >= 0)
							{
								unsigned int organ = animals[ world[worldI].identity] .body[occupyingCell].organ;
								if (organIsANeuron(  organ ) || organIsASensor(organ))
								{
									amount = animals[world[worldI].identity].body[occupyingCell].signalIntensity ; //* 2.0f;
								}
								if (animals[world[worldI].identity].isMachine)
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
	Vec_f2 worldMousePos = Vec_f2( mousePositionX, mousePositionY);
	drawTile( worldMousePos, displayColor);
}

void displayComputerText()
{
	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;
	if (ecologyComputerDisplay)
	{
		for (int i = 0; i < numberOfSpecies; ++i)
		{
			printText2D(   std::string("Species ") + std::to_string(i) +   std::string(" pop. " + std::to_string(speciesPopulationCounts[i])) + " hits " + std::to_string(speciesAttacksPerTurn[i]) , menuX, menuY, textSize);
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
// The 4th terminal is under water in a teeming coral reef. It contains tracker glasses that allow the adversary to be identified and found.
// The adversary is killed and life no longer has a source, but will continue existing where it does. The adversary drops neuro glasses that the player needs to edit brain connections.
// If all life in the simulation is destroyed, a message will become available stating that the animals broke out into the real world and caused widespread disaster
	if (computerdisplays[0])
	{
		printText2D(   std::string("Animals are groups of tiles that move around. Each tile has a dedicated purpose.") , menuX, menuY, textSize);
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
	else if (computerdisplays[1])
	{
		printText2D(   std::string("Use the hospital terminal to add a gill to your body. It will enable you to explore underwater.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Find a building under the water and retrieve the tracker glasses. These can identity the adversary.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (computerdisplays[2])
	{
		printText2D(   std::string("Activate the tracker glasses to see the trails that animals leave.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("You will recognize the adversary by its white trail.") , menuX, menuY, textSize);
		menuY -= spacing;
		printText2D(   std::string("Take the weapon, find the adversary and kill it.") , menuX, menuY, textSize);
		menuY -= spacing;
	}
	else if (computerdisplays[3])
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
	if (showInstructions)
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
	for (int i = 0; i < animals[playerCreature].cellsUsed; ++i)
	{
		if (animals[playerCreature].body[i].organ == ORGAN_GRABBER)
		{
			if (animals[playerCreature].body[i].grabbedCreature >= 0)
			{
				std::string stringToPrint = std::string("Holding ") + animals[  animals[playerCreature].body[i].grabbedCreature ].displayName + std::string(". [f] to drop.");
				if (animals[  animals[playerCreature].body[i].grabbedCreature ].isMachine)
				{
					stringToPrint += std::string(" [lmb, rmb] to use.");
				}
				printText2D( stringToPrint   , menuX, menuY, textSize);
				menuY += spacing;
				holding = true;
			}
		}
	}
	if (!holding && playerCanPickup && playerCanPickupItem >= 0 && playerCanPickupItem < numberOfAnimals)
	{
		printText2D(   std::string("[g] pick up ") + std::string(animals[playerCanPickupItem].displayName) , menuX, menuY, textSize);
		menuY += spacing;
	}
	if (palette)
	{
		printText2D(   std::string("[lmb] add, [rmb] delete ") , menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[y] select next, [h] select last ") , menuX, menuY, textSize);
		menuY += spacing;
	}
	int cursorPosX = cameraPositionX +  mousePositionX ;
	int cursorPosY = cameraPositionY + mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	if (worldCursorPos < worldSquareSize)
	{
		int heightInt = world[worldCursorPos].height;
		printText2D(   std::string("x ") + std::to_string(cursorPosX ) + std::string(" y ") + std::to_string(cursorPosY) + std::string(" height ") + std::to_string(heightInt) , menuX, menuY, textSize);
		menuY += spacing;
		cursorAnimal = world[worldCursorPos].identity;
		bool animalInSquare = false;
		if (cursorAnimal >= 0 && cursorAnimal < numberOfAnimals)
		{
			unsigned int cursorAnimalSpecies = cursorAnimal / numberOfAnimalsPerSpecies;
			int occupyingCell = isAnimalInSquare(cursorAnimal, worldCursorPos);
			std::string selectString( " [e] to select.");
			if (selectedAnimal >= 0)
			{
				std::string selectString( " [e] to deselect. [k] save animal.");
			}
			if ( occupyingCell >= 0)
			{
				if (cursorAnimalSpecies == 0)
				{
					if (cursorAnimal == playerCreature)
					{
						printText2D(   std::string("This is you.") + selectString, menuX, menuY, textSize);
						menuY += spacing;
					}
					else
					{
						printText2D(  std::string("A ") +  std::string(animals[cursorAnimal].displayName) + std::string(".") + selectString , menuX, menuY, textSize);
						menuY += spacing;
					}
				}
				else
				{
					printText2D(   std::string("An animal of species ") + std::to_string(cursorAnimalSpecies ) + std::string(".") + selectString, menuX, menuY, textSize);
					menuY += spacing;
				}
				printText2D(  tileDescriptions(  animals[  cursorAnimal].body[occupyingCell].organ ), menuX, menuY, textSize);
				menuY += spacing;
				animalInSquare = true;
			}
		}
		if (!animalInSquare)
		{
			if (world[worldCursorPos].material != MATERIAL_NOTHING)
			{
				printText2D(  tileDescriptions(world[worldCursorPos].material ), menuX, menuY, textSize);
				menuY += spacing;
			}
			else
			{
				printText2D(  tileDescriptions (world[worldCursorPos].terrain ), menuX, menuY, textSize);
				menuY += spacing;
			}
		}
	}
	if (playerCreature >= 0)
	{
		int playerPheromoneSensor = getRandomCellOfType( playerCreature, ORGAN_SENSOR_PHEROMONE ) ;// if the player has a nose, print what it smells like here.
		if (playerPheromoneSensor >= 0)
		{
			unsigned int playerPheromoneSensorWorldPos = animals[playerCreature].body[playerPheromoneSensor].worldPositionI;
			if (world[playerPheromoneSensorWorldPos].pheromoneChannel >= 0 &&  world[playerPheromoneSensorWorldPos].pheromoneChannel < numberOfSpeakerChannels)
			{
				printText2D(   pheromoneChannelDescriptions[  world[playerPheromoneSensorWorldPos].pheromoneChannel ] , menuX, menuY, textSize);
				menuY += spacing;
			}
			else
			{
				printText2D(   std::string("You can't smell anything in particular.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
		if (!playerCanSee)// if the player is blind, say so!
		{
			printText2D(   std::string("You can't see anything. ") , menuX, menuY, textSize);
			menuY += spacing;
		}
		int playerGill = getRandomCellOfType( playerCreature, ORGAN_GILL ) ;
		if (playerGill >= 0)
		{
			if (animals[playerCreature].body[playerGill].signalIntensity < 0.0f)
			{
				printText2D(   std::string("You have no oxygen left.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (animals[playerCreature].body[playerGill].signalIntensity < baseLungCapacity / 2)
			{
				printText2D(   std::string("You're half out of oxygen.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
		playerGill = getRandomCellOfType( playerCreature, ORGAN_LUNG ) ;
		if (playerGill >= 0)
		{
			if (animals[playerCreature].body[playerGill].signalIntensity < 0.0f)
			{
				printText2D(   std::string("You have no air left.") , menuX, menuY, textSize);
				menuY += spacing;
			}
			else if (animals[playerCreature].body[playerGill].signalIntensity < baseLungCapacity / 2)
			{
				printText2D(   std::string("You're half out of air.") , menuX, menuY, textSize);
				menuY += spacing;
			}
		}
		if (animals[playerCreature].damageReceived < (animals[playerCreature].mass) * 0.5)
		{
			;
		}
		if (animals[playerCreature].damageReceived < (animals[playerCreature].mass) * 0.75)
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
			printText2D(   logs[i] , menuX, menuY, textSize);
			menuY += spacing;
		}
		menuY += spacing;
	}
	if (showInstructions)
	{
		menuY += spacing;
		printText2D(   std::string("[esc] quit"), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[arrows] pan, [-,=] zoom"), menuX, menuY, textSize);
		menuY += spacing;
		std::string pauseString = std::string("[p] pause ");
		if (paused)
		{
			pauseString = std::string("[p] resume ");
		}
		printText2D(   std::string("[l] limit frame rate"), menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[o] save, ") + pauseString, menuX, menuY, textSize);
		menuY += spacing;
		printText2D(   std::string("[space] return mouse") , menuX, menuY, textSize);
		menuY += spacing;
		if (playerCreature >= 0)
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

	if (palette)
	{
		drawPalette();
	}
}

void a( unsigned int animalIndex , unsigned int organ,  Vec_i2 * p , Color color)
{
	appendCell( animalIndex, organ, *p);
	animals[animalIndex].body[animals[animalIndex].cellsUsed - 1].color = color;
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
	animals[i].isMachine = true;
	animals[i].machineCallback = MACHINECALLBACK_LIGHTER;
	std::string gunDescription = std::string("lighter");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );

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
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
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
	animals[i].isMachine = true;
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
	animals[i].machineCallback = MACHINECALLBACK_TRACKERGLASSES;
	std::string gunDescription = std::string("tracker glasses");
	strcpy( &(animals[i].displayName[0]) , gunDescription.c_str() );
}

void setupNeuroGlasses(int i)
{
	animals[i].machineCallback = MACHINECALLBACK_NEUROGLASSES;
	std::string gunDescription = std::string("neuro glasses");
	strcpy( &(animals[i].displayName[0]) , gunDescription.c_str() );
}

void setupExampleGun(int i)
{
	resetAnimal(i);
	animals[i].isMachine = true;
	animals[i].machineCallback = MACHINECALLBACK_PISTOL;
	std::string gunDescription = std::string("pistol");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );

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
	animals[i].isMachine = true;
	animals[i].machineCallback = MACHINECALLBACK_KNIFE;
	std::string gunDescription = std::string("knife");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );

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
	animals[i].isMachine = true;
	animals[i].fAngle = 0.0f;

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
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
	animals[i].machineCallback = MACHINECALLBACK_ECOLOGYCOMPUTER;
}

void setupMessageComputer(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("message terminal");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
	animals[i].machineCallback = MACHINECALLBACK_MESSAGECOMPUTER;
}

void setupHospitalComputer(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("hospital");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
	animals[i].machineCallback = MACHINECALLBACK_HOSPITAL;
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
			avgHeight += world[i].height;
			tally++;
			world[i].terrain = MATERIAL_VOIDMETAL;
			if (  !(world[i].wall == MATERIAL_NOTHING || world[i].wall == MATERIAL_WATER) )
			{
				world[i].wall = MATERIAL_NOTHING;
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
		if (abs(xdiff) < baseSize && abs(ydiff) < baseSize)
		{
			world[i].height = avgHeight;
		}
	}
	cameraPositionX  = worldPositionX;
	cameraPositionY = worldPositionY;
}

void spawnAdversary(unsigned int targetWorldPositionI)
{
	adversary = numberOfAnimalsPerSpecies + 1; // adversary animal is a low number index in the 1th species. 0th is for players and machines.
	spawnAnimalIntoSlot(adversary, champion, targetWorldPositionI, false);
	animals[adversary].position = targetWorldPositionI;
	animals[adversary].uPosX = targetWorldPositionI % worldSize;
	animals[adversary].uPosY = targetWorldPositionI / worldSize;
	animals[adversary].fPosX = animals[adversary].uPosX;
	animals[adversary].fPosY = animals[adversary].uPosY;
}

void spawnPlayer()
{
	if (playerCreature == -1)
	{
		unsigned int targetWorldPositionI =  playerRespawnPos;
		int i = 1;
		setupExampleHuman(i);
		playerCreature = 0;
		spawnAnimalIntoSlot(playerCreature, animals[i], targetWorldPositionI, false);
		cameraTargetCreature = playerCreature;
		int randomLung = getRandomCellOfType(playerCreature, ORGAN_LUNG);
		if (randomLung >= 0)
		{
			animals[playerCreature].body[randomLung].signalIntensity = baseLungCapacity;
		}
		animals[playerCreature].energy = animals[playerCreature].maxEnergy;
		animals[playerCreature].damageReceived = 0;
		appendLog( std::string("Spawned the player.") );
	}
	else
	{
		killAnimal(playerCreature);
	}
}

void saveParticularAnimal(unsigned int animalIndex, std::string filename )
{
	std::ofstream out7( filename .c_str());
	out7.write( (char*)(&animals[selectedAnimal]), sizeof(Animal));
	out7.close();
}

void loadParticlarAnimal(unsigned int animalIndex, std::string filename)
{
	std::ifstream in7(filename.c_str());
	in7.read( (char*)(&animals[selectedAnimal]), sizeof(Animal));
	in7.close();
}

void saveSelectedAnimal ( )
{
	if (selectedAnimal >= 0)
	{
		saveParticularAnimal(selectedAnimal, std::string("save/selectedAnimal") );
	}
}

void spawnTournamentAnimals()
{
	if (adversary >= 0)
	{
		for (int i = (1 * numberOfAnimalsPerSpecies); i < numberOfAnimals; ++i)// animals in the tournament are not in the 0th species, which is for players and machines.
		{
			unsigned int targetWorldPositionI = animals[adversary].position;
			int j = 1;
			setupExampleAnimal2(j);
			spawnAnimalIntoSlot(i, animals[j], targetWorldPositionI, true);
		}
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
		world[worldPositionI].height = prelimMap[prelimSampleIndex] ;
	}
}

void recomputeTerrainLighting()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		computeLight( worldPositionI, sunXangle, sunYangle);
		if (world[worldPositionI].height < seaLevel)
		{
			world[worldPositionI].wall = MATERIAL_WATER;
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

unsigned int getRandomPosition(bool underwater)
{
	while (true)
	{
		unsigned int randomI = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
		if (!underwater)
		{
			if (world[randomI].height > biome_coastal)
			{
				return randomI;
			}
		}
		else
		{
			if (world[randomI].height < seaLevel)
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
	spawnAnimalIntoSlot(3, animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25;
	setupMessageComputer( i);
	spawnAnimalIntoSlot(9, animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25 * worldSize;
	playerRespawnPos = targetWorldPositionI;
	spawnPlayer();

	targetWorldPositionI =  getRandomPosition(true);
	adversaryRespawnPos = targetWorldPositionI;
	spawnAdversary(targetWorldPositionI);

	targetWorldPositionI =  getRandomPosition(false);
	setupBuilding_playerBase(targetWorldPositionI);
	setupHospitalComputer(i);
	spawnAnimalIntoSlot(5, animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25 * worldSize;
	setupMessageComputer( i);
	spawnAnimalIntoSlot(10, animals[i], targetWorldPositionI, false);

	targetWorldPositionI =  getRandomPosition(true);
	setupBuilding_playerBase(targetWorldPositionI);
	setupTrackerGlasses(i);
	spawnAnimalIntoSlot(4, animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25;
	setupExampleGun(i);
	spawnAnimalIntoSlot(2, animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25 * worldSize;
	setupMessageComputer( i);
	spawnAnimalIntoSlot(11, animals[i], targetWorldPositionI, false);

	targetWorldPositionI =  getRandomPosition(true);
	setupBuilding_playerBase(targetWorldPositionI);
	setupExampleKnife(i);
	spawnAnimalIntoSlot(6, animals[i], targetWorldPositionI, false);

	targetWorldPositionI += 25;
	setupExampleLighter(i);
	spawnAnimalIntoSlot(7, animals[i], targetWorldPositionI, false);

	targetWorldPositionI =  getRandomPosition(true);
	setupBuilding_playerBase(targetWorldPositionI);
	setupNeuroGlasses(i);
	spawnAnimalIntoSlot(8, animals[i], targetWorldPositionI, false);
}

void applyPretties()
{
	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; worldPositionI++)
	{
		if (  world[worldPositionI].terrain == MATERIAL_ROCK )
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
		world[worldPositionI].terrain = MATERIAL_ROCK;
		if (x < wallThickness || x > worldSize - wallThickness || y < wallThickness  || y > worldSize - wallThickness)	// walls around the world edge
		{
			world[worldPositionI].wall = MATERIAL_VOIDMETAL;
		}
		if (world[worldPositionI].height < seaLevel)
		{
			world[worldPositionI].material = MATERIAL_GRASS;
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
	if (adversary < 0)
	{
		spawnAdversary(adversaryRespawnPos);
	}
	if (adversary >= 0 && adversary < numberOfAnimals)
	{
		if (animals[adversary].retired)
		{
			spawnAdversary(adversaryRespawnPos);
		}
		else
		{
			if (animals[adversary].position >= 0 && animals[adversary].position < worldSquareSize)
			{
				adversaryRespawnPos = animals[adversary].position;
				unsigned int adversaryRespawnPosX = adversaryRespawnPos % worldSize;
				unsigned int adversaryRespawnPosY = adversaryRespawnPos / worldSize;
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
				adversaryRespawnPos = (adversaryRespawnPosY * worldSize ) + adversaryRespawnPosX;
			}
		}
	}

	if (respawnLowSpecies)
	{
		unsigned int totalpop = 0;
		for (unsigned int speciesIndex = 1; speciesIndex < numberOfSpecies; speciesIndex++) // start at 1 to ignore the non-natural species 0.
		{
			totalpop += speciesPopulationCounts[speciesIndex] ;
			if (speciesPopulationCounts[speciesIndex] == 0)
			{
				int foundAnimal = -1;// if there is another species who is successful, duplicate an animal from them.
				int foundSpecies = -1;
				for (unsigned int j = 1; j < numberOfSpecies; ++j)
				{
					if (speciesPopulationCounts[j] >= 1)
					{
						for (unsigned int k = extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies - 2); k < numberOfAnimalsPerSpecies; ++k)
						{
							unsigned int animalToCopy = (j * numberOfAnimalsPerSpecies) + k;
							if (!animals[animalToCopy].retired)
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
					memcpy(    &animals[ (speciesIndex * numberOfAnimalsPerSpecies) ] , &animals[ foundAnimal ], sizeof(Animal)    );
					resetAnimal(foundAnimal);
				}
			}
		}

		if (totalpop == 0 && adversary >= 0)// life went extinct but the adversary is still alive. Spawn a bunch more stuff to get it going again.
		{
			int j = 1;
			for (int k = j + 1; k < numberOfAnimals / 2; ++k)// spawn lots of the example animal
			{
				if (k != adversary)
				{
					setupExampleAnimal2(j);
					int domingo = spawnAnimal( 1, animals[j], animals[adversary].position, true);

					if (domingo >= 0)
					{
						paintAnimal(domingo);
						animals[domingo].energy = animals[domingo].maxEnergy;
						animals[domingo].damageReceived = 0;
					}
				}
			}

			for (int k = (numberOfAnimals / 2) + 1; k < numberOfAnimals; ++k)// spawn lots of the champion
			{
				if (k != adversary)
				{
					int domingo = spawnAnimal( 1,  champion, animals[adversary].position, true);
					if (domingo >= 0)
					{
						paintAnimal(domingo);
						animals[domingo].energy = animals[domingo].maxEnergy;
						animals[domingo].damageReceived = 0;
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
	if (!paused)
	{
		tournamentController();
		computeAllAnimalsOneTurn();
		updateMap();
	}
	modelFrameCount++;
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	usPerFrame = elapsed.count();

	if (!lockfps && usPerFrame > 0)
	{
		fps = (1000000.0f / usPerFrame) ;
	}
	if (lockfps) { fps = 1.0f;}
}

void modelSupervisor()
{
	while (true)
	{
		if (!lockfps)
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
	if (exists_test3(std::string("save/animals")) && exists_test3(std::string("save/world")) )
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
		printText2D(   std::string("loading animals from file "), menuX, menuY, textSize);
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
	champion = animals[j];
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
	std::ofstream out6(std::string("save/world").c_str());
	out6.write( (char*)(world), sizeof(Square) *  worldSquareSize);
	out6.close();
	std::ofstream out7(std::string("save/animals").c_str());
	out7.write( (char*)(animals), sizeof(Animal) *  numberOfAnimals);
	out7.close();
}

void load()
{
	worldCreationStage = 12;
	std::ifstream in6(std::string("save/world").c_str());
	in6.read( (char *)(&(world)), sizeof(Square) *  worldSquareSize);
	in6.close();
	worldCreationStage = 11;
	std::ifstream in7(std::string("save/animals").c_str());
	in7.read( (char *)(&(animals)), sizeof(Animal) *  numberOfAnimals);
	in7.close();
	worldCreationStage = 10;
	setFlagReady();
}

