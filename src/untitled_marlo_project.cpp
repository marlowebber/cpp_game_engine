// Untitled Marlo Project.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cstring>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"


// #include <SDL2/SDL.h>
// #include <SDL2/SDL_audio.h>
// #include <iostream>
// #include <cmath>


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
#define ORGAN_MOUTH_VEG                1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH_SCAVENGE               2
#define ORGAN_GONAD               3
#define ORGAN_MUSCLE              4
#define ORGAN_BONE                5
#define ORGAN_WEAPON              6
#define ORGAN_LIVER               7
#define ORGAN_MUSCLE_TURN         8
#define ORGAN_SENSOR_EYE          9
#define ORGAN_SENSOR_TOUCH        10

#define ORGAN_MOUTH_CARNIVORE     10
#define ORGAN_MOUTH_PARASITE      11
#define ORGAN_ADDOFFSPRINGENERGY  12
#define ORGAN_ADDLIFESPAN         13
// #define ORGAN_ADDSTRIDE           14
#define ORGAN_NEURON              15
#define ORGAN_BIASNEURON          16    // can be thought of as ORGAN_SENSOR_CONSTANTVALUE
#define ORGAN_SENSOR_TIMER        17
#define ORGAN_SENSOR_BODYANGLE	  18
#define ORGAN_SENSOR_NOSE         19
#define ORGAN_SPEAKER             20
#define ORGAN_SENSOR_EAR          21
#define ORGAN_MUSCLE_STRAFE       22
#define ORGAN_SENSOR_PHEROMONE    23
#define ORGAN_EMITTER_PHEROMONE   24
#define ORGAN_MEMORY_RX           25
#define ORGAN_MEMORY_TX           26
#define ORGAN_GILL                27

#define numberOfOrganTypes        28 // the number limit of growable genes
#define MATERIAL_FOOD             32 //           
#define MATERIAL_ROCK             33 //    
#define MATERIAL_MEAT             34
#define MATERIAL_BONE             35
#define MATERIAL_BLOOD            36
#define MATERIAL_GRASS            37
// #define MATERIAL_SEAWEED          38


#define TERRAIN_STONE             50
// #define TERRAIN_GRASS             51
#define TERRAIN_WATER             52 //
// #define TERRAIN_SEAWEED           53
#define TERRAIN_LAVA              54

#define MARKER                    35 //      // numbers above 25 don't correspond to lower-case letters(0..25) so we don't use them in the gene code. But (26..31) are still compatible with our masking scheme.

#define CONDITION_GREATER         41
#define CONDITION_EQUAL           42
#define CONDITION_LESS            43

// #define WORLD_RANDOM 1
#define WORLD_EXAMPLECREATURE 2
// #define WORLD_ARENA 3


#define NUMBER_OF_CONNECTIONS 8

// const int ioMatrixSize = 32;

const bool brownianMotion        = false;
const bool immortality           = false;
const bool doReproduction        = true;
const bool doMuscles             = true;
const bool growingCostsEnergy    = true;
bool lockfps               = false;
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

int mousePositionX =  -430;
int mousePositionY =  330;

unsigned int worldToLoad = WORLD_EXAMPLECREATURE;

const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.

float fps = 1.0f;

const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const int animalSize     = 8;
const unsigned int animalSquareSize      = animalSize * animalSize;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfSpecies = 20;
unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);
const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.001f;        // a multiplier for how much it costs animals just to exist.
const float movementEnergyScale    = 0.01f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.9f;         // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float grassEnergy            = 0.3f;         // how much you get from eating a square of grass

const float neuralNoise = 0.1f;

const float liverStorage = 20.0f;
const unsigned int baseLifespan = 1000;
const float signalPropagationConstant = 0.1f;      // how strongly sensor organs compel the animal.
float energyScaleIn             = 1.0f;            // a multiplier for how much energy is gained from food and light.
float minimumEntropy = 0.1f;
const float musclePower = 40.0f;
const float thresholdOfBoredom = 0.1f;
bool playerInControl = false;
int playerCreature = -1;




// float state = 0;
// const int rate = 44100;
// float freq = 440;
// void audio_callback(void *userdata, Uint8 *_stream, int len) {
//   // Cast byte stream into 16-bit signed int stream
//   Sint16 *stream = (Sint16*) _stream;
//   len /= 2;
//   for (int i = 0; i < len; ++i) {
//     stream[i] = ((1<<15)-1) * sin(state);
//     state += 1./rate*2*M_PI*freq;
//     state = fmod(state, 2*M_PI);
//   }
// }



// void sdlToneGenerator()
// {
//   SDL_Init(SDL_INIT_AUDIO);

//   SDL_AudioSpec spec;
//   spec.freq = rate;
//   spec.format = AUDIO_S16SYS;
//   spec.channels = 1;
//   spec.samples = 1024;
//   spec.callback = audio_callback;
//   spec.userdata = nullptr;
//   SDL_AudioSpec obtainedSpec;
//   SDL_OpenAudio(&spec, &obtainedSpec);
//   SDL_PauseAudio(0);

//   float nfreq;
//   while (std::cin >> nfreq) {
//     // Lock the audio thread to safely subsitute the frequency
//     SDL_LockAudio();
//     freq = nfreq;
//     SDL_UnlockAudio();
//   }
//   SDL_CloseAudio();
// }


// #include <math.h>
// #include <SDL.h>
// #include <SDL_audio.h>

// const int AMPLITUDE = 28000;
// const int SAMPLE_RATE = 44100;

// void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes)
// {
// 	Sint16 *buffer = (Sint16*)raw_buffer;
// 	int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
// 	int &sample_nr(*(int*)user_data);

// 	for (int i = 0; i < length; i++, sample_nr++)
// 	{
// 		double time = (double)sample_nr / (double)SAMPLE_RATE;
// 		buffer[i] = (Sint16)(AMPLITUDE * sin(2.0f * M_PI * 441.0f * time)); // render 441 HZ sine wave
// 	}
// }



// void audio_callback(void* userdata, uint8_t* stream, int len)
// {
//     uint64_t* samples_played = (uint64_t*)userdata;
//     float* fstream = (float*)(stream);

//     static const float volume = 0.2;
//     static const float frequency = 200.0;

//     for(int sid = 0; sid < (len / 8); ++sid)
//     {
//         double time = (*samples_played + sid) / 44100.0;
//         double x = 2.0 * M_PI * time * frequency;
//         fstream[2 * sid + 0] = volume * sin(x); /* L */
//         fstream[2 * sid + 1] = volume * sin(x); /* R */
//     }

//     *samples_played += (len / 8);
// }


// void snone_lenerator()
// {













// //     return 0;
// // }









// }




const unsigned int numberOfSpeakerChannels = 256;
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
int cellNeighbourOffsets[] =
{
	- 1,
	- animalSize - 1,
	- animalSize ,
	- animalSize  + 1,
	+ 1,
	+animalSize + 1,
	+animalSize,
	+animalSize - 1
};

unsigned int cameraPositionX = 0 ;
unsigned int cameraPositionY = 0 ;
unsigned int modelFrameCount = 0;

int champion = -1;
int championScore = 0;
int tournamentInterval = 10000;
int tournamentCounter  = 0;

int cameraTargetCreature = -1;
unsigned int usPerFrame = 0;
unsigned int populationCount = 0;
unsigned int cameraFrameCount = 0;

struct Square
{
	unsigned int material;
	unsigned int terrain;
	int identity; // id of the last animal to cross the tile
	float trail;  // movement direction of the last animal to cross the tile
	int height;
	float light;
	float pheromoneIntensity;    //
	int pheromoneChannel; //
};

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
	// float sign;
	float signalIntensity;
	float timerFreq;
	float timerPhase;  // also used as memory state
	Color color;
	int speakerChannel;
	// Color eyeColor;
	// unsigned int eyeLook; // this is a positional offset indicating which square the eye is looking at. It is constant for a particular eye (i.e. one eye cannot look around unless the animal moves).

	int eyeLookX;
	int eyeLookY;
	float damage;
	Connection connections[NUMBER_OF_CONNECTIONS];
};

struct Animal
{
	Cell body[animalSquareSize];
	Cell genes[animalSquareSize];
	unsigned int mass;
	// int stride;
	unsigned int numberOfTimesReproduced;
	unsigned int damageDone;
	unsigned int damageReceived;
	unsigned int birthLocation;
	unsigned int age;
	int lifespan;
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
	// float dAngle;  // the direction of the most recent movement in global coords
	// float prevHighestIntensity;
	bool parentAmnesty;
	int totalMuscle;
	bool canBreatheUnderwater;
};


bool speciesVacancies [numberOfSpecies];
unsigned int speciesPopulationCounts [numberOfSpecies];
unsigned int populationCountUpdates  [numberOfSpecies];
unsigned int speciesAttacksPerTurn[numberOfSpecies];
// Animal exampleAnimal2;

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

// void scrambleConnections(unsigned int animalIndex)
// {
// 	if (animalIndex < numberOfAnimals)
// 	{
// 		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
// 		{
// 			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
// 			{
// 				animals[animalIndex].body[cellLocalPositionI].connections[i].used = false;
// 				if (RNG() < 0.5f) { animals[animalIndex].body[cellLocalPositionI].connections[i].used = true; }
// 				animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
// 				animals[animalIndex].body[cellLocalPositionI].connections[i].weight = (RNG() - 0.5f * 2.0f);
// 			}
// 		}
// 	}
// }

void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
			animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
			animals[animalIndex].body[cellLocalPositionI].color  = color_darkgrey;
			animals[animalIndex].body[cellLocalPositionI].damage = 0.0f;


			animals[animalIndex].body[cellLocalPositionI].eyeLookX = 0;
			animals[animalIndex].body[cellLocalPositionI].eyeLookY = 0;

			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;
				animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[cellLocalPositionI].connections[i].weight = RNG() - 0.5f;

			}


			animals[animalIndex].genes[cellLocalPositionI] = animals[animalIndex].body[cellLocalPositionI];
		}
		animals[animalIndex].mass = 0;
		// animals[animalIndex].stride = 1;
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
		// animals[animalIndex].prevHighestIntensity = 0.0f;
		animals[animalIndex].parentAmnesty = true;
		animals[animalIndex].retired = true;
		animals[animalIndex].canBreatheUnderwater = false;
	}
}

void resetAnimals()
{
	for ( int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		resetAnimal(animalIndex);
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
		world[i].height = 0;
		world[i].light = 1.0f;
		world[i].pheromoneIntensity = 0.0f;
		world[i].pheromoneChannel = 0;

		// unsigned int material;
		// unsigned int terrain;
		// int identity; // id of the last animal to cross the tile
		// float trail;  // movement direction of the last animal to cross the tile
		// int height;
		// float light;
		// float pheromoneIntensity;    //
		// int pheromoneChannel; //
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
	    organ == ORGAN_SENSOR_NOSE      ||
	    organ == ORGAN_SENSOR_EAR        ||
	    organ == ORGAN_SENSOR_PHEROMONE ||
	    organ == ORGAN_MEMORY_RX
	)
	{
		return true;
	}
	return false;
}


// // some genes have permanent effects, or effects that need to be known immediately at birth. Compute them here.
void measureAnimalQualities(unsigned int animalIndex)
{
	// update sensor ranges
	for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
	{
		unsigned int organ = animals[animalIndex].body[cellLocalPositionI].organ;
		if (    organIsASensor(organ))
		{
			unsigned int nSensors = 1; // starts from 1 coz the organ itself is a sensor
			for (unsigned int n = 0; n < nNeighbours; ++n)
			{
				unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
				if (cellNeighbour < animalSquareSize)
				{
					if (animals[animalIndex].body[cellNeighbour].organ  == organ)
					{
						nSensors++;
					}
				}
			}
		}
	}

	// update mass and debt
	animals[animalIndex].mass = 0;
	animals[animalIndex].energyDebt = 0.0f;
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].body[i].organ != MATERIAL_NOTHING)
		{
			animals[animalIndex].mass++;
			animals[animalIndex].energyDebt += 1.0f;
		}
	}

	// tally organ types if necessary
	animals[animalIndex].totalMuscle = 0;
	// animals[animalIndex].stride = 1;
	animals[animalIndex].offspringEnergy = 1.0f;
	animals[animalIndex].lifespan = baseLifespan;

	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].body[i].organ == ORGAN_MUSCLE ||
		        animals[animalIndex].body[i].organ == ORGAN_MUSCLE_TURN ||
		        animals[animalIndex].body[i].organ == ORGAN_MUSCLE_STRAFE

		   )
		{

			animals[animalIndex].totalMuscle ++;
		}
		// if (animals[animalIndex].body[i].organ == ORGAN_ADDSTRIDE)
		// {
		// 	animals[animalIndex].stride += animals[animalIndex].stride;
		// }

		if (animals[animalIndex].body[i].organ == ORGAN_ADDOFFSPRINGENERGY)
		{
			animals[animalIndex].offspringEnergy += animals[animalIndex].offspringEnergy ;
		}

		if (animals[animalIndex].body[i].organ == ORGAN_ADDLIFESPAN)
		{
			animals[animalIndex].lifespan += baseLifespan;
		}

		if (animals[animalIndex].body[i].organ == ORGAN_GILL)
		{
			animals[animalIndex].canBreatheUnderwater = true;
		}
	}
}

bool isCellConnectable(unsigned int organ)
{
	if (
	    // organ == ORGAN_NEURON ||
	    // organ == ORGAN_BIASNEURON ||
	    // organ == ORGAN_SENSOR_EYE ||
	    // organ == ORGAN_SENSOR_TIMER
	    organIsASensor(organ) ||
	    organIsANeuron(organ)
	)
	{
		return true;
	}

	return false;
}


bool isCellConnecting(unsigned int organ)
{
	if (
	    // organ == ORGAN_NEURON ||
	    // organ == ORGAN_BIASNEURON ||
	    // organ == ORGAN_SENSOR_EYE ||
	    // organ == ORGAN_SENSOR_TIMER
	    organIsAnActuator(organ) ||
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
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (isCellConnectable(  animals[animalIndex].genes[i].organ ))
		{
			cellsOfType.push_back(i);
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

// choose a random cell of any type that can put forth a connection, which includes all neurons and actuators.
int getRandomConnectingCell( unsigned int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (isCellConnecting(  animals[animalIndex].genes[i].organ ))
		{
			cellsOfType.push_back(i);
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
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].genes[i].organ == organType)
		{
			cellsOfType.push_back(i);
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
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].genes[i].organ != MATERIAL_NOTHING)
		{
			cellsOfType.push_back(i);
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



Color mutateColor(Color in)
{
	Color out = in;
	out.r += (RNG() - 0.5) * 0.1f;
	out.g += (RNG() - 0.5) * 0.1f;
	out.b += (RNG() - 0.5) * 0.1f;
	return clampColor(out);
}


void mutateAnimal(unsigned int animalIndex)
{
	if (!doMutation) {return;}

	if (
	    true
	    &&
	    animalIndex < numberOfAnimals
	    // extremelyFastNumberFromZeroTo(1) == 0
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

				animals[animalIndex].genes[mutantCell].organ = MATERIAL_NOTHING;// randomLetter();
			}
		}

		else if (whatToMutate == 1)
		{

			// add an organ
			int mutantCell = getRandomCellOfType(animalIndex, MATERIAL_NOTHING);
			if (mutantCell >= 0)
			{
				animals[animalIndex].genes[mutantCell].organ = randomLetter();
			}
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
				// unsigned int mutationAmount  = RNG() - 0.5;
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


			// swap an existing cell location without messing up the connections. adjust every incoming connection to point at the new cell.
			int mutantCell = getRandomPopulatedCell(animalIndex);
			if (mutantCell >= 0)
			{


				unsigned int destination =  getRandomCellOfType( animalIndex, MATERIAL_NOTHING );

				Cell temp = animals[animalIndex].genes[mutantCell];

				for (int i = 0; i < animalSquareSize; ++i)
				{
					for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
					{
						if (animals[animalIndex].genes[i].connections[j].connectedTo == mutantCell)
						{
							animals[animalIndex].genes[i].connections[j].connectedTo = destination;
						}
					}
				}

				if (destination < animalSquareSize)
				{
					animals[animalIndex].genes[destination] = temp;
				}

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
					animals[animalIndex].genes[mutantCellA].speakerChannel = mutantChannel; //mutateColor(	animals[animalIndex].body[mutantCell].color);
				}
				if (mutantCellB >= 0 && mutantCellB < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellB].speakerChannel = mutantChannel  ; //mutateColor(	animals[animalIndex].body[mutantCell].color);
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
					animals[animalIndex].genes[mutantCellA].speakerChannel = mutantChannel; //mutateColor(	animals[animalIndex].body[mutantCell].color);
				}
				if (mutantCellB >= 0 && mutantCellB < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellB].speakerChannel = mutantChannel  ; //mutateColor(	animals[animalIndex].body[mutantCell].color);
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
					animals[animalIndex].genes[mutantCellA].speakerChannel = mutantChannel; //mutateColor(	animals[animalIndex].body[mutantCell].color);
				}
				if (mutantCellB >= 0 && mutantCellB < animalSquareSize)
				{
					animals[animalIndex].genes[mutantCellB].speakerChannel = mutantChannel  ; //mutateColor(	animals[animalIndex].body[mutantCell].color);
				}


			}

			else if (auxMutation == 6)
			{

				// mutate an eyelook

				int mutantCell = getRandomCellOfType(animalIndex, ORGAN_SENSOR_EYE);
				if (mutantCell >= 0 && mutantCell < animalSquareSize)
				{
					// animals[animalIndex].genes[mutantCell].timerFreq += RNG() - 0.5f * 0.1;


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
	}

	for (int i = 0; i < animalSquareSize; ++i)
	{
		animals[animalIndex].body[i] = animals[animalIndex].genes[i];
	}



	// animals[animalIndex].mass = 0;
	// animals[animalIndex].numberOfTimesReproduced = 0;
	// animals[animalIndex].damageDone = 0;
	// animals[animalIndex].damageReceived = 0;
	// animals[animalIndex].birthLocation = 0;
	// animals[animalIndex].age = 0;
	// animals[animalIndex].parentIdentity = -1;
	// animals[animalIndex].retired = true;
	// animals[animalIndex].energy   = 0.0f;
	// animals[animalIndex].energyDebt   = 0.0f;
	// animals[animalIndex].maxEnergy   = 0.0f;
	// animals[animalIndex].fPosX = 0.0f;
	// animals[animalIndex].fPosY = 0.0f;
	// animals[animalIndex].position = 0;
	// animals[animalIndex].uPosX = 0;
	// animals[animalIndex].uPosY = 0;
	// // animals[animalIndex].prevHighestIntensity = 0.0f;
	// animals[animalIndex].parentAmnesty = true;
	animals[animalIndex].retired = false;
	// animals[animalIndex].mass = 0;
	// animals[animalIndex].age = 0;
	// animals[animalIndex].energyDebt = 0.0f;
	// animals[animalIndex].numberOfTimesReproduced = 0;
	// animals[animalIndex].damageDone = 0;
	// animals[animalIndex].damageReceived = 0;



	animals[animalIndex].position = position;
	animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = position / worldSize;
	animals[animalIndex].birthLocation = position;

	mutateAnimal( animalIndex);
	measureAnimalQualities(animalIndex);
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
	for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI) // process organs and signals and clear animalIndex on grid
	{
		unsigned int cellLocalPositionX  = cellLocalPositionI % animalSize;
		unsigned int cellLocalPositionY  = cellLocalPositionI / animalSize;
		unsigned int cellWorldPositionX  = cellLocalPositionX + animalWorldPositionX;
		unsigned int cellWorldPositionY  = cellLocalPositionY + animalWorldPositionY;
		unsigned int cellWorldPositionI  = (cellWorldPositionY * worldSize) + cellWorldPositionX;
		if (cellWorldPositionI < worldSquareSize)
		{
			if (animals[animalIndex].body[cellLocalPositionI].organ != MATERIAL_NOTHING)
			{
				if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
				{
					world[cellWorldPositionI].material = MATERIAL_FOOD;
				}

				if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_BONE)
				{
					world[cellWorldPositionI].material = MATERIAL_BONE;
				}


			}
		}
	}
}

// rotates an animal sprite, so it's like the animal is facing another direction!
// void turnAnimal(unsigned int animalIndex, unsigned int direction)
// {
// 	Animal tempAnimal = Animal();
// 	for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)  // you cannot add stuff back into the animal while you are working on it, so create a copy to hold your updates.
// 	{
// 		int originalXDiff = (cellLocalPositionI % animalSize) - (animalSize / 2);
// 		int originalYDiff = (cellLocalPositionI / animalSize) - (animalSize / 2);
// 		int rotatedXDiff = originalYDiff;
// 		int rotatedYDiff = originalXDiff;
// 		if (direction > 3)
// 		{
// 			int rotatedXDiff = originalYDiff * -1;
// 			int rotatedYDiff = originalXDiff * -1;
// 		}
// 		int rotatedX = rotatedXDiff + (animalSize / 2);
// 		int rotatedY = rotatedYDiff + (animalSize / 2);
// 		int rotatedI = (rotatedY * animalSize) + rotatedX;
// 		tempAnimal.body[cellLocalPositionI] = animals[animalIndex].body[rotatedI];
// 	}
// 	for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
// 	{
// 		animals[animalIndex].body[cellLocalPositionI] =	tempAnimal.body[cellLocalPositionI];
// 	}
// }


// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionI)
{
	if (world[cellWorldPositionI].identity >= 0 )
	{
		if (!animals[animalIndex].retired)
		{
			unsigned int cellWorldPositionX = cellWorldPositionI % worldSize;
			unsigned int cellWorldPositionY = cellWorldPositionI / worldSize;
			unsigned int targetWorldPositionX = animals[animalIndex].position % worldSize;
			unsigned int targetWorldPositionY = animals[animalIndex].position / worldSize;
			int targetLocalPositionX = cellWorldPositionX - targetWorldPositionX ;
			int targetLocalPositionY = cellWorldPositionY - targetWorldPositionY  ;
			if ( abs(targetLocalPositionX) < animalSize && abs(targetLocalPositionY) < animalSize
			        && targetLocalPositionX >= 0 && targetLocalPositionY >= 0)
			{
				unsigned int targetLocalPositionI = (targetLocalPositionY * animalSize) + targetLocalPositionX;
				if (targetLocalPositionI < animalSquareSize)
				{
					if (animals[animalIndex].body[targetLocalPositionI].organ != MATERIAL_NOTHING  )
					{
						return targetLocalPositionI;
					}
				}
			}
		}
	}
	return -1;
}



Color terrainColors(unsigned int terrain)
{
	switch (terrain)
	{
	// case TERRAIN_GRASS:
	// 	return color_green;
	case TERRAIN_STONE:
		return color_grey;
	case TERRAIN_WATER:
		return color_lightblue;
	case TERRAIN_LAVA:
		return color_orange;
	}
	return color_yellow;
}


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

		// case MATERIAL_SEAWEED:
		// 	return color_darkgreen;
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
	// case ORGAN_LEAF:
	// 	return color_green;
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


		if (isCellConnectable(animals[viewedAnimal].body[occupyingCell].organ ) )
		{
			float amount = animals[viewedAnimal].body[occupyingCell].signalIntensity * 2.0f;
			displayColor.r *= amount ;
			displayColor.g *= amount;
			displayColor.b *= amount;
		}

	}
	else
	{
		// if (world[worldI].material == MATERIAL_NOTHING)
		// {

		// displayColorA=  terrainColors(world[worldI].terrain);
		// }
		// else
		// {

		// displayColor = materialColors(world[worldI].material);


		// Color terrainColor
		// displayColorB = materialColors(world[worldI].material);;
		// if (world[worldI].terrain == TERRAIN_WATER)
		// {

		// displayColor = multiplyColor( materialColors(world[worldI].material), terrainColors(world[worldI].terrain) );
		// }

		displayColor = filterColor( terrainColors(world[worldI].terrain),  materialColors(world[worldI].material) );

		// }
	}
	return displayColor;
}


void updateMap()
{


	unsigned int mapUpdateFidelity = worldSquareSize / 50000;

	for (unsigned int i = 0; i < mapUpdateFidelity; ++i)
	{
		unsigned int randomX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomY = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomI = (randomY * worldSize) + randomX;
		if (randomI < worldSquareSize)
		{
			// if (world[randomI].terrain == TERRAIN_STONE)
			// {
			// world[randomI].terrain = TERRAIN_GRASS;
			if (world[randomI].material == MATERIAL_NOTHING)
			{
				if (world[randomI].terrain == TERRAIN_STONE ||
				        world[randomI].terrain == TERRAIN_WATER )
				{

					world[randomI].material = MATERIAL_GRASS;
				}
			}


			if (world[randomI].terrain == TERRAIN_LAVA)
			{

				if (world[randomI].material != MATERIAL_NOTHING)
				{
					world[randomI].material = MATERIAL_NOTHING;
				}

			}
			// }



			// if (world[randomI].terrain == TERRAIN_WATER)
			// {
			// 	// world[randomI].terrain = TERRAIN_GRASS;
			// 	if (world[randomI].material == MATERIAL_NOTHING)
			// 	{
			// 		world[randomI].material = MATERIAL_SEAWEED;
			// 	}

			// }

			// if (world[randomI].terrain == TERRAIN)
			// {
			// 	world[randomI].terrain = TERRAIN_GRASS;
			// }

			if (world[randomI].material == MATERIAL_BLOOD)
			{
				world[randomI].material = MATERIAL_NOTHING;
			}

			if (world[randomI].material == MATERIAL_BONE)
			{
				world[randomI].material = MATERIAL_NOTHING;
			}

		}

	}


}

int defenseAtPoint(unsigned int animalIndex, unsigned int cellLocalPositionI)
{
	int nBones = 0;
	for (unsigned int n = 0; n < nNeighbours; ++n)
	{
		unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
		if (cellNeighbour < animalSquareSize)
		{
			if (animals[animalIndex].body[cellNeighbour].organ  == ORGAN_BONE)
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


// the animal is a grid of living cells that do different things. this function describes what they do each turn.
void organs_all()
{
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			unsigned int cellsDone = 0;
			float totalLiver = 0;
			unsigned int totalGonads = 0;
			// unsigned int destinationThisTurn = animals[animalIndex].destination;
			float highestIntensity = 0.0f;

			for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				if (animals[animalIndex].body[cellLocalPositionI].organ == MATERIAL_NOTHING)
				{
					continue;
				}
				else
				{
					cellsDone++;

				}


				unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
				unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
				int cellLocalPositionX = cellLocalPositionI % animalSize;
				int cellLocalPositionY = cellLocalPositionI / animalSize;


				// rotate the animals cells when they interact with the world. This step is critical because their directions are relative to the animal direction.

				// place the center of the sprite at zero
				cellLocalPositionX -= (animalSize / 2);
				cellLocalPositionY -= (animalSize / 2);

				// add the eyelook
				cellLocalPositionX += animals[animalIndex].body[cellLocalPositionI].eyeLookX;
				cellLocalPositionY += animals[animalIndex].body[cellLocalPositionI].eyeLookY;

				// rotate by animal angle
				cellLocalPositionX *= cos(animals[animalIndex].fAngle);
				cellLocalPositionY *= sin(animals[animalIndex].fAngle);

				// move the center back to bottom left
				cellLocalPositionX += (animalSize / 2);
				cellLocalPositionY += (animalSize / 2);

				// world position now takes animal rotation into account (the drawings will not show that it is rotating, but it affects what the animal perceives.).
				unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
				unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
				unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
				if (cellWorldPositionI >= worldSquareSize)
				{
					continue;
				}

				unsigned int organ = animals[animalIndex].body[cellLocalPositionI].organ;

				switch (organ)
				{


				case ORGAN_MEMORY_RX:
				{

					// don't need to do anything, the tx part does all the work.

				}


				case ORGAN_MEMORY_TX:
				{



					// sum inputs. if exceeding a threshold, find a corresponding memory RX cell and copy it the input sum.


					animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
						{
							unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
							}
						}
					}




					if (animals[animalIndex].body[cellLocalPositionI].signalIntensity > 1.0f || animals[animalIndex].body[cellLocalPositionI].signalIntensity  < -1.0f)
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
							// std::advance(iterator, extremelyFastNumberFromZeroTo( found - 1)) ;
							// return *iterator;



							for (iterator = cellsOfType.begin(); iterator != cellsOfType.end(); ++iterator)
							{
								if ( animals[animalIndex].body[(*iterator)].speakerChannel == animals[animalIndex].body[cellLocalPositionI].speakerChannel  )
								{
									correspondingCellRX = (*iterator);
								}
							}

						}

						if (correspondingCellRX >= 0 && correspondingCellRX < animalSquareSize)
						{

							animals[animalIndex].body[correspondingCellRX].signalIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity ;

						}


					}
















				}

				case ORGAN_SENSOR_PHEROMONE:
				{

					animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0;
					if (animals[animalIndex].body[cellLocalPositionI]. speakerChannel ==   world[cellWorldPositionI].pheromoneChannel)
					{

						animals[animalIndex].body[cellLocalPositionI].signalIntensity  = world[cellWorldPositionI].pheromoneIntensity;
					}

					break;
				}

				case ORGAN_EMITTER_PHEROMONE:
				{

					animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
					for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
					{
						if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
						{
							unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
							}
						}
					}

					world[cellWorldPositionI].pheromoneChannel = animals[animalIndex].body[cellLocalPositionI]. speakerChannel ;
					world[cellWorldPositionI].pheromoneIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity;



				}





				case ORGAN_SPEAKER:
				{


					if ( animals[animalIndex].body[cellLocalPositionI].speakerChannel < numberOfSpeakerChannels)
					{

						// go through the list of connections and sum their values.
						animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
							{
								unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
								}
							}
						}

						if (animals[animalIndex].body[cellLocalPositionI].signalIntensity > 1.0f)
						{
							animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;
						}
						else if (animals[animalIndex].body[cellLocalPositionI].signalIntensity < -1.0f)
						{
							animals[animalIndex].body[cellLocalPositionI].signalIntensity = -1.0f;
						}



						speakerChannels[  animals[animalIndex].body[cellLocalPositionI].speakerChannel ] += animals[animalIndex].body[cellLocalPositionI].signalIntensity ;
					}
					else
					{
						animals[animalIndex].body[cellLocalPositionI].speakerChannel = 0;
					}


					break;


				}


				case ORGAN_SENSOR_EAR:
				{

					if (animals[animalIndex].body[cellLocalPositionI].speakerChannel < numberOfSpeakerChannels)
					{

						animals[animalIndex].body[cellLocalPositionI].signalIntensity = speakerChannelsLastTurn[ animals[animalIndex].body[cellLocalPositionI].speakerChannel ];

					}
					else
					{
						animals[animalIndex].body[cellLocalPositionI].speakerChannel = 0;
					}
					break;
				}

				case ORGAN_SENSOR_NOSE:
				{


					animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;

					if ( world [cellWorldPositionI].identity != animalIndex )
					{
						animals[animalIndex].body[cellLocalPositionI].signalIntensity = world[cellWorldPositionI].trail;
					}

					break;
				}


				case ORGAN_SENSOR_BODYANGLE:
				{

					animals[animalIndex].body[cellLocalPositionI].signalIntensity = animals[animalIndex].fAngle;
					break;
				}

				case ORGAN_SENSOR_EYE:
				{
					// unsigned int eyeLookX = animals[animalIndex].body[cellLocalPositionI].eyeLook % worldSize;
					// unsigned int eyeLookY = animals[animalIndex].body[cellLocalPositionI].eyeLook / worldSize;
					// unsigned int targetWorldPositionX = cellWorldPositionX + eyeLookX;
					// unsigned int targetWorldPositionY = cellLocalPositionY + eyeLookY;
					// unsigned int targetWorldPositionI = (targetWorldPositionY * worldSize) + targetWorldPositionX;

					Color receivedColor = whatColorIsThisSquare(cellWorldPositionI);
					Color perceivedColor = multiplyColor( receivedColor, animals[animalIndex].body[cellLocalPositionI].color  );
					animals[animalIndex].body[cellLocalPositionI].signalIntensity = colorAmplitude(perceivedColor );


					// printf("eye value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );

					break;
				}

				case ORGAN_SENSOR_TOUCH:
				{


					animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0;

					// unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
					// unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
					// unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
					// unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
					// unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
					// unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
					// unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;


					for (int i = 0; i < nNeighbours; ++i)
					{

						unsigned int neighbour = cellWorldPositionI + cellNeighbourOffsets[i];
						if (neighbour < worldSquareSize)
						{
							if (world[neighbour].identity >= 0)
							{
								if (isAnimalInSquare( world[neighbour].identity , neighbour ))
								{
									animals[animalIndex].body[cellLocalPositionI].signalIntensity += 0.5f;
								}

								else if (world[neighbour].material != MATERIAL_NOTHING)
								{
									animals[animalIndex].body[cellLocalPositionI].signalIntensity += 0.5f;
								}
							}
						}

					}


					if (world[cellWorldPositionI].identity >= 0)
					{
						if (world[cellWorldPositionI].identity != animalIndex)
						{
							if (isAnimalInSquare( world[cellWorldPositionI].identity , cellWorldPositionI ))
							{
								animals[animalIndex].body[cellLocalPositionI].signalIntensity += 0.5f;
							}

							else if (world[cellWorldPositionI].material != MATERIAL_NOTHING)
							{
								animals[animalIndex].body[cellLocalPositionI].signalIntensity += 0.5f;
							}
						}
					}

					break;
				}


				case ORGAN_SENSOR_TIMER:
				{
					animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0;
					if (useTimers)
					{
						animals[animalIndex].body[cellLocalPositionI].timerPhase += animals[animalIndex].body[cellLocalPositionI].timerFreq;
						animals[animalIndex].body[cellLocalPositionI].signalIntensity = sin(animals[animalIndex].body[cellLocalPositionI].timerPhase);
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
						if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
						{
							unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
							if (connected_to_cell < animalSquareSize)
							{
								float connected_signal = animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
								sum += connected_signal;
							}
							// printf("connected_signal %f \n", connected_signal);

						}
					}

					animals[animalIndex].body[cellLocalPositionI].signalIntensity = fast_sigmoid(sum);

					// printf("neuron value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );

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
								unsigned int adjustedPos = cellWorldPositionI - (    ( (animalSize / 2) * worldSize)   + animalSize / 2           ) ;
								if (adjustedPos < worldSquareSize)
								{
									unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;
									int result = spawnAnimal( speciesIndex,
									                          animals[animalIndex],
									                          adjustedPos, true );
									if (result >= 0)
									{
										animals[animalIndex].body[cellLocalPositionI].organ = MATERIAL_NOTHING;
										animals[animalIndex].numberOfTimesReproduced++;
										animals[animalIndex].energy -= animals[animalIndex].offspringEnergy;
										animals[result].energy       =  animals[animalIndex].offspringEnergy;
										animals[result].parentIdentity       = animalIndex;
									}
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

						// go through the list of connections and sum their values.
						animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
							{
								unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
								// printf("connected_to_cell %u\n", connected_to_cell);
								if (connected_to_cell < animalSquareSize)
								{
									animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
								}
							}
						}

						if (animals[animalIndex].body[cellLocalPositionI].signalIntensity > 1.0f)
						{
							animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;
						}
						else if (animals[animalIndex].body[cellLocalPositionI].signalIntensity < -1.0f)
						{
							animals[animalIndex].body[cellLocalPositionI].signalIntensity = -1.0f;
						}

						// printf("forward muscle value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );
						animals[animalIndex].fPosX += animals[animalIndex].body[cellLocalPositionI].signalIntensity * 10 * cos(animals[animalIndex].fAngle);
						animals[animalIndex].fPosY += animals[animalIndex].body[cellLocalPositionI].signalIntensity * 10 * sin(animals[animalIndex].fAngle);
					}
					break;
				}



				case ORGAN_MUSCLE_STRAFE :
				{
					if (doMuscles)
					{

						// go through the list of connections and sum their values.
						animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
							{
								unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;

								}

							}
						}

						if (animals[animalIndex].body[cellLocalPositionI].signalIntensity > 1.0f)
						{
							animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;
						}
						else if (animals[animalIndex].body[cellLocalPositionI].signalIntensity < -1.0f)
						{
							animals[animalIndex].body[cellLocalPositionI].signalIntensity = -1.0f;
						}

						// printf("forward muscle value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );

						// on the strafe muscle the sin and cos are reversed, that's all.
						animals[animalIndex].fPosX += animals[animalIndex].body[cellLocalPositionI].signalIntensity * 10 * sin(animals[animalIndex].fAngle);
						animals[animalIndex].fPosY += animals[animalIndex].body[cellLocalPositionI].signalIntensity * 10 * cos(animals[animalIndex].fAngle);
					}
					break;
				}


				case ORGAN_MUSCLE_TURN:
				{
					if (doMuscles)
					{
						// go through the list of connections and sum their values.
						animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
						for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
						{
							if (animals[animalIndex].body[cellLocalPositionI].connections[i] .used)
							{
								unsigned int connected_to_cell = animals[animalIndex].body[cellLocalPositionI].connections[i] .connectedTo;
								if (connected_to_cell < animalSquareSize)
								{
									animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
								}
							}
						}


						// if (animals[animalIndex].body[cellLocalPositionI].signalIntensity > 1.0f)
						// {
						// 	animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;
						// }
						// else if (animals[animalIndex].body[cellLocalPositionI].signalIntensity < -1.0f)
						// {
						// 	animals[animalIndex].body[cellLocalPositionI].signalIntensity = -1.0f;
						// }


						if (setOrSteerAngle)
						{

							animals[animalIndex].fAngle = (animals[animalIndex].body[cellLocalPositionI].signalIntensity ) ;
						}
						else {

							animals[animalIndex].fAngle += (animals[animalIndex].body[cellLocalPositionI].signalIntensity ) * 0.1f;
						}

						const float pi = 3.1415f;
						if (animals[animalIndex].fAngle > pi)
						{
							animals[animalIndex].fAngle -= 2 * pi;
						}

						if (animals[animalIndex].fAngle < -pi)
						{
							animals[animalIndex].fAngle += 2 * pi;
						}

						// printf("turning muscle value %f, fangle %f \n", animals[animalIndex].body[cellLocalPositionI].signalIntensity, animals[animalIndex].fAngle );
					}
				}

				}


				if (cellsDone >= animals[animalIndex].mass) {break;}
			}

			if (totalGonads == 0)
			{
				printf("gonads exploded after fucking too much\n");
				killAnimal (animalIndex);
			} // no point in infertile mature animals existing

			// animals[animalIndex].prevHighestIntensity = highestIntensity;
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
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			// if (brownianMotion)
			// {
			// 	if (animals[animalIndex].mass > 0)
			// 	{
			// 		animals[animalIndex].fPosY +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
			// 		animals[animalIndex].fPosX +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
			// 	}
			// }

			// unsigned int worldPositionI = (animals[animalIndex].uPosY * worldSize) + animals[animalIndex].uPosX;
			// if (world[worldPositionI].material == MATERIAL_ROCK)
			// {

			// 	animals[animalIndex].uPosX  = animals[animalIndex].fPosX;
			// 	animals[animalIndex].uPosY  = animals[animalIndex].fPosY;
			// }
			// else
			// {

			// 	// clip back to whatever integer it already was.
			// 	animals[animalIndex].fPosX  = animals[animalIndex].uPosX;
			// 	animals[animalIndex].fPosY  = animals[animalIndex].uPosY;



			// }


			// calculate direction of movement.
			// ufpos is the last guaranteed place of the animal, in floats
			float ufposx = animals[animalIndex].uPosX;
			float ufposy = animals[animalIndex].uPosY;
			// get the diff between the last actual place and the proposed new place
			float fdiffx = ufposx - animals[animalIndex].fPosX;
			float fdiffy = ufposy - animals[animalIndex].fPosY;
			// use atan2 to turn the diff into an angle.
			float dAngle = atan2(fdiffy, fdiffx);





			unsigned int newPosX  = animals[animalIndex].fPosX;
			unsigned int newPosY  = animals[animalIndex].fPosY;
			unsigned int newPosition  =  (newPosY * worldSize) + newPosX; // move




			if (newPosition < worldSquareSize)
			{


				if (world[newPosition].material == MATERIAL_ROCK)
				{
					animals[animalIndex].fPosX  = animals[animalIndex].uPosX;
					animals[animalIndex].fPosY  = animals[animalIndex].uPosY;
				}
				else
				{
					animals[animalIndex].uPosX  = animals[animalIndex].fPosX;
					animals[animalIndex].uPosY  = animals[animalIndex].fPosY;
				}




				// if ( world[newPosition].material != MATERIAL_ROCK)
				// {
				animals[animalIndex].position = newPosition;


				if (world[newPosition].terrain == TERRAIN_WATER)
				{
					if (! animals[animalIndex].canBreatheUnderwater)
					{
						animals[animalIndex].damageReceived ++;
					}
				}

				if (world[newPosition].terrain == TERRAIN_LAVA)
				{
					animals[animalIndex].damageReceived += 10;
				}


				unsigned int cellsDone = 0;
				for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
				{
					if ( (animals[animalIndex].body[cellLocalPositionI].organ != MATERIAL_NOTHING) )
					{
						cellsDone ++;
						if (taxIsByMass)
						{
							animals[animalIndex].energy -= taxEnergyScale *  organUpkeepCost(animals[animalIndex].body[cellLocalPositionI].organ); // * speciesEnergyOuts[speciesIndex] ;
						}
						bool okToStep = true;
						unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
						unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
						unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
						unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
						unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
						unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
						unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
						if (world[cellWorldPositionI].identity >= 0 && world[cellWorldPositionI].identity != animalIndex && world[cellWorldPositionI].identity < numberOfAnimals)
						{
							int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionI);
							if (targetLocalPositionI >= 0)
							{
								okToStep = false;
								if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_WEAPON ||
								        animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_MOUTH_CARNIVORE )
								{

									if (animals[animalIndex].parentAmnesty) // don't allow the animal to harm its parent until the amnesty period is over.
									{
										if (world[cellWorldPositionI].identity == animals[animalIndex].parentIdentity)
										{
											continue;
										}
									}

									float defense = defenseAtPoint(world[cellWorldPositionI].identity, targetLocalPositionI);

									if (defense > 0)
									{
										animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].damage += 1.0f / defense;
									}

									if (defense == 0 || animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].damage > 1.0f )
									{
										animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].organ = MATERIAL_NOTHING;
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

										if (world[cellWorldPositionI].identity == animals[animalIndex].parentIdentity)
										{
											printf("an animal attacked its parent!\n");
										}

										speciesAttacksPerTurn[speciesIndex] ++;

										if (animals[world[cellWorldPositionI].identity].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
										{
											if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_WEAPON)
											{
												if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
												{
													world[cellWorldPositionI].material = MATERIAL_FOOD;
												}
											}
											if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_MOUTH_CARNIVORE)
											{
												animals[animalIndex].energy += foodEnergy * energyScaleIn;
											}

										}
									}
								}
								else	if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_MOUTH_PARASITE )
								{
									float amount = (animals[world[cellWorldPositionI].identity].energy) / animalSquareSize;
									animals[animalIndex].energy += amount;
									animals[world[cellWorldPositionI].identity].energy -= amount;
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
							world[cellWorldPositionI].trail    = dAngle;
						}
					}
					if (cellsDone >= animals[animalIndex].mass) { break;}
				}
				// }
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
			if (animalIndex == playerCreature)
			{
				if (animals[animalIndex].damageReceived > animals[animalIndex].mass) // player can only be killed by MURDER
				{
					execute = true;
				}
			}
			else
			{
				if (!immortality) // reasons an npc can die
				{
					if (speciesPopulationCounts[speciesIndex] > (( numberOfAnimals / numberOfSpecies) / 4) ) // only kill off weak animals if there is some population.
						if (animals[animalIndex].energy < 0.0f)
						{
							execute = true;
							printf("died of low energy!\n");
						}
					if (animals[animalIndex].age > animals[animalIndex].lifespan)
					{
						printf("died of old age!\n");
						execute = true;
					}
					if (animals[animalIndex].damageReceived > animals[animalIndex].mass)
					{
						printf("murdered to death (or drowned)!\n");
						execute = true;
					}
					if (animals[animalIndex].mass <= 0)
					{
						printf("banished for being massless!\n");
						execute = true;
					}
				}
			}
			if (execute)
			{
				killAnimal( animalIndex);
			}
			if (tournament)
			{
				int animalScore = animals[animalIndex].damageDone + animals[animalIndex].damageReceived ;
				if ( animalScore > championScore)
				{
					championScore = animalScore;
					champion = animalIndex;
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

	// snone_lenerator();


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
				displayColor = whatColorIsThisSquare(worldI);
				drawTile( Vec_f2( fx, fy ), displayColor);
			}
		}
	}

	// highlight the player's eye look squares, for debug purposes
	// if (playerCreature >= 0)
	// {
	// 	if (!animals[playerCreature].retired)
	// 	{
	// 		for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
	// 		{

	// 			if (animals[playerCreature].body[cellLocalPositionI].organ == ORGAN_SENSOR_EYE)
	// 			{

	// 				// int eyeLookX = animals[playerCreature].body[cellLocalPositionI].eyeLook / worldSize;
	// 				// int eyeLookY = animals[playerCreature].body[cellLocalPositionI].eyeLook / worldSize;


	// 				unsigned int animalWorldPositionX    = animals[playerCreature].position % worldSize;
	// 				unsigned int animalWorldPositionY    = animals[playerCreature].position / worldSize;
	// 				unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
	// 				unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
	// 				unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX + eyeLookX;
	// 				unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY + eyeLookY;
	// 				unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
	// 				float fx = cellWorldPositionX;
	// 				float fy = cellWorldPositionY;

	// 				Color displayColor = whatColorIsThisSquare(cellWorldPositionI);
	// 				displayColor.r += 0.2f;
	// 				displayColor.g += 0.2f;
	// 				displayColor.b += 0.2f;
	// 				displayColor = clampColor(displayColor);
	// 				drawTile( Vec_f2( fx, fy ), displayColor);

	// 			}

	// 		}
	// 	}

	// }


	float fmx = mousePositionX;
	float fmy = mousePositionY;
	Color displayColor = color_white; //whatColorIsThisSquare(worldI);

	Vec_f2 worldMousePos = Vec_f2( fmx, fmy);

	drawTile( worldMousePos, displayColor);




}










void drawGameInterfaceText()
{


	int menuX = 50;
	int menuY = 500;
	int textSize = 10;
	int spacing = 20;


	for (int i = 0; i < numberOfSpecies; ++i)
	{

		printText2D(   std::string("Species ") + std::to_string(i) +   std::string(" pop. " + std::to_string(speciesPopulationCounts[i])) + " hits " + std::to_string(speciesAttacksPerTurn[i]) , menuX, menuY, textSize);
		menuY -= spacing;
	}
	menuY -= spacing;

	printText2D(   std::string("Player ") + std::to_string(playerCreature) , menuX, menuY, textSize);
	menuY -= spacing;

	if (playerCreature >= 0)
	{
		printText2D(   std::string("Energy ") + std::to_string(animals[playerCreature].energy ) , menuX, menuY, textSize);
		menuY -= spacing;

		printText2D(   std::string("Energy debt ") + std::to_string(animals[playerCreature].energyDebt ) , menuX, menuY, textSize);
		menuY -= spacing;


		printText2D(   std::string("Max energy ") + std::to_string(animals[playerCreature].maxEnergy ) , menuX, menuY, textSize);
		menuY -= spacing;

		printText2D(   std::string("Offspring energy ") + std::to_string(animals[playerCreature].offspringEnergy ) , menuX, menuY, textSize);
		menuY -= spacing;

		printText2D(   std::string("Reproduces at ") + std::to_string( ((animals[playerCreature].maxEnergy / 2) + (animals[playerCreature].offspringEnergy )) ) , menuX, menuY, textSize);
		menuY -= spacing;



	}
	menuY -= spacing;

	printText2D(   std::string("FPS ") + std::to_string(fps ) , menuX, menuY, textSize);
	menuY -= spacing;

	printText2D(   std::string("Mouse X ") + std::to_string(mousePositionX ) + std::string(" Y ") + std::to_string(mousePositionY) , menuX, menuY, textSize);
	menuY -= spacing;






}




void animalAppendCell(unsigned int animalIndex, unsigned int cellLocalPositionI, unsigned int organType)
{
	// puts a cell into an animal 'properly', connecting it up to existing neurons if applicable.
	animals[animalIndex].genes[cellLocalPositionI].organ = organType;


	if (
	    isCellConnecting(organType)

	) // if the cell is supposed to have connections, go hook it up
	{

		unsigned int randomNumberOfConnections = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS);



		for (int i = 0; i < randomNumberOfConnections; ++i)
		{

			// pick a random connectable cell to connect to.
			unsigned int connectableCell = getRandomConnectableCell( animalIndex);

			// check if you are already connected to it.
			// std::list<unsigned int>::iterator alreadyConnectedCell;
			bool alreadyConnected =  false;
			for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
			{
				if (  animals[animalIndex].genes[cellLocalPositionI].connections[j].connectedTo == connectableCell &&
				        animals[animalIndex].genes[cellLocalPositionI].connections[j] .used
				   )
				{
					alreadyConnected = true;
				}
			}

			// make the new connection if appropriate.
			if (!alreadyConnected)
			{

				for (int j = 0; j < NUMBER_OF_CONNECTIONS; ++j)
				{
					if ( ! (animals[animalIndex].genes[cellLocalPositionI].connections[j].used))
					{

						animals[animalIndex].genes[cellLocalPositionI].connections[j].used = true;
						animals[animalIndex].genes[cellLocalPositionI].connections[j].connectedTo = connectableCell;
						animals[animalIndex].genes[cellLocalPositionI].connections[j].weight = (RNG() - 0.5f ) * 2;
						break;

					}
				}









			}

		}

	}

}


void setupExampleAnimal2()
{
	// set the example back to the default state or it wont work properly.
	resetAnimal(0);
	// exampleAnimal2 = animals[0];

	// for (int i = 0; i < animalSquareSize; ++i)
	// {

	// int x = i % animalSize;
	// int y = i / animalSize;


	// exampleAnimal2.body[i].color = color_lightblue;


	// if (i < animalSize)
	// {
	// }
	// if (i > animalSize && i < (animalSize + 3))
	// {
	unsigned int i = 0;
	animalAppendCell( 0, i, ORGAN_MOUTH_VEG );           i++;
	animalAppendCell( 0, i, ORGAN_MOUTH_VEG );           i++;
	animalAppendCell( 0, i, ORGAN_MOUTH_VEG );           i++;
	animalAppendCell( 0, i, ORGAN_MOUTH_VEG );           i++;
	animalAppendCell( 0, i, ORGAN_MOUTH_VEG );           i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_EYE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_EYE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_EYE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_EYE );          i++;
	// animalAppendCell( 0, i, ORGAN_SENSOR_TIMER );        i++;
	// animalAppendCell( 0, i, ORGAN_SENSOR_TIMER );        i++;

	animalAppendCell( 0, i, ORGAN_SENSOR_EAR );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_EAR );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_PHEROMONE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_PHEROMONE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_NOSE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_NOSE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_TOUCH );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_TOUCH );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_BODYANGLE );          i++;
	animalAppendCell( 0, i, ORGAN_SENSOR_BODYANGLE );          i++;




	animalAppendCell( 0, i, ORGAN_MEMORY_RX );              i++;
	animalAppendCell( 0, i, ORGAN_MEMORY_RX );              i++;
	animalAppendCell( 0, i, ORGAN_MEMORY_RX );              i++;
	animalAppendCell( 0, i, ORGAN_MEMORY_RX );              i++;


	animalAppendCell( 0, i, ORGAN_BIASNEURON );          i++;
	animalAppendCell( 0, i, ORGAN_BIASNEURON );          i++;
	animalAppendCell( 0, i, ORGAN_BIASNEURON );          i++;
	animalAppendCell( 0, i, ORGAN_BIASNEURON );          i++;
	animalAppendCell( 0, i, ORGAN_BIASNEURON );          i++;
	animalAppendCell( 0, i, ORGAN_BIASNEURON );          i++;

	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;
	animalAppendCell( 0, i, ORGAN_NEURON );              i++;

	animalAppendCell( 0, i, ORGAN_MEMORY_TX );              i++;
	animalAppendCell( 0, i, ORGAN_MEMORY_TX );              i++;
	animalAppendCell( 0, i, ORGAN_MEMORY_TX );              i++;
	animalAppendCell( 0, i, ORGAN_MEMORY_TX );              i++;

	animalAppendCell( 0, i, ORGAN_MUSCLE );              i++;
	animalAppendCell( 0, i, ORGAN_MUSCLE_TURN );         i++;
	animalAppendCell( 0, i, ORGAN_LIVER );               i++;
	animalAppendCell( 0, i, ORGAN_LIVER );               i++;
	animalAppendCell( 0, i, ORGAN_LIVER );               i++;
	animalAppendCell( 0, i, ORGAN_ADDOFFSPRINGENERGY );  i++;
	animalAppendCell( 0, i, ORGAN_ADDOFFSPRINGENERGY );  i++;
	animalAppendCell( 0, i, ORGAN_ADDOFFSPRINGENERGY );  i++;
	animalAppendCell( 0, i, ORGAN_GONAD );               i++;
	animalAppendCell( 0, i, ORGAN_GONAD );               i++;
	animalAppendCell( 0, i, ORGAN_GONAD );               i++;
	animalAppendCell( 0, i, ORGAN_GONAD );               i++;




	// }






	// if (y == 2 && x < 3)
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_SENSOR_EYE;
	// 	exampleAnimal2.body[i].eyeColor = color_green;
	// // 	exampleAnimal2.body[i].eyeLook = 0;
	// // 	exampleAnimal2.body[i].eyeLook += ( (extremelyFastNumberFromZeroTo(10) - 5) * animalSize  ) + (extremelyFastNumberFromZeroTo(10) - 5);

	// // }
	// if (y == 3 && x < 3)
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_MUSCLE;
	// }
	// if (y == 3 && x > 3 && x < 6)
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_MUSCLE_TURN;
	// }
	// if (y == 4 )
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_NEURON;
	// }
	// if (y == 5 )
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_BIASNEURON;
	// }
	// if (y == 6 )
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_LIVER;
	// }
	// if (y == 7 && x < 5)
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_ADDOFFSPRINGENERGY;
	// }
	// if (y == 3 && x > 3)
	// {
	// 	exampleAnimal2.body[i].organ = ORGAN_GONAD;
	// }
	// // }

	// scrambleConnections(playerCreature);

	// exampleAnimal2 = animals[0];
}

void spawnPlayer()
{

	if (playerCreature == -1)
	{


		unsigned int targetWorldPositionX = cameraPositionX ;
		unsigned int targetWorldPositionY = cameraPositionY ;
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		setupExampleAnimal2();
		playerCreature = 0;
		spawnAnimalIntoSlot(playerCreature,
		                    animals[0],
		                    targetWorldPositionI, false);
		cameraTargetCreature = playerCreature;

		printf("spawned player creature\n");
	}
	else
	{

		killAnimal(playerCreature);
		printf("suicided player creature\n");
	}
}


void spawnTournamentAnimals()
{

	for (int i = 0; i < numberOfAnimals; ++i)
	{
		// unsigned int targetWorldPositionX = cameraPositionX ;
		// unsigned int targetWorldPositionY = cameraPositionY ;
		unsigned int targetWorldPositionI = extremelyFastNumberFromZeroTo(worldSquareSize) - 1; //( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		// playerCreature = 0;

		setupExampleAnimal2();

		spawnAnimalIntoSlot(i,
		                    animals[0],
		                    targetWorldPositionI, true);
		// cameraTargetCreature = playerCreature;
	}

}

// void makeRandomAnimal(unsigned int animalIndex, unsigned int targetWorldPositionI)
// {
// 	spawnAnimalIntoSlot(animalIndex,
// 	                    animals[0],
// 	                    targetWorldPositionI, false);

// 	for (int j = 0; j < animalSquareSize; ++j)
// 	{
// 		animals[animalIndex].body[j].organ = randomLetter();
// 		animals[animalIndex].body[j].color = Color(RNG(), RNG(), RNG(), 1.0f);
// 		// animals[animalIndex].body[j].eyeColor = Color(RNG(), RNG(), RNG(), 1.0f);
// 	}

// 	unsigned int randomcell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
// 	animals[animalIndex].body[randomcell].organ = ORGAN_GONAD; // make sure every animal has at least three gonads- two to propagate a winning population, and one to test out mutants.
// 	randomcell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
// 	animals[animalIndex].body[randomcell].organ = ORGAN_GONAD;
// 	randomcell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
// 	animals[animalIndex].body[randomcell].organ = ORGAN_GONAD;


// }

// void setupTournamentAnimals()
// {
// 	for (unsigned int i = 0; i < numberOfAnimals;  ++i)	// initial random creatures.
// 	{
// 		unsigned int targetWorldPositionX = cameraPositionX + extremelyFastNumberFromZeroTo(viewFieldX) - (viewFieldX / 2);
// 		unsigned int targetWorldPositionY = cameraPositionY + extremelyFastNumberFromZeroTo(viewFieldY) - (viewFieldY / 2);
// 		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;


// 		makeRandomAnimal(i, targetWorldPositionI);
// 	}
// }

void setupRandomWorld()
{
	resetAnimals();
	resetGrid();
	setupExampleAnimal2();

	// spawn the example creature in the center field of view in an empty world.
	if (worldToLoad == WORLD_EXAMPLECREATURE)
	{
		unsigned int wallthickness = 8;



		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{

			world[worldPositionI].terrain = TERRAIN_STONE;


			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;
			// walls around the world edge
			if (x < wallthickness || x > worldSize - wallthickness || y < wallthickness  || y > worldSize - wallthickness)
			{
				world[worldPositionI].material = MATERIAL_ROCK;
			}
			else
			{

				// world[worldPositionI].material = MATERIAL_GRASS;
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
					world[square].material = MATERIAL_ROCK;
				}
			}
		}


		// lava
		for (int i = 0; i < 35; ++i)
		{
			unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
			unsigned int x = randompos % worldSize;
			unsigned int y = randompos / worldSize;
			int rocksize = 250;
			for (int j = 0; j < rocksize; ++j)
			{
				for (int k = 0; k < rocksize; ++k)
				{
					unsigned int square = ( (y + j) * worldSize ) + ( x + k );
					world[square].terrain = TERRAIN_LAVA;
				}
			}
		}



		// water

		// unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
		unsigned int x = worldSize / 2 ; //randompos % worldSize;
		unsigned int y = worldSize / 2 ; //randompos / worldSize;
		int rocksize = 2500;
		for (int j = 0; j < rocksize; ++j)
		{
			for (int k = 0; k < rocksize; ++k)
			{


				unsigned int square = ( (y + j) * worldSize ) + ( x + k );
				world[square].terrain = TERRAIN_WATER;

				// world[square].material = MATERIAL_NOTHING;

			}

		}








	}





	// else if (worldToLoad == WORLD_ARENA)
	// {

	// 	unsigned int wallthickness = 8;
	// 	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
	// 	{
	// 		int x = worldPositionI % worldSize;
	// 		int y = worldPositionI / worldSize;
	// 		if (x < wallthickness || x > worldSize - wallthickness || y < wallthickness  || y > worldSize - wallthickness)
	// 		{
	// 			world[worldPositionI].material = MATERIAL_ROCK;
	// 		}
	// 		world[worldPositionI].terrain = TERRAIN_GRASS;
	// 	}





	// 	// setupTournamentAnimals();
	// }
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
		for (int i = 0; i < numberOfSpecies; ++i)
		{
			if (speciesPopulationCounts[i] == 0)
			{

				unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);

				unsigned int randomSpeciesSlot = (i * numberOfAnimalsPerSpecies) + (extremelyFastNumberFromZeroTo(numberOfAnimalsPerSpecies - 1));



				// if there is another species who is successful, duplicate an animal from them.
				int foundAnimal = -1;
				for (unsigned int j = 0; j < numberOfSpecies; ++j)
				{
					if (speciesPopulationCounts[j] > 1)
					{

						for (unsigned int k = 0; k < numberOfAnimalsPerSpecies; ++k)
						{
							unsigned int animalToCopy = (j * numberOfAnimalsPerSpecies) + k;

							if (!animals[animalToCopy].retired)
							{
								foundAnimal = animalToCopy;
								// printf("FOUNDANENENENE\n");
								break;
							}
						}

					}
				}

				if (foundAnimal >= 0 && foundAnimal < numberOfAnimals)
				{

					spawnAnimal(i, animals[foundAnimal], randompos, false);

				}
				else
				{
					// makeRandomAnimal(randomSpeciesSlot, randompos);
				}

			}
		}
	}

}

void sprinkleFood()
{
	if (extremelyFastNumberFromZeroTo(100) == 0)
	{
		unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);
		if (world[randompos].material == MATERIAL_NOTHING)
		{
			world[randompos].material = MATERIAL_FOOD;
		}
	}
}

void model()
{
	auto start = std::chrono::steady_clock::now();
	updateMap();
	computeAllAnimalsOneTurn();
	if (tournament)
	{
		tournamentController();
	}
	sprinkleFood();
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

#ifdef TRACY_ENABLE//
		FrameMark;
#endif
	}
}

void startSimulation()
{
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