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

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

#include "utilities.h"
#include "graphics.h"
#include "untitled_marlo_project.h"


// float RNG()
// {
// 	static std::default_random_engine e;
// 	e.seed(std::chrono::system_clock::now().time_since_epoch().count());
// 	static std::uniform_real_distribution<> dis(0, 1);
// 	return dis(e);
// }
// unsigned int rn_x = 1;
// unsigned int rn_y = 0;
// unsigned int rn_z = 1;
// unsigned int rn_a = 0;

// inline unsigned int extremelyFastRandomNumber()
// {
// 	unsigned int t = rn_x ^ (rn_x << 8);
// 	rn_x = rn_y;
// 	rn_y = rn_z;
// 	rn_z = rn_a;
// 	rn_a = rn_z ^ t ^ ( rn_z >> 1) ^ (t << 1);
// 	return rn_a;
// }

// inline unsigned int extremelyFastNumberFromZeroTo( unsigned int to)
// {
// 	return ( extremelyFastRandomNumber() % ( to + 1 ) );
// }

#define MATERIAL_NOTHING          0
#define ORGAN_LEAF                1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
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
#define ORGAN_SENSOR_INVERT       12
#define ORGAN_SENSOR_PARENT       13
#define ORGAN_SENSOR_HOME         14
#define CONDITION_GREATEROREQUAL         15
#define CONDITION_EQUAL           16
#define CONDITION_LESSOREQUAL            17
#define GROW_LIFESPAN             18 // W   // these genes are meta instructions that control how the gene sequence should be read.
#define GROW_END                  19 // X   
#define GROW_ADDOFFSPRINGENERGY   20 // Y    
#define GROW_STRIDE               21 // Z    
#define GROW_SEQUENCE             22 // [  
#define GROW_JUMP                 23
#define GROW_STOP                 24
#define GROW_COLORA_R
#define GROW_COLORA_G
#define GROW_COLORA_B
#define GROW_COLORB_R
#define GROW_COLORB_G
#define GROW_COLORB_B
#define MATERIAL_FOOD             25 //           
#define MATERIAL_ROCK             26 //           
#define MATERIAL_WATER            27 //           
#define MARKER                    28 //      // numbers above 25 don't correspond to lower-case letters(0..25) so we don't use them in the gene code. But (26..31) are still compatible with our masking scheme.


#define WORLD_RANDOM 1
#define WORLD_EXAMPLECREATURE 2

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
const bool sexualViolence        = false;
const bool variedGrowthCost      = true;
const bool variedUpkeep          = true;

unsigned int worldToLoad = WORLD_RANDOM;

// const unsigned int viewFieldX = 203; // 80 columns, 24 rows is the default size of a terminal window
// const unsigned int viewFieldY = 55 - 3;  // 203 columns, 55 rows is the max size i can make one on my pc.


const unsigned int viewFieldX = 512; //80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 512; //203 columns, 55 rows is the max size i can make one on my pc.


const unsigned int viewFieldSize = viewFieldX * viewFieldY;
const int animalSize     = 16;
const unsigned int animalSquareSize      = animalSize * animalSize;
// const int worldSize      = 512;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int genomeSize      = 16;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfSpecies = 10;
unsigned int numberOfAnimalsPerSpecies = (numberOfAnimals / numberOfSpecies);
const unsigned int nNeighbours     = 8;
const float growthEnergyScale      = 1.0f;        // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.01f;        // a multiplier for how much it costs animals just to exist.
const float lightEnergy            = 0.0101f;   // how much energy an animal gains each turn from having a leaf. if tax is by mass, must be higher than taxEnergyScale to be worth having leaves at all.
const float movementEnergyScale    = 0.02f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.5f;                     // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float liverStorage = 10.0f;
const unsigned int baseLifespan = 30000;
const unsigned int baseSensorRange = 10;
const float signalPropagationConstant = 0.1f; // how strongly sensor organs compel the animal.
float energyScaleIn             = 1.0f;     // a multiplier for how much energy is gained from food and light.
float minimumEntropy = 0.1f;
// float energyScaleOut           = minimumEntropy;


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

unsigned int cameraPositionX = (worldSize / 2);
unsigned int cameraPositionY = (worldSize / 2);
unsigned int modelFrameCount = 0;

int champion = -1;
char championGenes[genomeSize];
int championScore = 0;
int tournamentInterval = 10000;
int tournamentCounter  = 0;
// int numberOfKnights = 12;

int cameraTargetCreature = -1;
unsigned int threadTimer = 0;
unsigned int populationCount = 0;
unsigned int cameraFrameCount = 0;

struct Square
{
	unsigned int material;
	int identity;
	int height;
	float light;
};

struct Square world[worldSquareSize];

struct Cell
{
	unsigned int organ;
	unsigned int geneCursor;
	unsigned int grown;
	unsigned int origin;
	unsigned int sequenceNumber;
	unsigned int crystalCondition;
	unsigned int crystalN;
	int sign;
	int sensorRange;
	float signalIntensity;
	unsigned int target;
	Color outerColor;
	float damage;
};

struct Animal
{
	Cell body[animalSquareSize];
	char genes[genomeSize];
	unsigned int mass;
	unsigned int stride;
	unsigned int numberOfTimesReproduced;
	unsigned int damageDone;
	unsigned int damageReceived;
	unsigned int birthLocation;
	unsigned int age;
	unsigned int lifespan;
	int parentIdentity;
	bool retired;
	float fangle;
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
	Color colorA;
	Color colorB;
};

float speciesEnergyOuts              [numberOfSpecies];
unsigned int speciesPopulationCounts [numberOfSpecies];
unsigned int populationCountUpdates  [numberOfSpecies];

float organGrowthCost(unsigned int organ)
{
	float growthCost = 0.0f;
	if (growingCostsEnergy)
	{
		growthCost = 1.0f;
		if (variedGrowthCost)
		{
			switch (organ)
			{
			case ORGAN_LEAF:
				growthCost *= 1.0f;
				break;
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
		case ORGAN_LEAF:
			upkeepCost *= 0.0f;
			break;
		case ORGAN_BONE:
			upkeepCost *= 0.0f;
			break;
		case ORGAN_WEAPON:
			upkeepCost *= 0.0f;
			break;
		case ORGAN_MOUTH:
			upkeepCost *= 0.5f;
			break;
		case ORGAN_MUSCLE:
			upkeepCost *= 1.0f;
			break;
		case ORGAN_GONAD:
			upkeepCost *= 2.0f;
			break;
		case ORGAN_LIVER:
			upkeepCost *= 1.0f;
			break;
		case ORGAN_SENSOR_FOOD:
			upkeepCost *= 3.0f;
			break;
		case ORGAN_SENSOR_LIGHT:
			upkeepCost *= 3.0f;
			break;
		case ORGAN_SENSOR_CREATURE:
			upkeepCost *= 3.0f;
			break;
		}
	}

	return upkeepCost;
}

char exampleAnimal[genomeSize] ;
Animal animals[numberOfAnimals];

void resetAnimal(unsigned int animalIndex)
{
	if (animalIndex >= 0 && animalIndex < numberOfAnimals)
	{
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
			animals[animalIndex].body[cellLocalPositionI].geneCursor = 0;
			animals[animalIndex].body[cellLocalPositionI].grown = true;
			animals[animalIndex].body[cellLocalPositionI].origin = 0;
			animals[animalIndex].body[cellLocalPositionI].sign = 1;
			animals[animalIndex].body[cellLocalPositionI].sensorRange = baseSensorRange;
			animals[animalIndex].body[cellLocalPositionI].crystalCondition = CONDITION_GREATEROREQUAL;
			animals[animalIndex].body[cellLocalPositionI].crystalN = 0;
		}
		memset( &(animals[animalIndex].genes[0]), 0x00, genomeSize );
		animals[animalIndex].offspringEnergy = 1.0f;
		animals[animalIndex].energy   = 0.0f;
		animals[animalIndex].energyDebt   = 0.0f;
		animals[animalIndex].mass = 0;
		animals[animalIndex].stride = 1;
		animals[animalIndex].fPosX = 0.0f;
		animals[animalIndex].fPosY = 0.0f;
		animals[animalIndex].fangle   = 0.0f;
		animals[animalIndex].position = 0;
		animals[animalIndex].uPosX = 0;
		animals[animalIndex].uPosY = 0;
		animals[animalIndex].parentIdentity = -1;
		animals[animalIndex].numberOfTimesReproduced = 0;
		animals[animalIndex].retired = true;
		animals[animalIndex].damageDone = 0;
		animals[animalIndex].damageReceived = 0;
		animals[animalIndex].birthLocation = 0;
		animals[animalIndex].age = 0;
		animals[animalIndex].lifespan = baseLifespan;
		animals[animalIndex].destination = 0;
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

char geneCodeToChar( unsigned int n )
{
	return (char)('A' + n);
}

unsigned int charToGeneCode( char c )
{
	return (unsigned int)(c - 'A');
}

char randomLetter()
{
	return geneCodeToChar (extremelyFastNumberFromZeroTo(31));
}

void examplePlant (unsigned int animalIndex)
{
	for (int i = 0; i < genomeSize; ++i)
	{
		animals[animalIndex].genes[i] = geneCodeToChar( MATERIAL_NOTHING);
	}
	animals[animalIndex].genes[1] = geneCodeToChar( CONDITION_EQUAL );
	animals[animalIndex].genes[2] = 'A' + 1 ;//geneCodeToChar( CONDITION_EQUAL );
	animals[animalIndex].genes[3] = geneCodeToChar( ORGAN_LEAF );
	animals[animalIndex].genes[4] = geneCodeToChar( ORGAN_LEAF);
	animals[animalIndex].genes[5] = geneCodeToChar( ORGAN_LEAF);
	animals[animalIndex].genes[6] = geneCodeToChar( ORGAN_GONAD);
	animals[animalIndex].genes[7] = geneCodeToChar( ORGAN_GONAD);
	animals[animalIndex].energy = 1.0f;
}

void setupExampleAnimal(unsigned int animalIndex)
{
	for (int i = 0; i < genomeSize; ++i)
	{
		animals[animalIndex].genes[i] = geneCodeToChar( MATERIAL_NOTHING);
	}
	animals[animalIndex].genes[1] = geneCodeToChar( CONDITION_EQUAL );
	animals[animalIndex].genes[2] = 'A' + 1 ;//geneCodeToChar( CONDITION_EQUAL );
	animals[animalIndex].genes[3] = geneCodeToChar( ORGAN_MUSCLE);
	animals[animalIndex].genes[4] = geneCodeToChar( ORGAN_MUSCLE);
	animals[animalIndex].genes[5] = geneCodeToChar( ORGAN_SENSOR_RANDOM);
	animals[animalIndex].genes[6] = geneCodeToChar( ORGAN_MOUTH );
	animals[animalIndex].genes[7] = geneCodeToChar( ORGAN_WEAPON );
	animals[animalIndex].genes[8] = geneCodeToChar( ORGAN_GONAD );
	animals[animalIndex].energy = 1.0f;
}

void setupRandomCreature(unsigned int animalIndex)
{
	for (int i = 0; i < genomeSize; ++i)
	{
		animals[animalIndex].genes[i] = randomLetter();//geneCodeToChar( MATERIAL_NOTHING);
	}
	animals[animalIndex].energy = 1.0f;
}

void grow( int animalIndex, unsigned int cellLocalPositionI)
{
	animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
	if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
	unsigned int gene =  charToGeneCode( (animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ]) );

	if (gene == GROW_END)
	{
		unsigned int origin =   animals[animalIndex].body[cellLocalPositionI].origin  ;
		int sequenceNumber = 	animals[animalIndex].body[   origin    ].sequenceNumber;
		if (sequenceNumber == 0)
		{
			animals[animalIndex].body[cellLocalPositionI].grown = true;
			animals[animalIndex].body[    origin      ].grown = false;
			animals[animalIndex].body[    origin      ].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor;
		}
		else  if (sequenceNumber > 0)                                                                   // if sequence number > 0, you are in a sequence, start the sequence over again.
		{
			animals[animalIndex].body[  origin     ].sequenceNumber--;
			animals[animalIndex].body[cellLocalPositionI] = animals[animalIndex].body[    origin    ]; // copy the origin cell to the neighbour and begin the sequence again.
			animals[animalIndex].body[cellLocalPositionI].origin  = origin;                            // except, the new cell's origin is still set to this sequence's origin, not the origin's origin.
		}
		return;
	}
	else if (gene == GROW_ADDOFFSPRINGENERGY)	// increase the amount of energy given to newborn offspring.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].offspringEnergy  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		return;
	}
	else if (gene == GROW_STRIDE) // increase how often the animal's path is updated, which affects its random path length.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].stride  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		return;
	}
	else if (gene == GROW_LIFESPAN) // set how long the animal lives
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].lifespan  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] ) * baseLifespan;
		return;
	}
	else if (gene == GROW_SEQUENCE) // repeat a motif a number of times in a row.
	{
		// whenever a break is encountered at a sequence number higher than 0, the new growing cell's properties are copied from the origin, and the sequence number is decremented.
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].body[cellLocalPositionI].sequenceNumber  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		animals[animalIndex].body[cellLocalPositionI].origin     = cellLocalPositionI;
		return;
	}
	else if (gene == GROW_JUMP) // increase how often the animal's path is updated, which affects its random path length.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].body[cellLocalPositionI].geneCursor  += (charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] ) - 13 ) ;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		return;
	}
	else if (gene == GROW_STOP || gene == MATERIAL_NOTHING) // increase how often the animal's path is updated, which affects its random path length.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor = genomeSize;
		animals[animalIndex].body[cellLocalPositionI].grown = true;
		return;
	}
	else if (gene == CONDITION_GREATEROREQUAL || gene == CONDITION_EQUAL || gene == CONDITION_LESSOREQUAL)	// increase the amount of energy given to newborn offspring.
	{
		animals[animalIndex].body[cellLocalPositionI].crystalCondition = gene;
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].body[cellLocalPositionI]. crystalN  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		return;
	}
	else                                                                                                                               // if the gene is a growable organ, grow it in this square, mark the square as completed, and then journey on to the activated neighbours
	{
		unsigned int occupiedNeighbours = 0;
		for (unsigned int n = 0; n < nNeighbours; ++n)
		{
			unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
			if (cellNeighbour < animalSquareSize)
			{
				if (animals[animalIndex].body[cellNeighbour].organ == gene)
				{
					animals[animalIndex].body[cellLocalPositionI].sensorRange += baseSensorRange;
					animals[animalIndex].body[cellNeighbour].sensorRange = animals[animalIndex].body[cellLocalPositionI].sensorRange;
				}
				if (animals[animalIndex].body[cellNeighbour].organ == ORGAN_SENSOR_INVERT)
				{
					animals[animalIndex].body[cellLocalPositionI].sign = -1;
				}
				if (gene == ORGAN_SENSOR_INVERT)
				{
					animals[animalIndex].body[cellNeighbour].sign = -1;
				}
				if (animals[animalIndex].body[cellNeighbour].organ == MATERIAL_NOTHING )
				{
					animals[animalIndex].body[cellNeighbour].origin               = animals[animalIndex].body[cellLocalPositionI].origin ;
					animals[animalIndex].body[cellNeighbour].geneCursor           = animals[animalIndex].body[cellLocalPositionI].geneCursor ;
					animals[animalIndex].body[cellNeighbour]. crystalN            = animals[animalIndex].body[cellLocalPositionI].crystalN;
					animals[animalIndex].body[cellNeighbour]. crystalCondition    = animals[animalIndex].body[cellLocalPositionI].crystalCondition;
					animals[animalIndex].body[cellNeighbour].grown = false;
				}
				else
				{
					occupiedNeighbours++;
				}
			}
		}
		bool readyToGrow = false;
		if (animals[animalIndex].body[cellLocalPositionI]. crystalCondition == CONDITION_GREATEROREQUAL )
		{
			if (occupiedNeighbours >= animals[animalIndex].body[cellLocalPositionI].crystalN)
			{
				readyToGrow = true;
			}
		}
		else if (animals[animalIndex].body[cellLocalPositionI]. crystalCondition == CONDITION_EQUAL )
		{
			if (occupiedNeighbours == animals[animalIndex].body[cellLocalPositionI].crystalN)
			{
				readyToGrow = true;
			}
		}
		else if (animals[animalIndex].body[cellLocalPositionI]. crystalCondition == CONDITION_LESSOREQUAL )
		{
			if (occupiedNeighbours <= animals[animalIndex].body[cellLocalPositionI].crystalN)
			{
				readyToGrow = true;
			}
		}
		if (	readyToGrow )
		{
			animals[animalIndex].body[cellLocalPositionI].organ = gene;
			animals[animalIndex].mass++;
			animals[animalIndex].energyDebt += organGrowthCost(gene);
			animals[animalIndex].body[cellLocalPositionI].grown = true;
		}
		return;
	}
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

void mutateGenes( int animalIndex)
{
	unsigned int mutationType = extremelyFastNumberFromZeroTo(3);
	if (mutationType == 0)  // randomise a letter
	{
		unsigned int geneIndex = 1 + extremelyFastNumberFromZeroTo(genomeSize - 1);
		animals[animalIndex].genes[geneIndex] = randomLetter();
	}
	else if (mutationType == 1)  // insert a random letter at a random place, pushing subsequent letters back
	{
		unsigned int geneIndex = 1 + extremelyFastNumberFromZeroTo(genomeSize - 1);
		for (unsigned int i = genomeSize; i > geneIndex; --i)
		{
			animals[animalIndex].genes[i] = animals[animalIndex].genes[i - 1];
		}
		animals[animalIndex].genes[geneIndex] = randomLetter();
	}
	else if (mutationType == 2)  // delete a letter at a random place, pulling subsequent letters forward
	{
		unsigned int geneIndex = 1 +  extremelyFastNumberFromZeroTo(genomeSize - 2);
		for (unsigned int i = (geneIndex + 1); i < genomeSize; ++i)
		{
			animals[animalIndex].genes[i - 1] = animals[animalIndex].genes[i];
		}
	}
	else if (mutationType == 3)  // swap the order of two adjacent letters
	{
		unsigned int geneIndex = 1 +  extremelyFastNumberFromZeroTo(genomeSize - 2);
		char temp = animals[animalIndex].genes[geneIndex];
		animals[animalIndex].genes[geneIndex] = animals[animalIndex].genes[geneIndex + 1];
		animals[animalIndex].genes[geneIndex + 1] = temp;
	}
}

void spawnAnimalIntoSlot( unsigned int animalIndex,  char * genes, unsigned int position, bool mutation)
{
	resetAnimal(animalIndex);
	animals[animalIndex].retired = false;
	unsigned int cellLocalPositionX = animalSize / 2; // place a seed in the middle of the sprite
	unsigned int cellLocalPositionY = animalSize / 2;
	unsigned int cellLocalPositionI = (cellLocalPositionY * animalSize) + cellLocalPositionX;
	animals[animalIndex].body[cellLocalPositionI].origin = cellLocalPositionI;
	animals[animalIndex].body[cellLocalPositionI].organ = animals[animalIndex].body[cellLocalPositionI].organ = ORGAN_LEAF;
	animals[animalIndex].body[cellLocalPositionI].grown = false;
	animals[animalIndex].body[cellLocalPositionI].geneCursor = 0;
	animals[animalIndex].mass = 1;
	memcpy (  &( animals[animalIndex].genes[0] ), genes, genomeSize * sizeof(char)  ); // transfer the parent entire gene code
	if (mutation)
	{
		if (extremelyFastNumberFromZeroTo(1) == 0) // don't mutate at all 50% of the time, so a population can be maintained against drift
		{
			mutateGenes(animalIndex);
		}
	}
	animals[animalIndex].position = position;
	animals[animalIndex].fPosX = animals[animalIndex].position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = animals[animalIndex].position / worldSize;
	animals[animalIndex].destination = animals[animalIndex].position;
	animals[animalIndex].birthLocation = animals[animalIndex].position;
}

int spawnAnimal( unsigned int speciesIndex, char * genes, unsigned int position, bool mutation)
{
	int animalIndex = getNewIdentity(speciesIndex);
	if (animalIndex >= 0) // an animalIndex was available
	{
		spawnAnimalIntoSlot(animalIndex, genes, position, mutation);
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

int igetRelativeDistance(unsigned int a, unsigned int b)
{
	int ia = a;
	int ib = b;
	int aX =  ia % worldSize;
	int aY =  ia / worldSize;
	int bX =  ib % worldSize;
	int bY =  ib / worldSize;
	int diffX = bX - aX;
	int diffY = bY - aY;
	int result = (abs(diffX) + abs(diffY));
	return result;
}

// find the angle in radians between two world positions
float getRelativeDirection (unsigned int a, unsigned int b)
{
	int ia = a;
	int ib = b;
	int aX =  ia % worldSize;
	int aY =  ia / worldSize;
	int bX =  ib % worldSize;
	int bY =  ib / worldSize;
	int diffX = bX - aX;
	int diffY = bY - aY;
	float fdiffX = diffX;
	float fdiffY = diffY;
	float angle = atan2(fdiffY, fdiffX);
	return angle;
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionX,  unsigned int cellWorldPositionY,  unsigned int cellWorldPositionI)
{
	if (world[cellWorldPositionI].identity >= 0 )
	{
		if (!animals[animalIndex].retired)
		{
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

void sensor(int animalIndex, unsigned int cellLocalPositionI)
{

	if (animalIndex == playerCreature)
	{
		animals[animalIndex].destination = animals[animalIndex].position;
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

	if (x < worldSize && x > 0 && y < worldSize && y > 0)
	{
		unsigned int targetWorldPositionI =    (( y * worldSize ) + x ); // center the search area on the cell's world position.
		// world[targetWorldPositionI].material = MATERIAL_ROCK;

		if (animalIndex == playerCreature)
		{
			if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
			{

				targetWorldPositionI = animals[animalIndex].position;



				detected = true;


			}
		}
		else if (organ == ORGAN_SENSOR_RANDOM)   // random sensors just random-walk the creature.
		{
			if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
			{

				// world[targetWorldPositionI].material = MATERIAL_ROCK;

				x =  (animalWorldPositionX - sensorRange) + extremelyFastNumberFromZeroTo( sensorRange * 2); // re center on the animal so it doesnt tend to one direction
				y =  (animalWorldPositionY - sensorRange) + extremelyFastNumberFromZeroTo( sensorRange * 2);

				// unsigned int adjustedRandomPositionX =  animalWorldPositionX + x   ;// x - cellLocalPositionX;
				// unsigned int adjustedRandomPositionY =  animalWorldPositionY + y   ;// y - cellLocalPositionY;

				// unsigned int randompos = (animalWorldPositionY + extremelyFastNumberFromZeroTo(animals[animalIndex].stride)) *


				targetWorldPositionI	= (y * worldSize) + x      ; // RNG() * worldSquareSize;
				detected  = true;
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
				if ( isAnimalInSquare(world[targetWorldPositionI].identity , x, y, targetWorldPositionI) > 0)
				{
					detected = true;
				}
			}
		}

		if (detected)
		{



			int targetX = targetWorldPositionI % worldSize;
			int targetY = targetWorldPositionI / worldSize;


			targetX += extremelyFastNumberFromZeroTo(animalSize / 2)  - (animalSize / 4);
			targetY += (extremelyFastNumberFromZeroTo(animalSize / 2) - (animalSize  / 4)) * worldSize;


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
				// world[finalTarget].material = MATERIAL_ROCK;
				animals[animalIndex].body[cellLocalPositionI].target = finalTarget;
				animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;








				// get the x and y components of the current destination.
				int destinationX = animals[animalIndex].destination % worldSize;
				int destinationY = animals[animalIndex].destination / worldSize;


				// int targetX = animals[animalIndex].body[cellLocalPositionI].target % worldSize;
				// int targetY = animals[animalIndex].body[cellLocalPositionI].target / worldSize;


				// drag it toward the sensor's target according to how strong the signal is.
				destinationX += (targetX - destinationX) * animals[animalIndex].body[cellLocalPositionI].signalIntensity * signalPropagationConstant;
				destinationY += (targetY - destinationY) * animals[animalIndex].body[cellLocalPositionI].signalIntensity * signalPropagationConstant;

				// printf("DESITNATIONX %i Y %i \n", destinationX, destinationY);
				// while(true)
				// 	{;}

				// apply the change.
				int newDestination = (destinationY * worldSize) + destinationX;

				if (newDestination < worldSquareSize && newDestination > 0)
				{
					animals[animalIndex].destination = newDestination;
				}




			}

		}
		else
		{
			// animals[animalIndex].body[cellLocalPositionI].signalIntensity *= 0.99f;
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

void grow_all()
{
	// grow all growable cells.
	// unsigned int newpop = 0;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{


		// if (animalIndex > populationCount) {break;}

		if (!animals[animalIndex].retired)
		{
			// newpop++;
			if (animals[animalIndex].energyDebt > 0.0f)
			{
				if (animals[animalIndex].energy > animals[animalIndex].maxEnergy / 2.0f)
				{
					float amount = animals[animalIndex].energy - (animals[animalIndex].maxEnergy / 2.0f);
					animals[animalIndex].energyDebt -= amount;
					animals[animalIndex].energy     -= amount;
				}
			}
			else
			{
				for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI) // process organs and signals and clear animalIndex on grid
				{
					if (!(animals[animalIndex].body[cellLocalPositionI].grown))
					{
						grow( animalIndex,  cellLocalPositionI);
					}
				}
			}
		}
	}
	// populationCount = newpop;
}

void organs_all()
{
	// perform organ functionality.
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{


		// if (animalIndex > populationCount) {break;}

		unsigned int speciesIndex = animalIndex / numberOfAnimalsPerSpecies;
		if (!animals[animalIndex].retired)
		{
			unsigned int cellsDone = 0;
			float totalLiver = 0;
			for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				if (animals[animalIndex].body[cellLocalPositionI].organ == MATERIAL_NOTHING)
				{continue;}
				else
				{cellsDone++;}

				unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
				unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
				unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
				unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
				unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX ;//- (animalSize / 2);
				unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY ;//- (animalSize / 2);
				unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
				if (cellWorldPositionI >= worldSquareSize) {continue;}

				unsigned int organ = animals[animalIndex].body[cellLocalPositionI].organ;


				if (
				    organ == ORGAN_SENSOR_FOOD ||
				    organ == ORGAN_SENSOR_LIGHT ||
				    organ == ORGAN_SENSOR_CREATURE ||
				    organ == ORGAN_SENSOR_RANDOM ||
				    organ == ORGAN_SENSOR_INVERT ||
				    organ == ORGAN_SENSOR_HOME ||
				    organ == ORGAN_SENSOR_PARENT
				)
				{
					sensor(animalIndex, cellLocalPositionI);




					continue;
				}
				else
				{
					switch (organ)
					{
					case ORGAN_GONAD:
					{
						if (doReproduction && animals[animalIndex].body[cellLocalPositionI].grown )
						{
							if (animals[animalIndex].energy > ((animals[animalIndex].mass / 2 ) + animals[animalIndex].offspringEnergy ))
							{
								if (cellWorldPositionI < worldSquareSize)
								{
									unsigned int adjustedPos = cellWorldPositionI - (    ( (animalSize / 2) * worldSize)   + animalSize / 2           ) ;
									if (adjustedPos < worldSquareSize)
									{
										unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;

										int result = spawnAnimal( speciesIndex, animals[animalIndex].genes , adjustedPos, true );
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
					case ORGAN_LEAF:
					{
						if (doPhotosynth)
						{
							if (world[cellWorldPositionI].identity == animalIndex)
							{
								animals[animalIndex].energy += world[cellWorldPositionI].light * lightEnergy * energyScaleIn;
							}
						}
						break;
					}
					case ORGAN_MOUTH :
					{
						if (world[cellWorldPositionI].material == MATERIAL_FOOD
						        // && world[cellWorldPositionI].identity == animalIndex
						   )
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
							int destinationX = animals[animalIndex].destination % worldSize;
							int destinationY = animals[animalIndex].destination / worldSize;
							float muscleX = 0.0f;
							float muscleY = 0.0f;
							int diffX = (destinationX - animalWorldPositionX);
							int diffY = (destinationY - animalWorldPositionY);
							// float diffSum = abs(diffX) + abs(diffY);

							// printf("DOFFX %i Y %i\n", diffX, diffY);


							// if (diffSum != 0.0f)
							// {
							muscleX = diffX;// / diffSum;
							muscleY = diffY;// / diffSum;
							// }

							if (muscleX > 1.0f) { muscleX = 1.0f;}
							else if (muscleX < -1.0f) { muscleX = -1.0f;}

							if (muscleY > 1.0f) { muscleY = 1.0f;}
							else if (muscleY < -1.0f) { muscleY = -1.0f;}



							// printf("MUSCLESX %f Y %f\n", muscleX, muscleY);
							if (animals[animalIndex].mass > 0)
							{
								animals[animalIndex].fPosX += ( muscleX ) / animals[animalIndex].mass;
								animals[animalIndex].fPosY += ( muscleY ) / animals[animalIndex].mass;
							}
							animals[animalIndex].energy -= ( abs(muscleX) + abs(muscleY)) * movementEnergyScale * speciesEnergyOuts[speciesIndex];
						}
						break;
					}
					}
				}
				if (cellsDone >= animals[animalIndex].mass) {break;}
			}
			animals[animalIndex].maxEnergy = animals[animalIndex].mass + (totalLiver * liverStorage);

		}
	}
}

void move_all() // perform movement, feeding, and combat.
{

	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		// if (animalIndex > populationCount) {break;}

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
			if (animals[animalIndex].fPosX < 0.0f) {animals[animalIndex].fPosX = worldSize;}
			else if (animals[animalIndex].fPosX > worldSize) { animals[animalIndex].fPosX = 0;}
			if (animals[animalIndex].fPosY < 0.0f) {animals[animalIndex].fPosY = worldSize;}
			else if (animals[animalIndex].fPosY > worldSize) { animals[animalIndex].fPosY = 0;}
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
								animals[animalIndex].energy -= taxEnergyScale * speciesEnergyOuts[speciesIndex] * organUpkeepCost(animals[animalIndex].body[cellLocalPositionI].organ);
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
								int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI);
								if (targetLocalPositionI >= 0)
								{
									okToStep = false;
									if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_WEAPON)
									{
										int defense = defenseAtPoint(world[cellWorldPositionI].identity, targetLocalPositionI);
										if (extremelyFastNumberFromZeroTo(defense) == 0)
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
												world[cellWorldPositionI].material = MATERIAL_FOOD;
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
		}
	}
}

void energy_all() // perform energies.
{


	// for (int i = 0; i < numberOfSpecies; ++i)
	// {
	// 	speciesPopulationCounts [i] = 0;
	// }




	for (int i = 0; i < numberOfSpecies; ++i)
	{
		populationCountUpdates[i] = 0;
	}


	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{

		unsigned int speciesIndex  = animalIndex / numberOfAnimalsPerSpecies;



		if (!animals[animalIndex].retired && speciesIndex < numberOfSpecies)
		{
			// newpop ++;
			populationCountUpdates[speciesIndex]++;

			animals[animalIndex].age++;
			if (animals[animalIndex].energy > animals[animalIndex].maxEnergy) {animals[animalIndex].energy = animals[animalIndex].maxEnergy;}


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

					if (animals[animalIndex].energy < 0.0f)
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
					if (animals[animalIndex].age > animals[animalIndex].lifespan)
					{
						execute = true;
					}


				}
			}

			if (execute)
			{
				killAnimal( animalIndex);
				// return;
			}




			if (tournament)
			{
				int animalScore = animals[animalIndex].damageDone + animals[animalIndex].damageReceived ;
				if ( animalScore > championScore)
				{
					championScore = animalScore;
					champion = animalIndex;
					for (int i = 0; i < genomeSize; ++i)
					{
						championGenes[i] = animals[animalIndex].genes[i];
					}
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
		boost::thread t7{ grow_all   };
		boost::thread t8{ organs_all };
		boost::thread t9{ move_all   };
		boost::thread t10{ energy_all };
		t10.join();
		t9.join();
		t8.join();
		t7.join();
	}
	else
	{
		grow_all();
		organs_all();
		move_all();
		energy_all();
	}
}

void camera()
{
	for ( int vy = viewFieldY - 1; vy >= 0; --vy) // correct for Y axis inversion
	{
		for ( int vx = 0; vx < viewFieldX; ++vx)
		{



			if (cameraFollowsPlayer && playerCreature >= 0 )
			{
				cameraTargetCreature = playerCreature;
				// viewPanX = 0.0f;
				// viewPanY = 0.0f;
				viewPanSetpointX = 0.0f;
				viewPanSetpointY = 0.0f;
			}
			else if (cameraFollowsChampion && champion >= 0)
			{
				cameraTargetCreature = champion;

				viewPanSetpointX = 0.0f;
				viewPanSetpointY = 0.0f;
			}
			// else
			// {
			// 	for (int i = 0; i < numberOfAnimals; ++i)
			// 	{
			// 		if (!animals[i].retired)
			// 		{
			// 			cameraTargetCreature = i;
			// 		}
			// 	}
			// }



			if (cameraTargetCreature >= 0)
			{
				int creatureX = animals[cameraTargetCreature].position;
				int creatureY = animals[cameraTargetCreature].position;

				int newCameraPositionX = creatureX;// - (viewFieldX / 2) % worldSize; // allow the camera position to wrap around the world edge
				int newCameraPositionY = creatureY;// - (viewFieldY / 2) % worldSize;

				// if (newCameraPositionX > 0 && newCameraPositionX < worldSize && newCameraPositionY > 0 && newCameraPositionY < worldSize)
				// {
				cameraPositionX = newCameraPositionX;
				cameraPositionY = newCameraPositionY;
				// }
				if (animals[cameraTargetCreature].retired)
				{
					cameraTargetCreature = -1;
				}
			}





			int worldX = (cameraPositionX + vx) % worldSize; // center the view on the targeted position, instead of having it in the corner
			int worldY = (cameraPositionY + vy) % worldSize;

			// if (worldX < 0 || worldX > worldSize || worldY < 0 || worldY > worldSize) { continue;} // prevent the view from wrapping around the world edge

			int worldI = (worldY * worldSize) + worldX;









			char displayChar = ' ';
			Color displayColor = color_black;


			if (worldX > 0 && worldX < worldSize && worldY > 0 && worldY < worldSize)
			{
				if (worldI < worldSquareSize)
				{
					if (world[worldI].material == MATERIAL_ROCK)
					{
						displayChar = '#';
						displayColor = color_brown;
					}
					else if (world[worldI].material == MATERIAL_FOOD)
					{
						displayChar = '@';
						displayColor = color_yellow;
					}
					else if (world[worldI].material == MATERIAL_WATER)
					{
						displayChar = '~';
						displayColor = color_lightblue;
					}

					if (world[worldI].identity > -1)
					{
						if (world[worldI].identity == cameraTargetCreature)
						{
							displayChar = '-';
							displayColor = color_grey;

							if (world[worldI].material == MATERIAL_ROCK)
							{
								displayChar = '#';
								displayColor = color_brown;
							}
							else if (world[worldI].material == MATERIAL_FOOD)
							{
								displayChar = '@';
								displayColor = color_yellow;
							}
							else if (world[worldI].material == MATERIAL_WATER)
							{
								displayChar = '~';
								displayColor = color_lightblue;
							}
						}
						else
						{
							displayChar = '_';

							displayColor = color_darkgrey;
						}
						if (world[worldI].identity < numberOfAnimals)
						{
							if (!animals[world[worldI].identity].retired)
							{
								int targetLocalPositionI = isAnimalInSquare(world[worldI].identity , worldX, worldY, worldI);

								if (targetLocalPositionI >= 0)
								{
									if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ != MATERIAL_NOTHING))
									{
										displayChar = '?';
										displayColor = color_purple;
									}
									if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == 0x00))
									{
										displayChar = '0';
									}
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_LIVER)           { displayColor = color_darkred; displayChar = 'L'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_BONE)            { displayColor = color_white; displayChar = 'B'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_WEAPON)          { displayColor = color_purple; displayChar = 'W'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_LEAF)            { displayColor = color_green; displayChar = 'P'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_MOUTH)           { displayColor = color_black;  displayChar = 'O'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_GONAD)           { displayColor = color_offwhite; displayChar = 'G'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_MUSCLE)          { displayColor = color_brightred; displayChar = 'M'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_FOOD)     { displayColor = color_darkgrey; displayChar = 'F'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_CREATURE) { displayColor = color_darkgrey; displayChar = 'C'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_LIGHT)    { displayColor = color_darkgrey; displayChar = 'Y'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_RANDOM)   { displayColor = color_pink; displayChar = 'R'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_INVERT)   { displayColor = color_pink; displayChar = 'I'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_HOME)     { displayColor = color_pink; displayChar = 'H'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_PARENT)   { displayColor = color_pink; displayChar = 'K'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_END)                { displayChar = '!'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_LIFESPAN)             { displayChar = '@'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_STRIDE)             { displayChar = '#'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_SEQUENCE)           { displayChar = '$'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_ADDOFFSPRINGENERGY) { displayChar = '%'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == MATERIAL_ROCK)           { displayChar = '&'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == MATERIAL_WATER)          { displayChar = '*'; }
									// if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == MARKER)                  { displayChar = '('; }
								}
							}
						}
					}
				}
			}

			// print borders (this is useful to see if the image is distorted- if the border is not a clean square)
			if (vx == 0 || vx == viewFieldX - 1 )
			{
				displayChar = '|';
			}
			if (vy == 0 || vy == viewFieldY - 1 )
			{
				displayChar = '_';
			}

			// printf("%c", displayChar);



			float fvx = vx;
			float fvy = vy;
			drawTile( Vec_f2(fvx, fvy) , displayColor);


		}
		// printf("\n");
	}

	float usPerFrame = threadTimer;

	float fps = 1.0f;
	if (!lockfps && usPerFrame > 0)
	{
		fps = (1000000.0f / usPerFrame) ;
	}


	unsigned int speciesIndex = cameraTargetCreature / numberOfAnimalsPerSpecies;


	// printf("speciesIndex %u, numberOfAnimals %u, numberOfSpecies %u \n", speciesIndex, numberOfAnimals, numberOfSpecies);

	// if (speciesIndex < numberOfSpecies)
	// {


	// 	printf( "x%u y%u, %f turns/s | t. %u of %u | species %i, %u/%u animals, %f in, %f out | world %u/%u animals  |  \n | animal %i energy %f of %f, debt %f, dmgdlt %u, dmgrecv %u, position x%u y%u destination x%u y%u | \n",
	// 	        cameraPositionX, cameraPositionY, fps,  tournamentCounter, tournamentInterval,
	// 	        speciesIndex,  speciesPopulationCounts[speciesIndex], numberOfAnimalsPerSpecies,	         energyScaleIn, speciesEnergyOuts[speciesIndex],
	// 	        populationCount, numberOfAnimals,
	// 	        cameraTargetCreature, animals[cameraTargetCreature].energy,
	// 	        animals[cameraTargetCreature].maxEnergy , animals[cameraTargetCreature].energyDebt,  animals[cameraTargetCreature].damageDone, animals[cameraTargetCreature].damageReceived,
	// 	        animals[cameraTargetCreature].position % worldSize, animals[cameraTargetCreature].position / worldSize,
	// 	        animals[cameraTargetCreature].destination % worldSize, animals[cameraTargetCreature].destination / worldSize);
	// }
}

void populationController()
{

	unsigned int newPopulationCount = 0;
	for (int speciesIndex = 0; speciesIndex < numberOfSpecies; ++speciesIndex)
	{

		newPopulationCount += speciesPopulationCounts[speciesIndex] ;
		float populationDifference = speciesPopulationCounts[speciesIndex] ;
		speciesEnergyOuts[speciesIndex] = tan (   ((populationDifference) / numberOfAnimalsPerSpecies ) * 0.5f * 3.1415f   )  ;

	}

	populationCount = newPopulationCount;





}

// void regenerateKnights()
// {
// 	for (unsigned int i = 0; i < numberOfKnights; ++i)	// initial random creatures.
// 	{
// 		unsigned int targetWorldPositionX = extremelyFastNumberFromZeroTo(worldSize - 1);
// 		unsigned int targetWorldPositionY = extremelyFastNumberFromZeroTo(worldSize - 1);
// 		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
// 		spawnAnimalIntoSlot( i,  championGenes, targetWorldPositionI, true);
// 		if (champion < 0 || champion > numberOfAnimals)
// 		{
// 			for (int j = 0; j < genomeSize; ++j)
// 			{
// 				animals[i].genes[j] = randomLetter();
// 			}
// 		}
// 	}
// }





void spawnPlayer()
{
	unsigned int targetWorldPositionX = cameraPositionX ;
	unsigned int targetWorldPositionY = cameraPositionY ;
	unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
	playerCreature = 0;
	killAnimal(playerCreature);
	spawnAnimalIntoSlot(playerCreature, exampleAnimal, targetWorldPositionI, false);
	setupExampleAnimal(playerCreature);

	printf("spawned player creature\n");
}


void setupTournamentAnimals()
{
	for (unsigned int i = 0; i < (numberOfSpecies ); ++i)	// initial random creatures.
	{


		unsigned int targetWorldPositionX = cameraPositionX + extremelyFastNumberFromZeroTo(viewFieldX) - (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + extremelyFastNumberFromZeroTo(viewFieldY) - (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;

		// unsigned int speciesIndex  = i / (numberOfAnimals / numberOfSpecies);

		// if (speciesIndex < numberOfSpecies)
		// {



		int newAnimal = spawnAnimal(i,  exampleAnimal, targetWorldPositionI, true);
		if (newAnimal >= 0)
		{

			if (extremelyFastNumberFromZeroTo(1) == 0)
			{
				setupExampleAnimal(newAnimal);

			}
			else
			{

				examplePlant(newAnimal);
			}

		}
		// }
	}
}

void setupRandomWorld()
{
	resetAnimals();
	resetGrid();

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	// srand((unsigned int)time(NULL));
	// rn_x = 1;
	// rn_y = 0;
	// rn_z = 1;
	// rn_a = 0;
	// if (RNG() < 0.5f) {rn_x = !rn_x;}
	// if (RNG() < 0.5f) {rn_y = !rn_y;}
	// if (RNG() < 0.5f) {rn_z = !rn_z;}
	// if (RNG() < 0.5f) {rn_a = !rn_a;}

	// spawn the example creature in the center field of view in an empty world.
	if (worldToLoad == WORLD_EXAMPLECREATURE)
	{
		unsigned int targetWorldPositionX = cameraPositionX + (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;

		unsigned int speciesIndex  = 0;//animalIndex / numberOfSpecies;

		int animalIndex = spawnAnimal(speciesIndex, exampleAnimal, targetWorldPositionI, false);
		if (animalIndex >= 0)
		{
			cameraTargetCreature = animalIndex;
			setupExampleAnimal(animalIndex);
			// examplePlant(animalIndex);
			champion = cameraTargetCreature;
		}
		unsigned int foodpos = targetWorldPositionI + (10 * worldSize) + 10;
		world[foodpos].material = MATERIAL_FOOD;
	}
	else if (worldToLoad == WORLD_RANDOM)
	{
		printf("placing materials\n");
		// initial random materials.
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;

			// if (extremelyFastNumberFromZeroTo(1000) == 0)
			// {
			// 	world[worldPositionI].material = MATERIAL_LIGHT;
			// 	for (unsigned int n = 0; n < nNeighbours; ++n)
			// 	{
			// 		unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
			// 		if (worldNeighbour < worldSquareSize)
			// 		{
			// 			world[worldNeighbour].material = MATERIAL_LIGHT;
			// 		}
			// 	}
			// }
			if (extremelyFastNumberFromZeroTo(50) == 0)
			{
				world[worldPositionI].material = MATERIAL_FOOD;
			}
		}

		printf("growing materials \n");
		// for (int i = 0; i < 10; ++i)
		// {

		// 	printf("%i\n", i);
		// 	// expand the light
		// 	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		// 	{
		// 		unsigned int nRockNeighbours = 0;
		// 		for (unsigned int n = 0; n < nNeighbours; ++n)
		// 		{
		// 			unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
		// 			if (worldNeighbour < worldSquareSize)
		// 			{
		// 				if (world[worldNeighbour].material == MATERIAL_LIGHT)
		// 				{
		// 					nRockNeighbours++;
		// 				}
		// 			}
		// 		}
		// 		if (nRockNeighbours == 1) {  world[worldPositionI].material = MARKER;}
		// 	}
		// 	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		// 	{
		// 		if (world[worldPositionI].material == MARKER)
		// 		{
		// 			world[worldPositionI].material = MATERIAL_LIGHT;
		// 		}
		// 	}
		// }

		setupTournamentAnimals();
		// regenerateKnights();
	}
}

void tournamentController()
{
	if (tournamentCounter >= tournamentInterval )
	{
		tournamentCounter = 0;
		// regenerateKnights();
	}
	else
	{
		tournamentCounter++;
	}
}

void model()
{
	// ZoneScoped;
	auto start = std::chrono::steady_clock::now();
	computeAllAnimalsOneTurn();
	populationController();
	if (tournament)
	{
		tournamentController();
	}
	modelFrameCount++;
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	threadTimer = elapsed.count();
}

// void interfaceSupervisor()
// {
// 	while (true)
// 	{
// 		if (lockfps)
// 		{
// 			model();
// 		}
// 		// camera();
// 		usleep(50000);
// 		cameraFrameCount++;
// 	}
// }

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
	// boost::thread t7{ interfaceSupervisor };
	boost::thread t7{ modelSupervisor };
	// modelSupervisor();
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