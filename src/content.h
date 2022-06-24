#ifndef CONTENT_H
#define CONTENT_H


#include "untitled_marlo_project.h"


#define MATERIAL_NOTHING           0
#define ORGAN_MOUTH_VEG            1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH_SCAVENGE       2
#define ORGAN_GONAD                3
#define ORGAN_MUSCLE               4
#define ORGAN_BONE                 5
#define ORGAN_WEAPON               6
#define ORGAN_LIVER                7
#define ORGAN_MUSCLE_TURN          8
#define ORGAN_SENSOR_EYE           9
#define ORGAN_MOUTH_CARNIVORE      10
#define ORGAN_MOUTH_PARASITE       11
#define ORGAN_ADDOFFSPRINGENERGY   12
#define ORGAN_ADDLIFESPAN          13
#define ORGAN_NEURON               15
#define ORGAN_BIASNEURON           16    // can be thought of as ORGAN_SENSOR_CONSTANTVALUE
#define ORGAN_SENSOR_BODYANGLE	   17
#define ORGAN_SENSOR_TRACKER       18
#define ORGAN_SPEAKER              19
#define ORGAN_SENSOR_EAR           20
#define ORGAN_MUSCLE_STRAFE        21
#define ORGAN_SENSOR_PHEROMONE     22
#define ORGAN_EMITTER_PHEROMONE    23
#define ORGAN_MEMORY_RX            24
#define ORGAN_MEMORY_TX            25
#define ORGAN_GILL                 26
#define ORGAN_LUNG                 27
#define ORGAN_SENSOR_HUNGER        28
#define ORGAN_SENSOR_AGE           29
#define ORGAN_SENSOR_LAST_STRANGER 30
#define ORGAN_SENSOR_LAST_KIN      31
#define ORGAN_SENSOR_PARENT        32
#define ORGAN_SENSOR_BIRTHPLACE    33
#define ORGAN_SENSOR_TOUCH         34
#define ORGAN_COLDADAPT            35
#define ORGAN_HEATADAPT            36
#define ORGAN_GRABBER              37
#define ORGAN_SENSOR_PAIN          38
#define ORGAN_HAIR                 39

#define ORGAN_GENITAL_A            40
#define ORGAN_GENITAL_B            41
#define ORGAN_SENSOR_PLEASURE      42

#define numberOfOrganTypes        42 // the number limit of growable genes

#define MARKER                    50  // there are some tiles which are used by the program, but can't be grown.
#define TILE_DESTROYER_EYE        51 


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
#define MATERIAL_FIRE              71
#define MATERIAL_SAND    72
#define MATERIAL_DIRT    73
#define MATERIAL_SOIL    74
#define MATERIAL_BASALT  75
#define MATERIAL_DUST    76
#define MATERIAL_GRAVEL  78
#define MATERIAL_FABRIC  79

#define MATERIAL_LEAF      80       // for energy capture and transmission.
#define MATERIAL_SEED      81       // to produce copies of plant.
#define MATERIAL_BUD       82       // a seed whose debt has not paid off.
#define MATERIAL_WOOD      83       
#define MATERIAL_POLLEN    84     

#define MACHINECALLBACK_PISTOL           100
#define MACHINECALLBACK_KNIFE            101
#define MACHINECALLBACK_HOSPITAL         102
#define MACHINECALLBACK_MESSAGECOMPUTER  103
#define MACHINECALLBACK_TRACKERGLASSES   104
#define MACHINECALLBACK_NEUROGLASSES     105
#define MACHINECALLBACK_ECOLOGYCOMPUTER  106
#define MACHINECALLBACK_LIGHTER          107
#define MACHINECALLBACK_DESTROYER        108

#define MUTATION_CONNECTIONWEIGHT 10006
#define MUTATION_ALTERBIAS        10007
#define MUTATION_SKINCOLOR        10009
#define MUTATION_SWITCHCONNECTION 10004
#define MUTATION_RECONNECT        10005
#define MUTATION_MOVECELL         10008
#define MUTATION_EYELOOK          10011
#define MUTATION_ERASEORGAN       10002
#define MUTATION_ADDORGAN         10003
#define MUTATION_SPEAKERCHANNEL   10010


#define VISUALIZER_TRUECOLOR           1001
#define VISUALIZER_TRACKS              1003
#define VISUALIZER_IDENTITY            1004
#define VISUALIZER_NEURALACTIVITY      1006




// plant genes must be higher than the highest nNeighbour but must be lower than the maximum size of a char (255).

#define PLANTGENE_GROW_SYMM_H 9
#define PLANTGENE_GROW_SYMM_V 10
#define PLANTGENE_BUD         11
#define PLANTGENE_LEAF        12
#define PLANTGENE_WOOD        13
#define PLANTGENE_GOTO        14
#define PLANTGENE_SEQUENCE    15
#define PLANTGENE_BREAK		  16

#define PLANTGENE_RED     17
#define PLANTGENE_GREEN   18
#define PLANTGENE_BLUE    19
#define PLANTGENE_LIGHT   20
#define PLANTGENE_DARK    21

// #define PLANTGENE_SEED_RED     22
// #define PLANTGENE_SEED_GREEN   23
// #define PLANTGENE_SEED_BLUE    24
// #define PLANTGENE_SEED_LIGHT   25
// #define PLANTGENE_SEED_DARK    26

#define numberOfPlantGenes 26

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
	std::string( "It smells like the rain after a hot summer day." ),
	std::string( "It smells like the salt air at the beach." ),
	std::string( "It smells like the perfume of a frangipani's flower." ),
	std::string( "It smells like pool chlorine." ),
	std::string( "It smells like electricity." ),
	std::string( "It smells like rotting meat. Yuck!" ),
	std::string( "You can smell raspberries. Incredible!" ),
	std::string( "It smells like vomit. Disgusting." ),
};

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
		return std::string("A sensitive gland filled with potential offspring.");
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
		return std::string("A part of the brain which remembers meeting a stranger.");
	}
	case ORGAN_SENSOR_LAST_KIN:
	{
		return std::string("A part of the brain which carries the memory of a friend.");
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




	case ORGAN_SENSOR_PAIN:
	{
		return std::string("A part of the brain which feels the trauma of injury.");
	}
	case ORGAN_HEATADAPT:
	{
		return std::string("A gland which secretes liquid crystal to raise the boiling point.");
	}
	case ORGAN_COLDADAPT:
	{
		return std::string("A gland which produces syrup to lower the boiling point.");
	}
	case ORGAN_HAIR:
	{
		return std::string("Soft, shiny strands of hair.");
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
	case MATERIAL_WATER:
	{
		return std::string("Cold clear water.");
	}


	case MATERIAL_SAND:
	{
		return std::string("Fine-grained sand.");
	}
	case MATERIAL_DIRT:
	{
		return std::string("Dry sandy dirt.");
	}
	case MATERIAL_SOIL:
	{
		return std::string("Rich living soil.");
	}
	case MATERIAL_BASALT:
	{
		return std::string("Hard volcanic rock.");
	}
	case MATERIAL_DUST:
	{
		return std::string("Dry alkaline dust.");
	}
	case MATERIAL_GRAVEL:
	{
		return std::string("Scattered rubble.");
	}

	case MATERIAL_FABRIC:
	{
		return std::string("Soft fibre woven into a mesh.");
	}



case MATERIAL_SEED:
	{
		return std::string("A seed of a plant.");
	}


case MATERIAL_BUD:
	{
		return std::string("A plant's flowers.");
	}


case MATERIAL_WOOD:
	{
		return std::string("A branch with bark.");
	}


case MATERIAL_LEAF:
	{
		return std::string("Leaves and twigs.");
	}



case MATERIAL_POLLEN:
	{
		return std::string("Fragrant, dusty pollen.");
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


	case ORGAN_SENSOR_PAIN:
	{
		return std::string("Pain sensor");
	}

	case ORGAN_COLDADAPT:
	{
		return std::string("Cold adapt");
	}

	case ORGAN_HEATADAPT:
	{
		return std::string("Heat adapt");
	}

	case ORGAN_HAIR:
	{
		return std::string("Hair");
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
	case MATERIAL_WATER:
	{
		return std::string("Water.");
	}
	case MATERIAL_SAND:
	{
		return std::string("Sand.");
	}
	case MATERIAL_DIRT:
	{
		return std::string("Dirt.");
	}
	case MATERIAL_SOIL:
	{
		return std::string("Soil.");
	}
	case MATERIAL_BASALT:
	{
		return std::string("Basalt.");
	}
	case MATERIAL_DUST:
	{
		return std::string("Dust.");
	}
	case MATERIAL_GRAVEL:
	{
		return std::string("Gravel.");
	}
	case MATERIAL_FABRIC:
	{
		return std::string("Fabric.");
	}


	case MATERIAL_SEED:
	{
		return std::string("Seed.");
	}


case MATERIAL_BUD:
	{
		return std::string("Flower.");
	}


case MATERIAL_WOOD:
	{
		return std::string("Wood.");
	}


case MATERIAL_LEAF:
	{
		return std::string("Leaf.");
	}



case MATERIAL_POLLEN:
	{
		return std::string("Pollen.");
	}





	}
	return std::string("Unknown");
}


float organGrowthCost(unsigned int organ)
{
	// float growthCost = 0.0f;
	// growthCost = growthEnergyScale;
	if (variedGrowthCost)
	{
		switch (organ)
		{
		case ORGAN_MUSCLE:
			// growthCost *= 1.0f;
			// break;
			return 1.0f;
		case ORGAN_BONE:
			return 1.0f;
		case ORGAN_WEAPON:
			return 1.0f;
		case ORGAN_GONAD:
			return 1.0f;
		case ORGAN_MOUTH_VEG:
			return 1.0f;
		case ORGAN_MOUTH_SCAVENGE:
			
			return 1.0f;
		case ORGAN_MOUTH_CARNIVORE:
			
			return 1.0f;
		case ORGAN_MOUTH_PARASITE:
			
			return 1.0f;
		}
	}

	return 1.0f;
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


bool organUsesSpeakerChannel(unsigned int organ)
{
	if (    organ == ORGAN_SENSOR_PHEROMONE ||
	        organ == ORGAN_EMITTER_PHEROMONE ||
	        organ == ORGAN_MEMORY_TX ||
	        organ == ORGAN_MEMORY_RX  ||
	        organ == ORGAN_SPEAKER ||
	        organ == ORGAN_SENSOR_EAR)
	{
		return true;
	}
	return false;
}

bool organIsAnActuator(unsigned int organ)
{
	if (    organ == ORGAN_MUSCLE ||
	        organ == ORGAN_MUSCLE_TURN ||
	        organ == ORGAN_MUSCLE_STRAFE ||
	        organ == ORGAN_SPEAKER  ||
	        organ == ORGAN_EMITTER_PHEROMONE ||
	        organ == ORGAN_MEMORY_TX)
	{
		return true;
	}
	return false;
}

bool organIsANeuron(unsigned int organ)
{
	if (    organ == ORGAN_NEURON ||
	        organ == ORGAN_BIASNEURON )
	{
		return true;
	}
	return false;
}

bool organIsASensor(unsigned int organ)
{
	if (organ == ORGAN_SENSOR_EYE ||
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
	        organ == ORGAN_SENSOR_PARENT ||
	        organ == ORGAN_SENSOR_PAIN ||
	        organ == ORGAN_SENSOR_PLEASURE
	        )
	{
		return true;
	}
	return false;
}

bool isCellConnecting(unsigned int organ)
{
	if ( organIsAnActuator(organ) ||
	        organIsANeuron(organ) )
	{
		return true;
	}
	return false;
}

bool isCellConnectable(unsigned int organ)
{
	if (organIsASensor(organ) ||
	        organIsANeuron(organ))
	{
		return true;
	}
	return false;
}

void setupExampleAnimal2(int i, bool underwater)
{
	// set the example back to the default state or it wont work properly.
	resetAnimal(i);
	if (underwater)
	{

	animalAppendCell( i, ORGAN_GILL );
	}
	else
	{

	animalAppendCell( i, ORGAN_LUNG );
	}
	animalAppendCell( i, ORGAN_LIVER );
	animalAppendCell( i, ORGAN_LIVER );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );
	animalAppendCell( i, ORGAN_GONAD );

	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EYE );
	animalAppendCell( i, ORGAN_SENSOR_EAR );
	animalAppendCell( i, ORGAN_SENSOR_PHEROMONE );
	animalAppendCell( i, ORGAN_SENSOR_TRACKER );

	animalAppendCell( i, ORGAN_SENSOR_AGE );
	animalAppendCell( i, ORGAN_SENSOR_HUNGER );

	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_BIASNEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );
	animalAppendCell( i, ORGAN_NEURON );

	animalAppendCell( i, ORGAN_MUSCLE );
	animalAppendCell( i, ORGAN_MUSCLE );
	animalAppendCell( i, ORGAN_MUSCLE_STRAFE );
	animalAppendCell( i, ORGAN_MUSCLE_STRAFE );
	animalAppendCell( i, ORGAN_MUSCLE_TURN );
	animalAppendCell( i, ORGAN_MUSCLE_TURN );
	animalAppendCell( i, ORGAN_EMITTER_PHEROMONE );
	animalAppendCell( i, ORGAN_SPEAKER );

	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );
	animalAppendCell( i, ORGAN_MOUTH_VEG );


}







void setupExampleLighter(int i)
{
	// game.animals[i].isMachine = true;
	// game.animals[i].machineCallback = MACHINECALLBACK_LIGHTER;
	char lighter[] =
	{
		'1', ' ',
		'1', '1',
		'1', '1',
		'1', '1',
	};
	setupCreatureFromCharArray( i, lighter, 8, 2 , std::string("lighter"), MACHINECALLBACK_LIGHTER);
}

void setupExampleHuman(int i)
{
	// resetAnimal(i);
	// std::string gunDescription = std::string("human");
	// strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );
	char humanBody[] =
	{
		' ', ' ', ' ', '/', 'B', '/', ' ', ' ', ' ',
		' ', ' ', '/', 'B', 'B', 'B', '/', ' ', ' ',
		' ', ' ', 'B', 'B', 'B', 'B', 'B', '/', ' ',
		' ', '/', 'B', 'B', 'B', 'B', 'B', '/', ' ',
		' ', 'R', 'B', 'E', 'B', 'E', 'B', 'R', ' ',
		' ', '/', '/', 'B', 'N', 'B', '/', '/', ' ',
		' ', ' ', '/', '/', 'S', '/', '/', ' ', ' ',
		' ', ' ', ' ', 'B', 'M', 'B', ' ', ' ', ' ',
		' ', 'B', 'B', 'B', 'B', 'B', 'B', 'B', ' ',
		'B', 'M', 'M', 'U', 'B', 'U', 'M', 'M', 'B',
		'B', 'M', 'B', 'B', 'B', 'B', 'B', 'M', 'B',
		'B', '/', 'M', 'X', 'B', 'H', 'M', '/', 'B',
		'B', ' ', 'B', 'B', 'B', 'B', 'B', ' ', 'B',
		'M', ' ', 'T', 'M', 'B', 'M', 'T', ' ', 'M',
		'B', ' ', ' ', 'B', 'B', 'B', ' ', ' ', 'B',
		'B', ' ', ' ', 'L', 'B', 'L', ' ', ' ', 'B',
		'B', ' ', 'M', 'B', 'B', 'B', 'M', ' ', 'B',
		'G', ' ', 'B', 'O', 'B', 'O', 'B', ' ', 'G',
		' ', 'M', 'A', 'B', 'D', 'B', 'A', 'M', ' ',
		' ', 'M', 'B', 'M', '/', 'M', 'B', 'M', ' ',
		' ', 'M', 'B', 'M', ' ', 'M', 'B', 'M', ' ',
		' ', 'M', 'B', 'M', ' ', 'M', 'B', 'M', ' ',
		' ', 'M', 'B', 'M', ' ', 'M', 'B', 'M', ' ',
		' ', 'M', 'M', 'M', ' ', 'M', 'M', 'M', ' ',
		' ', ' ', 'B', 'M', ' ', 'M', 'B', ' ', ' ',
		' ', 'M', 'B', 'M', ' ', 'M', 'B', 'M', ' ',
		' ', 'M', 'B', ' ', ' ', ' ', 'B', 'M', ' ',
		' ', 'M', 'B', ' ', ' ', ' ', 'B', 'M', ' ',
		' ', 'M', 'B', ' ', ' ', ' ', 'B', 'M', ' ',
		' ', ' ', 'B', ' ', ' ', ' ', 'B', ' ', ' ',
		' ', ' ', 'B', ' ', ' ', ' ', 'B', ' ', ' ',
		' ', ' ', 'A', ' ', ' ', ' ', 'A', ' ', ' ',

	};
	setupCreatureFromCharArray( i, humanBody, (9 * 33), 9 ,  std::string("human"), -1 );




	char humanPaint[] =
	{
		' ', ' ', ' ', 'R', 'R', 'R', ' ', ' ', ' ', 
		' ', ' ', 'R', 'R', 'R', 'R', 'R', ' ', ' ', 
		' ', 'R', 'R', 'R', 'T', 'T', 'R', 'R', ' ', 
		' ', 'R', 'R', 'T', 'T', 'T', 'T', 'R', ' ', 
		' ', 'P', 'R', 'B', 'T', 'B', 'T', 'P', ' ', 
		' ', 'R', 'R', 'T', 'P', 'T', 'R', 'R', ' ', 
		' ', ' ', 'R', 'R', 'T', 'R', 'R', ' ', ' ', 
		' ', ' ', ' ', 'T', 'T', 'T', ' ', ' ', ' ', 
		' ', 'T', 'T', 'T', 'T', 'T', 'T', 'T', ' ', 
		'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 
		'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 
		'T', 'R', 'P', 'T', 'T', 'T', 'P', 'R', 'T', 
		'T', ' ', 'T', 'T', 'T', 'T', 'T', ' ', 'T', 
		'T', ' ', 'T', 'T', 'T', 'T', 'T', ' ', 'T', 
		'T', ' ', ' ', 'T', 'T', 'T', ' ', ' ', 'T', 
		'T', ' ', ' ', 'T', 'P', 'T', ' ', ' ', 'T', 
		'T', ' ', 'T', 'T', 'T', 'T', 'T', ' ', 'T', 
		'P', ' ', 'T', 'T', 'T', 'T', 'T', ' ', 'P', 
		' ', 'T', 'T', 'R', 'R', 'R', 'T', 'T', ' ', 
		' ', 'T', 'T', 'T', 'R', 'T', 'T', 'T', ' ', 
		' ', 'T', 'T', 'T', ' ', 'T', 'T', 'T', ' ', 
		' ', 'T', 'T', 'T', ' ', 'T', 'T', 'T', ' ', 
		' ', 'T', 'T', 'T', ' ', 'T', 'T', 'T', ' ', 
		' ', 'T', 'T', 'T', ' ', 'T', 'T', 'T', ' ', 
		' ', ' ', 'T', 'T', ' ', 'T', 'T', ' ', ' ', 
		' ', 'T', 'T', 'T', ' ', 'T', 'T', 'T', ' ', 
		' ', 'T', 'T', ' ', ' ', ' ', 'T', 'T', ' ', 
		' ', 'T', 'T', ' ', ' ', ' ', 'T', 'T', ' ', 
		' ', 'T', 'T', ' ', ' ', ' ', 'T', 'T', ' ', 
		' ', ' ', 'T', ' ', ' ', ' ', 'T', ' ', ' ', 
		' ', ' ', 'T', ' ', ' ', ' ', 'T', ' ', ' ', 
		' ', ' ', 'P', ' ', ' ', ' ', 'P', ' ', ' ', 

	};







	paintCreatureFromCharArray( i, humanPaint, (9*33), 9 );
}




// void paintExampleHuman(int i)
// {
// 	// resetAnimal(i);
// 	// std::string gunDescription = std::string("human");
// 	// strcpy( &game.animals[i].displayName[0] , gunDescription.c_str() );

// }



void setupDestroyer(int i)
{
	char destroyer[] =
	{
		' ', ' ', '1', ' ', ' ',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		'1', '1', '1', '1', '1',
		'1', '1', '3', '1', '1',
		'1', '1', '1', '1', '1',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		' ', '1', '1', '1', ' ',
		' ', ' ', '1', ' ', ' ',
		' ', ' ', '1', ' ', ' ',
		' ', ' ', '1', ' ', ' ',
		' ', ' ', '1', ' ', ' ',
		' ', ' ', '1', ' ', ' ',
		' ', ' ', '1', ' ', ' ',
		' ', ' ', '1', ' ', ' ',
	};
	setupCreatureFromCharArray( i, destroyer, 20*5, 5 , std::string("destroyer") , MACHINECALLBACK_DESTROYER);
}




void setupTrackerGlasses(int i)
{
	char trackerGlasses[] =
	{
		' ', '2', ' ', '2', ' ',
		'2', '2', '1', '2', '2',
		' ', '2', ' ', '2', ' ',
	};
	setupCreatureFromCharArray( i, trackerGlasses, 15, 5 , std::string("tracker glasses") , MACHINECALLBACK_TRACKERGLASSES);
}

void setupNeuroGlasses(int i)
{
	char neuroGlasses[] =
	{
		'1', '1', ' ', '1', '1',
		'1', '2', '1', '2', '1',
		'1', '1', ' ', '1', '1',
	};
	setupCreatureFromCharArray( i, neuroGlasses, 15, 5 , std::string("tracker glasses") , MACHINECALLBACK_NEUROGLASSES);
}

void setupExampleGun(int i)
{
	char pistol[] =
	{
		'1', '1', '1', '1',
		'1', '1', '1', '1',
		' ', '1', '1', ' ',
		'1', '1', ' ', ' ',
	};
	setupCreatureFromCharArray( i, pistol, 16, 4 , std::string("pistol") , MACHINECALLBACK_PISTOL);
}

void setupExampleKnife(int i)
{
	char knife[] =
	{
		' ', '1', ' ', ' ',
		'1', '1', '1', '1',
		' ', '1', ' ', ' ',
	};
	setupCreatureFromCharArray( i, knife, 12, 4 , std::string("knife") , MACHINECALLBACK_KNIFE);

}


void setupEcologyCompter(int i)
{
	char computer[] =
	{
		'1', '1', '1', '1', '1',
		'1', '2', '2', '2', '1',
		'1', '2', '2', '2', '1',
		'1', '1', '1', '1', '1',
		' ', ' ', '1', ' ', ' ',
		'1', '1', '1', '1', '1',
	};
	setupCreatureFromCharArray( i, computer, 30, 5 , std::string("ecology terminal") , MACHINECALLBACK_ECOLOGYCOMPUTER);
}

void setupMessageComputer(int i)
{
	char computer[] =
	{
		'2', '2', '2', '2', '2',
		'2', '2', '2', '2', '2',
		'2', '2', '2', '2', '2',
		'2', '2', '2', '2', '2',
		' ', ' ', '1', ' ', ' ',
		'1', '1', '1', '1', '1',
	};
	setupCreatureFromCharArray( i, computer, 30, 5 , std::string("message terminal") , MACHINECALLBACK_MESSAGECOMPUTER);
}

void setupHospitalComputer(int i)
{
	char hospital[] =
	{
		' ', '2', ' ',
		'2', '2', '2',
		'2', '2', '2',
		'2', '2', '2',
		' ', '2', ' ',
		'2', '2', '2',
	};
	setupCreatureFromCharArray( i, hospital, 18, 3 , std::string("hospital terminal") , MACHINECALLBACK_HOSPITAL);
}










bool materialBlocksMovement(unsigned int material)
{
	if (material == MATERIAL_ROCK ||
	        material == MATERIAL_VOIDMETAL ||
	        material == MATERIAL_METAL ||
	        material == MATERIAL_BASALT
	   )
	{
		return true;
	}
	return false;
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



bool materialSupportsGrowth(unsigned int material)
{
	if (material == MATERIAL_ROCK ||
	        material == MATERIAL_SAND   ||
	        material == MATERIAL_DIRT   ||
	        material == MATERIAL_SOIL   ||
	        material == MATERIAL_GRAVEL)
	{
		return true;
	}
	return false;
}




float materialFertility(unsigned int material)
{
	// if (material == MATERIAL_ROCK ||
	//         material == MATERIAL_SAND   ||
	//         material == MATERIAL_DIRT   ||
	//         material == MATERIAL_SOIL   ||
	//         material == MATERIAL_GRAVEL)
	// {
	// 	return true;
	// }
	// return false;



	switch (material)
	{
	case MATERIAL_ROCK:
		return 0.1f;
	case MATERIAL_SAND:
		return 0.125f;
	case MATERIAL_DIRT:
		return 1.0f;
	case MATERIAL_SOIL:
		return 0.5f;
	case MATERIAL_GRAVEL:
		return 0.125f;

	}
	return 0.0f;
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
	case MATERIAL_METAL:
		return color_darkgrey;
	case MATERIAL_VOIDMETAL:
		return color_charcoal;
	case MATERIAL_SMOKE:
		return color_lightgrey;
	case MATERIAL_GLASS:
		return color_lightblue;
	case MATERIAL_WATER:
		return color_blue_thirdClear;
	case MATERIAL_FIRE:
		return color_orange;
	case MATERIAL_SAND:
		return color_tan;
	case MATERIAL_DIRT:
		return color_lightbrown;
	case MATERIAL_SOIL:
		return color_brown;
	case MATERIAL_BASALT:
		return color_darkgrey;
	case MATERIAL_DUST:
		return color_lightgrey;
	case MATERIAL_GRAVEL:
		return color_grey;
	}
	return color_yellow;
}

Color organColors(unsigned int organ)
{
	switch (organ)
	{
	case ORGAN_MOUTH_VEG            :
		return color_charcoal;
	case ORGAN_MOUTH_SCAVENGE       :
		return color_charcoal;
	case ORGAN_GONAD                :
		return color_cream;
	case ORGAN_MUSCLE               :
		return color_darkred;
	case ORGAN_BONE                 :
		return color_offwhite;
	case ORGAN_WEAPON               :
		return color_offwhite;
	case ORGAN_LIVER                :
		return color_puce;
	case ORGAN_MUSCLE_TURN          :
		return color_muscles1;
	case ORGAN_SENSOR_EYE           :
		return color_charcoal;
	case ORGAN_MOUTH_CARNIVORE      :
		return color_charcoal;
	case ORGAN_MOUTH_PARASITE       :
		return color_charcoal;
	case ORGAN_ADDOFFSPRINGENERGY   :
		return color_brains1;
	case ORGAN_ADDLIFESPAN          :
		return color_brains2;
	case ORGAN_NEURON               :
		return color_brains3;
	case ORGAN_BIASNEURON           :
		return color_brains4;
	case ORGAN_SENSOR_BODYANGLE	    :
		return color_tan;
	case ORGAN_SENSOR_TRACKER       :
		break;
	case ORGAN_SPEAKER              :
		return color_offwhite;
	case ORGAN_SENSOR_EAR           :
		return color_charcoal;
	case ORGAN_MUSCLE_STRAFE        :
		return color_muscles2;
	case ORGAN_SENSOR_PHEROMONE     :
		return color_charcoal;
	case ORGAN_EMITTER_PHEROMONE    :
		return color_peach_light;
	case ORGAN_MEMORY_RX            :
		return color_brains1;
	case ORGAN_MEMORY_TX            :
		return color_brains2;
	case ORGAN_GILL                 :
		return color_brightred;
	case ORGAN_LUNG                 :
		return color_lungs1;
	case ORGAN_SENSOR_HUNGER        :
		return color_brains1;
	case ORGAN_SENSOR_AGE           :
		return color_brains2;
	case ORGAN_SENSOR_LAST_STRANGER :
		return color_brains3;
	case ORGAN_SENSOR_LAST_KIN      :
		return color_brains1;
	case ORGAN_SENSOR_PARENT        :
		return color_brains2;
	case ORGAN_SENSOR_BIRTHPLACE    :
		return color_brains3;
	case ORGAN_SENSOR_TOUCH         :
		return color_tan;
	case ORGAN_COLDADAPT            :
		return color_violet;
	case ORGAN_HEATADAPT            :
		return color_peach;
	case ORGAN_GRABBER              :
		return color_peach_light;
	case ORGAN_HAIR              :
		return color_clear;
	}
	return color_yellow;
}

bool organVisible(unsigned int organ)
{
	if (organ == ORGAN_MOUTH_VEG ||
	        organ == ORGAN_MOUTH_SCAVENGE ||
	        organ == ORGAN_SENSOR_EYE ||
	        organ == ORGAN_MOUTH_CARNIVORE ||
	        organ == ORGAN_MOUTH_PARASITE ||
	        organ == ORGAN_SENSOR_TRACKER ||
	        organ == ORGAN_SPEAKER )
	{
		return true;
	}
	return false;
}


#endif //CONTENT_H