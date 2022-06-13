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

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

#include "utilities.h"
#include "graphics.h"
#include "untitled_marlo_project.h"
#include "menus.h"
#include "main.h"

#define MATERIAL_NOTHING          0
#define ORGAN_MOUTH_VEG           1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH_SCAVENGE      2
#define ORGAN_GONAD               3
#define ORGAN_MUSCLE              4
#define ORGAN_BONE                5
#define ORGAN_WEAPON              6
#define ORGAN_LIVER               7
#define ORGAN_MUSCLE_TURN         8
#define ORGAN_SENSOR_EYE          9

#define ORGAN_MOUTH_CARNIVORE     10
#define ORGAN_MOUTH_PARASITE      11
#define ORGAN_ADDOFFSPRINGENERGY  12
#define ORGAN_ADDLIFESPAN         13
#define ORGAN_NEURON              15
#define ORGAN_BIASNEURON          16    // can be thought of as ORGAN_SENSOR_CONSTANTVALUE
#define ORGAN_SENSOR_TIMER        17
#define ORGAN_SENSOR_BODYANGLE	  18
#define ORGAN_SENSOR_TRACKER         19
#define ORGAN_SPEAKER             20
#define ORGAN_SENSOR_EAR          21
#define ORGAN_MUSCLE_STRAFE       22
#define ORGAN_SENSOR_PHEROMONE    23
#define ORGAN_EMITTER_PHEROMONE   24
#define ORGAN_MEMORY_RX           25
#define ORGAN_MEMORY_TX           26
#define ORGAN_GILL                27

#define ORGAN_LUNG                 28
#define ORGAN_SENSOR_HUNGER        29
#define ORGAN_SENSOR_AGE           30
#define ORGAN_SENSOR_LAST_STRANGER 31
#define ORGAN_SENSOR_LAST_KIN      32
#define ORGAN_SENSOR_PARENT        33
#define ORGAN_SENSOR_BIRTHPLACE    34
#define ORGAN_SENSOR_TOUCH         35


#define ORGAN_COLDADAPT            2001
#define ORGAN_HEATADAPT            2002

#define ORGAN_GRABBER              36

#define numberOfOrganTypes        37 // the number limit of growable genes
#define MATERIAL_FOOD             60
#define MATERIAL_ROCK             61
#define MATERIAL_MEAT             62
#define MATERIAL_BONE             63
#define MATERIAL_BLOOD            64
#define MATERIAL_GRASS            65
#define MATERIAL_METAL            66
#define MATERIAL_VOIDMETAL        67
#define MATERIAL_SMOKE           68
#define MATERIAL_GLASS            69
#define MATERIAL_WATER            70

// #define TERRAIN_STONE             50
// #define TERRAIN_WATER             52
// #define TERRAIN_LAVA              54
// #define TERRAIN_VOIDMETAL         68

#define MARKER                    35 // numbers above 25 don't correspond to lower-case letters(0..25) so we don't use them in the gene code. But (26..31) are still compatible with our masking scheme.

#define CONDITION_GREATER         41
#define CONDITION_EQUAL           42
#define CONDITION_LESS            43

#define WORLD_EXAMPLECREATURE 2

int visualizer = VISUALIZER_TRUECOLOR;

#define NUMBER_OF_CONNECTIONS 8

const bool brownianMotion        = false;
const bool immortality           = false;
const bool doReproduction        = true;
const bool doMuscles             = true;
const bool growingCostsEnergy    = true;
const bool tournament            = true;
const bool taxIsByMass           = true;
const bool threading             = true;
const bool cameraFollowsChampion = false;
const bool cameraFollowsPlayer   = true;
const bool variedGrowthCost      = false;
const bool variedUpkeep          = false;
const bool respawnLowSpecies     = true;
const bool doMutation            = true;
const bool sensorJiggles         = false;
const bool useTimers             = false;
const bool setOrSteerAngle       = true;
// const bool useLava               = true;

const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.

const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const unsigned int animalSquareSize      = 128;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfSpecies = 8;
const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.0002f;        // a multiplier for how much it costs animals just to exist.
const float movementEnergyScale    = 0.0002f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.9f;         // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float grassEnergy            = 0.3f;         // how much you get from eating a square of grass

const float neuralNoise = 0.1f;

const float liverStorage = 20.0f;
const unsigned int baseLifespan = 5000;
const float signalPropagationConstant = 0.1f;      // how strongly sensor organs compel the animal.
const float musclePower = 40.0f;
const float thresholdOfBoredom = 0.1f;

const unsigned int displayNameSize = 32;

const unsigned int numberOfSpeakerChannels = 16;


const float const_pi = 3.1415f;

unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);

bool lockfps               = false;

bool paused = false;

int mousePositionX =  -430;
int mousePositionY =  330;

float fmousePositionX = mousePositionX;
float fmousePositionY = mousePositionY;


int selectedAnimal = -1;
int cursorAnimal = -1;

unsigned int worldToLoad = WORLD_EXAMPLECREATURE;

float fps = 1.0f;

bool playerGrabState = false;
bool playerInControl = true;
int playerCreature = -1;
bool playerCanSee = true;
bool playerCanHear = true;
bool playerCanSmell = true;
bool palette = false;

bool computer1display = false;
bool computer2display = false;

float energyScaleIn             = 1.0f;            // a multiplier for how much energy is gained from food and light.
float minimumEntropy = 0.1f;

float speakerChannels[numberOfSpeakerChannels];
float speakerChannelsLastTurn[numberOfSpeakerChannels];

int neighbourOffsets[] =
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

unsigned int cameraPositionX = 0 ;
unsigned int cameraPositionY = 0 ;
unsigned int modelFrameCount = 0;

int paletteMenuX = 200;
int paletteMenuY = 500;
int paletteTextSize = 10;
int paletteSpacing = 20;


unsigned int paletteSelectedOrgan = 0;
unsigned int paletteWidth = 3;


int championScore = 0;
int tournamentInterval = 10000;
int tournamentCounter  = 0;

int adversary = -1;

int cameraTargetCreature = -1;
unsigned int usPerFrame = 0;
unsigned int populationCount = 0;
unsigned int cameraFrameCount = 0;


const unsigned int nLogs = 32;
const unsigned int logLength = 64;

char logs[logLength][nLogs];




void appendLog( std::string input)
{
	for (int i = nLogs; i > 0; i--)
	{
		memcpy(  &logs[i] , &logs[i - 1], sizeof(char) * logLength   );
	}
	// memcpy(  &logs[0] , &input, sizeof(char) * logLength   );
	strcpy( &logs[0][0] , input.c_str() );
}

struct Square
{

	unsigned int wall;      //  material filling the volume of the square. You probably can't move through it.
	unsigned int material;  // a piece of material sitting on the ground. You can move over it, no matter what it is made of.
	unsigned int terrain;   // the floor itself. If it is not solid, you may have to swim in it.



	int identity;   // id of the last animal to cross the tile
	int occupyingCell; // id of the last cell to cross this tile
	float trail;    // movement direction of the last animal to cross the tile
	int height;
	Color light;
	float temperature;
	float pheromoneIntensity;
	int pheromoneChannel;

	Color grassColor;


};


// struct Plant
// {

// 	int plantIdentity;
// 	float plantEnergy;
// 	Color plantColor;
// 	unsigned int plantState;
// 	char plantGenes[32];
// 	unsigned int plantGeneCursor;
// 	unsigned seedGenes[32];
// };


// #define PLANT_LEAF   1
// #define PLANT_SEED   2
// #define PLANT_WOOD   3
// #define PLANT_FLOWER 4
// #define PLANT_ROOT   5

// void plantTurn(unsigned int worldPositionI)
// {



// // // add some energy to leaves.
// // 	world[worldPositionI].plantEnergy += world[worldPositionI].light;

// // // grow into surrounding squares if energy available.
// // 	if ()

// // 		// seeds shift into neighbour square at random.



// }







struct Square world[worldSquareSize];

struct Connection
{
	bool used;
	unsigned int connectedTo;
	float weight;
};

struct Cell
{
	unsigned int organ;
	float signalIntensity;
	float timerFreq;
	float timerPhase;  // also used as memory state
	Color color;
	unsigned int speakerChannel;
	int eyeLookX;
	int eyeLookY;
	int localPosX;
	int localPosY;
	unsigned int worldPositionI;
	float damage;
	bool dead;
	int grabbedCreature;
	Connection connections[NUMBER_OF_CONNECTIONS];
};

struct Animal
{
	Cell body[animalSquareSize];
	Cell genes[animalSquareSize];
	unsigned int mass;
	unsigned int numberOfTimesReproduced;
	unsigned int damageDone;
	unsigned int damageReceived;
	unsigned int birthLocation;
	unsigned int age;
	unsigned int lifespan;
	int parentIdentity;
	bool retired;
	float offspringEnergy;
	float energy;
	float maxEnergy;
	float energyDebt;
	unsigned int position;
	unsigned int uPosX;
	unsigned int uPosY;
	float fPosX;
	float fPosY;
	float fAngle;  // the direction the animal is facing
	float fAngleSin;
	float fAngleCos;
	bool parentAmnesty;
	unsigned int totalMuscle;
	unsigned int totalGonads;
	bool canBreatheUnderwater;
	bool canBreatheAir;
	unsigned int lastTouchedStranger;
	unsigned int lastTouchedKin;
	unsigned int cellsUsed;
	Color identityColor;
	// bool grabbed;
	// std::string displayName;
	float temp_limit_low;
	float temp_limit_high;
	char displayName[displayNameSize];

	float lastfposx ;
	float lastfposy;

	bool isMachine;
	// unsigned int machineType;
	void (* machineCallback)(int, int);
};

Animal champion;


std::string pheromoneChannelDescriptions[numberOfSpeakerChannels] =
{
	std::string( "It has an earthy smell." ),
	std::string( "It has a musty smell." ),
	std::string( "It smells of urine." ),
	std::string( "It smells like soap." ),
	std::string( "It has a gamey smell." ),
	std::string( "It smells like a cat's fur." ),
	std::string( "It smells like dried hay." ),
	std::string( "It smells like fresh dried laundry. Delightful." ),
	std::string( "It smells like the rain after a hot day." ),
	std::string( "It smells like the salt air at the beach." ),
	std::string( "It smells like the perfume of a frangipani's flower." ),
	std::string( "It smells like pool chlorine." ),
	std::string( "It smells like electricity." ),
	std::string( "It smells like rotting meat. Yuck!" ),
	std::string( "You can smell raspberries. Incredible!" ),
	std::string( "It smells like vomit." ),
};

// std::string terrainDescriptions(unsigned int terrain)
// {
// 	switch (terrain)
// 	{
// 	case TERRAIN_STONE:
// 	{
// 		return std::string("The ground here is solid rock.");
// 	}
// 	case TERRAIN_WATER:
// 	{
// 		return std::string("There is deep, cold water here.");
// 	}
// 	case TERRAIN_LAVA:
// 	{
// 		return std::string("The ground here is molten burning lava.");
// 	}
// 	case TERRAIN_VOIDMETAL:
// 	{
// 		return std::string("The ground here is black, shiny void metal.");
// 	}
// 	}
// 	return std::string("You can't tell if the ground is safe here.");
// }

std::string tileDescriptions(unsigned int tile)
{
	switch (tile)
	{
	case ORGAN_MOUTH_VEG:
	{
		return std::string("A chomping mouth with flat teeth that chews side-to-side.");
	}
	case ORGAN_MOUTH_SCAVENGE:
	{
		return std::string("A sucker-like mouth that slurps up detritus.");
	}
	case ORGAN_GONAD:
	{
		return std::string("A bulging testicle filled with potential offspring.");
	}
	case ORGAN_MUSCLE:
	{
		return std::string("A muscular limb that can pull the animal along.");
	}
	case ORGAN_BONE:
	{
		return std::string("A strong slab of bone.");
	}
	case ORGAN_WEAPON:
	{
		return std::string("A wicked razor-sharp claw.");
	}
	case ORGAN_LIVER:
	{
		return std::string("A brownish slab of flesh that stores energy.");
	}
	case ORGAN_MUSCLE_TURN:
	{
		return std::string("A muscular limb good for turning and spinning.");
	}
	case ORGAN_SENSOR_EYE:
	{
		return std::string("A monochrome, single pixel eye.");
	}
	case ORGAN_MOUTH_CARNIVORE:
	{
		return std::string("A grinning mouth with serrated, backward curving teeth.");
	}
	case ORGAN_MOUTH_PARASITE:
	{
		return std::string("A leech-like mouth that drains vital energy from victims.");
	}
	case ORGAN_ADDOFFSPRINGENERGY:
	{
		return std::string("A womb-like organ that imbues the offspring with energy before they are born.");
	}
	case ORGAN_ADDLIFESPAN:
	{
		return std::string("A mysterious organ that promotes long life.");
	}
	case ORGAN_NEURON:
	{
		return std::string("A basic brain cell that connects to form networks.");
	}
	case ORGAN_BIASNEURON:
	{
		return std::string("A part of the brain that provides a constant output.");
	}
	case ORGAN_SENSOR_BODYANGLE:
	{
		return std::string("Part of the brain that senses the body's orientation.");
	}
	case ORGAN_SENSOR_TRACKER:
	{
		return std::string("Part of the brain that seeks prey.");
	}
	case ORGAN_SPEAKER:
	{
		return std::string("A resonant chamber that is blown with air to produce sound.");
	}
	case ORGAN_SENSOR_EAR:
	{
		return std::string("A chamber full of tiny hairs that detect vibrations.");
	}
	case ORGAN_MUSCLE_STRAFE:
	{
		return std::string("A muscular limb good for moving sideways.");
	}
	case ORGAN_SENSOR_PHEROMONE:
	{
		return std::string("A pocket that detects chemical signals.");
	}
	case ORGAN_EMITTER_PHEROMONE:
	{
		return std::string("A scent-producing gland that is plump with waxy secretions.");
	}
	case ORGAN_MEMORY_TX:
	{
		return std::string("A part of the brain responsible for storing knowledge in memory.");
	}
	case ORGAN_MEMORY_RX:
	{
		return std::string("A part of the brain that retrieves knowledge from memory.");
	}
	case ORGAN_GILL:
	{
		return std::string("A red, frilly gill for breathing water.");
	}
	case ORGAN_LUNG:
	{
		return std::string("A pink, spongy, air breathing lung.");
	}
	case ORGAN_SENSOR_HUNGER:
	{
		return std::string("This part of the brain feels the pain of hunger.");
	}
	case ORGAN_SENSOR_AGE:
	{
		return std::string("This part of the brain feels the weight of age.");
	}
	case ORGAN_SENSOR_LAST_STRANGER:
	{
		return std::string("A part of the brain which contains a memory of meeting an unknown animal.");
	}
	case ORGAN_SENSOR_LAST_KIN:
	{
		return std::string("A part of the brain which contains a memory of the animal's peer.");
	}
	case ORGAN_SENSOR_PARENT:
	{
		return std::string("This part contains a memory of the animal's mother.");
	}
	case ORGAN_SENSOR_BIRTHPLACE:
	{
		return std::string("This part contains a memory of a childhood home.");
	}
	case ORGAN_SENSOR_TOUCH:
	{
		return std::string("Soft, pillowy flesh that responds to touch.");
	}
	case ORGAN_GRABBER:
	{
		return std::string("A bony hand which can clutch items and grab animals.");
	}
	case MATERIAL_FOOD:
	{
		return std::string("There's a piece of dried-out old meat.");
	}
	case MATERIAL_ROCK:
	{
		return std::string("There's a solid grey rock.");
	}

	case MATERIAL_MEAT:
	{
		return std::string("A bleeding chunk of flesh.");
	}
	case MATERIAL_BONE:
	{
		return std::string("A fragment of bone.");
	}
	case MATERIAL_BLOOD:
	{
		return std::string("Splatters of coagulating blood.");
	}
	case MATERIAL_METAL:
	{
		return std::string("This is made of smooth, polished metal.");
	}
	case MATERIAL_VOIDMETAL:
	{
		return std::string("It's made of impenetrable void metal.");
	}
	case MATERIAL_SMOKE:
	{
		return std::string("A wisp of smoke.");
	}
	case MATERIAL_GLASS:
	{
		return std::string("A pane of glass.");
	}
	case MATERIAL_NOTHING:
	{
		return std::string("There's nothing there.");
	}
	case MATERIAL_GRASS:
	{
		return std::string("Grass and weeds.");
	}


	}
	return std::string("You don't know what this is.");
}




std::string tileShortNames(unsigned int tile)
{
	switch (tile)
	{
	case ORGAN_MOUTH_VEG:
	{
		return std::string("Herbivore mouth");
	}
	case ORGAN_MOUTH_SCAVENGE:
	{
		return std::string("Scavenger mouth");
	}
	case ORGAN_GONAD:
	{
		return std::string("Asexual gonad");
	}
	case ORGAN_MUSCLE:
	{
		return std::string("Forward muscle");
	}
	case ORGAN_BONE:
	{
		return std::string("Bone");
	}
	case ORGAN_WEAPON:
	{
		return std::string("Claw");
	}
	case ORGAN_LIVER:
	{
		return std::string("Liver");
	}
	case ORGAN_MUSCLE_TURN:
	{
		return std::string("Turning muscle");
	}
	case ORGAN_SENSOR_EYE:
	{
		return std::string("Eye");
	}
	case ORGAN_MOUTH_CARNIVORE:
	{
		return std::string("Carnivore mouth");
	}
	case ORGAN_MOUTH_PARASITE:
	{
		return std::string("Parasite mouth");
	}
	case ORGAN_ADDOFFSPRINGENERGY:
	{
		return std::string("Add offspring energy");
	}
	case ORGAN_ADDLIFESPAN:
	{
		return std::string("Add lifespan");
	}
	case ORGAN_NEURON:
	{
		return std::string("Neuron");
	}
	case ORGAN_BIASNEURON:
	{
		return std::string("Bias neuron");
	}
	case ORGAN_SENSOR_BODYANGLE:
	{
		return std::string("Body angle sensor");
	}
	case ORGAN_SENSOR_TRACKER:
	{
		return std::string("Tracker sensor");
	}
	case ORGAN_SPEAKER:
	{
		return std::string("Speaker");
	}
	case ORGAN_SENSOR_EAR:
	{
		return std::string("Ear");
	}
	case ORGAN_MUSCLE_STRAFE:
	{
		return std::string("Strafe muscle");
	}
	case ORGAN_SENSOR_PHEROMONE:
	{
		return std::string("Pheromone sensor");
	}
	case ORGAN_EMITTER_PHEROMONE:
	{
		return std::string("Pheromone emitter");
	}
	case ORGAN_MEMORY_TX:
	{
		return std::string("Memory TX");
	}
	case ORGAN_MEMORY_RX:
	{
		return std::string("Memory RX");
	}
	case ORGAN_GILL:
	{
		return std::string("Gill");
	}
	case ORGAN_LUNG:
	{
		return std::string("Lung");
	}
	case ORGAN_SENSOR_HUNGER:
	{
		return std::string("Hunger sensor");
	}
	case ORGAN_SENSOR_AGE:
	{
		return std::string("Age sensor");
	}
	case ORGAN_SENSOR_LAST_STRANGER:
	{
		return std::string("Direction of last stranger sensor");
	}
	case ORGAN_SENSOR_LAST_KIN:
	{
		return std::string("Direction of last peer sensor");
	}
	case ORGAN_SENSOR_PARENT:
	{
		return std::string("Direction of parent sensor");
	}
	case ORGAN_SENSOR_BIRTHPLACE:
	{
		return std::string("Direction of birthplace sensor");
	}
	case ORGAN_SENSOR_TOUCH:
	{
		return std::string("Touch sensor");
	}
	case ORGAN_GRABBER:
	{
		return std::string("Grabber");
	}
	case MATERIAL_FOOD:
	{
		return std::string("Food");
	}
	case MATERIAL_ROCK:
	{
		return std::string("Rock");
	}

	case MATERIAL_MEAT:
	{
		return std::string("Meat");
	}
	case MATERIAL_BONE:
	{
		return std::string("Bone");
	}
	case MATERIAL_BLOOD:
	{
		return std::string("Blood");
	}
	case MATERIAL_METAL:
	{
		return std::string("Metal");
	}
	case MATERIAL_VOIDMETAL:
	{
		return std::string("Void metal");
	}
	case MATERIAL_SMOKE:
	{
		return std::string("Smoke");
	}
	case MATERIAL_GLASS:
	{
		return std::string("Glass");
	}
	case MATERIAL_NOTHING:
	{
		return std::string("Nothing");
	}
	case MATERIAL_GRASS:
	{
		return std::string("Grass");
	}


	}
	return std::string("Unknown");
}




// std::string materialDescriptions(unsigned int material)
// {
// 	switch (material)
// 	{

// 	case MATERIAL_FOOD:
// 	{
// 		return std::string("There's a piece of dried-out old meat.");
// 	}

// 	case MATERIAL_ROCK:
// 	{
// 		return std::string("There's a solid grey rock.");
// 	}

// 	case MATERIAL_MEAT:
// 	{
// 		return std::string("A bleeding chunk of flesh.");
// 	}
// 	case MATERIAL_BONE:
// 	{
// 		return std::string("A fragment of bone.");
// 	}
// 	case MATERIAL_BLOOD:
// 	{
// 		return std::string("Splatters of coagulating blood.");
// 	}
// 	case MATERIAL_METAL:
// 	{
// 		return std::string("This is made of smooth, polished metal.");
// 	}
// 	case MATERIAL_VOIDMETAL:
// 	{
// 		return std::string("It's made of impenetrable void metal.");
// 	}
// 	case MATERIAL_SMOKE:
// 	{
// 		return std::string("A wisp of smoke.");
// 	}
// 	case MATERIAL_GLASS:
// 	{
// 		return std::string("A pane of glass.");
// 	}
// 	case MATERIAL_NOTHING:
// 	{
// 		return std::string("There's nothing there.");
// 	}
// 	case MATERIAL_GRASS:
// 	{
// 		return std::string("Scruffy green grass.");
// 	}
// 	}
// 	return std::string("An unknown material.");
// }

bool speciesVacancies [numberOfSpecies];
unsigned int speciesPopulationCounts [numberOfSpecies];
unsigned int populationCountUpdates  [numberOfSpecies];
unsigned int speciesAttacksPerTurn   [numberOfSpecies];

float organGrowthCost(unsigned int organ)
{
	float growthCost = 0.0f;
	if (growingCostsEnergy)
	{
		growthCost = growthEnergyScale;
		if (variedGrowthCost)
		{
			switch (organ)
			{
			case ORGAN_MUSCLE:
				growthCost *= 1.0f;
				break;
			case ORGAN_BONE:
				growthCost *= 1.0f;
				break;
			case ORGAN_WEAPON:
				growthCost *= 2.0f;
				break;
			case ORGAN_GONAD:
				growthCost *= 10.0f;
				break;
			case ORGAN_MOUTH_VEG:
				growthCost *= 10.0f;
				break;
			case ORGAN_MOUTH_SCAVENGE:
				growthCost *= 10.0f;
				break;
			case ORGAN_MOUTH_CARNIVORE:
				growthCost *= 10.0f;
				break;
			case ORGAN_MOUTH_PARASITE:
				growthCost *= 10.0f;
				break;
			}
		}
	}
	return growthCost;
}

float organUpkeepCost(unsigned int organ)
{
	float upkeepCost = 1.0f;

	if (variedUpkeep)
	{
		switch (organ)
		{
		case ORGAN_GONAD:
			upkeepCost = 2.0f;
			break;
		}
	}
	return upkeepCost;
}

Animal animals[numberOfAnimals];

void resetMouseCursor()
{
	mousePositionX = 0;
	mousePositionY = 0;
	fmousePositionX = 0.0f;
	fmousePositionY = 0.0f;
}
void togglePause ()
{
	paused = !paused;
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
	animals[animalIndex].body[cellLocalPositionI].dead = false;
	animals[animalIndex].body[cellLocalPositionI].grabbedCreature = -1;

	for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
	{
		resetConnection(animalIndex, cellLocalPositionI, i);
	}
	animals[animalIndex].genes[cellLocalPositionI] = animals[animalIndex].body[cellLocalPositionI];

}

void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{


		std::string gunDescription = std::string("An animal");
		strcpy( &animals[animalIndex].displayName[0] , gunDescription.c_str() );

		// animals[animalIndex].displayName = std::string("").c_str();
		// memset( (&animals[animalIndex].displayName), ' ', 32);
		animals[animalIndex].mass = 0;
		animals[animalIndex].numberOfTimesReproduced = 0;
		animals[animalIndex].damageDone = 0;
		// animals[animalIndex].grabbed = false;
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
		animals[animalIndex].canBreatheUnderwater = false;
		animals[animalIndex].canBreatheAir = false;
		animals[animalIndex].cellsUsed = 0;
		animals[animalIndex].identityColor = Color(RNG(), RNG(), RNG(), 1.0f);
		animals[animalIndex].isMachine = false;
		animals[animalIndex].machineCallback == nullptr;
		animals[animalIndex].temp_limit_low = 273.0f;
		animals[animalIndex].temp_limit_high = 323.0f;

		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{

			resetCell(animalIndex, cellLocalPositionI );
		}
	}
}

// check if a cell has an empty neighbour.
bool isCellAnEdge(unsigned int animalIndex, unsigned int cellIndex)
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

bool organIsAnActuator(unsigned int organ)
{
	if (    organ == ORGAN_MUSCLE ||
	        organ == ORGAN_MUSCLE_TURN ||
	        organ == ORGAN_MUSCLE_STRAFE ||
	        organ == ORGAN_SPEAKER  ||
	        organ == ORGAN_EMITTER_PHEROMONE ||
	        organ == ORGAN_MEMORY_TX
	   )
	{
		return true;
	}
	return false;
}

bool organIsANeuron(unsigned int organ)
{
	if (    organ == ORGAN_NEURON ||
	        organ == ORGAN_BIASNEURON
	   )
	{
		return true;
	}
	return false;
}

bool organIsASensor(unsigned int organ)
{
	if (
	    organ == ORGAN_SENSOR_EYE ||
	    organ == ORGAN_SENSOR_TOUCH ||
	    organ == ORGAN_SENSOR_TIMER ||
	    organ == ORGAN_SENSOR_BODYANGLE ||
	    organ == ORGAN_SENSOR_TRACKER      ||
	    organ == ORGAN_SENSOR_EAR        ||
	    organ == ORGAN_SENSOR_PHEROMONE ||
	    organ == ORGAN_MEMORY_RX ||
	    organ == ORGAN_SENSOR_LAST_STRANGER ||
	    organ == ORGAN_SENSOR_LAST_KIN ||
	    organ == ORGAN_SENSOR_HUNGER ||
	    organ == ORGAN_SENSOR_AGE ||
	    organ == ORGAN_SENSOR_BIRTHPLACE ||
	    organ == ORGAN_SENSOR_PARENT
	)
	{
		return true;
	}
	return false;
}

bool isCellConnecting(unsigned int organ)
{
	if (
	    organIsAnActuator(organ) ||
	    organIsANeuron(organ)
	)
	{
		return true;
	}

	return false;
}

bool isCellConnectable(unsigned int organ)
{
	if (
	    organIsASensor(organ) ||
	    organIsANeuron(organ)
	)
	{
		return true;
	}

	return false;
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
				// pick a random connectable cell to connect to.
				unsigned int connectableCell = getRandomConnectableCell( animalIndex);

				// check if you are already connected to it.
				bool alreadyConnected =  false;
				for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
				{
					if (  animals[animalIndex].genes[cellIndex].connections[j].connectedTo == connectableCell &&
					        animals[animalIndex].genes[cellIndex].connections[j] .used)
					{
						alreadyConnected = true;
					}
				}

				// make the new connection if appropriate.
				if (!alreadyConnected)
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
	// figure out a new position anywhere on the animal edge
	Vec_i2 newPosition   = getRandomEmptyEdgeLocation(animalIndex);
	appendCell(animalIndex, organType,  newPosition);
}

void setupExampleAnimal2(int i)
{
	// set the example back to the default state or it wont work properly.
	resetAnimal(i);

	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EAR );
	animalAppendCell( i, ORGAN_SENSOR_PHEROMONE );
	animalAppendCell( i, ORGAN_SENSOR_TRACKER );
	animalAppendCell( i, ORGAN_SENSOR_TOUCH );
	animalAppendCell( i, ORGAN_SENSOR_BODYANGLE );
	animalAppendCell( i, ORGAN_MEMORY_RX );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_MEMORY_TX );
	animalAppendCell( i, ORGAN_MUSCLE );
	animalAppendCell( i, ORGAN_MUSCLE_TURN );
	animalAppendCell( i, ORGAN_LUNG );
	animalAppendCell( i, ORGAN_LIVER );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
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
	championScore = 1;
}

void resetGrid()
{
	for (int i = 0; i < worldSquareSize; ++i)
	{
		world[i].terrain = MATERIAL_NOTHING;
		world[i].material = MATERIAL_NOTHING;
		world[i].identity = -1;
		world[i].trail = 0.0f;
		world[i].height = 0;
		world[i].light = color_white;
		world[i].pheromoneIntensity = 0.0f;
		world[i].pheromoneChannel = -1;
		world[i].grassColor = color_green;
	}
}

char randomLetter()
{
	return	extremelyFastNumberFromZeroTo(numberOfOrganTypes);
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

bool materialBlocksMovement(unsigned int material)
{
	if (material == MATERIAL_ROCK ||
	        material == MATERIAL_VOIDMETAL ||
	        material == MATERIAL_METAL)
	{
		return true;
	}
	return false;
}

// // some genes have permanent effects, or effects that need to be known immediately at birth. Compute them here.
// this function studies the phenotype, not the genotype.
// returns whether the animal is fit to live.
bool measureAnimalQualities(unsigned int animalIndex)
{
	// update mass and debt
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
		        animals[animalIndex].body[cellIndex].organ == ORGAN_MUSCLE_STRAFE
		   )
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

Color mutateColor(Color in)
{
	Color out = in;
	out.r += (RNG() - 0.5) * 0.1f;
	out.g += (RNG() - 0.5) * 0.1f;
	out.b += (RNG() - 0.5) * 0.1f;
	return clampColor(out);
}

// the opposite of append cell- remove a cell from the genes and body, shifting all other cells backwards and updating all connections.
void eliminateCell( unsigned int animalIndex, unsigned int cellToDelete )
{
	// shift array of cells down 1, overwriting the lowest modified cell (the cell to delete)
	for (int cellIndex = cellToDelete + 1; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
	{
		animals[animalIndex].body[cellIndex - 1] = animals[animalIndex].body[cellIndex];
	}

	// clear the end cell which would have been duplicated
	animals[animalIndex].cellsUsed--;

	// go through all cells and update connections
	for (int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; ++cellIndex)
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
}

void mutateAnimal(unsigned int animalIndex)
{
	if (!doMutation) {return;}

	if (
	    true
	    &&
	    animalIndex < numberOfAnimals
	) // don't mutate at all sometimes, to preserve a population against drift
	{
		// the mutations should not be evenly distributed, but the most important stuff should be focused on.
		unsigned int whatToMutate = extremelyFastNumberFromZeroTo(8);

		if (whatToMutate == 0)
		{
			// erase an organ
			int mutantCell = getRandomPopulatedCell( animalIndex);
			if (mutantCell >= 0)
			{
				eliminateCell(animalIndex, mutantCell);
			}
		}

		else if (whatToMutate == 1)
		{
			// add an organ
			unsigned int newOrgan = randomLetter();
			animalAppendCell(animalIndex, newOrgan);
		}

		else if (whatToMutate == 2)
		{
			// turn a connection on or off.
			int mutantCell =  getRandomConnectingCell(animalIndex);
			if (mutantCell >= 0)
			{
				unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
				animals[animalIndex].genes[mutantCell].connections[mutantConnection].used = !(animals[animalIndex].genes[mutantCell].connections[mutantConnection].used );
			}
		}

		else if (whatToMutate == 3)
		{
			// randomise a connection partner.
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
		}
		else if (whatToMutate == 4)
		{
			// randomise a connection weight.
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
		}
		else if (whatToMutate == 6)
		{
			// randomise a bias neuron.
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
		}
		else if (whatToMutate == 7)
		{
			// swap an existing cell location without messing up the connections.
			int mutantCell = getRandomPopulatedCell(animalIndex);
			if (mutantCell >= 0)
			{
				Vec_i2 destination  = getRandomEmptyEdgeLocation(animalIndex);

				animals[animalIndex].body[mutantCell].localPosX = destination.x;
				animals[animalIndex].body[mutantCell].localPosY = destination.y;
			}
		}
		else if (whatToMutate == 8)
		{
			// other stuff.
			unsigned int auxMutation = extremelyFastNumberFromZeroTo(6);
			if (auxMutation == 0)
			{
				int mutantCell = getRandomPopulatedCell(animalIndex);
				if (mutantCell >= 0 && mutantCell < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCell].color = mutateColor(	animals[animalIndex].genes[mutantCell].color);
				}
			}
			else if (auxMutation == 1)
			{
				int mutantCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_EYE);
				if (mutantCell >= 0 && mutantCell < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCell].color = mutateColor(	animals[animalIndex].genes[mutantCell].color);
				}
			}
			else if (auxMutation == 2)
			{
				// mutate a timers freq
				int mutantCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_TIMER);
				if (mutantCell >= 0 && mutantCell < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCell].timerFreq += RNG() - 0.5f * 0.1;
				}
			}
			else if (auxMutation == 3)
			{
				// mutate a speaker channel
				int mutantCellA = getRandomCellOfType(animalIndex, ORGAN_SPEAKER);
				int mutantCellB = getRandomCellOfType(animalIndex, ORGAN_SENSOR_EAR);
				unsigned int mutantChannel = extremelyFastNumberFromZeroTo(numberOfSpeakerChannels - 1);
				if (mutantCellA >= 0 && mutantCellA < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellA].speakerChannel = mutantChannel;
				}
				if (mutantCellB >= 0 && mutantCellB < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellB].speakerChannel = mutantChannel  ;
				}
			}
			else if (auxMutation == 4)
			{
				// mutate a pheromone channel
				int mutantCellA = getRandomCellOfType(animalIndex, ORGAN_SENSOR_PHEROMONE);
				int mutantCellB = getRandomCellOfType(animalIndex, ORGAN_EMITTER_PHEROMONE);
				unsigned int mutantChannel = extremelyFastNumberFromZeroTo(numberOfSpeakerChannels - 1);
				if (mutantCellA >= 0 && mutantCellA < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellA].speakerChannel = mutantChannel;
				}
				if (mutantCellB >= 0 && mutantCellB < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellB].speakerChannel = mutantChannel  ;
				}
			}
			else if (auxMutation == 5)
			{
				// mutate a memory channel
				int mutantCellA = getRandomCellOfType(animalIndex, ORGAN_MEMORY_RX);
				int mutantCellB = getRandomCellOfType(animalIndex, ORGAN_MEMORY_TX);
				unsigned int mutantChannel = extremelyFastNumberFromZeroTo(numberOfSpeakerChannels - 1);
				if (mutantCellA >= 0 && mutantCellA < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellA].speakerChannel = mutantChannel;
				}
				if (mutantCellB >= 0 && mutantCellB < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellB].speakerChannel = mutantChannel  ;
				}
			}
			else if (auxMutation == 6)
			{
				// mutate an eyelook
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
			}
		}
	}
}

void spawnAnimalIntoSlot( unsigned int animalIndex,
                          Animal parent,
                          unsigned int position, bool mutation)
{
	resetAnimal(animalIndex);

	// copy genes from the parent and then copy body from own new genes.
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
				return cellIndex;
			}
		}
	}

	return -1;
}

// Color terrainColors(unsigned int terrain)
// {
// 	switch (terrain)
// 	{
// 	case TERRAIN_STONE:
// 		return color_grey;
// 	case TERRAIN_WATER:
// 		return color_lightblue;
// 	case TERRAIN_LAVA:
// 		return color_orange;
// 	case TERRAIN_VOIDMETAL:
// 		return color_charcoal;
// 	}
// 	return color_yellow;
// }

Color materialColors(unsigned int material)
{
	switch (material)
	{
	case MATERIAL_NOTHING:
		return color_clear;
	case MATERIAL_FOOD:
		return color_brown;
	case MATERIAL_ROCK:
		return color_grey;
	case MATERIAL_BONE:
		return color_white;
	case MATERIAL_BLOOD:
		return color_brightred;
	case MATERIAL_GRASS:
		return color_green;
	case MATERIAL_METAL:
		return color_darkgrey;
	case MATERIAL_VOIDMETAL:
		return color_charcoal;
	case MATERIAL_SMOKE:
		return color_lightgrey;
	case MATERIAL_GLASS:
		return color_lightblue;
	}
	return color_yellow;
}

Color organColors(unsigned int organ)
{
	switch (organ)
	{
	case ORGAN_MUSCLE:
		return color_brightred;
	case ORGAN_GONAD:
		return color_offwhite;
	case ORGAN_MOUTH_VEG:
		return color_darkgrey;
	case ORGAN_MOUTH_SCAVENGE:
		return color_darkgrey;
	case ORGAN_MOUTH_CARNIVORE:
		return color_darkgrey;
	case ORGAN_MOUTH_PARASITE:
		return color_darkgrey;
	case ORGAN_LIVER:
		return color_darkred;
	case ORGAN_BONE:
		return color_white;
	case ORGAN_WEAPON:
		return color_purple;
	}
	return color_yellow;
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
		if ( animals[viewedAnimal].body[occupyingCell].damage > 0.5f )
		{
			displayColor = organColors(animals[viewedAnimal].body[occupyingCell].organ );
		}
		else
		{
			displayColor = animals[viewedAnimal].body[occupyingCell].color;
		}



		// outline selected animal.
		// to draw an outline around the selected animal:
		// 1. make a list of all the animal squares and note the animal's bounding box.
		// 2. use it to make a list of all neighbours.
		// 3. subtract any of the neighbours that are within the animal's body (leaving just an outline).
		// 4. when drawing the view, check if you are within the bounding box, and if so, draw an outline tile if you are drawing a tile that is not part of the body.


		// highlight selected animal.
		if (viewedAnimal == selectedAnimal)
		{
			displayColor = filterColor(displayColor, tint_selected);
		}

	}
	else
	{

		// if (world[worldI].material == MATERIAL_GRASS)
		// {
		// displayColor =
		// }
		// else
		// {

		Color floorColor ;
		if (world[worldI].material == MATERIAL_GRASS)
		{
			floorColor = world[worldI].grassColor;
		}
		else
		{
			floorColor = materialColors(world[worldI].terrain);
		}



		// you can see the three material layers in order, wall then material then floor.
		displayColor = filterColor( floorColor,  materialColors(world[worldI].material) );
		displayColor = filterColor( displayColor,  materialColors(world[worldI].wall) );
		// }

	}

	displayColor = multiplyColor(displayColor, world[worldI].light);

	return displayColor;
}


bool materialDegrades(unsigned int material)
{
	if (material == MATERIAL_FOOD ||
	        material == MATERIAL_BONE ||
	        material == MATERIAL_BLOOD ||
	        material == MATERIAL_SMOKE)
	{return true;}

	return false;
}

void updateMap()
{
	unsigned int mapUpdateFidelity = worldSquareSize / 25000;
	for (unsigned int i = 0; i < mapUpdateFidelity; ++i)
	{
		unsigned int randomX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomY = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomI = (randomY * worldSize) + randomX;
		if (randomI < worldSquareSize)
		{


			// slowly reduce pheromones over time.
			if (world[randomI].pheromoneIntensity > 0.2f)
			{
				world[randomI].pheromoneIntensity -= 0.2f;
			}
			else
			{
				world[randomI].pheromoneChannel = -1;
			}


			// if (world[randomI].material == MATERIAL_NOTHING)
			// {
			// 	if (world[randomI].terrain == MATERIAL_ROCK ||  world[randomI].terrain == MATERIAL_WATER )
			// 	{
			// 		world[randomI].material = MATERIAL_GRASS;
			// 	}
			// }

			// if (world[randomI].terrain == TERRAIN_LAVA)
			// {
			// 	if (
			// 	    world[randomI].material != MATERIAL_NOTHING
			// 	    &&
			// 	    world[randomI].material != MATERIAL_VOIDMETAL // voidmetal is indestructible

			// 	)
			// 	{
			// 		world[randomI].material = MATERIAL_NOTHING;
			// 	}
			// }


			if (world[randomI].material == MATERIAL_NOTHING && world[randomI].wall == MATERIAL_NOTHING)
			{

				for (int i = 0; i < nNeighbours; ++i)
				{
					unsigned int neighbour = randomI + neighbourOffsets[i];
					if (neighbour < worldSquareSize)
					{
						if (world[neighbour].material == MATERIAL_GRASS)
						{
							world[randomI].material = MATERIAL_GRASS;

							world[randomI].grassColor = world[neighbour].grassColor;

							world[randomI].grassColor.r += (RNG() - 0.5f) * 0.1f;
							world[randomI].grassColor.g += (RNG() - 0.5f) * 0.1f;
							world[randomI].grassColor.b += (RNG() - 0.5f) * 0.1f;

							world[randomI].grassColor = clampColor(world[randomI].grassColor);


						}
					}
				}

			}

			if ( materialDegrades( world[randomI].material) )
			{
				world[randomI].material = MATERIAL_NOTHING;
			}

			if ( materialDegrades( world[randomI].wall) )
			{
				world[randomI].wall = MATERIAL_NOTHING;
			}






			// if (world[randomI].material == MATERIAL_BLOOD)
			// {
			// 	world[randomI].material = MATERIAL_NOTHING;
			// }

			// if (world[randomI].material == MATERIAL_BONE)
			// {
			// 	world[randomI].material = MATERIAL_NOTHING;
			// }
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
	return nBones * nBones;
}

float fast_sigmoid(float in)
{
	// https://stackoverflow.com/questions/10732027/fast-sigmoid-algorithm
	float out = (in / (1 + abs(in)));
	return  out;
}



void paletteCallback()
{
	// printf("a\n");
	// add the selected organ to the selected animal
	if (selectedAnimal >= 0 && selectedAnimal < numberOfAnimals)
	{
	// printf("b\n");
		if (paletteSelectedOrgan >= 0 && paletteSelectedOrgan < numberOfOrganTypes)
		{


	// printf("c\n");
			int newPosX = mousePositionX -   ( cameraPositionX  - animals[selectedAnimal].uPosX);
			int newPosY = mousePositionY -   ( cameraPositionY  - animals[selectedAnimal].uPosY);

			// animalAppendCell()
			// printf("appended cell to %i %i \n", newPosX, newPosY);

			appendCell(selectedAnimal, paletteSelectedOrgan, Vec_i2(newPosX, newPosY));

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

		// int diffX = mousePositionX - paletteFinalX;
		// int diffY = mousePositionY - paletteFinalY;

		// int absoluteDistance = abs(diffX) + abs(diffY);

		// if ( absoluteDistance < closestValue )
		// {
		// 	closestValue = absoluteDistance;
		// 	tempClosestToMouse = i;
		// }


		if (i == paletteSelectedOrgan)
		{

			printText2D(  std::string("X ") +  tileShortNames(i) , paletteFinalX, paletteFinalY, paletteTextSize);
		}
		else
		{

			printText2D(   tileShortNames(i) , paletteFinalX, paletteFinalY, paletteTextSize);
		}
	}

	// paletteSelectedOrgan= tempClosestToMouse;

	// draw a white box under the selected one.

	// unsigned int paletteX = paletteSelectedOrgan % paletteWidth;
	// unsigned int paletteY = paletteSelectedOrgan / paletteWidth;

	// int paletteFinalX = paletteMenuX + (paletteX * paletteSpacing * 6);
	// int paletteFinalY = paletteMenuY + (paletteY * paletteSpacing);

	// drawTile( Vec_f2(paletteFinalX, paletteFinalY), color_brightred );
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

// occurs whenever a left click is received.
void activateGrabbedMachine()
{
	if (playerCreature >= 0 && playerInControl)
	{

		if (palette)
		{
			paletteCallback();
		}


		for (int i = 0; i < animals[playerCreature].cellsUsed; ++i)
		{
			if (animals[playerCreature].body[i].organ == ORGAN_GRABBER)
			{
				if (animals[playerCreature].body[i].grabbedCreature >= 0 && animals[playerCreature].body[i].grabbedCreature < numberOfAnimals)
				{
					if (animals [   animals[playerCreature].body[i].grabbedCreature  ].isMachine)
					{
						if (animals [   animals[playerCreature].body[i].grabbedCreature  ].machineCallback != nullptr)
						{
							(*animals [   animals[playerCreature].body[i].grabbedCreature  ].machineCallback)( animals[playerCreature].body[i].grabbedCreature , playerCreature) ;
							break;
						}
					}
				}
			}
		}
	}
}

// the animal is a grid of living cells that do different things. this function describes what they do each turn.
void organs_all()
{
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			// unsigned int cellsDone = 0;
			float totalLiver = 0;
			unsigned int totalGonads = 0;
			float highestIntensity = 0.0f;
			bool canBreatheUnderwater = false;
			bool canBreatheAir        = false;

			for (unsigned int cellIndex = 0; cellIndex < animals[animalIndex].cellsUsed; cellIndex++)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				// unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
				// unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
				// int cellLocalPositionX =  animals[animalIndex].body[cellIndex].localPosX ;//cellLocalPositionI % animalSize;
				// int cellLocalPositionY =  animals[animalIndex].body[cellIndex].localPosY ;//cellLocalPositionI / animalSize;

				// // add the eyelook
				// cellLocalPositionX += animals[animalIndex].body[cellIndex].eyeLookX;
				// cellLocalPositionY += animals[animalIndex].body[cellIndex].eyeLookY;

				// // rotate by animal angle
				// cellLocalPositionX *= animals[animalIndex].fAngleCos;
				// cellLocalPositionY *= animals[animalIndex].fAngleSin;

				// // world position now takes animal rotation into account (the drawings will not show that it is rotating, but it affects what the animal perceives.).
				// unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
				// unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
				// unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;


				// get the world position, add the rotated eyelook, and then find the eyelook world position.
				unsigned int cellWorldPositionI = animals[animalIndex].body[cellIndex].worldPositionI;
				unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
				unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;





				if (cellWorldPositionI >= worldSquareSize) {continue;}

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

					// if greater than 0, grab.
					if (animals[animalIndex].body[cellIndex].signalIntensity  >= 1.0f && animals[animalIndex].body[cellIndex].grabbedCreature  == -1)
					{
						int grabArea = 5;
						bool grabbedSomething = false;
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


											// finally, make sure the item is not grabbed by another of your own grabbers.
											bool grabbedByAnotherGrabber = false;
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
												animals[animalIndex].body[cellIndex].grabbedCreature = world[neighbour].identity;
												animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
												grabbedSomething = true;



												// appendLog( std::string ("you picked up an item") );

												break;
											}
										}

									}
								}
							}
							if (grabbedSomething)
							{
								break;
							}
						}

						// }

					}





					// if there is a grabbed creature, adjust its position to the grabber.
					if (animals[animalIndex].body[cellIndex].grabbedCreature >= 0 )
					{
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosX = cellWorldPositionX;
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].uPosY = cellWorldPositionY;

						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosX = cellWorldPositionX;
						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fPosY = cellWorldPositionY;

						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].position = cellWorldPositionI;



						// also, if grabbed by the player, adjust the angle of the grabbed object so it points at the mouse cursor. for aiming weapons.

						float fposx = cellWorldPositionX;
						float fposy = cellWorldPositionY;


						float angleToCursor = atan2(   fmousePositionY - (  cameraPositionY - fposy)  ,  fmousePositionX - (cameraPositionX - fposx));


						animals [ animals[animalIndex].body[cellIndex].grabbedCreature  ].fAngle = angleToCursor;

						if (  animals[animalIndex].body[cellIndex].signalIntensity  <= -1.0f)
						{
							// printf("animals[animalIndex].body[cellIndex].signalIntensity %f\n", animals[animalIndex].body[cellIndex].signalIntensity);
							animals[animalIndex].body[cellIndex].grabbedCreature = -1;
							animals[animalIndex].body[cellIndex].signalIntensity = 0.0f;
						}
					}
					break;

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
						float targetWorldPositionX =   animals[animalIndex]. birthLocation % worldSize;  ;//animals[  animals[animalIndex].parent   ]  .fPosX;
						float targetWorldPositionY =   animals[animalIndex]. birthLocation / worldSize;  ;//animals[  animals[animalIndex].parent   ]  .fPosY;
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
					canBreatheAir = true;
					break;

				}
				case ORGAN_GILL:
				{
					canBreatheUnderwater = true;
					break;

				}

				case ORGAN_MEMORY_RX:
				{
					// don't need to do anything, the tx part does all the work.
					break;
				}

				case ORGAN_MEMORY_TX:
				{
					// sum inputs. if exceeding a threshold, find a corresponding memory RX cell and copy it the input sum.
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
						// go through the list of connections and sum their values.
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

				case ORGAN_SENSOR_TIMER:
				{
					animals[animalIndex].body[cellIndex].signalIntensity = 0;
					if (useTimers)
					{
						animals[animalIndex].body[cellIndex].timerPhase += animals[animalIndex].body[cellIndex].timerFreq;
						animals[animalIndex].body[cellIndex].signalIntensity = sin(animals[animalIndex].body[cellIndex].timerPhase);
					}
					break;
				}

				case ORGAN_NEURON:
				{
					// go through the list of connections and sum their values.
					float sum = 0.0f;
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
								int result = spawnAnimal( speciesIndex,
								                          animals[animalIndex],
								                          animals[animalIndex].position, true );
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
						animals[animalIndex].energy += grassEnergy * energyScaleIn;
						world[cellWorldPositionI].material = MATERIAL_NOTHING;
					}
					break;
				}

				case ORGAN_MOUTH_SCAVENGE :
				{
					if (world[cellWorldPositionI].material == MATERIAL_FOOD)
					{
						animals[animalIndex].energy += foodEnergy * energyScaleIn;
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
							// go through the list of connections and sum their values.
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
							// go through the list of connections and sum their values.
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

						// on the strafe muscle the sin and cos are reversed, that's all.
						animals[animalIndex].fPosX += animals[animalIndex].body[cellIndex].signalIntensity * 10 * sin(animals[animalIndex].fAngle);
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
							// go through the list of connections and sum their values.
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
			animals[animalIndex].canBreatheAir = canBreatheAir;
			animals[animalIndex].canBreatheUnderwater = canBreatheUnderwater;
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
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;

		if (!animals[animalIndex].retired)
		{
			// calculate direction of movement.
			// ufpos is the last guaranteed place of the animal, in floats
			// float ufposx = animals[animalIndex].uPosX;
			// float ufposy = animals[animalIndex].uPosY;
			// get the diff between the last actual place and the proposed new place

			bool trailUpdate = false;
			float dAngle = 0.0f;
			if (animals[animalIndex].fPosX != animals[animalIndex].lastfposx ||
			        animals[animalIndex].fPosY != animals[animalIndex].lastfposy  )
			{
				float fdiffx =  animals[animalIndex].fPosX - animals[animalIndex].lastfposx;
				float fdiffy =  animals[animalIndex].fPosY - animals[animalIndex].lastfposy;
				// use atan2 to turn the diff into an angle.
				dAngle = atan2(fdiffy, fdiffx);

				dAngle -= 0.5 * const_pi;

				if (dAngle < const_pi)
				{
					dAngle += (2 * const_pi);
				}

				trailUpdate = true;
				// if (animalIndex == playerCreature)
				// {
				// 	printf("magle dangle = %f\n", dAngle);
				// }

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


				if (! animals[animalIndex].isMachine)
				{
					if (world[newPosition].terrain == MATERIAL_WATER )
					{
						if (! animals[animalIndex].canBreatheUnderwater )
						{
							animals[animalIndex].damageReceived ++;
						}
					}
					else
					{
						if (! animals[animalIndex].canBreatheAir)
						{
							animals[animalIndex].damageReceived ++;
						}
					}
				}


				if (world[newPosition].temperature > animals[animalIndex].temp_limit_high)
				{
					animals[animalIndex].damageReceived += abs(world[newPosition].temperature  - animals[animalIndex].temp_limit_high);
				}


				if (world[newPosition].temperature < animals[animalIndex].temp_limit_low)
				{
					animals[animalIndex].damageReceived += abs(world[newPosition].temperature  - animals[animalIndex].temp_limit_low);
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

								if (animals[animalIndex].body[cellIndex].organ == ORGAN_WEAPON ||
								        animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_CARNIVORE )
								{
									if (animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
									{
										if (world[cellWorldPositionI].identity == animals[animalIndex].parentIdentity)
										{
											continue;
										}
									}

									float defense = defenseAtWorldPoint(world[cellWorldPositionI].identity, cellWorldPositionI);

									if (defense > 0)
									{
										animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].damage += 1.0f / defense;
									}

									if (defense == 0 || animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].damage > 1.0f )
									{
										animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].dead = true;

										if (animals[world[cellWorldPositionI].identity].mass >= 1)
										{
											animals[world[cellWorldPositionI].identity].mass--;
										}
										animals[world[cellWorldPositionI].identity].damageReceived++;
										okToStep = true;
										animals[animalIndex].damageDone++;

										if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
										{
											world[cellWorldPositionI].material = MATERIAL_BLOOD;
										}

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
												animals[animalIndex].energy += foodEnergy * energyScaleIn;
											}

										}
									}
								}
								else if (animals[animalIndex].body[cellIndex].organ == ORGAN_MOUTH_PARASITE )
								{
									float amount = (animals[world[cellWorldPositionI].identity].energy) / animalSquareSize;
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
						world[cellWorldPositionI].identity = animalIndex;
						world[cellWorldPositionI].occupyingCell = cellIndex;
						if (trailUpdate)
						{

							world[cellWorldPositionI].trail    = dAngle;
						}
						animals[animalIndex].body[cellIndex].worldPositionI = cellWorldPositionI;

						if (animalIndex == adversary)
						{
							if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
							{
								world[cellWorldPositionI].material = MATERIAL_GRASS;

							}
						}

					}
				}
			}
			else
			{
				animals[animalIndex].position = 0;
			}
		}
	}
}

void energy_all() // perform energies.
{
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
			if (animalIndex == playerCreature )
			{
				if (animals[animalIndex].damageReceived > animals[animalIndex].mass) // player can only be killed by MURDER
				{
					// printf("A machine or player was harmed until death! dmg %u mass %u\n", animals[animalIndex].damageReceived, animals[animalIndex].mass);
					execute = true;
				}
			}
			else
			{
				if (!immortality && !animals[animalIndex].isMachine && animalIndex) // reasons an npc can die
				{
					if (speciesPopulationCounts[speciesIndex] > (( numberOfAnimals / numberOfSpecies) / 4) ) // only kill off weak animals if there is some population.
						if (animals[animalIndex].energy < 0.0f)
						{
							execute = true;
							// printf("died of low energy!\n");
						}
					if (animals[animalIndex].age > animals[animalIndex].lifespan)
					{
						// printf("died of old age!\n");
						execute = true;
					}
					if (animals[animalIndex].totalGonads == 0)
					{
						// printf("genitals exploded and died!\n");
						execute = true;
					}
					if (animals[animalIndex].damageReceived > animals[animalIndex].mass)
					{
						// printf("murdered to death (or drowned)!\n");
						execute = true;
					}
					if (animals[animalIndex].mass <= 0)
					{
						// printf("banished for being massless!\n");
						execute = true;
					}
				}
			}
			if (execute)
			{
				// printf("execute animal %u \n", animalIndex);
				// ;

				if (animalIndex == adversary && adversary >= 0)
				{
					unsigned int adversaryPos = animals[adversary].position;
					setupExampleAnimal2(adversary);
					spawnAnimalIntoSlot( adversary, animals[adversary], adversaryPos, true  )  ;

				}
				else
				{
					killAnimal( animalIndex);
				}

			}
			if (tournament)
			{
				int animalScore = animals[animalIndex].damageDone + animals[animalIndex].damageReceived  + animals[animalIndex].numberOfTimesReproduced ;
				if ( animalScore > championScore)
				{
					championScore = animalScore;
					champion = animals[animalIndex];
				}
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




void selectCursorAnimal()
{

	if (selectedAnimal >= 0)
	{
		selectedAnimal = -1;
	}
	else {

		int cursorPosX = cameraPositionX +  mousePositionX ;
		int cursorPosY = cameraPositionY + mousePositionY;
		unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
		if (worldCursorPos < worldSquareSize)
		{
			int tempCursorAnimal = world[worldCursorPos].identity;
			unsigned int cursorAnimalSpecies = tempCursorAnimal / numberOfAnimalsPerSpecies;
			if (tempCursorAnimal >= 0 && tempCursorAnimal < numberOfAnimals)
			{
				cursorAnimal = tempCursorAnimal;
				int occupyingCell = isAnimalInSquare(cursorAnimal, worldCursorPos);
				if ( occupyingCell >= 0)
				{
					selectedAnimal = cursorAnimal;
				}
			}
		}
	}
}



void viewAdversary()
{
	if (adversary >= 0 && playerCreature >= 0)
	{
		if (cameraTargetCreature == playerCreature)
		{
			cameraTargetCreature = adversary;
		}
		else
		{
			cameraTargetCreature = playerCreature;
		}
	}
}


void camera()
{



	if (cameraTargetCreature >= 0)
	{
		cameraPositionX = animals[cameraTargetCreature].position % worldSize;
		cameraPositionY = animals[cameraTargetCreature].position / worldSize;
	}

	// if the player doesn't have any eyes, don't draw anything!

	if (playerCreature >= 0 && cameraTargetCreature == playerCreature && playerInControl)
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

					case VISUALIZER_IDENTITY:
					{

						// displayColor = whatColorIsThisSquare(worldI);
						if (world[worldI].identity < numberOfAnimals && world[worldI].identity >= 0)
						{
							displayColor = animals[ world[worldI].identity ].identityColor;
						}

						drawTile( Vec_f2( fx, fy ), displayColor);
						break;
					}

					case VISUALIZER_TRACKS:
					{

						// displayColor = whatColorIsThisSquare(worldI);
						if (world[worldI].identity < numberOfAnimals && world[worldI].identity >= 0)
						{
							displayColor = animals[ world[worldI].identity ].identityColor;
							drawPointerTriangle( Vec_f2( fx, fy ), displayColor, world[worldI].trail );
						}
						break;
					}

					case VISUALIZER_NEURALACTIVITY:
					{

						// displayColor = whatColorIsThisSquare(worldI);
						if (world[worldI].identity < numberOfAnimals && world[worldI].identity >= 0)
						{
							displayColor = color_grey;//animals[ world[worldI].identity ].identityColor;
							// drawPointerTriangle( Vec_f2( fx, fy ), displayColor, world[worldI].trail );

							// if (isCellConnectable(animals[viewedAnimal].body[occupyingCell].organ ) )
							// {

							int occupyingCell = isAnimalInSquare(world[worldI].identity, worldI);
							if ( occupyingCell >= 0)
							{


								float amount = animals[world[worldI].identity].body[occupyingCell].signalIntensity ;//* 2.0f;
								displayColor.r *= amount ;
								displayColor.g *= amount;
								displayColor.b *= amount;
							}

							drawTile( Vec_f2( fx, fy ), displayColor);
						}
						break;
					}










					}

				}
			}
		}
	}





	// draw the cursor.
	Color displayColor = color_white;
	Vec_f2 worldMousePos = Vec_f2( fmousePositionX, fmousePositionY);
	drawTile( worldMousePos, displayColor);





}





void displayComputerText()
{

	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;




	if (computer1display)
	{


		// printText2D(  "computer1display\n" , menuX, menuY, textSize);
		// menuY -= spacing;

		for (int i = 0; i < numberOfSpecies; ++i)
		{

			printText2D(   std::string("Species ") + std::to_string(i) +   std::string(" pop. " + std::to_string(speciesPopulationCounts[i])) + " hits " + std::to_string(speciesAttacksPerTurn[i]) , menuX, menuY, textSize);
			menuY -= spacing;
		}
		menuY -= spacing;


	}



	if (computer2display)
	{


		/**
		Cast of characters.

		Fred Hoover. Project manager.

		Melissa Kelly. Astrobiologist.

		General Bill "Bullseye" Strickland.

		You. Psychic cadet.	 


		Computer. A Dell Inspiron 7500 from 1999. Intel pentium III capable of 750MHz. 512MB RAM, 30GB storage.




		story progression in 5 terminals.

		1. intro, speak to the general
		2. the adversary
		3. alien computer
		4.
		5.




		 **/






		/**
	

Project CONQUEST. Classification: Above top secret.
Private conversation between M. Kelly and F. Hoover.

	How did the meeting go?

	Good. Well, bad I guess. It all went sideways pretty quickly.

	What happened?

	I thought we'd be talking to some pencil pushers. We must have impressed someone up there though. They sent General Strickland to review our progress.

	Geez. We're not ready for that.

	Yeah. I wasn't ready. But I delivered our report and our findings.

	What did they say?

	He immediately tripled our budget. And granted our request for more personnel. And he's going to buy us a new computer.

	****! What did YOU say?
	
	Um, it was more about how I said it. See he's over in Virginia, so I spoke to them on the video telephone...

	... He thought you were in the computer.

	He thought I was in the computer. I should have said something, but I didn't realize until the call was finished.




		 * */



/**
 *



	Strickland's like a hundred years old, he doesn't know anything about computers. 

	That's the thing, you could see his eyes glaze over when I talk about virtual ecology. But as soon as I said 'adversary', he switched right on.

	You shouldn't have called it that. It's just a pet name.

	It just slipped out. I'm sorry. He fixated on it and I couldn't take it back.

	We don't even know what it is yet.

	Just the idea of having some opponent made him so excited. I guess he spent all day meeting with the other projects.. Psychic projection... Astrobiology..

	Hmf.

	Sorry Melissa. It's just that having anything to show at all kind of means we're the poster child. 

	What does he want us to do?

	Well, he wants us to occupy the simulation, like it's a pacific island territory. We're supposed to set up a forward operating base and have a constant presence there.

	Haha! Really?

	Yeah. And keep doing our research. I'm supposed to train the cadets how to live and work in the sim. They figured out pretty quick it's basically a video game. They're all pretty young, but they're bright.

	What about the adversary?

	The cadet's mission is to find it, and kill it...




 * 
 * */



/**
 *

Melissa, the new computer came in today.

Tell me!

Oh I'll tell you. But you won't believe me.

Try it.

Seven fifty megahertz.

... Seven and a half megahertz?

Nope. Seven hundred and fifty megahertz. 

No way. There's no way.

Half a BILLION bytes of RAM.

It must be the size of a house!

That's the thing, you could pick it up in your hand...

How?

It's only as big as a lunch box I guess. I've never seen anything like it. It unfolds in half and there's a screen and a keyboard inside.
We're not allowed to touch it. But the techs have wired in our PCs so we can work on it remotely. We're going to move the simulation code over to it.

Gosh. If the simulation ran on something like that... We could make a whole country, we could do anything.

I haven't even told you the craziest part yet. The system clock is set to December 2, 1999.

What.

The rumor is, NRO met with an extraterrestrial entity, and traded for it. A human computer from our own future.

Wait, aliens are really real? Don't you hang up on- !!

As I end the call to Melissa, I can't help but wonder. What price did the NRO pay for this machine?



**/




/***
 * 
 


Project CONQUEST. Classification: Above top secret.
Private conversation between F. Hoover and cadet R. Bienvenida.

We don't know anything about it, really. We think it's an enemy, at least we've been told to treat it that way.

Has it taken any offensive action, against us?

Yeah. Sort of. It attacked and killed one of the researchers, and hurt another. Well they were fine in the real world I guess but it still counts.

Is that all?

No. It has a corrupting influence on the simulation. Whatever it touches starts growing and mutating. Over time its creations 












 * 
 * */


/***
 * 
 
 Secret diary of Melissa Kelly, Astrobiologist.






*/








		printText2D(  "    \n" , menuX, menuY, textSize);
		menuY -= spacing;


	}


}




// void checkPalette(vec_i2 input)
// {


// }


void drawGameInterfaceText()
{

	int menuX = 50;
	int menuY = 50;
	int textSize = 10;
	int spacing = 20;


	// printText2D(   std::string("FPS ") + std::to_string(fps ) , menuX, menuY, textSize);
	// menuY += spacing;


	// printText2D(   std::string("Player ") + std::to_string(playerCreature) , menuX, menuY, textSize);
	// menuY -= spacing;

	// if (playerCreature >= 0)
	// {
	// 	printText2D(   std::string("Energy ") + std::to_string(animals[playerCreature].energy ) , menuX, menuY, textSize);
	// 	menuY -= spacing;

	// 	printText2D(   std::string("Energy debt ") + std::to_string(animals[playerCreature].energyDebt ) , menuX, menuY, textSize);
	// 	menuY -= spacing;


	// 	printText2D(   std::string("Max energy ") + std::to_string(animals[playerCreature].maxEnergy ) , menuX, menuY, textSize);
	// 	menuY -= spacing;

	// 	printText2D(   std::string("Offspring energy ") + std::to_string(animals[playerCreature].offspringEnergy ) , menuX, menuY, textSize);
	// 	menuY -= spacing;

	// 	printText2D(   std::string("Reproduces at ") + std::to_string( ((animals[playerCreature].maxEnergy / 2) + (animals[playerCreature].offspringEnergy )) ) , menuX, menuY, textSize);
	// 	menuY -= spacing;
	// }
	// menuY -= spacing;


	// printText2D(   std::string("Zoom ") + std::to_string(viewZoom ) , menuX, menuY, textSize);
	// menuY -= spacing;


	// printText2D(   std::string("Mouse X ") + std::to_string(mousePositionX ) + std::string(" Y ") + std::to_string(mousePositionY) , menuX, menuY, textSize);
	// menuY -= spacing;


	int cursorPosX = cameraPositionX +  mousePositionX ;
	int cursorPosY = cameraPositionY + mousePositionY;
	unsigned int worldCursorPos = (cursorPosY * worldSize) + cursorPosX;
	if (worldCursorPos < worldSquareSize)
	{
		// int tempCursorAnimal = world[worldCursorPos].identity;
		cursorAnimal = world[worldCursorPos].identity;
		bool animalInSquare = false;
		if (cursorAnimal >= 0 && cursorAnimal < numberOfAnimals)
		{
			unsigned int cursorAnimalSpecies = cursorAnimal / numberOfAnimalsPerSpecies;
			// cursorAnimal = tempCursorAnimal;
			int occupyingCell = isAnimalInSquare(cursorAnimal, worldCursorPos);
			if ( occupyingCell >= 0)
			{

				if (cursorAnimalSpecies == 0)
				{
					if (cursorAnimal == playerCreature)
					{
						printText2D(   std::string("This is you."), menuX, menuY, textSize);
						menuY += spacing;
					}
					else
					{

						// printf(" eeeee %s \n", animals[cursorAnimal].displayName);
						printText2D(   std::string(animals[cursorAnimal].displayName) , menuX, menuY, textSize);
						menuY += spacing;

					}



				}



				else
				{
					printText2D(   std::string("An animal of species ") + std::to_string(cursorAnimalSpecies ) , menuX, menuY, textSize);
					menuY += spacing;



				}

				// describe the organ.
				printText2D(  tileDescriptions(  animals[  cursorAnimal].body[occupyingCell].organ ), menuX, menuY, textSize);
				menuY += spacing;
				animalInSquare = true;

			}
		}


		if (!animalInSquare)
		{

			if (world[worldCursorPos].material != MATERIAL_NOTHING)
			{
				// printText2D(   std::string("Material ") + std::to_string(world[worldCursorPos].material ) , menuX, menuY, textSize);
				// menuY -= spacing;
				// describe the material.
				printText2D(  tileDescriptions(world[worldCursorPos].material ), menuX, menuY, textSize);
				menuY += spacing;

			}
			else
			{
				// printText2D(   std::string("Terrain ") + std::to_string(world[worldCursorPos].terrain ) , menuX, menuY, textSize);
				// menuY -= spacing;
				// describe the terrain
				printText2D(  tileDescriptions (world[worldCursorPos].terrain ), menuX, menuY, textSize);
				menuY += spacing;
			}


		}





	}


	if (playerCreature >= 0)
	{
		// if the player has a nose, print what it smells like here.
		int playerPheromoneSensor = getRandomCellOfType( playerCreature, ORGAN_SENSOR_PHEROMONE ) ;
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


		// if the player is blind, say so!
		if (!playerCanSee)
		{
			printText2D(   std::string("You can't see anything. ") , menuX, menuY, textSize);
			menuY += spacing;
		}


		// print grabber states

		for (int i = 0; i < animals[playerCreature].cellsUsed; ++i)
		{
			if (animals[playerCreature].body[i].organ == ORGAN_GRABBER)
			{
				if (animals[playerCreature].body[i].grabbedCreature >= 0)
				{
					printText2D(   std::string("Holding ") + animals[  animals[playerCreature].body[i].grabbedCreature ].displayName , menuX, menuY, textSize);
					menuY += spacing;
				}

			}
		}
	}
// if (world[worldCursorPos])


	displayComputerText();




	// int menuX = 50;
	// int menuY = 50;
	// int textSize = 10;
	// int spacing = 20;

	menuY += spacing;
	for (int i = 0; i < 8; ++i)
	{
		printText2D(   logs[i] , menuX, menuY, textSize);
		menuY += spacing;


	}
	menuY += spacing;

	printText2D(   std::string("FPS ") + std::to_string(fps ) , menuX, menuY, textSize);
	menuY += spacing;






	// draw edit palette
	if (palette)
	{
		drawPalette();
	}


}








void setupExampleHuman(int i)
{


	resetAnimal(i);

	// std::string("A human.").c_str().cop


	// snprintf (animals[i].displayName, 32, "A human.");

	// strcpy( &animals[i].displayName[0] , std::string("A human.").c_str() );


	std::string gunDescription = std::string("A human.");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );

	appendCell( i, ORGAN_BONE, Vec_i2(0, 1 ));


	appendCell( i, ORGAN_SENSOR_EYE, Vec_i2(-1, 0) );
	appendCell( i, ORGAN_BONE, Vec_i2(0, 0) );
	appendCell( i, ORGAN_SENSOR_EYE, Vec_i2(1, 0) );



	appendCell( i, ORGAN_SENSOR_EAR, Vec_i2(-2, -1) );
	appendCell( i, ORGAN_BONE, Vec_i2(-1, -1) );
	appendCell( i, ORGAN_SENSOR_PHEROMONE, Vec_i2(0, -1) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -1) );
	appendCell( i, ORGAN_SENSOR_EAR, Vec_i2(2, -1) );



	appendCell( i, ORGAN_BONE , Vec_i2(-1, -2));
	appendCell( i, ORGAN_SPEAKER , Vec_i2(0, -2));
	appendCell( i, ORGAN_BONE , Vec_i2(1, -2));


	appendCell( i, ORGAN_BONE, Vec_i2(0, -3) );

	appendCell( i, ORGAN_BONE, Vec_i2(-2, -4) );
	appendCell( i, ORGAN_BONE, Vec_i2(-1, -4) );
	appendCell( i, ORGAN_BONE, Vec_i2(0, -4) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -4) );
	appendCell( i, ORGAN_BONE, Vec_i2(2, -4) );


	appendCell( i, ORGAN_MUSCLE, Vec_i2(-3, -5) );
	appendCell( i, ORGAN_BONE, Vec_i2(-1, -5) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, -5) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -5) );
	appendCell( i, ORGAN_MUSCLE, Vec_i2(3, -5) );



	appendCell( i, ORGAN_MUSCLE, Vec_i2(-3, -6) );
	appendCell( i, ORGAN_BONE, Vec_i2(-1, -6) );
	appendCell( i, ORGAN_LUNG, Vec_i2(0, -6) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -6) );
	appendCell( i, ORGAN_MUSCLE, Vec_i2(3, -6) );




	appendCell( i, ORGAN_BONE, Vec_i2(-3, -7) );
	appendCell( i, ORGAN_LIVER, Vec_i2(1, -7) );
	appendCell( i, ORGAN_LIVER, Vec_i2(0, -7) );
	appendCell( i, ORGAN_LIVER, Vec_i2(1, -7) );
	appendCell( i, ORGAN_BONE, Vec_i2(3, -7) );



	appendCell( i, ORGAN_GRABBER, Vec_i2(-3, -8) );
	appendCell( i, ORGAN_MUSCLE, Vec_i2(-1, -8) );
	appendCell( i, ORGAN_GONAD, Vec_i2(0, -8) );
	appendCell( i, ORGAN_MUSCLE, Vec_i2(1, -8) );
	appendCell( i, ORGAN_GRABBER, Vec_i2(3, -8) );



	appendCell( i, ORGAN_MUSCLE, Vec_i2(-1, -9 ));
	appendCell( i, ORGAN_MUSCLE, Vec_i2(1, -9) );


	appendCell( i, ORGAN_MUSCLE_STRAFE, Vec_i2(-1, -10) );
	appendCell( i, ORGAN_MUSCLE_STRAFE, Vec_i2(1, -10) );


	appendCell( i, ORGAN_BONE, Vec_i2(-1, -11) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -11) );


	appendCell( i, ORGAN_BONE, Vec_i2(-1, -12) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -12) );


	appendCell( i, ORGAN_BONE, Vec_i2(-1, -13) );
	appendCell( i, ORGAN_BONE, Vec_i2(1, -13) );


}





void ecologyComputerCallback( int gunIndex, int shooterIndex)
{
	computer1display = !computer1display;
}

void communicationComputerCallback( int gunIndex, int shooterIndex)
{
	computer2display = !computer2display;
}



void hospitalCallback( int gunIndex, int shooterIndex)
{
	// computer1display = !computer1display;
	palette = !palette;
}




void exampleGunCallback( int gunIndex, int shooterIndex)
{

	if (gunIndex >= 0)
	{



		printf(" you hear a gunshot! \n");


		// trace a line from the gun and destroy any tissue found on the way.
		unsigned int range = 1000;

		float bulletPosX = animals[gunIndex].fPosX;
		float bulletPosY = animals[gunIndex].fPosY;
		float angle      =  animals[gunIndex].fAngle;

		for (int i = 0; i < range; ++i)
		{

			bulletPosX += 1.0f * (cos(angle));
			bulletPosY += 1.0f * (sin(angle));
			unsigned int ubulletPosX = bulletPosX;
			unsigned int ubulletPosY = bulletPosY;

			unsigned int shootWorldPosition = (ubulletPosY * worldSize) + ubulletPosX;

			if (world[shootWorldPosition].identity >= 0 && world[shootWorldPosition].identity != gunIndex && world[shootWorldPosition].identity < numberOfAnimals

			        && world[shootWorldPosition].identity != shooterIndex
			   )
			{
				unsigned int shotOffNub = isAnimalInSquare(world[shootWorldPosition].identity, shootWorldPosition);
				if (shotOffNub >= 0 && shotOffNub < animalSquareSize)
				{

					// eliminateCell(world[shootWorldPosition].identity, )
					animals[world[shootWorldPosition].identity].body[shotOffNub] .damage += 0.5 + RNG();
				}

			}



			if (world[shootWorldPosition].wall == MATERIAL_NOTHING )
			{
				world[shootWorldPosition].wall = MATERIAL_SMOKE;
			}

			if ( materialBlocksMovement( world[shootWorldPosition].wall)
			   )
			{
				world[shootWorldPosition].wall = MATERIAL_NOTHING;
				break;
			}

		}
	}
}


void trackerGlassesCallback( int gunIndex, int shooterIndex)
{
	// printf("example glasses callback\n");
	if (visualizer == VISUALIZER_TRUECOLOR)
	{
		visualizer = VISUALIZER_TRACKS;
	}
	else
	{
		visualizer = VISUALIZER_TRUECOLOR;
	}

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

	animals[i].machineCallback = trackerGlassesCallback;


	std::string gunDescription = std::string("A pair of tracker glasses.");
	strcpy( &(animals[i].displayName[0]) , gunDescription.c_str() );


}


void setupExampleGun(int i)
{

	resetAnimal(i);
	animals[i].isMachine = true;
	animals[i].machineCallback = exampleGunCallback;

	// animals[i].displayName = std::string("A pistol.").c_str();

	// snprintf (animals[i].displayName, 32, "A pistol.");

	std::string gunDescription = std::string("A pistol.");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );

	appendCell( i, MATERIAL_METAL, Vec_i2(-1, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(1, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(2, 1) );
	appendCell( i, MATERIAL_METAL, Vec_i2(0, 0) );
	appendCell( i, MATERIAL_METAL, Vec_i2(-1, -1) );



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

	// animals[i].displayName = std::string("A computer terminal.").c_str();

	// snprintf (animals[i].displayName, 32, "A computer terminal.");

}



void setupEcologyCompter(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("A ecology terminal.");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
	animals[i].machineCallback = ecologyComputerCallback;
}


void setupMessageComputer(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("A communication terminal.");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
	animals[i].machineCallback = communicationComputerCallback;
}

void setupHospitalComputer(int i)
{
	setupExampleComputer(i);
	std::string gunDescription = std::string("A hospital.");
	strcpy( &animals[i].displayName[0] , gunDescription.c_str() );
	animals[i].machineCallback = hospitalCallback;
}


void setupBuilding_playerBase(unsigned int worldPositionI)
{


	unsigned int worldPositionX = worldPositionI % worldSize;
	unsigned int worldPositionY = worldPositionI / worldSize;

	int baseSize = 100;
	int wallThickness = 8;
	int doorThickness = 16;


	for (unsigned int i = 0; i < worldSquareSize; ++i)
	{
		int x = i % worldSize;
		int y = i / worldSize;




		int xdiff = x - worldPositionX;
		int ydiff = y - worldPositionY;




		// set all the tiles around the position to a floor tile
		if (abs(xdiff) < baseSize && abs(ydiff) < baseSize)
		{
			world[i].terrain = MATERIAL_VOIDMETAL;
			world[i].material = MATERIAL_NOTHING;
			world[i].wall = MATERIAL_NOTHING;
		}


		if (abs(xdiff) < baseSize * 1.5 && abs(ydiff) < baseSize * 1.5)
		{

			world[i].material = MATERIAL_NOTHING;
		}


		// make walls around it

		if (

		    // a square border of certain thickness
		    (((x > worldPositionX - baseSize - wallThickness) && (x < worldPositionX - baseSize + wallThickness) ) ||
		     ((x > worldPositionX + baseSize - wallThickness) && (x < worldPositionX + baseSize + wallThickness) ) ||
		     ((y > worldPositionY - baseSize - wallThickness) && (y < worldPositionY - baseSize + wallThickness) ) ||
		     ((y > worldPositionY + baseSize - wallThickness) && (y < worldPositionY + baseSize + wallThickness) ) )

		    &&

		    (abs(xdiff) < (baseSize + wallThickness) &&
		     abs(ydiff) < (baseSize + wallThickness))

		    &&

		    // with doors in the middle of each wall
		    ((abs(xdiff) > doorThickness) &&
		     (abs(ydiff) > doorThickness) )

		)
		{
			world[i].wall = MATERIAL_VOIDMETAL;

		}




	}


	cameraPositionX  = worldPositionX;
	cameraPositionY = worldPositionY;


	// add equipment.



}

void playerGrab()
{
	if (playerCreature >= 0)
	{
		// playerGrabState = !playerGrabState;
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
		// playerGrabState = !playerGrabState;
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
		// animals[playerCreature].fPosX += pos.x;
		// animals[playerCreature].fPosY += pos.y;
		animals[playerCreature].fAngle = 0.0f;
// getRandomCellOfType(unsigned int animalIndex, unsigned int organType)
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


void spawnAdversary()
{

// printf("setting up animal %i\n", i);
	unsigned int targetWorldPositionI = extremelyFastNumberFromZeroTo(worldSquareSize - 1); //( targetWorldPositionY * worldSize ) + targetWorldPositionX;
	int j = 1;


	adversary = numberOfAnimalsPerSpecies + 1; // adversary animal is a low number index in the 1th species. 0th is for players and machines.

	setupExampleAnimal2(j);


	animals[adversary].position = targetWorldPositionI;
	animals[adversary].uPosX = targetWorldPositionI % worldSize;
	animals[adversary].uPosY = targetWorldPositionI / worldSize;
	animals[adversary].fPosX = animals[adversary].uPosX;
	animals[adversary].fPosY = animals[adversary].uPosY;



	// loadParticlarAnimal(j, std::string("save/macrolongus_smigmanosa"));
	spawnAnimalIntoSlot(adversary,
	                    animals[j],
	                    targetWorldPositionI, true);



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
		unsigned int targetWorldPositionX = cameraPositionX ;
		unsigned int targetWorldPositionY = cameraPositionY ;

		fmousePositionX = cameraPositionX;
		fmousePositionY = cameraPositionY;
		mousePositionX = cameraPositionX;
		mousePositionY = cameraPositionY;

		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		int i = 1;
		setupExampleHuman(i);

		playerCreature = 0;
		spawnAnimalIntoSlot(playerCreature,
		                    animals[i],
		                    targetWorldPositionI, false);

		cameraTargetCreature = playerCreature;

		printf("spawned player creature\n");


		appendLog( std::string("Spawned the player.") );
	}
	else
	{
		killAnimal(playerCreature);
		printf("suicided player creature\n");
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
		// animals in the tournament are not in the 0th species, which is for players and machines.
		for (int i = (1 * numberOfAnimalsPerSpecies); i < numberOfAnimals; ++i)
		{
			// printf("setting up animal %i\n", i);
			unsigned int targetWorldPositionI = animals[adversary].position;//extremelyFastNumberFromZeroTo(worldSquareSize) - 1; //( targetWorldPositionY * worldSize ) + targetWorldPositionX;
			int j = 1;
			// setupExampleAnimal2(j);


			loadParticlarAnimal(j, std::string("save/macrolongus_smigmanosa"));

			spawnAnimalIntoSlot(i,
			                    animals[j],
			                    targetWorldPositionI, true);
		}
	}
}

void setupRandomWorld()
{
	resetAnimals();
	resetGrid();

	// spawn the example creature in the center field of view in an empty world.
	if (worldToLoad == WORLD_EXAMPLECREATURE)
	{
		unsigned int wallthickness = 8;
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{

			world[worldPositionI].temperature = 300.0f;
			world[worldPositionI].light = color_white;

			world[worldPositionI].terrain = MATERIAL_ROCK;
			// world[worldPositionI].material = MATERIAL_GRASS;

			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;
			// walls around the world edge
			if (x < wallthickness || x > worldSize - wallthickness || y < wallthickness  || y > worldSize - wallthickness)
			{
				world[worldPositionI].material = MATERIAL_ROCK;
			}
		}

		// rock
		for (int i = 0; i < 100; ++i)
		{
			unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
			unsigned int x = randompos % worldSize;
			unsigned int y = randompos / worldSize;
			int rocksize = 150;
			for (int j = 0; j < rocksize; ++j)
			{
				for (int k = 0; k < rocksize; ++k)
				{
					unsigned int square = ( (y + j) * worldSize ) + ( x + k );
					world[square].wall = MATERIAL_ROCK;
				}
			}
		}

		// lava
		// if (useLava)
		// {
		// 	for (int i = 0; i < 35; ++i)
		// 	{
		// 		unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
		// 		unsigned int x = randompos % worldSize;
		// 		unsigned int y = randompos / worldSize;
		// 		int rocksize = 250;
		// 		for (int j = 0; j < rocksize; ++j)
		// 		{
		// 			for (int k = 0; k < rocksize; ++k)
		// 			{
		// 				unsigned int square = ( (y + j) * worldSize ) + ( x + k );
		// 				world[square].terrain = TERRAIN_LAVA;
		// 			}
		// 		}
		// 	}
		// }

		// water
		unsigned int x = worldSize / 2 ;
		unsigned int y = worldSize / 2 ;
		int rocksize = 2500;
		for (int j = 0; j < rocksize; ++j)
		{
			for (int k = 0; k < rocksize; ++k)
			{
				unsigned int square = ( (y + j) * worldSize ) + ( x + k );
				world[square].terrain = MATERIAL_WATER;
			}
		}


		// items



		unsigned int targetWorldPositionX = 200 ;
		unsigned int targetWorldPositionY = 200 ;
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;

		setupBuilding_playerBase(targetWorldPositionI);

		int i = 1;
		setupExampleGun(i);
		spawnAnimalIntoSlot(2,
		                    animals[i],
		                    targetWorldPositionI, false);




		targetWorldPositionI += (400);
		// targetWorldPositionX = 200 ;
		// targetWorldPositionY = 300 ;
		// targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;

		setupBuilding_playerBase(targetWorldPositionI);

		// int i = 1;
		// setupExampleComputer(i);


		setupEcologyCompter( i);
		spawnAnimalIntoSlot(3,
		                    animals[i],
		                    targetWorldPositionI, false);

		animals[3].fAngle = 0.0f;







		targetWorldPositionI += (400);
		// targetWorldPositionX = 200 ;
		// targetWorldPositionY = 300 ;
		// targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;

		setupBuilding_playerBase(targetWorldPositionI);

		// int i = 1;
		// setupExampleGlasses(i);
		setupTrackerGlasses(i);
		spawnAnimalIntoSlot(4,
		                    animals[i],
		                    targetWorldPositionI, false);

		// animals[3].fAngle = 0.0f;





	targetWorldPositionI += (400);
		// targetWorldPositionX = 200 ;
		// targetWorldPositionY = 300 ;
		// targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;

		setupBuilding_playerBase(targetWorldPositionI);

		// int i = 1;
		// setupExampleGlasses(i);
		// setupTrackerGlasses(i)

	 setupHospitalComputer(i);
		spawnAnimalIntoSlot(5,
		                    animals[i],
		                    targetWorldPositionI, false);

		// animals[3].fAngle = 0.0f;








		// spawn the player
		spawnPlayer();



		// spawn the adversary
		spawnAdversary();




	}
}

void tournamentController()
{
	if (tournamentCounter >= tournamentInterval )
	{
		tournamentCounter = 0;
	}
	else
	{
		tournamentCounter++;
	}

	if (respawnLowSpecies)
	{
		unsigned int totalpop = 0;
		for (unsigned int speciesIndex = 1; speciesIndex < numberOfSpecies; speciesIndex++) // start at 1 to ignore the non-natural species 0.
		{
			totalpop += speciesPopulationCounts[speciesIndex] ;
			if (speciesPopulationCounts[speciesIndex] == 0)
			{
				// printf("a species is empty\n");
				// unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
				// unsigned int randomSpeciesSlot = (i * numberOfAnimalsPerSpecies) + (extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies - 1));

				// if there is another species who is successful, duplicate an animal from them.
				int foundAnimal = -1;
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
					// printf("repopulated endangered species %i from species %i animal %u\n", ispeciesindex, foundSpecies, foundAnimal );
					// int animalIndex = spawnAnimal(i, animals[foundAnimal], randompos, false);

					// int animalIndex = getNewIdentity(i);
					// // if (animalIndex >= 0) // an animalIndex was available
					// // {

					// if (animalIndex >= 0 )
					// {
					// 	// animals[animalIndex].energy = animals[animalIndex].maxEnergy;

					// 	// just swap the animal between species without disturbing it.
					// 	animals[animalIndex] = animals[foundAnimal];
					// 	animals[foundAnimal].retired = true;
					// }



					// int spawnAnimal( unsigned int speciesIndex,
					//                  Animal parent,
					//                  unsigned int position, bool mutation)

					memcpy(    &animals[ (speciesIndex * numberOfAnimalsPerSpecies) ] , &animals[ foundAnimal ], sizeof(Animal)    );

					resetAnimal(foundAnimal);


					// int newAnimal = spawnAnimal(speciesIndex, animals[foundAnimal], animals[foundAnimal].position, false);
					// if (newAnimal >= 0)
					// {
					// 	// 	animals[newAnimal].energy = animals[newAnimal].maxEnergy;
					// 	// }


					// 	// animals[animalIndex].body[cellIndex].organ = MATERIAL_NOTHING;
					// 	// animals[animalIndex].numberOfTimesReproduced++;
					// 	// animals[animalIndex].energy -= animals[animalIndex].offspringEnergy;
					// 	animals[newAnimal].energy       =animals[newAnimal].maxEnergy  ;//  animals[animalIndex].offspringEnergy;
					// 	animals[newAnimal].parentIdentity       = foundAnimal;
					// }

				}
			}
		}
		// printf("totalpop %u\n", totalpop);
		if (totalpop == 0 && adversary >= 0)
		{

			int j = 1;

			for (int k = j + 1; k < numberOfAnimalsPerSpecies; ++k)
			{
				// unsigned int targetWorldPositionI = ;//extremelyFastNumberFromZeroTo(worldSquareSize) - 1; //( targetWorldPositionY * worldSize ) + targetWorldPositionX;


				// if (championScore > 20)
				// {
				// 	// setupExampleAnimal2(j);
				// 	spawnAnimal( 1,
				// 	             champion,
				// 	             targetWorldPositionI, true);

				// }
				// else {
				setupExampleAnimal2(j);
				spawnAnimal( 1,
				             animals[j],
				             animals[adversary].position, true);

				// }




			}
		}
	}
}

// void sprinkleFood()
// {
// 	if (extremelyFastNumberFromZeroTo(100) == 0)
// 	{
// 		unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
// 		if (world[randompos].material == MATERIAL_NOTHING)
// 		{
// 			world[randompos].material = MATERIAL_FOOD;
// 		}
// 	}
// }

void model()
{
	auto start = std::chrono::steady_clock::now();

	if (!paused)
	{
		computeAllAnimalsOneTurn();
		updateMap();
		// sprinkleFood();
	}
	if (tournament)
	{
		tournamentController();
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

void startSimulation()
{

	// setupLogs();
	setupRandomWorld();
	boost::thread t7{ modelSupervisor };
}

void save()
{
	std::ofstream out6(std::string("save/world").c_str());
	out6.write( (char*)(world), sizeof(Square) *  worldSize);
	out6.close();

	std::ofstream out7(std::string("save/animals").c_str());
	out7.write( (char*)(animals), sizeof(Animal) *  numberOfAnimals);
	out7.close();
}

void load()
{
	std::ifstream in6(std::string("save/world").c_str());
	in6.read( (char *)(&(world)), sizeof(Square) *  worldSize);
	in6.close();

	std::ifstream in7(std::string("save/animals").c_str());
	in7.read( (char *)(&(animals)), sizeof(Animal) *  numberOfAnimals);
	in7.close();
}

