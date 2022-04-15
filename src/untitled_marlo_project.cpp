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
// #define ORGAN_LEAF                1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH               2
#define ORGAN_GONAD               3
#define ORGAN_MUSCLE              4
#define ORGAN_BONE                5
#define ORGAN_WEAPON              6
#define ORGAN_LIVER               7
#define ORGAN_SENSOR_FOOD         8
#define ORGAN_SENSOR_CREATURE     9
#define ORGAN_SENSOR_LIGHT        10
#define ORGAN_SENSOR_RANDOM       11
#define ORGAN_SENSOR_PARENT       12
#define ORGAN_SENSOR_HOME         13
#define ORGAN_SENSOR_EYE          14

#define ORGAN_MULTIPLIER   15

#define ORGAN_MODIFIER_HUNGRY 16
#define ORGAN_MODIFIER_HURT          17
#define ORGAN_MODIFIER_DEBTPAID      18
#define ORGAN_MODIFIER_NOVALIDTARGET 19


#define numberOfOrganTypes        20 // the number limit of growable genes

#define MATERIAL_FOOD             32 //           
#define MATERIAL_ROCK             33 //    
#define MATERIAL_MEAT             34
#define MATERIAL_BONE             35

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

const bool brownianMotion        = false;
const bool immortality           = false;
const bool doReproduction        = true;
const bool doMuscles             = true;
const bool doPhotosynth          = true;
const bool growingCostsEnergy    = true;
const bool lockfps               = false;
const bool tournament            = true;
const bool taxIsByMass           = true;
const bool threading             = true;
const bool cameraFollowsChampion = false;
const bool cameraFollowsPlayer   = true;
const bool variedGrowthCost     = false;
const bool variedUpkeep          = false;
const bool respawnLowSpecies     = true;
const bool entropicController    = false;

unsigned int worldToLoad = WORLD_ARENA;

const unsigned int viewFieldX = 1024; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 1024; //203 columns, 55 rows is the max size i can make one on my pc.

float fps = 1.0f;

const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const int animalSize     = 8;
const unsigned int animalSquareSize      = animalSize * animalSize;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfSpecies = 6;
unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);
const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;         // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.000f;        // a multiplier for how much it costs animals just to exist.
// const float lightEnergy            = 0.01f;        // how much energy an animal gains each turn from having a leaf. if tax is by mass, must be higher than taxEnergyScale to be worth having leaves at all.
const float movementEnergyScale    = 0.001f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.9f;         // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float grassEnergy            = 0.01f;         // how much you get from eating a square of grass

const float liverStorage = 10.0f;
const unsigned int baseLifespan = 10000;
const unsigned int baseSensorRange = 20;
const float signalPropagationConstant = 0.1f;      // how strongly sensor organs compel the animal.
float energyScaleIn             = 1.0f;            // a multiplier for how much energy is gained from food and light.
float minimumEntropy = 0.1f;
const float musclePower = 20.0f;
const float thresholdOfBoredom = 0.1f;

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

struct Cell
{
	unsigned int organ;
	float sign;
	int sensorRange;
	float signalIntensity;
	unsigned int target;
	Color color;
	Color eyeColor;
	float multiplierFactor;
	float damage;
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
	unsigned int destination;
	unsigned int uPosX;
	unsigned int uPosY;
	float fPosX;
	float fPosY;
	float prevHighestIntensity;
};

// float speciesEnergyOuts              [numberOfSpecies];
unsigned int speciesPopulationCounts [numberOfSpecies];
unsigned int populationCountUpdates  [numberOfSpecies];

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
			case ORGAN_SENSOR_FOOD:
				growthCost *= 2.0f;
				break;
			case ORGAN_SENSOR_LIGHT:
				growthCost *= 2.0f;
				break;
			case ORGAN_SENSOR_CREATURE:
				growthCost *= 2.0f;
				break;
			case ORGAN_GONAD:
				growthCost *= 10.0f;
				break;
			case ORGAN_MOUTH:
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
		case ORGAN_SENSOR_FOOD:
			upkeepCost = 3.0f;
			break;
		case ORGAN_SENSOR_LIGHT:
			upkeepCost = 3.0f;
			break;
		case ORGAN_SENSOR_CREATURE:
			upkeepCost = 3.0f;
			break;
		}
	}
	return upkeepCost;
}

Animal animals[numberOfAnimals];

void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
			animals[animalIndex].body[cellLocalPositionI].sign = 1.0f;
			animals[animalIndex].body[cellLocalPositionI].sensorRange = baseSensorRange;
			animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
			animals[animalIndex].body[cellLocalPositionI].multiplierFactor = 1.0f;
			animals[animalIndex].body[cellLocalPositionI].target = 0;
			animals[animalIndex].body[cellLocalPositionI].color = color_grey;
			animals[animalIndex].body[cellLocalPositionI].color = color_grey;
			animals[animalIndex].body[cellLocalPositionI].damage = 0.0f;
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
		animals[animalIndex].retired = true;
		animals[animalIndex].offspringEnergy = 1.0f;
		animals[animalIndex].energy   = 0.0f;
		animals[animalIndex].energyDebt   = 0.0f;
		animals[animalIndex].maxEnergy   = 0.0f;
		animals[animalIndex].fPosX = 0.0f;
		animals[animalIndex].fPosY = 0.0f;
		animals[animalIndex].position = 0;
		animals[animalIndex].uPosX = 0;
		animals[animalIndex].uPosY = 0;
		animals[animalIndex].destination = 0;
		animals[animalIndex].prevHighestIntensity = 0.0f;
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



bool organIsASensor(unsigned int organ)
{
	if (    organ == ORGAN_SENSOR_FOOD ||
	        organ == ORGAN_SENSOR_LIGHT ||
	        organ == ORGAN_SENSOR_CREATURE ||
	        organ == ORGAN_SENSOR_RANDOM ||
	        organ == ORGAN_SENSOR_HOME ||
	        organ == ORGAN_SENSOR_PARENT
	   )
	{
		return true;
	}
	return false;
}


bool organIsAModifier(unsigned int organ)
{
	if (    organ == ORGAN_MODIFIER_HURT ||
	        organ == ORGAN_MODIFIER_HUNGRY ||
	        organ == ORGAN_MODIFIER_DEBTPAID ||
	        organ == ORGAN_MODIFIER_NOVALIDTARGET
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
			unsigned int nSensors = 0;
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
			animals[animalIndex].body[cellLocalPositionI].sensorRange = nSensors * baseSensorRange;
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
}




//
void mutateAnimal(unsigned int animalIndex)
{

	if (extremelyFastNumberFromZeroTo(1) == 0) // don't mutate at all 50% of the time, so a population can be maintained against drift
	{



		if (extremelyFastNumberFromZeroTo(1) == 0) // 50% chance to erase an organ instead of changing or adding one
		{



			unsigned int randomnessSum = numberOfOrganTypes + 10; // number of possible things to mutate

			unsigned int chosenMutationType = extremelyFastNumberFromZeroTo(randomnessSum);
			if (chosenMutationType < numberOfOrganTypes)         // the more types of organs there are, the more often they will be chosen as mutation options.
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].organ = randomLetter();
			}





			// mutate characteristics of animal
			else if (chosenMutationType == numberOfOrganTypes )
			{
				int mutationAmount = (extremelyFastNumberFromZeroTo(2) - 1 );
				animals[animalIndex].stride += mutationAmount;
				if (animals[animalIndex].stride < 1) {animals[animalIndex].stride = 1;}
			}
			else if (chosenMutationType == numberOfOrganTypes + 1)
			{
				float mutationAmount = RNG() - 0.5f;
				animals[animalIndex].offspringEnergy += mutationAmount;
				if (animals[animalIndex].offspringEnergy < 0.0f) {animals[animalIndex].offspringEnergy = 0.0f;}
			}
			else if (chosenMutationType == numberOfOrganTypes + 2)
			{
				int mutationAmount = (extremelyFastNumberFromZeroTo(2) - 1 ) * (baseLifespan / 10);
				animals[animalIndex].lifespan += mutationAmount;
				if (animals[animalIndex].lifespan < (baseLifespan / 2)) {animals[animalIndex].lifespan = (baseLifespan / 2);}
			}



			// mutate colors of animal
			else if (chosenMutationType == numberOfOrganTypes + 3)         // the more types of organs there are, the more often they will be chosen as mutation options.
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].color.r += (RNG() - 0.5) * 0.1f;
				animals[animalIndex].body[mutantCell].color = clampColor(animals[animalIndex].body[mutantCell].color);
			}
			else if (chosenMutationType == numberOfOrganTypes + 4)
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].color.g += (RNG() - 0.5) * 0.1f;
				animals[animalIndex].body[mutantCell].color = clampColor(animals[animalIndex].body[mutantCell].color);
			}
			else if (chosenMutationType == numberOfOrganTypes + 5)
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].color.b += (RNG() - 0.5) * 0.1f;
				animals[animalIndex].body[mutantCell].color = clampColor(animals[animalIndex].body[mutantCell].color);
			}



			// mutate eye color
			else if (chosenMutationType == numberOfOrganTypes + 6)
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].eyeColor.r += (RNG() - 0.5) * 0.1f;
				animals[animalIndex].body[mutantCell].eyeColor = clampColor(animals[animalIndex].body[mutantCell].eyeColor);
			}
			else if (chosenMutationType == numberOfOrganTypes + 7)
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].eyeColor.g += (RNG() - 0.5) * 0.1f;
				animals[animalIndex].body[mutantCell].eyeColor = clampColor(animals[animalIndex].body[mutantCell].eyeColor);
			}
			else if (chosenMutationType == numberOfOrganTypes + 8)
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
				animals[animalIndex].body[mutantCell].eyeColor.b += (RNG() - 0.5) * 0.1f;
				animals[animalIndex].body[mutantCell].eyeColor = clampColor(animals[animalIndex].body[mutantCell].eyeColor);
			}


			// mutate multiplier factor
			else if (chosenMutationType == numberOfOrganTypes + 9)
			{
				unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);

				if (animals[animalIndex].body[mutantCell].organ == ORGAN_MULTIPLIER)
				{
					// if it is actually a multiplier, it can mutate between about -10 and +10
					animals[animalIndex].body[mutantCell].multiplierFactor += (RNG() - 0.5);
					if (animals[animalIndex].body[mutantCell].multiplierFactor > 10.0f)
					{
						animals[animalIndex].body[mutantCell].multiplierFactor = 10.0f;
					}
					else if (animals[animalIndex].body[mutantCell].multiplierFactor < -10.0f)
					{
						animals[animalIndex].body[mutantCell].multiplierFactor = -10.0f;
					}
				}

				if ( organIsAModifier(animals[animalIndex].body[mutantCell].organ))
				{
					// if it is actually a modifier, it can really only be positive or negative.
					if (	animals[animalIndex].body[mutantCell].multiplierFactor  == -1 || animals[animalIndex].body[mutantCell].multiplierFactor  == 1)
					{
						animals[animalIndex].body[mutantCell].multiplierFactor  = animals[animalIndex].body[mutantCell].multiplierFactor   * -1;
					}
					else
					{
						if (animals[animalIndex].body[mutantCell].multiplierFactor  < 0)
						{
							animals[animalIndex].body[mutantCell].multiplierFactor   = -1;
						}
						else
						{
							animals[animalIndex].body[mutantCell].multiplierFactor  = 1;
						}
					}


				}
			}





		}
		else
		{
			unsigned int mutantCell = extremelyFastNumberFromZeroTo(animalSquareSize - 1);
			animals[animalIndex].body[mutantCell].organ = MATERIAL_NOTHING;// randomLetter();
		}
	}
}




void spawnAnimalIntoSlot( unsigned int animalIndex,
                          Animal parent,
                          unsigned int position, bool mutation)
{
	// resetAnimal(animalIndex);

	animals[animalIndex] = parent;

	animals[animalIndex].retired = false;
	animals[animalIndex].mass = 0;
	animals[animalIndex].age = 0;
	animals[animalIndex].energyDebt = 0.0f;
	animals[animalIndex].numberOfTimesReproduced = 0;
	animals[animalIndex].damageDone = 0;
	animals[animalIndex].damageReceived = 0;


	animals[animalIndex].position = position;
	animals[animalIndex].fPosX = animals[animalIndex].position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = animals[animalIndex].position / worldSize;
	animals[animalIndex].destination = animals[animalIndex].position;
	animals[animalIndex].birthLocation = animals[animalIndex].position;


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
	case ORGAN_MOUTH:
		return color_darkgrey;
	case ORGAN_LIVER:
		return color_darkred;
	case ORGAN_BONE:
		return color_white;
	case ORGAN_WEAPON:
		return color_purple;
	case ORGAN_SENSOR_FOOD:
		return color_black;
	case ORGAN_SENSOR_LIGHT:
		return color_black;
	case ORGAN_SENSOR_CREATURE:
		return color_black;
	case ORGAN_MULTIPLIER:
		return color_pink;
	case ORGAN_MODIFIER_HURT:
		return color_pink;
	case ORGAN_MODIFIER_HUNGRY:
		return color_pink;
	case ORGAN_MODIFIER_DEBTPAID:
		return color_pink;
	case ORGAN_MODIFIER_NOVALIDTARGET:
		return color_pink;
	case ORGAN_SENSOR_RANDOM:
		return color_pink;
	case ORGAN_SENSOR_HOME:
		return color_pink;
	case ORGAN_SENSOR_PARENT:
		return color_pink;
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


	unsigned int mapUpdateFidelity = worldSquareSize / 10000;

	for (unsigned int i = 0; i < mapUpdateFidelity; ++i)
	{
		unsigned int randomX = extremelyFastNumberFromZeroTo(worldSize);
		unsigned int randomY = extremelyFastNumberFromZeroTo(worldSize);
		unsigned int randomI = (randomY * worldSize) + randomX;
		if (randomI < worldSquareSize)
		{
			if (world[randomI].terrain == TERRAIN_STONE)
			{
				world[randomI].terrain == TERRAIN_GRASS;
			}
		}

	}


}

void sensor(int animalIndex, unsigned int cellLocalPositionI)
{
	if (animalIndex == playerCreature)
	{
		return;
	}





	unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
	unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
	unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
	unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
	unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
	unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
	unsigned int sensorRange = animals[animalIndex].body[cellLocalPositionI].sensorRange;
	bool detected = false;
	unsigned int organ = animals[animalIndex].body[cellLocalPositionI].organ;
	int x = (cellWorldPositionX - sensorRange) + extremelyFastNumberFromZeroTo( sensorRange * 2);
	int y = (cellWorldPositionY - sensorRange) + extremelyFastNumberFromZeroTo( sensorRange * 2);
	if (x < worldSize && x >= 0 && y < worldSize && y >= 0)
	{
		unsigned int targetWorldPositionI =    (( y * worldSize ) + x ); // center the search area on the cell's world position.

		if (organ == ORGAN_SENSOR_RANDOM)   // random sensors just random-walk the creature.
		{
			if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
			{
				animals[animalIndex].destination += ( (extremelyFastNumberFromZeroTo(sensorRange) - (sensorRange / 2))  * worldSize  ) + (extremelyFastNumberFromZeroTo(sensorRange) - (sensorRange / 2));
			}
		}
		else if (organ == ORGAN_SENSOR_PARENT)
		{
			if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
			{
				if (animals[animalIndex].parentIdentity > 0 && animals[animalIndex].parentIdentity < numberOfAnimals)
				{
					if (! animals[animals[animalIndex].parentIdentity].retired)
					{
						targetWorldPositionI = animals[animals[animalIndex].parentIdentity].position;
						detected = true;
					}
				}
			}
		}
		else if (organ == ORGAN_SENSOR_HOME)
		{
			if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
			{
				targetWorldPositionI	 = animals[animalIndex].birthLocation;
			}
			detected = true;
		}
		else if ( organ  == ORGAN_SENSOR_LIGHT)
		{
			unsigned int virtualDestination = animals[animalIndex].destination;
			if (virtualDestination < worldSquareSize)
			{
				if (world[targetWorldPositionI].light > world[virtualDestination].light )
				{
					detected = true;
				}
			}
		}
		else if (organ == ORGAN_SENSOR_FOOD)
		{
			if (world[targetWorldPositionI].material == MATERIAL_FOOD)
			{
				detected = true;
			}
		}
		else if (organ == ORGAN_SENSOR_CREATURE)
		{
			if (world[targetWorldPositionI].identity >= 0 &&
			        world[targetWorldPositionI].identity < numberOfAnimals &&
			        world[targetWorldPositionI].identity != animalIndex)
			{
				if ( isAnimalInSquare(world[targetWorldPositionI].identity , targetWorldPositionI) > 0)
				{
					detected = true;
				}
			}
		}
		else if (organ == ORGAN_SENSOR_EYE)
		{
			Color receivedColor = whatColorIsThisSquare(targetWorldPositionI);
			Color perceivedColor = multiplyColor( receivedColor, animals[animalIndex].body[cellLocalPositionI].eyeColor  );

			float stimulus = colorAmplitude(perceivedColor );

			if (stimulus > animals[animalIndex].body[cellLocalPositionI].signalIntensity )
			{
				detected = true;
			}
		}
		if (detected)
		{
			int targetX = targetWorldPositionI % worldSize;
			int targetY = targetWorldPositionI / worldSize;
			targetX += extremelyFastNumberFromZeroTo(animalSize / 2)  - (animalSize / 4);
			targetY += (extremelyFastNumberFromZeroTo(animalSize / 2) - (animalSize  / 4)) * worldSize;









			// check if nearby modifiers are blocking the sensor, and determine the cell's sign.
			float newSign = 1.0f;
			for (unsigned int n = 0; n < nNeighbours; ++n)
			{
				unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
				if (cellNeighbour < animalSquareSize)
				{
					if (animals[animalIndex].body[cellNeighbour].organ == ORGAN_MULTIPLIER)
					{
						newSign *= animals[animalIndex].body[cellNeighbour].multiplierFactor;
					}

					if (animals[animalIndex].body[cellNeighbour].organ == ORGAN_MODIFIER_HURT)
					{
						bool valid = false;
						if (animals[animalIndex].damageReceived > (animals[animalIndex].mass / 2))
						{
							valid = true;
						}
						if (animals[animalIndex].body[cellNeighbour].multiplierFactor < 0)
						{
							valid = !valid;
						}
						if (!valid) { newSign = 0.0f; }
					}

					if (animals[animalIndex].body[cellNeighbour].organ == ORGAN_MODIFIER_HUNGRY)
					{
						bool valid = false;
						if (animals[animalIndex].energy < (animals[animalIndex].mass / 2))
						{
							valid = true;
						}
						if (animals[animalIndex].body[cellNeighbour].multiplierFactor < 0)
						{
							valid = !valid;
						}
						if (!valid) { newSign = 0.0f;}
					}

					if (animals[animalIndex].body[cellNeighbour].organ == ORGAN_MODIFIER_DEBTPAID)
					{
						bool valid = false;
						if (animals[animalIndex].energyDebt <= 0.0f)
						{
							valid = true;
						}
						if (animals[animalIndex].body[cellNeighbour].multiplierFactor < 0)
						{
							valid = !valid;
						}
						if (!valid) { newSign = 0.0f;}
					}

					if (animals[animalIndex].body[cellNeighbour].organ == ORGAN_MODIFIER_NOVALIDTARGET)
					{
						bool valid = false;
						if (animals[animalIndex].prevHighestIntensity < thresholdOfBoredom)
						{
							valid = true;
						}
						if (animals[animalIndex].body[cellNeighbour].multiplierFactor < 0)
						{
							valid = !valid;
						}
						if (!valid) { newSign = 0.0f; }
					}
				}
			}
			animals[animalIndex].body[cellLocalPositionI].sign = newSign;










			// if the sensor is inverted, mirror the destination around the sensor.
			if (animals[animalIndex].body[cellLocalPositionI].sign < 0)
			{
				int diffX = targetX - cellWorldPositionX;
				int diffY = targetY - cellWorldPositionY;
				targetX = cellWorldPositionX - diffX;
				targetY = cellWorldPositionY - diffY;
			}
			int finalTarget = (targetY * worldSize) + targetX;
			if (finalTarget < worldSquareSize)
			{
				animals[animalIndex].body[cellLocalPositionI].target = finalTarget;
				animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f * animals[animalIndex].body[cellLocalPositionI].sign;

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
			unsigned int destinationThisTurn = animals[animalIndex].destination;
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
				if (organIsASensor(organ))
				{






					sensor(animalIndex, cellLocalPositionI);

					if (animals[animalIndex].body[cellLocalPositionI].signalIntensity > highestIntensity)
					{
						highestIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity;
						animals[animalIndex].destination = animals[animalIndex].body[cellLocalPositionI].target;
					}

					animals[animalIndex].body[cellLocalPositionI].signalIntensity *= (1.0f - ( 0.1f / animals[animalIndex].stride ));

					continue;
				}
				else
				{
					switch (organ)
					{
					case ORGAN_GONAD:
					{
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
					// case ORGAN_LEAF:
					// {
					// 	if (doPhotosynth)
					// 	{
					// 		if (world[cellWorldPositionI].identity == animalIndex)
					// 		{
					// 			animals[animalIndex].energy += world[cellWorldPositionI].light * lightEnergy * energyScaleIn;
					// 		}
					// 	}
					// 	break;
					// }
					case ORGAN_MOUTH :
					{
						if (world[cellWorldPositionI].material == MATERIAL_FOOD)
						{
							animals[animalIndex].energy += foodEnergy * energyScaleIn;
							world[cellWorldPositionI].material = MATERIAL_NOTHING;
						}

						if (world[cellWorldPositionI].terrain == TERRAIN_GRASS)
						{
							animals[animalIndex].energy += grassEnergy * energyScaleIn;
							world[cellWorldPositionI].terrain = TERRAIN_STONE;
						}
						break;


					}
					case ORGAN_MUSCLE :
					{
						if (doMuscles)
						{
							int destinationX = animals[animalIndex].destination % worldSize;
							int destinationY = animals[animalIndex].destination / worldSize;
							int diffX = (destinationX - animalWorldPositionX);
							int diffY = (destinationY - animalWorldPositionY);
							float muscleX = diffX;
							float muscleY = diffY;
							float muscleSignX = 1.0f;
							float muscleSignY = 1.0f;
							if (muscleX < 0.0f) {muscleSignX = -1.0f;}
							if (muscleY < 0.0f) {muscleSignY = -1.0f;}
							muscleX = abs(muscleX);
							muscleY = abs(muscleY);
							float sum = muscleX + muscleY;
							if (sum > 0)
							{
								float xContrib = muscleX / sum;
								float yContrib = muscleY / sum;
								muscleX = (xContrib) * muscleSignX * musclePower;
								muscleY = (yContrib) * muscleSignY * musclePower;
								if (animals[animalIndex].mass != 0.0f )
								{
									animals[animalIndex].fPosX += ( muscleX ) / animals[animalIndex].mass;
									animals[animalIndex].fPosY += ( muscleY ) / animals[animalIndex].mass;
								}
								animals[animalIndex].energy -= ( abs(muscleX) + abs(muscleY)) * movementEnergyScale ;//* speciesEnergyOuts[speciesIndex];
							}
						}
						break;
					}
					}
				}
				if (cellsDone >= animals[animalIndex].mass) {break;}
			}

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

			float minimumEdgeDistance = animalSize + 1; // animals are not allowed to go close to the edge, because it makes them act funny.

			if (animals[animalIndex].fPosX < minimumEdgeDistance) {animals[animalIndex].fPosX = minimumEdgeDistance;}
			if (animals[animalIndex].fPosX > worldSize - minimumEdgeDistance) { animals[animalIndex].fPosX = worldSize - minimumEdgeDistance;}
			if (animals[animalIndex].fPosY < minimumEdgeDistance) {animals[animalIndex].fPosY = minimumEdgeDistance;}
			if (animals[animalIndex].fPosY > worldSize - minimumEdgeDistance) { animals[animalIndex].fPosY = worldSize - minimumEdgeDistance;}

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
									if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_WEAPON)
									{
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

											if (animals[world[cellWorldPositionI].identity].energyDebt <= 0.0f) // if the animal can lose the limb, and create energetic food, before the debt is paid, infinite energy can be produced.
											{
												if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
												{
													world[cellWorldPositionI].material = MATERIAL_FOOD;
												}
											}
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
						}
					if (animals[animalIndex].age > animals[animalIndex].lifespan)
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
				displayColor = whatColorIsThisSquare( worldI);
			}
			float fx = vx;
			float fy = vy;
			drawTile( Vec_f2( fx, fy ), displayColor);
		}
	}
}

void spawnPlayer()
{
	unsigned int targetWorldPositionX = cameraPositionX ;
	unsigned int targetWorldPositionY = cameraPositionY ;
	unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
	playerCreature = 0;
	killAnimal(playerCreature);
	spawnAnimalIntoSlot(playerCreature,
	                    exampleAnimal2,
	                    targetWorldPositionI, false);
	cameraTargetCreature = playerCreature;
	printf("spawned player creature\n");
}

void setupTournamentAnimals()
{
	for (unsigned int i = 0; i < numberOfAnimals;  ++i)	// initial random creatures.
	{
		unsigned int targetWorldPositionX = cameraPositionX + extremelyFastNumberFromZeroTo(viewFieldX) - (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + extremelyFastNumberFromZeroTo(viewFieldY) - (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		spawnAnimalIntoSlot(i,
		                    animals[0],
		                    targetWorldPositionI, false);

		for (int j = 0; j < animalSquareSize; ++j)
		{
			animals[i].body[j].organ = randomLetter();
		}
	}
}

void setupRandomWorld()
{
	resetAnimals();
	resetGrid();
	// setupExampleAnimal2();

	// spawn the example creature in the center field of view in an empty world.
	if (worldToLoad == WORLD_EXAMPLECREATURE)
	{
		unsigned int targetWorldPositionX = cameraPositionX + (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		unsigned int speciesIndex  = 0;
		int animalIndex = spawnAnimal(speciesIndex,
		                              exampleAnimal2,
		                              targetWorldPositionI, false);
		if (animalIndex >= 0)
		{
			cameraTargetCreature = animalIndex;
			champion = cameraTargetCreature;
		}
		unsigned int foodpos = targetWorldPositionI + (10 * worldSize) + 10;
		world[foodpos].material = MATERIAL_FOOD;
	}
	else if (worldToLoad == WORLD_RANDOM)
	{
		printf("placing materials\n");
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;
			if (extremelyFastNumberFromZeroTo(50) == 0)
			{
				world[worldPositionI].material = MATERIAL_FOOD;
			}
		}
		printf("growing materials \n");
		for (unsigned int i = 0; i < (numberOfSpecies ); ++i)
		{
			unsigned int targetWorldPositionX = cameraPositionX + extremelyFastNumberFromZeroTo(viewFieldX) - (viewFieldX / 2);
			unsigned int targetWorldPositionY = cameraPositionY + extremelyFastNumberFromZeroTo(viewFieldY) - (viewFieldY / 2);
			unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
			for (int j = 0; j < 10; ++j)
			{
				int newAnimal = spawnAnimal(i,
				                            exampleAnimal2,
				                            targetWorldPositionI, true);
			}
		}
	}

	else if (worldToLoad == WORLD_ARENA)
	{
		printf("placing materials\n");
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;
			// if (extremelyFastNumberFromZeroTo(50) == 0)
			// {
			// 	world[worldPositionI].material = MATERIAL_FOOD;
			// }


			if (x == 0 || x == worldSize - 1 || y == 0 || y == worldSize - 1)
			{

				world[worldPositionI].material = MATERIAL_ROCK;
			}


			world[worldPositionI].terrain = TERRAIN_GRASS;

		}
		printf("growing materials \n");








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

				int randomAnimal = extremelyFastNumberFromZeroTo(numberOfAnimals);
				if (randomAnimal >= 0)
				{

					unsigned int randompos = extremelyFastNumberFromZeroTo(worldSquareSize - 1);

// int spawnAnimal( unsigned int speciesIndex,
//                  Animal parent,
//                  unsigned int position, bool mutation)
					spawnAnimal( i, animals[randomAnimal],  randompos, false) ;
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

unsigned int getPlayerDestination()
{
	if (playerCreature >= 0 && playerCreature < numberOfAnimals)
	{
		return animals[playerCreature].destination;
	}
	return 0;
}
void setPlayerDestination(unsigned int newDestination)
{
	if (playerCreature >= 0 && playerCreature < numberOfAnimals)
	{
		animals[playerCreature].destination = newDestination;
	}
}

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
	menus.push_back(*exampleMenuRoot);
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