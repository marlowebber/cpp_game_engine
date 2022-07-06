#ifndef CONTENT_H
#define CONTENT_H


#include "untitled_marlo_project.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

#define MATERIAL_NOTHING           0
#define ORGAN_MOUTH_VEG            1   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH_SCAVENGE       2
#define ORGAN_GONAD                3
#define ORGAN_MUSCLE               4
#define ORGAN_BONE                 5
#define ORGAN_CLAW                 6
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
#define ORGAN_EMITTER_WAX          35
#define ORGAN_EMITTER_HONEY        36
#define ORGAN_GRABBER              37
#define ORGAN_SENSOR_PAIN          38
#define ORGAN_HAIR                 39

#define ORGAN_GENITAL_A            40
#define ORGAN_GENITAL_B            41

#define ORGAN_SENSOR_RANDOM       43
#define ORGAN_LOCATIONREMEMBERER     44

#define ORGAN_SWITCH     45
#define ORGAN_COMPARATOR 46
#define ORGAN_ABSOLUTE   47
#define ORGAN_IIRLOW     48
#define ORGAN_IIRHIGH    49
#define ORGAN_DERIVATOR    50
#define ORGAN_ADDER      51


#define ORGAN_TIMER      52



#define numberOfOrganTypes        52 // the number limit of growable genes

#define MARKER                    65  // there are some tiles which are used by the program, but can't be grown.
#define TILE_DESTROYER_EYE        66


#define MATERIAL_FOOD             160
#define MATERIAL_ROCK             161
// #define MATERIAL_MEAT             62
#define MATERIAL_BONE             163
#define MATERIAL_BLOOD            164
#define MATERIAL_GRASS            165
#define MATERIAL_METAL            166
#define MATERIAL_VOIDMETAL        167
#define MATERIAL_SMOKE           168
#define MATERIAL_GLASS            169
#define MATERIAL_WATER            170
#define MATERIAL_FIRE              171
#define MATERIAL_SAND    172
#define MATERIAL_DIRT    173
#define MATERIAL_SOIL    174
#define MATERIAL_BASALT  175
#define MATERIAL_DUST    176
#define MATERIAL_GRAVEL  178
#define MATERIAL_FABRIC  179


#define MATERIAL_LEAF      180       // for energy capture and transmission.
#define MATERIAL_SEED      181       // to produce copies of plant.
#define MATERIAL_BUD       182       // a seed whose debt has not paid off.
#define MATERIAL_WOOD      183
#define MATERIAL_POLLEN    184


#define MATERIAL_SEMEN     185
#define MATERIAL_VOMIT     186
#define MATERIAL_HONEY     189
#define MATERIAL_WAX       190


#define MACHINECALLBACK_PISTOL           100
#define MACHINECALLBACK_KNIFE            101
#define MACHINECALLBACK_HOSPITAL         102
#define MACHINECALLBACK_TRACKERGLASSES   104
#define MACHINECALLBACK_NEUROGLASSES     105
#define MACHINECALLBACK_ECOLOGYCOMPUTER  106
#define MACHINECALLBACK_LIGHTER          107
#define MACHINECALLBACK_DESTROYER        108
#define MACHINECALLBACK_MESSAGECOMPUTER1  110
#define MACHINECALLBACK_MESSAGECOMPUTER2  111
#define MACHINECALLBACK_MESSAGECOMPUTER3  112
#define MACHINECALLBACK_MESSAGECOMPUTER4  113
#define MACHINECALLBACK_MESSAGECOMPUTER5  114

#define MUTATION_ADDWEIGHT        10001
#define MUTATION_ALTERBIAS        10002
#define MUTATION_SKINCOLOR        10003
#define MUTATION_MAKECONNECTION   10004
#define MUTATION_RECONNECT        10005
#define MUTATION_MOVECELL         10006
#define MUTATION_EYELOOK          10007
#define MUTATION_ERASEORGAN       10008
#define MUTATION_ADDORGAN         10009
#define MUTATION_SPEAKERCHANNEL   10010
#define MUTATION_BREAKCONNECTION  10011
#define MUTATION_MULTIPLYWEIGHT   10012

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
		return std::string("a chomping mouth with flat teeth that chews side-to-side.");
	}
	case ORGAN_MOUTH_SCAVENGE:
	{
		return std::string("a sucker-like mouth that slurps up detritus.");
	}
	case ORGAN_GONAD:
	{
		return std::string("a sensitive gland filled with potential offspring.");
	}
	case ORGAN_MUSCLE:
	{
		return std::string("a muscular limb that can pull the animal along.");
	}
	case ORGAN_BONE:
	{
		return std::string("a strong slab of bone.");
	}
	case ORGAN_CLAW:
	{
		return std::string("a wicked curved claw with a deadly point.");
	}
	case ORGAN_LIVER:
	{
		return std::string("a brownish slab of flesh that stores energy.");
	}
	case ORGAN_MUSCLE_TURN:
	{
		return std::string("a muscular limb good for turning and spinning.");
	}
	case ORGAN_SENSOR_EYE:
	{
		return std::string("a monochrome, single pixel eye.");
	}
	case ORGAN_MOUTH_CARNIVORE:
	{
		return std::string("a grinning mouth with serrated, backward curving teeth.");
	}
	case ORGAN_MOUTH_PARASITE:
	{
		return std::string("a leech-like sucker lined with drilling teeth.");
	}
	case ORGAN_ADDOFFSPRINGENERGY:
	{
		return std::string("a manifold of tissue that nourishes the fetus.");
	}
	case ORGAN_ADDLIFESPAN:
	{
		return std::string("a gland full of powerful antioxidants that promotes long life.");
	}
	case ORGAN_NEURON:
	{
		return std::string("a basic brain cell that connects to form networks.");
	}
	case ORGAN_BIASNEURON:
	{
		return std::string("a part of the brain that provides a constant output.");
	}
	case ORGAN_SENSOR_BODYANGLE:
	{
		return std::string("part of the brain that senses the body's orientation.");
	}
	case ORGAN_SENSOR_TRACKER:
	{
		return std::string("part of the brain that seeks prey.");
	}
	case ORGAN_SPEAKER:
	{
		return std::string("a resonant chamber that is blown with air to produce sound.");
	}
	case ORGAN_SENSOR_EAR:
	{
		return std::string("a chamber full of tiny hairs that detect vibrations.");
	}
	case ORGAN_MUSCLE_STRAFE:
	{
		return std::string("ranks of hard muscle that pull the animal sideways.");
	}
	case ORGAN_SENSOR_PHEROMONE:
	{
		return std::string("a pocket that detects chemical signals.");
	}
	case ORGAN_EMITTER_PHEROMONE:
	{
		return std::string("a scent-producing gland that is plump with waxy secretions.");
	}
	case ORGAN_MEMORY_TX:
	{
		return std::string("a part of the brain responsible for storing knowledge in memory.");
	}
	case ORGAN_MEMORY_RX:
	{
		return std::string("a part of the brain that retrieves knowledge from memory.");
	}
	case ORGAN_GILL:
	{
		return std::string("red frills that exchange oxygen with the water.");
	}
	case ORGAN_LUNG:
	{
		return std::string("pink, spongy, air-breathing lungs.");
	}
	case ORGAN_SENSOR_HUNGER:
	{
		return std::string("a twisted ganglion that feels the pain of hunger.");
	}
	case ORGAN_SENSOR_AGE:
	{
		return std::string("part of the brain that feels the weight of age.");
	}
	case ORGAN_SENSOR_LAST_STRANGER:
	{
		return std::string("part of the brain which remembers the last stranger the animal met.");
	}
	case ORGAN_SENSOR_LAST_KIN:
	{
		return std::string("part of the brain which remembers the animal's recent friend.");
	}
	case ORGAN_SENSOR_PARENT:
	{
		return std::string("part of the brain that remembers the animal's mother.");
	}
	case ORGAN_SENSOR_BIRTHPLACE:
	{
		return std::string("part of the brain that remembers a childhood home.");
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
		return std::string("a part of the brain which feels agony and suffering.");
	}
	// case ORGAN_HEATADAPT:
	// {
	// 	return std::string("A gland which secretes liquid crystal to raise the boiling point.");
	// }
	// case ORGAN_COLDADAPT:
	// {
	// 	return std::string("A gland which produces syrup to lower the boiling point.");
	// }
	case ORGAN_HAIR:
	{
		return std::string("soft, shiny strands of hair.");
	}

	case ORGAN_SENSOR_RANDOM:
	{
		return std::string("part of the brain which cannot bear to sit still.");
	}
	case ORGAN_LOCATIONREMEMBERER:
	{
		return std::string("a grid arrangement of cells used for navigation.");
	}





	case ORGAN_SWITCH:
	{
		return std::string("a neural switch, where one pathway turns another on and off.");
	}
	case ORGAN_COMPARATOR:
	{
		return std::string("a part of the brain that compares two things.");
	}
	case ORGAN_ABSOLUTE:
	{
		return std::string("part of the brain that gives the absolute value of an input.");
	}
	case ORGAN_DERIVATOR:
	{
		return std::string("part of the brain that determines the rate of change of the input.");
	}
	case ORGAN_IIRLOW:
	{
		return std::string("part of the brain that filters rapid changes out of a signal.");
	}
	case ORGAN_IIRHIGH:
	{
		return std::string("part of the brain that filters slow changes out of a signal.");
	}

	case ORGAN_TIMER:
	{
		return std::string("a pacemaking ganglion that provides a pulsing beat.");
	}


// #define ORGAN_GENITAL_A            40
// #define ORGAN_GENITAL_B            41
	case ORGAN_GENITAL_A:
	{
		return std::string("a penetrating harpoon that squirts fertilizing jelly.");
	}

	case ORGAN_GENITAL_B:
	{
		return std::string("a self-lubricating orifice that accepts genetic material.");
	}


	case MATERIAL_FOOD:
	{
		return std::string("a piece of dried-out old meat.");
	}
	case MATERIAL_ROCK:
	{
		return std::string("a solid grey rock.");
	}

	// case MATERIAL_MEAT:
	// {
	// 	return std::string("A bleeding chunk of flesh.");
	// }
	case MATERIAL_BONE:
	{
		return std::string("a fragment of bone.");
	}
	case MATERIAL_BLOOD:
	{
		return std::string("splatters of coagulating blood.");
	}
	case MATERIAL_METAL:
	{
		return std::string("cool, shiny metal.");
	}
	case MATERIAL_VOIDMETAL:
	{
		return std::string("unbreakable void metal.");
	}
	case MATERIAL_SMOKE:
	{
		return std::string("a wisp of smoke.");
	}
	case MATERIAL_GLASS:
	{
		return std::string("a pane of glass.");
	}
	case MATERIAL_NOTHING:
	{
		return std::string("nothing.");
	}
	case MATERIAL_GRASS:
	{
		return std::string("some grass.");
	}
	case MATERIAL_WATER:
	{
		return std::string("cold clear water.");
	}


	case MATERIAL_SAND:
	{
		return std::string("fine-grained sand.");
	}
	case MATERIAL_DIRT:
	{
		return std::string("dry sandy dirt.");
	}
	case MATERIAL_SOIL:
	{
		return std::string("rich living soil.");
	}
	case MATERIAL_BASALT:
	{
		return std::string("hard volcanic rock.");
	}
	case MATERIAL_DUST:
	{
		return std::string("dry alkaline dust.");
	}
	case MATERIAL_GRAVEL:
	{
		return std::string("scattered rubble.");
	}

	case MATERIAL_FABRIC:
	{
		return std::string("soft fibre woven into a mesh.");
	}



	case MATERIAL_SEED:
	{
		return std::string("a seed.");
	}


	case MATERIAL_BUD:
	{
		return std::string("flowers.");
	}


	case MATERIAL_WOOD:
	{
		return std::string("a branch with bark.");
	}


	case MATERIAL_LEAF:
	{
		return std::string("leaves and twigs.");
	}



	case MATERIAL_POLLEN:
	{
		return std::string("fragrant, dusty pollen.");
	}


	case TILE_DESTROYER_EYE:
	{
		return std::string("a menacing red lens that flickers with energy.");
	}






	case MATERIAL_SEMEN:
	{
		return std::string("globs of sticky, bleachy reproductive jelly.");
	}

	case MATERIAL_VOMIT:
	{
		return std::string("an acrid puddle of partially digested food.");
	}


	case MATERIAL_HONEY:
	{
		return std::string("a sweet, sticky sugar syrup.");
	}

	case MATERIAL_WAX:
	{
		return std::string("a malleable, waterproof solid.");
	}





	}
	return std::string("something you can't describe.");
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
	case ORGAN_CLAW:
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


case ORGAN_GENITAL_A:
	{
		return std::string("Penis.");
	}

	case ORGAN_GENITAL_B:
	{
		return std::string("Vagina.");
	}



	case ORGAN_SENSOR_PAIN:
	{
		return std::string("Pain sensor");
	}

	// case ORGAN_COLDADAPT:
	// {
	// 	return std::string("Cold adapt");
	// }

	// case ORGAN_HEATADAPT:
	// {
	// 	return std::string("Heat adapt");
	// }

	case ORGAN_HAIR:
	{
		return std::string("Hair");
	}


	case ORGAN_SENSOR_RANDOM:
	{
		return std::string("Random sensor");
	}





// #define ORGAN_SWITCH     45
// #define ORGAN_COMPARATOR 46
// #define ORGAN_ABSOLUTE   47
// #define ORGAN_IIRLOW     48
// #define ORGAN_IIRHIGH    49
// #define ORGAN_DERIVATOR    5049
// #define ORGAN_TIMER    50

	case ORGAN_SWITCH:
	{
		return std::string("Switch");
	}
	case ORGAN_COMPARATOR:
	{
		return std::string("Comparator");
	}
	case ORGAN_ABSOLUTE:
	{
		return std::string("Absolute value");
	}
	case ORGAN_IIRLOW:
	{
		return std::string("IIR lowpass");
	}
	case ORGAN_IIRHIGH:
	{
		return std::string("IIR highpass");
	}
	case ORGAN_DERIVATOR:
	{
		return std::string("Derivator");
	}
	case ORGAN_TIMER:
	{
		return std::string("Timer");
	}





	case MATERIAL_FOOD:
	{
		return std::string("Food");
	}
	case MATERIAL_ROCK:
	{
		return std::string("Rock");
	}

	// case MATERIAL_MEAT:
	// {
	// 	return std::string("Meat");
	// }
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

	case TILE_DESTROYER_EYE:
	{
		return std::string("A destroyer's eye.");
	}



	case MATERIAL_SEMEN:
	{
		return std::string("Semen.");
	}

	case MATERIAL_VOMIT:
	{
		return std::string("Vomit.");
	}



	case MATERIAL_WAX:
	{
		return std::string("Wax.");
	}

	case MATERIAL_HONEY:
	{
		return std::string("Honey.");
	}





	}
	return std::string("Unknown");
}


// a list of what tiles produce on being destroyed.
unsigned int organProduces(unsigned int organ)
{

	switch (organ)
	{
	case ORGAN_MOUTH_VEG:
	{
		return MATERIAL_VOMIT;
	}
	case ORGAN_MOUTH_SCAVENGE:
	{
		return MATERIAL_VOMIT;
	}
	case ORGAN_GONAD:
	{
		return MATERIAL_SEMEN;
	}
	case ORGAN_MUSCLE:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_BONE:
	{
		return MATERIAL_BONE;
	}
	case ORGAN_CLAW:
	{
		return MATERIAL_BONE;
	}
	case ORGAN_LIVER:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_MUSCLE_TURN:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_SENSOR_EYE:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_MOUTH_CARNIVORE:
	{
		return MATERIAL_VOMIT;
	}
	case ORGAN_MOUTH_PARASITE:
	{
		return MATERIAL_VOMIT;
	}
	case ORGAN_ADDOFFSPRINGENERGY:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_ADDLIFESPAN:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_NEURON:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_BIASNEURON:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_SENSOR_BODYANGLE:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_TRACKER:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_RANDOM:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SPEAKER:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_EAR:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_MUSCLE_STRAFE:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_SENSOR_PHEROMONE:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_EMITTER_PHEROMONE:
	{
		return MATERIAL_WAX;
	}
	case ORGAN_MEMORY_TX:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_MEMORY_RX:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_GILL:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_LUNG:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_SENSOR_HUNGER:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_AGE:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_LAST_STRANGER:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_LAST_KIN:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_PARENT:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_BIRTHPLACE:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_SENSOR_TOUCH:
	{
		return MATERIAL_BLOOD;
	}
	case ORGAN_GRABBER:
	{
		return MATERIAL_FOOD;
	}
	case ORGAN_SENSOR_PAIN:
	{
		return MATERIAL_BLOOD;
	}
		// case ORGAN_COLDADAPT:
		// {
		// 	return MATERIAL_HONEY;
		// }
		// case ORGAN_HEATADAPT:
		// {
		// 	return MATERIAL_HONEY;
		// }
	}
	return MATERIAL_SMOKE;
}

float organGrowthCost(unsigned int organ)
{
	if (variedGrowthCost)
	{
		switch (organ)
		{
		case ORGAN_MUSCLE:
			return 1.0f;
		case ORGAN_BONE:
			return 1.0f;
		case ORGAN_CLAW:
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
	        // organ == ORGAN_MEMORY_TX ||
	        // organ == ORGAN_MEMORY_RX  ||
	        organ == ORGAN_SPEAKER ||
	        organ == ORGAN_SENSOR_EAR ||
	        organ == ORGAN_DERIVATOR ||
	        organ == ORGAN_IIRLOW ||
	        organ == ORGAN_IIRHIGH



	   )
	{
		return true;
	}
	return false;
}

// organIsAnActuator is for a departing terminus of the network that takes some input, but should not be connected to by anthing else.
bool organIsAnActuator(unsigned int organ)
{
	if (    organ == ORGAN_MUSCLE ||
	        organ == ORGAN_MUSCLE_TURN ||
	        organ == ORGAN_MUSCLE_STRAFE ||
	        organ == ORGAN_SPEAKER  ||
	        organ == ORGAN_EMITTER_PHEROMONE
	        // organ == ORGAN_MEMORY_TX
	   )
	{
		return true;
	}
	return false;
}

//organIsANeuron is for interconnecting cells that have both inputs and outputs.
bool organIsANeuron(unsigned int organ)
{
	if (    organ == ORGAN_NEURON ||
	        organ == ORGAN_BIASNEURON ||

	        organ == ORGAN_SWITCH ||

	        organ == ORGAN_COMPARATOR ||
	        organ == ORGAN_DERIVATOR ||

	        organ == ORGAN_ABSOLUTE ||

	        organ == ORGAN_MEMORY_RX ||

	        organ == ORGAN_MEMORY_TX ||

	        organ == ORGAN_IIRLOW ||

	        organ == ORGAN_IIRHIGH




// 	        #define ORGAN_SWITCH     45
// #define ORGAN_COMPARATOR 46
// #define ORGAN_ABSOLUTE   47
// #define ORGAN_IIRLOW     48
// #define ORGAN_IIRHIGH    49




	   )
	{
		return true;
	}
	return false;
}

// organIsASensor signifies an arriving terminus where signals originate. other cells should connect to them freely, but they themselves do not depend on an upstream c
bool organIsASensor(unsigned int organ)
{
	if (organ == ORGAN_SENSOR_EYE               ||
	        organ == ORGAN_SENSOR_BODYANGLE     ||
	        organ == ORGAN_SENSOR_TRACKER       ||
	        organ == ORGAN_SENSOR_EAR           ||
	        organ == ORGAN_SENSOR_PHEROMONE     ||
	        organ == ORGAN_SENSOR_LAST_STRANGER ||
	        organ == ORGAN_SENSOR_LAST_KIN ||
	        organ == ORGAN_SENSOR_HUNGER ||
	        organ == ORGAN_SENSOR_AGE ||
	        organ == ORGAN_SENSOR_BIRTHPLACE ||
	        organ == ORGAN_SENSOR_PARENT ||
	        organ == ORGAN_SENSOR_PAIN ||
	        // organ == ORGAN_SENSOR_PLEASURE ||
	        organ == ORGAN_SENSOR_RANDOM ||
	        organ == ORGAN_MOUTH_PARASITE ||
	        organ == ORGAN_MOUTH_SCAVENGE ||
	        organ == ORGAN_MOUTH_CARNIVORE ||
	        organ == ORGAN_MOUTH_VEG ||
	        organ == ORGAN_CLAW ||


	        organ == ORGAN_GONAD ||
	        organ == ORGAN_GENITAL_A ||
	        organ == ORGAN_GENITAL_B ||
	        organ == ORGAN_GRABBER

	   )
	{
		return true;
	}
	return false;
}


bool materialIsTransparent(unsigned int material)
{

	if (material == MATERIAL_WATER)
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




const  char lighter[] =
{
	'1', ' ',
	'1', '1',
	'1', '1',
	'1', '1',
};

void setupExampleLighter(int i)
{
	// game.animals[i].isMachine = true;
	// game.animals[i].machineCallback = MACHINECALLBACK_LIGHTER;

	setupCreatureFromCharArray( i, lighter, 8, 2 , std::string("lighter"), MACHINECALLBACK_LIGHTER);
}
const 	char humanBody[] =
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


const 	char humanPaint[] =
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

void setupExampleHuman(int i)
{
	setupCreatureFromCharArray( i, humanBody, (9 * 33), 9 ,  std::string("human"), -1 );
	paintCreatureFromCharArray( i, humanPaint, (9 * 33), 9 );
}

const 	char destroyer[] =
{
	' ', ' ', '1', ' ', ' ',
	' ', '1', '1', '1', ' ',
	' ', '1', '1', '1', ' ',
	'1', '1', '1', '1', '1',
	'1', '1', '3', '1', '1',
	'1', '1', '1', '1', '1',
	'1', '1', '1', '1', '1',
	'1', '1', '1', '1', '1',
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

const 	char destroyerPaint[] =
{
	' ', ' ', 'V', ' ', ' ',
	' ', 'V', 'V', 'V', ' ',
	' ', 'V', 'V', 'V', ' ',
	'V', 'B', 'B', 'B', 'V',
	'V', 'B', 'Q', 'B', 'V',
	'V', 'B', 'B', 'B', 'V',
	'V', 'V', 'V', 'V', 'V',
	'V', 'V', 'V', 'V', 'V',
	' ', 'V', 'V', 'V', ' ',
	' ', 'V', 'V', 'V', ' ',
	' ', 'V', 'V', 'V', ' ',
	' ', 'V', 'V', 'V', ' ',
	' ', 'V', 'V', 'V', ' ',
	' ', ' ', 'V', ' ', ' ',
	' ', ' ', 'V', ' ', ' ',
	' ', ' ', 'V', ' ', ' ',
	' ', ' ', 'V', ' ', ' ',
	' ', ' ', 'V', ' ', ' ',
	' ', ' ', 'V', ' ', ' ',
	' ', ' ', 'V', ' ', ' ',
};



void setupDestroyer(int i)
{
	setupCreatureFromCharArray( i, destroyer, 20 * 5, 5 , std::string("destroyer") , MACHINECALLBACK_DESTROYER);
	paintCreatureFromCharArray( i, destroyerPaint, (20 * 5), 5);
}
const  char trackerGlasses[] =
{
	' ', '2', ' ', '2', ' ',
	'2', '2', '1', '2', '2',
	' ', '2', ' ', '2', ' ',
};


const char trackerGlassesPaint[] =
{
	' ', 'W', ' ', 'W', ' ',
	'W', 'L', 'M', 'L', 'W',
	' ', 'W', ' ', 'W', ' ',
};


void setupTrackerGlasses(int i)
{
	setupCreatureFromCharArray( i, trackerGlasses, 15, 5 , std::string("tracker glasses") , MACHINECALLBACK_TRACKERGLASSES);
	paintCreatureFromCharArray( i, trackerGlassesPaint, 15, 5 );
}

const  char neuroGlasses[] =
{
	'1', '1', ' ', '1', '1',
	'1', '2', '1', '2', '1',
	'1', '1', ' ', '1', '1',
};

const  char neuroGlassesPaint[] =
{
	'W', 'W', ' ', 'W', 'W',
	'W', 'K', 'M', 'K', 'W',
	'W', 'W', ' ', 'W', 'W',
};

void setupNeuroGlasses(int i)
{
	setupCreatureFromCharArray( i, neuroGlasses, 15, 5 , std::string("tracker glasses") , MACHINECALLBACK_NEUROGLASSES);
	paintCreatureFromCharArray( i, neuroGlassesPaint, 15, 5 );
}

const 	char pistol[] =
{
	'1', '1', '1', '1',
	'1', '1', '1', '1',
	' ', '1', '1', ' ',
	'1', '1', ' ', ' ',
};

const 	char pistolPaint[] =
{
	'M', 'M', 'M', 'M',
	'V', 'V', 'V', 'V',
	' ', 'V', 'D', ' ',
	'V', 'V', ' ', ' ',
};

void setupExampleGun(int i)
{
	setupCreatureFromCharArray( i, pistol, 16, 4 , std::string("pistol") , MACHINECALLBACK_PISTOL);
	paintCreatureFromCharArray( i, pistolPaint, 16, 4 );
}


const char knife[] =
{
	' ', '1', ' ', ' ',
	'1', '1', '1', '1',
	' ', '1', ' ', ' ',
};

const char knifePaint[] =
{
	' ', 'D', ' ', ' ',
	'V', 'D', 'M', 'M',
	' ', 'D', ' ', ' ',
};

void setupExampleKnife(int i)
{
	setupCreatureFromCharArray( i, knife, 12, 4 , std::string("knife") , MACHINECALLBACK_KNIFE);
	paintCreatureFromCharArray( i, knifePaint, 12, 4 );
}
const char computer[] =
{
	'1', '1', '1', '1', '1',
	'1', '2', '2', '2', '1',
	'1', '2', '2', '2', '1',
	'1', '1', '1', '1', '1',
	' ', ' ', '1', ' ', ' ',
	'1', '1', '1', '1', '1',
};

const char computerPaint[] =
{
	'B', 'B', 'B', 'B', 'B',
	'B', 'L', 'L', 'L', 'B',
	'B', 'L', 'L', 'L', 'B',
	'B', 'B', 'B', 'B', 'B',
	' ', ' ', 'B', ' ', ' ',
	'B', 'B', 'B', 'B', 'B',
};

void setupEcologyCompter(int i)
{
	setupCreatureFromCharArray( i, computer, 30, 5 , std::string("ecology terminal") , MACHINECALLBACK_ECOLOGYCOMPUTER);
	paintCreatureFromCharArray( i, computerPaint, 30, 5 );
}


const char ecoComputer[] =
{
	'2', '2', '2', '2', '2',
	'2', '2', '2', '2', '2',
	'2', '2', '2', '2', '2',
	'2', '2', '2', '2', '2',
	' ', ' ', '1', ' ', ' ',
	'1', '1', '1', '1', '1',
};

const char ecoComputerPaint[] =
{
	'L', 'L', 'L', 'L', 'L',
	'L', 'L', 'L', 'L', 'L',
	'L', 'L', 'L', 'L', 'L',
	'L', 'L', 'L', 'L', 'L',
	' ', ' ', 'G', ' ', ' ',
	'G', 'G', 'G', 'G', 'G',
};

void setupMessageComputer(int i, unsigned int messageComputerNumber)
{
	setupCreatureFromCharArray( i, ecoComputer, 30, 5 , std::string("message terminal") , MACHINECALLBACK_MESSAGECOMPUTER1 + messageComputerNumber);
	paintCreatureFromCharArray( i, ecoComputerPaint, 30, 5 );
}

const char hospital[] =
{
	' ', '2', ' ',
	'2', '2', '2',
	'2', '2', '2',
	'2', '2', '2',
	' ', '2', ' ',
	'2', '2', '2',
};

const char hospitalPaint[] =
{
	' ', 'M', ' ',
	'M', 'M', 'M',
	'D', 'M', 'D',
	'D', 'D', 'D',
	' ', 'V', ' ',
	'M', 'M', 'M',
};


void setupHospitalComputer(int i)
{
	setupCreatureFromCharArray( i, hospital, 18, 3 , std::string("hospital terminal") , MACHINECALLBACK_HOSPITAL);
	paintCreatureFromCharArray( i, hospitalPaint, 30, 5 );
}


bool materialBlocksMovement(unsigned int material)
{
	if (material == MATERIAL_ROCK ||
	        material == MATERIAL_VOIDMETAL ||
	        material == MATERIAL_METAL ||
	        material == MATERIAL_BASALT ||
	        material == MATERIAL_WAX
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
	        material == MATERIAL_SMOKE ||
	        material == MATERIAL_VOMIT ||
	        material == MATERIAL_SEMEN

	   )
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


	case MATERIAL_SEMEN:
		return color_offwhite;
	case MATERIAL_WAX:
		return color_offwhite;
	case MATERIAL_HONEY:
		return color_yellow;
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
	case ORGAN_CLAW               :
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
	// case ORGAN_COLDADAPT            :
	// 	return color_violet;
	// case ORGAN_HEATADAPT            :
	// 	return color_peach;
	case ORGAN_GRABBER              :
		return color_peach_light;
	case ORGAN_HAIR              :
		return color_clear;
	case TILE_DESTROYER_EYE              :
		return color_brightred;
	case ORGAN_SENSOR_RANDOM    :
		return color_brains3;

	// case ORGAN_SENSOR_PLEASURE    :
	// 	return color_pink;
	case ORGAN_GENITAL_A    :
		return color_pink;
	case ORGAN_GENITAL_B    :
		return color_pink;






// #define ORGAN_SWITCH     45
// #define ORGAN_COMPARATOR 46
// #define ORGAN_ABSOLUTE   47
// #define ORGAN_IIRLOW     48
// #define ORGAN_IIRHIGH    49
// #define ORGAN_DERIVATOR    5049
// #define ORGAN_TIMER    50


	case ORGAN_SWITCH    :
		return color_brains1;

	case ORGAN_COMPARATOR    :
		return color_brains2;

	case ORGAN_ABSOLUTE    :
		return color_brains3;

	case ORGAN_IIRHIGH    :
		return color_brains1;

	case ORGAN_IIRLOW    :
		return color_brains2;

	case ORGAN_DERIVATOR    :
		return color_brains3;

	case ORGAN_TIMER    :
		return color_brains1;




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