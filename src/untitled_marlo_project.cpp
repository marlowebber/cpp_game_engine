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
#define ORGAN_MOUTH_VEG                1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH_SCAVENGE               2
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
#define ORGAN_ADDSTRIDE           14
#define ORGAN_NEURON              15
#define ORGAN_BIASNEURON          16    // can be thought of as ORGAN_SENSOR_CONSTANTVALUE
#define numberOfOrganTypes        17 // the number limit of growable genes
#define MATERIAL_FOOD             32 //           
#define MATERIAL_ROCK             33 //    
#define MATERIAL_MEAT             34
#define MATERIAL_BONE             35
#define MATERIAL_BLOOD            36
#define TERRAIN_STONE             50
#define TERRAIN_GRASS             51
#define TERRAIN_WATER             52 //           
#define MARKER                    35 //      // numbers above 25 don't correspond to lower-case letters(0..25) so we don't use them in the gene code. But (26..31) are still compatible with our masking scheme.

#define CONDITION_GREATER         41
#define CONDITION_EQUAL           42
#define CONDITION_LESS            43

#define WORLD_RANDOM 1
#define WORLD_EXAMPLECREATURE 2
#define WORLD_ARENA 3


#define NUMBER_OF_CONNECTIONS 8

const int ioMatrixSize = 32;

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
const unsigned int numberOfAnimals = 100000;
const unsigned int numberOfSpecies = 12;
unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);
const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.0000f;        // a multiplier for how much it costs animals just to exist.
const float movementEnergyScale    = 0.000f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.9f;         // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float grassEnergy            = 0.2f;         // how much you get from eating a square of grass

const float neuralNoise = 0.01f;

const float liverStorage = 20.0f;
const unsigned int baseLifespan = 5000;
const float signalPropagationConstant = 0.1f;      // how strongly sensor organs compel the animal.
float energyScaleIn             = 1.0f;            // a multiplier for how much energy is gained from food and light.
float minimumEntropy = 0.1f;
const float musclePower = 40.0f;
const float thresholdOfBoredom = 0.1f;
bool playerInControl = false;
int playerCreature = -1;

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
	int identity;
	int height;
	float light;
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
	float sign;
	float signalIntensity;
	unsigned int target;
	Color color;
	Color eyeColor;
	unsigned int eyeLook; // this is a positional offset indicating which square the eye is looking at. It is constant for a particular eye (i.e. one eye cannot look around unless the animal moves).
	float damage;
	Connection connections[NUMBER_OF_CONNECTIONS];
};

struct Animal
{
	Cell body[animalSquareSize];
	unsigned int mass;
	int stride;
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
	// unsigned int destination;
	unsigned int uPosX;
	unsigned int uPosY;

	float fPosX;
	float fPosY;
	float fAngle;

	float prevHighestIntensity;
	bool parentAmnesty;
	int totalMuscle;

	float sensorium[ioMatrixSize];
	float outputs[ioMatrixSize];

	// fann * ann;
};


// fann * neuralNets[numberOfAnimals];

// float speciesEnergyOuts              [numberOfSpecies];
unsigned int speciesPopulationCounts [numberOfSpecies];
unsigned int populationCountUpdates  [numberOfSpecies];

unsigned int speciesAttacksPerTurn[numberOfSpecies];

Animal exampleAnimal2;



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
			// case ORGAN_LEAF:
			// 	growthCost *= 1.0f;
			// 	break;
			case ORGAN_MUSCLE:
				growthCost *= 1.0f;
				break;
			case ORGAN_BONE:
				growthCost *= 1.0f;
				break;
			case ORGAN_WEAPON:
				growthCost *= 2.0f;
				break;
			// case ORGAN_SENSOR_FOOD:
			// 	growthCost *= 2.0f;
			// 	break;
			// case ORGAN_SENSOR_LIGHT:
			// 	growthCost *= 2.0f;
			// 	break;
			// case ORGAN_SENSOR_CREATURE:
			// 	growthCost *= 2.0f;
			// 	break;
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
			// case ORGAN_SENSOR_FOOD:
			// 	upkeepCost = 3.0f;
			// 	break;
			// case ORGAN_SENSOR_LIGHT:
			// 	upkeepCost = 3.0f;
			// 	break;
			// case ORGAN_SENSOR_CREATURE:
			// 	upkeepCost = 3.0f;
			// 	break;
		}
	}
	return upkeepCost;
}

Animal animals[numberOfAnimals];

void scrambleConnections(unsigned int animalIndex)
{
	if (animalIndex < numberOfAnimals)
	{
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{


			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{

				animals[animalIndex].body[cellLocalPositionI].connections[i].used = false;
				if (RNG() < 0.5f) { animals[animalIndex].body[cellLocalPositionI].connections[i].used = true; }

				animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = extremelyFastNumberFromZeroTo(animalSquareSize - 1);


				animals[animalIndex].body[cellLocalPositionI].connections[i].weight = (RNG() - 0.5f * 2.0f);

			}

		}
	}

}


void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
			animals[animalIndex].body[cellLocalPositionI].sign = 1.0f;
			// animals[animalIndex].body[cellLocalPositionI].sensorRange = baseSensorRange;
			animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
			// animals[animalIndex].body[cellLocalPositionI].multiplierFactor = 1.0f;
			animals[animalIndex].body[cellLocalPositionI].target = 0;
			animals[animalIndex].body[cellLocalPositionI].color  = color_darkgrey;
			animals[animalIndex].body[cellLocalPositionI].eyeColor = color_grey;
			animals[animalIndex].body[cellLocalPositionI].damage = 0.0f;


			for (int i = 0; i < NUMBER_OF_CONNECTIONS; ++i)
			{
				animals[animalIndex].body[cellLocalPositionI].connections[i].used = true;
				animals[animalIndex].body[cellLocalPositionI].connections[i].connectedTo = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[cellLocalPositionI].connections[i].weight = RNG() - 0.5f;

			}
		}
		animals[animalIndex].mass = 0;
		animals[animalIndex].stride = 1;
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
		// animals[animalIndex].destination = 0;
		animals[animalIndex].prevHighestIntensity = 0.0f;
		animals[animalIndex].parentAmnesty = true;

		animals[animalIndex].retired = true;
	}
}


void resetBrain( unsigned int animalIndex)
{

	// animals[animalIndex].ann = createBrainFromLayerDiagram( );
}

void resetAnimals()
{
	for ( int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		resetAnimal(animalIndex);

		resetBrain( animalIndex);
	}
}

void resetGrid()
{
	for (int i = 0; i < worldSquareSize; ++i)
	{
		world[i].material = MATERIAL_NOTHING;
		world[i].identity = -1;
		world[i].light = 1.0f;
	}
}

char randomLetter()
{
	return	extremelyFastNumberFromZeroTo(numberOfOrganTypes);
}

int getNewIdentity(unsigned int speciesIndex)
{
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
	return -1;
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
	    // organ == ORGAN_SENSOR_FOOD ||
	    // organ == ORGAN_SENSOR_LIGHT ||
	    // organ == ORGAN_SENSOR_CREATURE ||
	    // organ == ORGAN_SENSOR_RANDOM ||
	    // organ == ORGAN_SENSOR_HOME ||
	    // organ == ORGAN_SENSOR_PARENT ||
	    organ == ORGAN_SENSOR_EYE
	)
	{
		return true;
	}
	return false;
}


// bool organIsAModifier(unsigned int organ)
// {
// 	if (    organ == ORGAN_MODIFIER_HURT ||
// 	        organ == ORGAN_MODIFIER_HUNGRY ||
// 	        organ == ORGAN_MODIFIER_DEBTPAID ||
// 	        organ == ORGAN_MODIFIER_NOVALIDTARGET
// 	   )
// 	{
// 		return true;
// 	}
// 	return false;
// }

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
			// animals[animalIndex].body[cellLocalPositionI].sensorRange = nSensors * baseSensorRange;
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
	animals[animalIndex].stride = 1;
	animals[animalIndex].offspringEnergy = 1.0f;
	animals[animalIndex].lifespan = baseLifespan;

	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].body[i].organ == ORGAN_MUSCLE)
		{
			animals[animalIndex].totalMuscle ++;
		}



		if (animals[animalIndex].body[i].organ == ORGAN_ADDSTRIDE)
		{
			animals[animalIndex].stride += animals[animalIndex].stride;
		}

		if (animals[animalIndex].body[i].organ == ORGAN_ADDOFFSPRINGENERGY)
		{
			animals[animalIndex].offspringEnergy += animals[animalIndex].offspringEnergy ;
		}

		if (animals[animalIndex].body[i].organ == ORGAN_ADDLIFESPAN)
		{
			animals[animalIndex].lifespan += baseLifespan;
		}

	}
}



// choose a random cell of a particular organ type in a given animal, or MATERIAL_NOTHING if the organ doesn't exist.
unsigned int getRandomCellOfType(unsigned int animalIndex, unsigned int organType)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].body[i].organ == organType)
		{
			cellsOfType.push_back(i);
			found++;
		}
	}

	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, found - 1);
		return *iterator;
	}
	return MATERIAL_NOTHING;
}

// choose any random populated cell.
unsigned int getRandomOrgan(unsigned int animalIndex)
{
	std::list<unsigned int> cellsOfType;
	unsigned int found = 0;
	for (int i = 0; i < animalSquareSize; ++i)
	{
		if (animals[animalIndex].body[i].organ != MATERIAL_NOTHING)
		{
			cellsOfType.push_back(i);
			found++;
		}
	}

	if (found > 0)
	{
		std::list<unsigned int>::iterator iterator = cellsOfType.begin();
		std::advance(iterator, found - 1);
		return *iterator;
	}
	return MATERIAL_NOTHING;
}



//
void mutateAnimal(unsigned int animalIndex)
{
	if (!doMutation) {return;}

	if (

	    extremelyFastNumberFromZeroTo(1) == 0
	) // don't mutate at all sometimes, to preserve a population against drift
	{

		// the mutations should not be evenly distributed, but the most important stuff should be focused on.

		unsigned int whatToMutate = extremelyFastNumberFromZeroTo(7);

		if (whatToMutate == 0)
		{
			// erase an organ
			unsigned int mutantCell = getRandomOrgan( animalIndex);
			animals[animalIndex].body[mutantCell].organ = MATERIAL_NOTHING;// randomLetter();
		}

		else if (whatToMutate == 1)
		{

			// add an organ
			unsigned int mutantCell = getRandomCellOfType(animalIndex, MATERIAL_NOTHING);
			animals[animalIndex].body[mutantCell].organ = randomLetter();
		}

		else if (whatToMutate == 2)
		{
			// turn a connection on or off.
			unsigned int mutantCell =  getRandomOrgan(animalIndex);
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			animals[animalIndex].body[mutantCell].connections[mutantConnection].used = !(animals[animalIndex].body[mutantCell].connections[mutantConnection].used );
		}

		else if (whatToMutate == 3)
		{
			// randomise a connection partner.
			unsigned int mutantCell =  getRandomOrgan( animalIndex);
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			unsigned int mutantPartner =  getRandomOrgan( animalIndex);
			animals[animalIndex].body[mutantCell].connections[mutantConnection].connectedTo = mutantPartner;
		}
		else if (whatToMutate == 4)
		{
			// randomise a connection weight.
			unsigned int mutantCell =  getRandomOrgan(animalIndex);
			unsigned int mutantConnection = extremelyFastNumberFromZeroTo(NUMBER_OF_CONNECTIONS - 1);
			unsigned int mutationAmount  = RNG() - 0.5;
			animals[animalIndex].body[mutantCell].connections[mutantConnection].weight += mutationAmount;

		}
		else if (whatToMutate == 6)
		{
			// randomise a bias neuron.
			unsigned int mutantCell = getRandomCellOfType(animalIndex, ORGAN_BIASNEURON);
			unsigned int mutationAmount  = RNG() - 0.5;
			animals[animalIndex].body[mutantCell].signalIntensity += mutationAmount;

		}


		else if (whatToMutate == 7)
		{


			// other stuff.

			unsigned int auxMutation = extremelyFastNumberFromZeroTo(8);


			if (auxMutation == 0)
			{

				unsigned int mutateColor = extremelyFastNumberFromZeroTo(2);
				// mutate colors of animal
				if (mutateColor == 0)         // the more types of organs there are, the more often they will be chosen as mutation options.
				{
					// printf("MON VOYAGE!\n");
					unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
					animals[animalIndex].body[mutantCell].color.r  += (RNG() - 0.5f) ;
					animals[animalIndex].body[mutantCell].color = clampColor(animals[animalIndex].body[mutantCell].color);
				}
				else if (mutateColor == 1)
				{
					unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
					animals[animalIndex].body[mutantCell].color.g += (RNG() - 0.5f);
					animals[animalIndex].body[mutantCell].color = clampColor(animals[animalIndex].body[mutantCell].color);
				}
				else if (mutateColor == 2)
				{
					unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
					animals[animalIndex].body[mutantCell].color.b += (RNG() - 0.5f) ;
					animals[animalIndex].body[mutantCell].color = clampColor(animals[animalIndex].body[mutantCell].color);
				}

			}

			else if (auxMutation == 1)
			{

				unsigned int mutateColor = extremelyFastNumberFromZeroTo(2);

				// mutate eye color
				if (mutateColor == 0)         // the more types of organs there are, the more often they will be chosen as mutation options.
				{
					unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
					animals[animalIndex].body[mutantCell].eyeColor.r += (RNG() - 0.5f);
					animals[animalIndex].body[mutantCell].eyeColor = clampColor(animals[animalIndex].body[mutantCell].eyeColor);
				}
				else	 if (mutateColor == 1)         // the more types of organs there are, the more often they will be chosen as mutation options.
				{
					unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
					animals[animalIndex].body[mutantCell].eyeColor.g += (RNG() - 0.5f);
					animals[animalIndex].body[mutantCell].eyeColor = clampColor(animals[animalIndex].body[mutantCell].eyeColor);
				}
				else 	 if (mutateColor == 2)         // the more types of organs there are, the more often they will be chosen as mutation options.
				{
					unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
					animals[animalIndex].body[mutantCell].eyeColor.b += (RNG() - 0.5f);
					animals[animalIndex].body[mutantCell].eyeColor = clampColor(animals[animalIndex].body[mutantCell].eyeColor);
				}

			}


		}









		// // mutate multiplier factor
		// else if (chosenMutationType == numberOfOrganTypes + 9)
		// {
		// 	unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);

		// 	if (animals[animalIndex].body[mutantCell].organ == ORGAN_MULTIPLIER)
		// 	{
		// 		// if it is actually a multiplier, it can mutate between about -10 and +10
		// 		animals[animalIndex].body[mutantCell].multiplierFactor += (RNG() - 0.5);
		// 		if (animals[animalIndex].body[mutantCell].multiplierFactor > 10.0f)
		// 		{
		// 			animals[animalIndex].body[mutantCell].multiplierFactor = 10.0f;
		// 		}
		// 		else if (animals[animalIndex].body[mutantCell].multiplierFactor < -10.0f)
		// 		{
		// 			animals[animalIndex].body[mutantCell].multiplierFactor = -10.0f;
		// 		}
		// 	}

		// 	if ( organIsAModifier(animals[animalIndex].body[mutantCell].organ))
		// 	{
		// 		// if it is actually a modifier, it can really only be positive or negative.
		// 		if (	animals[animalIndex].body[mutantCell].multiplierFactor  == -1 || animals[animalIndex].body[mutantCell].multiplierFactor  == 1)
		// 		{
		// 			animals[animalIndex].body[mutantCell].multiplierFactor  = animals[animalIndex].body[mutantCell].multiplierFactor   * -1;
		// 		}
		// 		else
		// 		{
		// 			if (animals[animalIndex].body[mutantCell].multiplierFactor  < 0)
		// 			{
		// 				animals[animalIndex].body[mutantCell].multiplierFactor   = -1;
		// 			}
		// 			else
		// 			{
		// 				animals[animalIndex].body[mutantCell].multiplierFactor  = 1;
		// 			}
		// 		}


		// 	}
		// }





		// }
		// else
		// {
		// }
	}
}




void spawnAnimalIntoSlot( unsigned int animalIndex,
                          Animal parent,
                          unsigned int position, bool mutation)
{
	// resetAnimal(animalIndex);

	animals[animalIndex] = parent;


	// reset the SHITTTT . but only the stuff that should change.

	animals[animalIndex].mass = 0;
	// animals[animalIndex].stride = 1;
	animals[animalIndex].numberOfTimesReproduced = 0;
	animals[animalIndex].damageDone = 0;
	animals[animalIndex].damageReceived = 0;
	animals[animalIndex].birthLocation = 0;
	animals[animalIndex].age = 0;
	// animals[animalIndex].lifespan = baseLifespan;
	animals[animalIndex].parentIdentity = -1;
	animals[animalIndex].retired = true;
	// animals[animalIndex].offspringEnergy = 1.0f;
	animals[animalIndex].energy   = 0.0f;
	animals[animalIndex].energyDebt   = 0.0f;
	animals[animalIndex].maxEnergy   = 0.0f;
	animals[animalIndex].fPosX = 0.0f;
	animals[animalIndex].fPosY = 0.0f;
	animals[animalIndex].position = 0;
	animals[animalIndex].uPosX = 0;
	animals[animalIndex].uPosY = 0;
	// animals[animalIndex].destination = 0;
	animals[animalIndex].prevHighestIntensity = 0.0f;
	animals[animalIndex].parentAmnesty = true;



	animals[animalIndex].retired = false;
	animals[animalIndex].mass = 0;
	animals[animalIndex].age = 0;
	animals[animalIndex].energyDebt = 0.0f;
	animals[animalIndex].numberOfTimesReproduced = 0;
	animals[animalIndex].damageDone = 0;
	animals[animalIndex].damageReceived = 0;


	animals[animalIndex].position = position;
	animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = position / worldSize;
	// animals[animalIndex].destination = position;
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
void turnAnimal(unsigned int animalIndex, unsigned int direction)
{
	Animal tempAnimal = Animal();
	for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)  // you cannot add stuff back into the animal while you are working on it, so create a copy to hold your updates.
	{
		int originalXDiff = (cellLocalPositionI % animalSize) - (animalSize / 2);
		int originalYDiff = (cellLocalPositionI / animalSize) - (animalSize / 2);
		int rotatedXDiff = originalYDiff;
		int rotatedYDiff = originalXDiff;
		if (direction > 3)
		{
			int rotatedXDiff = originalYDiff * -1;
			int rotatedYDiff = originalXDiff * -1;
		}
		int rotatedX = rotatedXDiff + (animalSize / 2);
		int rotatedY = rotatedYDiff + (animalSize / 2);
		int rotatedI = (rotatedY * animalSize) + rotatedX;
		tempAnimal.body[cellLocalPositionI] = animals[animalIndex].body[rotatedI];
	}
	for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
	{
		animals[animalIndex].body[cellLocalPositionI] =	tempAnimal.body[cellLocalPositionI];
	}
}


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
	case TERRAIN_GRASS:
		return color_green;
	case TERRAIN_STONE:
		return color_grey;
	case TERRAIN_WATER:
		return color_lightblue;
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
	if (animalIndex >= 0)
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
	}
	else
	{
		if (world[worldI].material == MATERIAL_NOTHING)
		{

			displayColor =  terrainColors(world[worldI].terrain);
		}
		else
		{

			displayColor = materialColors(world[worldI].material);
		}
	}
	return displayColor;
}


void updateMap()
{


	unsigned int mapUpdateFidelity = worldSquareSize / 100000;

	for (unsigned int i = 0; i < mapUpdateFidelity; ++i)
	{
		unsigned int randomX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomY = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int randomI = (randomY * worldSize) + randomX;
		if (randomI < worldSquareSize)
		{
			if (world[randomI].terrain == TERRAIN_STONE)
			{
				world[randomI].terrain = TERRAIN_GRASS;
			}

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
				unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
				unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
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



				case ORGAN_SENSOR_EYE:
				{
					unsigned int eyeLookX = animals[animalIndex].body[cellLocalPositionI].eyeLook % worldSize;
					unsigned int eyeLookY = animals[animalIndex].body[cellLocalPositionI].eyeLook / worldSize;
					unsigned int targetWorldPositionX = cellWorldPositionX + eyeLookX;
					unsigned int targetWorldPositionY = cellLocalPositionY + eyeLookY;
					unsigned int targetWorldPositionI = (targetWorldPositionY * worldSize) + targetWorldPositionX;

					Color receivedColor = whatColorIsThisSquare(targetWorldPositionI);
					Color perceivedColor = multiplyColor( receivedColor, animals[animalIndex].body[cellLocalPositionI].eyeColor  );
					animals[animalIndex].body[cellLocalPositionI].signalIntensity = colorAmplitude(perceivedColor );


					printf("eye value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );

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

							float connected_signal = animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;
							sum += connected_signal;
							printf("connected_signal %f \n", connected_signal);

						}
					}

					animals[animalIndex].body[cellLocalPositionI].signalIntensity = fast_sigmoid(sum);

					printf("neuron value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );

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


					if (world[cellWorldPositionI].terrain == TERRAIN_GRASS)
					{
						animals[animalIndex].energy += grassEnergy * energyScaleIn;
						world[cellWorldPositionI].terrain = TERRAIN_STONE;
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

								animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;

							}
						}
						printf("forward muscle value %f\n", animals[animalIndex].body[cellLocalPositionI].signalIntensity );
						animals[animalIndex].fPosX += animals[animalIndex].body[cellLocalPositionI].signalIntensity * cos(animals[animalIndex].fAngle);
						animals[animalIndex].fPosY += animals[animalIndex].body[cellLocalPositionI].signalIntensity * sin(animals[animalIndex].fAngle);
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

								animals[animalIndex].body[cellLocalPositionI].signalIntensity  += animals[animalIndex].body[connected_to_cell].signalIntensity * animals[animalIndex].body[cellLocalPositionI].connections[i] .weight;

							}
						}
						animals[animalIndex].fAngle += (animals[animalIndex].body[cellLocalPositionI].signalIntensity * 0.005f);
						printf("turning muscle value %f, fangle %f \n", animals[animalIndex].body[cellLocalPositionI].signalIntensity, animals[animalIndex].fAngle );
					}
				}

				}


				if (cellsDone >= animals[animalIndex].mass) {break;}
			}

			if (totalGonads == 0)
			{
				// printf("gonads exploded after fucking too much\n");
				killAnimal (animalIndex);
			} // no point in infertile mature animals existing

			animals[animalIndex].prevHighestIntensity = highestIntensity;
			animals[animalIndex].maxEnergy = animals[animalIndex].mass + (totalLiver * liverStorage);
		}
	}
}

void move_all()
{
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			if (brownianMotion)
			{
				if (animals[animalIndex].mass > 0)
				{
					animals[animalIndex].fPosY +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
					animals[animalIndex].fPosX +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
				}
			}

			// float minimumEdgeDistance = animalSize + 1; // animals are not allowed to go close to the edge, because it makes them act funny.

			// if (animals[animalIndex].fPosX < minimumEdgeDistance) {animals[animalIndex].fPosX = minimumEdgeDistance;}
			// if (animals[animalIndex].fPosX > worldSize - minimumEdgeDistance) { animals[animalIndex].fPosX = worldSize - minimumEdgeDistance;}
			// if (animals[animalIndex].fPosY < minimumEdgeDistance) {animals[animalIndex].fPosY = minimumEdgeDistance;}
			// if (animals[animalIndex].fPosY > worldSize - minimumEdgeDistance) { animals[animalIndex].fPosY = worldSize - minimumEdgeDistance;}



			if (!animals[animalIndex].retired)
			{
				// float * motorSignals = fann_run( animals[animalIndex].ann, animals[animalIndex].sensorium);
			}


			if (doMuscles)
			{
				// int destinationX = animals[animalIndex].destination % worldSize;
				// int destinationY = animals[animalIndex].destination / worldSize;
				// int uposx = animals[animalIndex].fPosX;
				// int uposy = animals[animalIndex].fPosY;
				// int diffX = (destinationX - uposx );
				// int diffY = (destinationY - uposy );
				// float muscleX = diffX;
				// float muscleY = diffY;
				// float muscleSignX = 1.0f;
				// float muscleSignY = 1.0f;
				// if (muscleX < 0.0f) {muscleSignX = -1.0f;}
				// if (muscleY < 0.0f) {muscleSignY = -1.0f;}
				// muscleX = abs(muscleX);
				// muscleY = abs(muscleY);
				// float sum = muscleX + muscleY;
				// if (sum > 0)
				// {
				// 	float xContrib = muscleX / sum;
				// 	float yContrib = muscleY / sum;
				// 	muscleX = (xContrib) * muscleSignX * musclePower * animals[animalIndex].totalMuscle;
				// 	muscleY = (yContrib) * muscleSignY * musclePower * animals[animalIndex].totalMuscle;
				// 	if (animals[animalIndex].mass != 0.0f )
				// 	{
				// 		animals[animalIndex].fPosX += ( muscleX ) / animals[animalIndex].mass;
				// 		animals[animalIndex].fPosY += ( muscleY ) / animals[animalIndex].mass;
				// 	}
				// 	animals[animalIndex].energy -= ( abs(muscleX) + abs(muscleY)) * movementEnergyScale ;
				// }
			}










			animals[animalIndex].uPosX  = animals[animalIndex].fPosX;
			animals[animalIndex].uPosY  = animals[animalIndex].fPosY;

			unsigned int newPosition  =  (animals[animalIndex].uPosY * worldSize) + animals[animalIndex].uPosX;  // move
			if (newPosition < worldSquareSize)
			{
				if ( world[newPosition].material != MATERIAL_ROCK)
				{
					animals[animalIndex].position = newPosition;
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
													// if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
													// {
													// world[cellWorldPositionI].material = MATERIAL_FOOD;

													animals[animalIndex].energy += foodEnergy * energyScaleIn;
													// world[cellWorldPositionI].material = MATERIAL_NOTHING;
													// }
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
							}
						}
						if (cellsDone >= animals[animalIndex].mass) { break;}
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
							// printf("died of low energy!\n");
						}
					if (animals[animalIndex].age > animals[animalIndex].lifespan)
					{
						// printf("died of old age!\n");
						execute = true;
					}
					if (animals[animalIndex].damageReceived > animals[animalIndex].mass)
					{
						// printf("killed to death!\n");
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
	if (playerCreature >= 0)
	{
		if (!animals[playerCreature].retired)
		{
			for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
			{

				if (animals[playerCreature].body[cellLocalPositionI].organ == ORGAN_SENSOR_EYE)
				{

					int eyeLookX = animals[playerCreature].body[cellLocalPositionI].eyeLook / worldSize;
					int eyeLookY = animals[playerCreature].body[cellLocalPositionI].eyeLook / worldSize;


					unsigned int animalWorldPositionX    = animals[playerCreature].position % worldSize;
					unsigned int animalWorldPositionY    = animals[playerCreature].position / worldSize;
					unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
					unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
					unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX + eyeLookX;
					unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY + eyeLookY;
					unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
					float fx = cellWorldPositionX;
					float fy = cellWorldPositionY;



					Color displayColor = whatColorIsThisSquare(cellWorldPositionI);
					displayColor.r += 0.2f;
					displayColor.g += 0.2f;
					displayColor.b += 0.2f;
					displayColor = clampColor(displayColor);
					drawTile( Vec_f2( fx, fy ), displayColor);

				}

			}
		}

	}


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


		// printText2D(   	std::string("Destination ") + std::to_string(animals[playerCreature].destination ) , menuX, menuY, textSize);
		// menuY -= spacing;



	}
	menuY -= spacing;

	printText2D(   std::string("FPS ") + std::to_string(fps ) , menuX, menuY, textSize);
	menuY -= spacing;

	printText2D(   std::string("Mouse X ") + std::to_string(mousePositionX ) + std::string(" Y ") + std::to_string(mousePositionY) , menuX, menuY, textSize);
	menuY -= spacing;






}




void animalAppendCell(unsigned int animal, unsigned int cellLocalPositionI, unsigned int organType)
{
	// puts a cell into an animal 'properly', connecting it up to existing neurons if applicable.

}


void setupExampleAnimal2()
{
	// set the example back to the default state or it wont work properly.
	resetAnimal(0);
	exampleAnimal2 = animals[0];

	for (int i = 0; i < animalSquareSize; ++i)
	{

		int x = i % animalSize;
		int y = i / animalSize;


		exampleAnimal2.body[i].color = color_lightblue;


		if (y == 1)
		{
			exampleAnimal2.body[i].organ = ORGAN_MOUTH_VEG;
		}


		// if (y == 2 && x == 0)
		// {
		// 	exampleAnimal2.body[i].organ = ORGAN_MULTIPLIER;
		// 	exampleAnimal2.body[i].multiplierFactor =
		// 	// exampleAnimal2.body[i].eyeColor = color_green;
		// }
		if (y == 2 && x < 3)
		{
			exampleAnimal2.body[i].organ = ORGAN_SENSOR_EYE;
			exampleAnimal2.body[i].eyeColor = color_green;
			exampleAnimal2.body[i].eyeLook = 0;
			exampleAnimal2.body[i].eyeLook += ( (extremelyFastNumberFromZeroTo(10) - 5) * animalSize  ) + (extremelyFastNumberFromZeroTo(10) - 5);

		}
		if (y == 3 && x < 3)
		{
			exampleAnimal2.body[i].organ = ORGAN_MUSCLE;
		}
		if (y == 3 && x > 3 && x < 6)
		{
			exampleAnimal2.body[i].organ = ORGAN_MUSCLE_TURN;
		}
		if (y == 4 )
		{
			exampleAnimal2.body[i].organ = ORGAN_NEURON;
		}
		if (y == 5 )
		{
			exampleAnimal2.body[i].organ = ORGAN_BIASNEURON;
		}
		if (y == 6 )
		{
			exampleAnimal2.body[i].organ = ORGAN_LIVER;
		}
		if (y == 7 && x < 5)
		{
			exampleAnimal2.body[i].organ = ORGAN_ADDOFFSPRINGENERGY;
		}
		if (y == 8 && x < 5)
		{
			exampleAnimal2.body[i].organ = ORGAN_GONAD;
		}




	}

	scrambleConnections(playerCreature);
}

void spawnPlayer()
{

	if (playerCreature == -1)
	{


		unsigned int targetWorldPositionX = cameraPositionX ;
		unsigned int targetWorldPositionY = cameraPositionY ;
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		playerCreature = 0;
		spawnAnimalIntoSlot(playerCreature,
		                    exampleAnimal2,
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


void makeRandomAnimal(unsigned int animalIndex, unsigned int targetWorldPositionI)
{
	spawnAnimalIntoSlot(animalIndex,
	                    animals[0],
	                    targetWorldPositionI, false);

	for (int j = 0; j < animalSquareSize; ++j)
	{
		animals[animalIndex].body[j].organ = randomLetter();
		animals[animalIndex].body[j].color = Color(RNG(), RNG(), RNG(), 1.0f);
		animals[animalIndex].body[j].eyeColor = Color(RNG(), RNG(), RNG(), 1.0f);
	}

	unsigned int randomcell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	animals[animalIndex].body[randomcell].organ = ORGAN_GONAD; // make sure every animal has at least three gonads- two to propagate a winning population, and one to test out mutants.
	randomcell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	animals[animalIndex].body[randomcell].organ = ORGAN_GONAD;
	randomcell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
	animals[animalIndex].body[randomcell].organ = ORGAN_GONAD;


}

void setupTournamentAnimals()
{
	for (unsigned int i = 0; i < numberOfAnimals;  ++i)	// initial random creatures.
	{
		unsigned int targetWorldPositionX = cameraPositionX + extremelyFastNumberFromZeroTo(viewFieldX) - (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + extremelyFastNumberFromZeroTo(viewFieldY) - (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;


		makeRandomAnimal(i, targetWorldPositionI);
	}
}

void setupRandomWorld()
{
	resetAnimals();
	resetGrid();
	setupExampleAnimal2();

	// spawn the example creature in the center field of view in an empty world.
	if (worldToLoad == WORLD_EXAMPLECREATURE)
	{
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;
			if (x == 0 || x == worldSize - 1 || y == 0 || y == worldSize - 1)
			{
				world[worldPositionI].material = MATERIAL_ROCK;
			}
			world[worldPositionI].terrain = TERRAIN_GRASS;
		}




	}


	else if (worldToLoad == WORLD_ARENA)
	{
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;
			if (x == 0 || x == worldSize - 1 || y == 0 || y == worldSize - 1)
			{
				world[worldPositionI].material = MATERIAL_ROCK;
			}
			world[worldPositionI].terrain = TERRAIN_GRASS;
		}





		setupTournamentAnimals();
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

// unsigned int getPlayerDestination()
// {
// 	if (playerCreature >= 0 && playerCreature < numberOfAnimals)
// 	{
// 		return animals[playerCreature].destination;
// 	}
// 	return 0;
// }
// void setPlayerDestination(unsigned int newDestination)
// {
// 	if (playerCreature >= 0 && playerCreature < numberOfAnimals)
// 	{
// 		animals[playerCreature].destination = newDestination;
// 	}
// }

void rebuildGameMenus ()
{
	int spacing = 10;

	menuItem * exampleMenuRoot = setupMenu ( std::string ("menu") , RIGHT, nullptr, (void *)exampleMenuCallback, nullptr, Color(0.1f, 0.1f, 0.1f, 1.0f), Vec_f2(200, 200));
	exampleMenuRoot->collapsed = false;

	uDataWrap *     tempDataWrap = new uDataWrap( (void*) & (fps), TYPE_UDATA_FLOAT  );
	menuItem * exampleMenuNumber = setupMenu ( std::string ("fps") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap);
	exampleMenuNumber->collapsed = false;

	tempDataWrap = new uDataWrap( (void*) & (cameraTargetCreature), TYPE_UDATA_INT  );
	exampleMenuNumber = setupMenu ( std::string ("cameraTargetCreature") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap);
	exampleMenuNumber->collapsed = false;

	for (int i = 0; i < numberOfSpecies; ++i)
	{
		tempDataWrap = new uDataWrap( (void*) & (speciesPopulationCounts[i]), TYPE_UDATA_UINT  );
		exampleMenuNumber = setupMenu ( std::string ("population:") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap);
		exampleMenuNumber->collapsed = false;

		// tempDataWrap = new uDataWrap( (void*) & (speciesEnergyOuts[i]), TYPE_UDATA_FLOAT  );
		// exampleMenuNumber = setupMenu ( std::string ("energyout:") , BELOW, exampleMenuRoot, (void *)editUserData, (void*)tempDataWrap);
		// exampleMenuNumber->collapsed = false;
	}
	// menus.push_back(*exampleMenuRoot);
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