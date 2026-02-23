#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include "stdlib.h"

enum NeuronFunc
{
    LINEAR,         // y = x
    SIGMOID,        // y = 2/(1 + e^-x) - 1
    HYBERBOLOID     // y = 1/(1 - x) - 1
};

enum NetworkType
{
    NETWORK_BUY,
    NETWORK_SELL
};

const size_t neuron_count_buy[] = {64, 100, 50, 25, 10, 1};
const size_t neuron_count_sell[] = {64, 100, 50, 30, 20, 3};

const size_t layers_count = sizeof(neuron_count_buy)/sizeof(neuron_count_buy[0]);

struct Neuron
{
    size_t koeff_count;
    double* koeffs;

    double sigmoid_ n k;
};

struct Network
{
    NetworkType type;

    Neuron* layers[layers_count];
};


void SetupNeuron(Neuron* neuron, NeuronFunc func, size_t koeff_count);
Network* CreateNetwork(NetworkType type);

void DestroyNetwork(Network* network);

void RunNetwork(Network* network); 

#endif // NEURAL_NETWORK_H