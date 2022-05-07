#include "neuro.h"

// NetworkDescriptor * createEmptyNetworkOfCorrectSize (fann * temp_ann) 
// {
// 	return new NetworkDescriptor(temp_ann);
// }

NeuronDescriptor * getNeuronByIndex (NetworkDescriptor * network, unsigned int windex) 
{
	std::list<LayerDescriptor>::iterator layer;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			if (neuron -> index == windex) 
			{
				return &(*neuron);
			}
		}
	}
	return nullptr;
}

void deleteNeuronByIndex (NetworkDescriptor * network, unsigned int windex) 
{

	// go through the entire brain and destroy any connection mentioning the target.
	// gather the connections to destroy in a separate list because you can't operate on a list while iterating through it.
	std::list<LayerDescriptor>::iterator layer;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			std::list<ConnectionDescriptor>::iterator connection;
			// note that the increment is removed from the loop and only performed if a deletion is NOT performed. https://stackoverflow.com/questions/16269696/erasing-while-iterating-an-stdlist
			for (connection = neuron->connections.begin(); connection != neuron->connections.end(); ) 
			{
				if (connection->connectedTo == windex ) 
				{
					connection = neuron->connections.erase(connection);
				}
				else 
				{
					connection++;
				}
			}
		}
	}

	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		// you can delete one neuron while iterating the list so long as you just do one and GTFO.
		bool allDone = false;
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; ++neuron) 
		{
			if (neuron->index == windex) 
			{
				neuron = layer->neurons.erase(neuron);
				allDone = true;
				break;
			}
			else 
			{
				neuron++;
			}
		}
		if (allDone) 
		{
			break;
		}
	}

	// go through the entire brain and decrement any neuron index greater than the target's index by 1.
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			if (neuron->index > windex) 
			{
				neuron->index--;
			}
		}
	}

	// go through the entire brain and decrement any connection index greater than the target's index.
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			std::list<ConnectionDescriptor>::iterator connection;
			for (connection = neuron->connections.begin(); connection != neuron->connections.end(); connection++) 
			{
				if (connection->connectedTo > windex) 
				{
					connection->connectedTo --;
				}
			}
		}
	}
}

ConnectionDescriptor::ConnectionDescriptor (unsigned int toNeuron)
{
	isUsed = false;
	connectedTo = toNeuron;
	connectionWeight = 0.0f;
}

NeuronDescriptor::NeuronDescriptor()
{
	activation_function = 0;
	activation_steepness = 1.0f;
	connections = std::list<ConnectionDescriptor>();
	index = 0; 			// the fann file index of this neuron. handy to refer to
	biasNeuron = false;
	locked = false;
	isUsed = false;
	selected = false;
}

LayerDescriptor::LayerDescriptor ()
{
	neurons = std::list<NeuronDescriptor>();
	isUsed = false;
	selected = false;
}

SenseConnector::SenseConnector ()
{
	connectedToLimb = 0;							// what limb the sense is coming from, or motor signal is going to.
	connectedToNeuron = 0;							// neuron index. The position of this neuron's layer will determine how the program uses it.
	sensorType =  0; 			// what kind of sense it is (touch, smell, etc.how the number will be treated)
	timerFreq = 0;									// if a timer, the frequency.
	timerPhase = 0.0f;
	recursorChannel = 0; 		//
	recursorDelay = 0;
	recursorCursor = 0;
	for (int i = 0; i < SenseConnector_BUFFERSIZE; ++i)
	{
		recursorBuffer[i] = 0.0f;
	}
}

// method to create a network descriptor in memory
NetworkDescriptor::NetworkDescriptor (fann * pann)
{
	if (pann != nullptr)
	{
		// query the number of layers.
		unsigned int num_layers = fann_get_num_layers(pann);
		unsigned int activation_function_hidden = 5;
		float activation_steepness_hidden = 0.5f;
		unsigned int activation_function_output = 0;
		float activation_steepness_output = 0;

		// get the layer cake. because FANN provides layer information as an array of integers, this is just a temporary variable to hold it.
		unsigned int layerCake[num_layers];
		fann_get_layer_array(pann, layerCake);
		unsigned int rollingIndexCounter = 0;
		for (unsigned int i = 0; i < num_layers; ++i) 
		{
			LayerDescriptor layer = LayerDescriptor();
			unsigned int nNeuronsIncludingBias = layerCake[i];
			if (i == num_layers - 1) 
			{
				;
			} 
			else 
			{
				nNeuronsIncludingBias += 1;
			}

			for (unsigned int j = 0; j < nNeuronsIncludingBias; ++j) 
			{
				NeuronDescriptor neuron = NeuronDescriptor();
				neuron.index = rollingIndexCounter;
				rollingIndexCounter ++;
				neuron.activation_function = activation_function_hidden;
				neuron.activation_steepness = activation_steepness_hidden;
				neuron.isUsed = true;
				// output neurons have a different function than the others. this applies to all in the last row.
				if (i == num_layers - 1) 
				{
					neuron.activation_function = activation_function_output;
					neuron.activation_steepness = activation_steepness_output;
				}
				layer.neurons.push_back(neuron);
			}
			this->layers.push_back(layer); // add a new layer descriptor
		}

		// to create the connection map, you must read in from the file.
		// figure out the total number of neurons, which is how they are indexed in FANN file.
		unsigned int sumOfNeurons = 0;
		for (unsigned int i = 0; i < num_layers; ++i) 
		{
			sumOfNeurons += layerCake[i];
		}

		std::list<LayerDescriptor>::iterator layer;
		unsigned int i = 0;
		unsigned int num_connections = fann_get_total_connections(pann);
		for (layer = this->layers.begin(); layer != this->layers.end(); ++layer)  
		{
			layer->isUsed = true;
			std::list<NeuronDescriptor>::iterator neuron;
			for (neuron = layer->neurons.begin(); neuron != layer->neurons.end(); ++neuron) 
			{
				neuron->activation_function = activation_function_hidden;
				neuron->activation_steepness = activation_steepness_hidden;
				neuron->isUsed = true;
				// output neurons have a different function than the others. this applies to all in the last row.
				if (i == num_layers - 1) 
				{
					neuron->activation_function = activation_function_output;
					neuron->activation_steepness = activation_steepness_output;
				}
			}
		}

		// get connection and weight information.
		struct fann_connection margles[num_connections] ;
		memset(&margles, 0x00, sizeof(fann_connection[num_connections]));
		struct fann_connection *con = margles;
		fann_get_connection_array(pann, con); // this DOES include bias neuron information.
		for (unsigned int c = 0; c < num_connections; ++c) 
		{
			ConnectionDescriptor connection = ConnectionDescriptor(con[c].to_neuron);
			connection.connectionWeight = con[c].weight;
			getNeuronByIndex(this, con[c].from_neuron)->connections.push_back(connection);
		}
	}
}

fann * createFANNbrainFromDescriptor (NetworkDescriptor * network) 
{ //create an empty fann brain of the right size and layer cake.
	unsigned int creationLayerCake[(unsigned long)network->layers.size()];
	std::list<LayerDescriptor>::iterator layer;
	unsigned int layerIndex = 0;

	// if you are not on the last layer, ignore bias neurons because they are included implicitly.
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		creationLayerCake[layerIndex] = 0;
		if (layerIndex == ((unsigned long)network->layers.size() - 1 )) 
		{
			creationLayerCake[layerIndex] = layer->neurons.size();
		}
		else {
			creationLayerCake[layerIndex] = layer->neurons.size() - 1;
		}

		if (creationLayerCake[layerIndex] < 1) 
		{
			creationLayerCake[layerIndex] = 1;
		}

		layerIndex++;
	}
	unsigned int num_layers = (unsigned long)network->layers.size();

	if (num_layers < 1) 
	{
		num_layers = 1;
	}

	fann * ann = fann_create_standard_array(num_layers, creationLayerCake);
	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
	fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
	unsigned int num_connections = fann_get_total_connections(ann);
	struct fann_connection margles[num_connections] ;
	memset(&margles, 0x00, sizeof(fann_connection[num_connections]));
	unsigned int connectionIndex = 0;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			std::list<ConnectionDescriptor>::iterator connection;
			for (connection = neuron->connections.begin(); connection != neuron->connections.end(); connection++) 
			{
				fann_connection conc ;
				conc.from_neuron = neuron->index;
				conc.to_neuron = connection->connectedTo;
				conc.weight = connection->connectionWeight;
				margles[connectionIndex] = conc;
				connectionIndex++;
			}
		}
	}
	fann_set_weight_array(ann, margles, num_connections);
	return ann;
}

// goes through the brain and adds 'biasNeuron' flag to neurons at the end of each layer. This is mainly so they can be drawn properly.
void flagBiasNeurons( NetworkDescriptor * network) 
{
	std::list<LayerDescriptor>::iterator layer;
	unsigned int layerIndex = 0;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		std::list<NeuronDescriptor>::iterator neuron;
		// iterate through all and flag them false
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			neuron->biasNeuron = false;
		}
		// on every layer except the last, flag the last neuron as bias.
		if ( layerIndex < (network->layers.size() - 1 )) 
		{
			neuron = layer->neurons.end();
			neuron--;
			neuron->biasNeuron = true;
		}
		layerIndex++;
	}
}

fann * createBrainFromLayerDiagram( )
{
		unsigned int creationLayerCake[] = {
			4,
			3,
			3,
			4
		};
		fann * ann = fann_create_standard_array(4, creationLayerCake);
		fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
		fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
		return ann;
}

void addNeuronIntoLivingBrain (NetworkDescriptor * network, unsigned int targetLayerIndex, bool bias) 
{
	NeuronDescriptor * newNeuron = new NeuronDescriptor();
	newNeuron->isUsed = true;
	newNeuron->biasNeuron = false;
	newNeuron->index = 0;
	std::list<LayerDescriptor>::iterator layer;
	std::list<LayerDescriptor>::iterator targetLayerIterator;
	std::list<NeuronDescriptor>::iterator neuron;
	unsigned int layerIndex = 0;

	// set the target layer iterator, so you can refer back to it later
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer)  
	{
		newNeuron->index += layer->neurons.size(); // always adding new neuron at the end of the layer.
		if (layerIndex == targetLayerIndex) 
		{
			targetLayerIterator = layer;
			break;
		}
		layerIndex++;
	}

	// if the neuron is not on the last layer, due to the presence of a bias neuron on the target layer, decrement index by 1.
	if (! (targetLayerIndex == network->layers.size() - 1) ) 
	{
		newNeuron->index--;
	}

	// all indexes in the connection map greater than the index of this neuron are incremented by 1.
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			if (neuron->index >= newNeuron->index) 
			{
				neuron->index ++;
			}

			std::list<ConnectionDescriptor>::iterator connection;
			for (connection = neuron->connections.begin(); connection != neuron->connections.end(); connection++) 
			{
				if (connection->connectedTo > newNeuron->index) 
				{
					connection->connectedTo ++;
				}
			}
		}
	}

	// make connections for all the next-layer neurons, set to 0, add them to the new neuron
	if (targetLayerIndex != (network->layers.size() - 1 ) ) 
	{ // if this isn't the last layer
		layer = targetLayerIterator;
		layer++;

		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			if ( !(neuron->biasNeuron)) 
			{
				unsigned int targetIndex = neuron->index;
				ConnectionDescriptor * newConnection = new ConnectionDescriptor(   targetIndex);
				newConnection->isUsed = true;
				newConnection->connectionWeight = 0.0f;
				newNeuron->connections.push_back( *newConnection  );
			}
		}
	}

	// if the neuron is not on the first layer (and not a bias neuron), all the previous-layer neurons get connections to this neuron.
	if (!bias) 
	{
		layer = targetLayerIterator;
		if (layer != network->layers.begin() ) 
		{ // and the next layer is not off the end of the array
			layer--;
			for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
			{
				ConnectionDescriptor * newConnection = new ConnectionDescriptor(   newNeuron->index);
				newConnection->isUsed = true;
				newConnection->connectionWeight = 0.0f;
				neuron->connections.push_back( *newConnection  );
			}
		}
		newNeuron ->biasNeuron = false;
	}
	else 
	{
		newNeuron ->biasNeuron = true;
	}
	if (targetLayerIndex == network->layers.size() - 1) 
	{
		targetLayerIterator->neurons.push_back( *newNeuron);		// there is no bias neuron on the last layer so you can drop it right at the end.
	}
	else 
	{
		neuron = targetLayerIterator->neurons.end();			// 'end' is actually 1 past the last element, in C++ list syntax. So retract by 1 to get the last element.
		neuron --;
		targetLayerIterator->neurons.insert( neuron, *newNeuron);
	}
	flagBiasNeurons(network);
}

void deleteLayer(NetworkDescriptor * network, unsigned int layerToDelete) 
{
	// connections from this layer to the next will be automatically destroyed when the neurons are deleted
	std::list<LayerDescriptor>::iterator layer;
	std::list<LayerDescriptor>::iterator targetLayerIterator;
	std::list<NeuronDescriptor>::iterator neuron;
	unsigned int layerIndex = 0;
	unsigned int numberOfNeuronsInLayer = 0;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer)  
	{
		if (layerIndex == layerToDelete) 
		{
			numberOfNeuronsInLayer = layer->neurons.size();
			break;
		}
		layerIndex++;
	}
	network->layers.erase(layer);
	// decrement the indexes of all subsequent neurons and connections by the amount that was removed
	unsigned int actualNeuronIndex = 0;
	layerIndex = 0;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer)  
	{
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			if (layerIndex >= layerToDelete ) 
			{
				neuron->index -= numberOfNeuronsInLayer;
				std::list<ConnectionDescriptor>::iterator connection;
				for (connection = neuron->connections.begin(); connection != neuron->connections.end(); connection++) 
				{
					connection->connectedTo -= numberOfNeuronsInLayer;
				}
			}
			actualNeuronIndex++;
		}
		layerIndex++;
	}
	// go through the preceding layer and make sure all the neurons are connected to the ones in the new layer (it might have changed if the layers were different sizes)
	layerIndex = 0;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer)  
	{
		// scroll ahead 1 layer, find the highest index in it.
		layer++;
		unsigned int maxIndexInNextLayer = 0;
		if (layer != network->layers.end()) 
		{
			neuron = layer->neurons.end();
			neuron--;
			if (layerIndex < network->layers.size() - 1) 
			{ // not the last layer
				neuron--;
				maxIndexInNextLayer = neuron->index;
			}
		}
		layer--;

		// if a neuron has connections to an index greater than what is available in the next level, add the spurious connection to a list
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			neuron->connections.remove_if( [maxIndexInNextLayer](ConnectionDescriptor connection)
			{
				if (connection.connectedTo > maxIndexInNextLayer) 
				{
					return true;
				}
				else 
				{
					return false;
				}
			}
			                             );
		}
		layerIndex++;
	}
	layerIndex = 0;
	std::list<NeuronDescriptor>::iterator neuronB;
	std::list<ConnectionDescriptor>::iterator connection;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer)  
	{
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			layer++;
			if (layer != network->layers.end() ) 
			{
				// go through the indexes in the layer after this one, make sure none of the neurons on this layer are missing potential connections.
				for ( neuronB = layer->neurons.begin(); neuronB != layer->neurons.end() ; neuronB++) 
				{
					if (neuronB->biasNeuron) 
					{
						continue;
					}
					// check if neuron has a connection to neuronB
					bool connectedToNeuronB = false;
					for (connection = neuron->connections.begin(); connection != neuron->connections.end(); connection++) 
					{
						if (connection->connectedTo == neuronB->index) 
						{
							connectedToNeuronB = true;
						}
					}
					if (!connectedToNeuronB) 
					{
						ConnectionDescriptor * newConnection = new ConnectionDescriptor(   neuronB->index);
						newConnection->isUsed = true;
						newConnection->connectionWeight = 0.0f;
						neuron->connections.push_back( *newConnection  );
					}
				}
			}
			layer--;
		}
		layerIndex++;
	}
}

void verifyNetworkDescriptor (NetworkDescriptor * network) 
{
	std::list<LayerDescriptor>::iterator layer;
	unsigned int layerIndex = 0;
	unsigned int neuronIndex = 0;
	unsigned int connectionIndex = 0;
	for (layer = network->layers.begin(); layer !=  network->layers.end(); ++layer) 	
	{
		printf("	layer %u neurons: %lu\n", layerIndex, (unsigned long)layer->neurons.size());
		std::list<NeuronDescriptor>::iterator neuron;
		for ( neuron = layer->neurons.begin(); neuron != layer->neurons.end() ; neuron++) 
		{
			printf("		neuron %u connections: %lu bias: %i\n", neuronIndex, (unsigned long)neuron->connections.size(), neuron->biasNeuron);
			std::list<ConnectionDescriptor>::iterator connection;
			for (connection = neuron->connections.begin(); connection != neuron->connections.end(); connection++) 
			{
				printf("			connection %u to: %u, weight:%f\n", connectionIndex, connection->connectedTo, connection->connectionWeight );
				connectionIndex++;
			}
			neuronIndex ++;
		}
		layerIndex ++;
	}
}

void addLayerIntoLivingBrain(NetworkDescriptor * network) 
{
	std::list<LayerDescriptor>::iterator layer;
	LayerDescriptor * newLayer = new LayerDescriptor;
	newLayer ->isUsed = true;
	newLayer ->selected = false;
	network->layers.push_back(*newLayer);
	layer = network->layers.end();
	layer --;
	layer --; // scroll back to the layer before the one you just added
	// add as many neurons as there are ouput connectors
	unsigned int sizeOfPrevLayer = layer->neurons.size();
	for (unsigned int i = 0; i < sizeOfPrevLayer; ++i)
	{
		addNeuronIntoLivingBrain( network, (network->layers.size() - 1), false );
	}
	// add a bias neuron to the layer that used to be last. and connect it to all the new neurons.
	addNeuronIntoLivingBrain( network, (network->layers.size() - 2), true);
}

NetworkDescriptor * combineTwoNetworks2 (NetworkDescriptor * partnerA, NetworkDescriptor * partnerB) 
{
	// copies are made of both parents.
	// blank neurons are added to the copies to make them the same shape.
	// then, connections can be chosen at random without having to compute anything special.
	NetworkDescriptor copyParentA = *partnerA;
	NetworkDescriptor copyParentB = *partnerB;

	// first, find the tallest network.
	unsigned int layersA = copyParentA.layers.size();
	unsigned int layersB = copyParentB.layers.size();
	unsigned int tallestPartner;

	if (layersA > layersB) 
	{
		tallestPartner = layersA;
		for (unsigned int i = 0; i < (layersA - layersB); ++i)
		{
			addLayerIntoLivingBrain(&copyParentB);
		}
	}
	else if (layersB > layersA) 
	{
		tallestPartner = layersB;
		for (unsigned int i = 0; i < (layersB - layersA); ++i)
		{
			addLayerIntoLivingBrain(&copyParentA);
		}
	}
	else 
	{
		// they are the same!
		tallestPartner = layersA;
	}

	// now go through them layer at a time.
	std::list<LayerDescriptor>::iterator layerA = copyParentA.layers.begin();
	std::list<LayerDescriptor>::iterator layerB = copyParentB.layers.begin();
	for (unsigned int i = 0; i < tallestPartner; ++i)
	{
		// find which is the widest.
		unsigned int widthA = layerA->neurons.size();
		unsigned int widthB = layerB->neurons.size();
		// add enough neurons to the other that makes them the same.
		if (widthA > widthB) 
		{
			// widestPartner = widthA;
			for (unsigned int j = 0; j < (widthA - widthB); ++j)
			{
				addNeuronIntoLivingBrain(&copyParentB, i, false);
			}
		}
		else if (widthB > widthA) 
		{
			// widestPartner = widthB;
			for (unsigned int j = 0; j < (widthB - widthA); ++j)
			{
				addNeuronIntoLivingBrain(&copyParentA, i, false);
			}
		}
		else 
		{
			// they are the same! do nothing.
		}
		layerA++;
		layerB++;
	}

	// congrats, the network copies are now exactly the same shape.
	// duplicate one of them to create the child.
	fann * wann = createFANNbrainFromDescriptor(&copyParentA);
	NetworkDescriptor * child = new NetworkDescriptor(wann);
	layerA = copyParentA.layers.begin();
	layerB = copyParentB.layers.begin();
	std::list<LayerDescriptor>::iterator layerC = child->layers.begin();
	std::list<NeuronDescriptor>::iterator neuronA;
	std::list<NeuronDescriptor>::iterator neuronB ;
	std::list<NeuronDescriptor>::iterator neuronC ;
	std::list<ConnectionDescriptor>::iterator connectionA;
	std::list<ConnectionDescriptor>::iterator connectionB;
	std::list<ConnectionDescriptor>::iterator connectionC;
	for (unsigned int i = 0; i < tallestPartner; ++i)
	{
		neuronA = layerA->neurons.begin();
		neuronB = layerB->neurons.begin();
		neuronC = layerC->neurons.begin();
		for (unsigned int j = 0; j < layerA->neurons.size(); ++j)
		{
			connectionA = neuronA->connections.begin();
			connectionB = neuronB->connections.begin();
			connectionC = neuronC->connections.begin();
			for (unsigned int k = 0; k < neuronA->connections.size(); ++k)
			{
				if (connectionA->connectionWeight != 0.0f && connectionB->connectionWeight !=  0.0f) 
				{
					if (RNG() > 0.5) 
					{ // if they are both nonzero, choose one at random
						connectionC -> connectionWeight = connectionA->connectionWeight ;
					}
					else 
					{
						connectionC -> connectionWeight = connectionB->connectionWeight ;
					}

				}
				else if (connectionA->connectionWeight !=  0.0f) 
				{
					connectionC -> connectionWeight = connectionA->connectionWeight ;
				}
				else if (connectionB->connectionWeight !=  0.0f) 
				{
					connectionC -> connectionWeight = connectionB->connectionWeight ;
				}
				else 
				{
					// the weight will be 0
				}
				connectionA++;
				connectionB++;
				connectionC++;
			}
			neuronA++;
			neuronB++;
			neuronC++;
		}
		layerA++;
		layerB++;
		layerC++;
	}
	return child;
}



// // this function takes sensory input, transforms it across the brain, and applies the output to the body
// void performNeuralActivity (BonyFish * fish, Species * currentSpecies, bool alreadyDrawnThisTurn) {


// 	// sensorium size is based on the size of the ANN. Whether or not it is populated with numbers depends on the size of the input connector matrix.
// 	unsigned long sizeOfInputLayer = 0;
// 	unsigned long sizeOfOutputLayer = 0;
// 	unsigned long num_layers =  (unsigned long)(fish->brain->layers.size());

// 	std::list<LayerDescriptor>::iterator layer;
// 	layer = fish->brain->layers.begin();
// 	sizeOfInputLayer = layer->neurons.size();
// 	std::advance(layer, num_layers - 1);
// 	sizeOfOutputLayer = layer->neurons.size();

// 	float sensorium[ sizeOfInputLayer ];

// 	for (unsigned int j = 0; j < sizeOfInputLayer; ++j)
// 	{
// 		if (j >= N_SenseConnectorS) {
// 			continue;	// if the sensorium array is bigger than the array of input connectors, you can just leave the rest blank.
// 		}

// 		switch (fish->inputMatrix[j].sensorType) {
// 		case SENSOR_FOODRADAR:
// 			sensorium[j] = fish->bones[ fish->inputMatrix[j].connectedToLimb  ]->sensation_radar;
// 			break;

// 		case SENSOR_ALTRADAR:
// 			sensorium[j] = fish->bones[ fish->inputMatrix[j].connectedToLimb  ]->sensation_altradar;
// 			break;

// 		case SENSOR_TOUCH:
// 			sensorium[j] = fish->bones[ fish->inputMatrix[j].connectedToLimb  ]->sensation_touch;
// 			break;

// 		case SENSOR_JOINTANGLE:
// 			sensorium[j] = fish->bones[ fish->inputMatrix[j].connectedToLimb  ]->sensation_jointangle;
// 			break;


// 		case SENSOR_ABSOLUTEANGLE:
// 			sensorium[j] = fish->bones[ fish->inputMatrix[j].connectedToLimb  ]->sensation_absoluteAngle;
// 			break;

// 		case SENSOR_EYE:

// 			// performEyeRays( &fish->inputMatrix[j], fish );



// 			sensorium[j] = fish->bones[ fish->inputMatrix[j].connectedToLimb  ]->sensation_eye;


// 			break;

// 		case SENSOR_TIMER:
// 			sensorium[j] = sin(2 * M_PI * fish->inputMatrix[j].timerFreq * 	fish->inputMatrix[j].timerPhase);
// 			fish->inputMatrix[j].timerPhase += 0.001;
// 			if (fish->inputMatrix[j].timerPhase > 1) {
// 				fish->inputMatrix[j].timerPhase = 0;
// 			}

// 			break;

// 		case SenseConnector_UNUSED:
// 			sensorium[j] = 0.0f;
// 			break;

// 		case SenseConnector_RECURSORRECEIVER:
// 			// how this works.
// 			// the transmitters just set their buffer first place to the sample. that's it

// 			// the receivers trawl the transmitter list and look for one on the same channel
// 			float receivedSample = 0.0f;
// 			for (unsigned int k = 0; k < sizeOfOutputLayer; ++k)
// 			{
// 				if (k >= N_SenseConnectorS) {
// 					continue;	// if the sensorium array is bigger than the array of input connectors, you can just leave the rest blank.
// 				}

// 				if (fish->outputMatrix[k].sensorType == SenseConnector_RECURSORTRANSMITTER && fish->outputMatrix[k].recursorChannel == fish->inputMatrix[j].recursorChannel ) {
// 					receivedSample = fish->outputMatrix[k].recursorBuffer[0];// = motorSignals[j];
// 					break;
// 				}
// 			}

// 			// they set their own sample, at the scrolling cursor, to it
// 			fish->inputMatrix[j].recursorBuffer[ fish->inputMatrix[j].recursorCursor ] = receivedSample;

// 			// then they take a sample from [delay] samples ago and apply it
// 			int cursorMinusDelay = fish->inputMatrix[j].recursorCursor - fish->inputMatrix[j].recursorDelay ; // this is signed because the operation can cause it to be negative. In this case it should be wrapped around the end of the buffer.
// 			if (cursorMinusDelay < 0) {
// 				cursorMinusDelay = SenseConnector_BUFFERSIZE + cursorMinusDelay; // cursorMinusDelay is always negative here, so this is a subtraction.
// 			}

// 			sensorium[j] = fish->inputMatrix[j].recursorBuffer[cursorMinusDelay];
// 			fish->inputMatrix[j].recursorCursor ++;

// 			// the buffer loops around the max size, but there is no performance downside to doing this because you're not rolling the entire buffer
// 			if (fish->inputMatrix[j].recursorCursor >= SenseConnector_BUFFERSIZE) {
// 				fish->inputMatrix[j].recursorCursor = 0;
// 			}

// 			break;
// 		}

// 		// add noise to every input neuron regardless of what it's connected to. Helps to 'start' from zero, as well as making everything more robust and natural-feeling.
// 		sensorium[j] += (RNG() - 0.5 ) * m_deepSeaSettings.noise;

// 		// add neurostimulator noise
// 		if (fish->flagStim)
// 		{
// 			sensorium[j] += (RNG() - 0.5 ) * neurostimulatorAmplitude;

// 		}

// 		if (fish == primarilySelectedFish) {primaryFishInputSensorium[j] = sensorium[j];}

// 	}

// 	// feed information into brain
// 	float * motorSignals = fann_run(fish->ann, sensorium);

// 	for (unsigned int j = 0; j < sizeOfOutputLayer; ++j)
// 	{
// 		if (j >= N_SenseConnectorS) {
// 			continue;	// if the sensorium array is bigger than the array of input connectors, you can just leave the rest blank.
// 		}

// 		switch (fish->outputMatrix[j].sensorType) {

// 		case SenseConnector_RECURSORTRANSMITTER:
// 			fish->outputMatrix[j].recursorBuffer[0] = motorSignals[j];
// 			break;

// 		case SenseConnector_COLORSPOT:
// 			fish->bones[fish->outputMatrix[j].connectedToLimb] ->colorspotCurrentBrightness = motorSignals[j];


// 		case SenseConnector_MOTOR:

// 			if ( limbUsed(fish->bones[fish->outputMatrix[j].connectedToLimb] ) ) {

// 				if (fish->bones[fish->outputMatrix[j].connectedToLimb]->isRoot) {
// 					continue;
// 				}

// 				if ( limbAttached( fish->bones[fish->outputMatrix[j].connectedToLimb]  ) ) {

// 					if (fish->bones[fish->outputMatrix[j].connectedToLimb]->p_joint != nullptr) {

// 						float motorSpeed = motorSignals[j] * 10; // this is just a constant which is used because the brain signals are pretty small. i've never needed to change it.

// 						if (true) {
// 							float speedLimit = 10.0f;

// 							if (motorSpeed > speedLimit) {
// 								motorSpeed = speedLimit;
// 							}
// 							else if (motorSpeed < -speedLimit) {
// 								motorSpeed = -speedLimit;
// 							}
// 						}

// 						if (fish->energy > 0)
// 						{
// 							float energyUsedThisTurn = abs( motorSpeed * fish->bones[fish->outputMatrix[j].connectedToLimb]->area * fish->bones[fish->outputMatrix[j].connectedToLimb]->density * currentSpecies->muscularEfficiency  ); // Moving a limb costs energy according to the limb mass times the speed
// 							fish->energy -= energyUsedThisTurn;
// 							fish->bones[fish->outputMatrix[j].connectedToLimb]->p_joint->SetMotorSpeed(motorSpeed);

// 						}

// 					}
// 				}
// 			}
// 			break;
// 		}


// 		if (fish == primarilySelectedFish) {primaryFishOutputSensorium[j] = motorSignals[j];}
// 	}

// 	// now is a good time to draw the brain, while you have access to the sensorium.
// 	// if ( false /* TestMain::getBrainWindowStatus() */ ) {
// 	// 	if (fish->selected && !alreadyDrawnThisTurn) {
// 	// 		alreadyDrawnThisTurn = true;
// 	// 		unsigned int spacesUsedSoFar = 0;
// 	// 		drawNeuralNetworkFromDescriptor(motorSignals, sensorium, &spacesUsedSoFar, &(*fish));
// 	// 	}
// 	// }
// }
