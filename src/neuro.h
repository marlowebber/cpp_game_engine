
#ifndef NEURO_H
#define NEURO_H



#include "fann.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <math.h>
#include <cmath>
#include <ctime>
#include <list>
#include <string.h>
#include "utilities.h"

#define SenseConnector_BUFFERSIZE 512
 
struct SenseConnector 
{
	unsigned int connectedToLimb;		// what limb the sense is coming from, or motor signal is going to.
	unsigned int connectedToNeuron;		// neuron index. The position of this neuron's layer will determine how the program uses it.
	unsigned int sensorType;  			// what kind of sense it is (touch, smell, etc.how the number will be treated)
	float timerFreq;					// if a timer, the frequency.
	float timerPhase;					// if a timer, the phase. Used so that the SenseConnector can be a fully self contained timer.
	unsigned int recursorChannel; 		// 
	unsigned int recursorDelay;
	float recursorBuffer[SenseConnector_BUFFERSIZE];
	unsigned int recursorCursor;
	SenseConnector();
};

/*!
* @brief a connection between two neurons inside the brain.
*
*/
struct ConnectionDescriptor 
{
	bool isUsed;
	unsigned int connectedTo;
	float connectionWeight;	
	ConnectionDescriptor(unsigned int toNeuron);
};

/*!
* @brief a single living brain cell.
*
*/
struct NeuronDescriptor 
{
	bool isUsed;
	unsigned int activation_function;
	float activation_steepness;
	std::list<ConnectionDescriptor> connections;
	unsigned int index; 			// the fann file index of this neuron. handy to refer to
	bool biasNeuron;
	bool selected;
	bool locked;
	NeuronDescriptor();
};

/*!
* @brief a layer of neurons inside the brain, the primary method of organisation for brain cells.
*
*/
struct LayerDescriptor 
{
	bool isUsed;
	std::list<NeuronDescriptor> neurons; // an array is a pointer to the start of the array, and this is actually an array of pointers to objects, so neurons[] is a double pointer.
	bool selected;
	LayerDescriptor();
};

/*!
* @brief a whole brain, many layers of many cells connected together.
*
*/
struct NetworkDescriptor 
{
	/*
	1. it's impossible to modify a FANN network once it is created
	2. modifying the network stored in text is possible, but hard
	3. i might as well make an easily modifiable descriptor file, and methods to turn it back into the text file.
	*/

	std::list<LayerDescriptor> layers;
	NetworkDescriptor(fann* pann);
};


#include "neuro.h"


fann * createBrainFromLayerDiagram( );

NeuronDescriptor * getNeuronByIndex (NetworkDescriptor * network, unsigned int windex) ;

void deleteNeuronByIndex (NetworkDescriptor * network, unsigned int windex) ;

// i have started converting this function from using arrays to using the list NetworkDescriptors, but i haven't finished yet.
NetworkDescriptor  * createNeurodescriptorFromFANN (fann * temp_ann) ;

fann * createFANNbrainFromDescriptor (NetworkDescriptor * network) ;

// goes through the brain and adds 'biasNeuron' flag to neurons at the end of each layer. This is mainly so they can be drawn properly.
void flagBiasNeurons( NetworkDescriptor * network) ;

void init_fishie();

void addNeuronIntoLivingBrain (NetworkDescriptor * network, unsigned int targetLayerIndex, bool bias) ;
void deleteLayer(NetworkDescriptor * network, unsigned int layerToDelete) ;

void verifyNetworkDescriptor (NetworkDescriptor * network) ;

void addLayerIntoLivingBrain(NetworkDescriptor * network) ;

NetworkDescriptor * combineTwoNetworks2 (NetworkDescriptor * partnerA, NetworkDescriptor * partnerB) ;

#endif