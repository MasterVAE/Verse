#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <stdlib.h>

struct Neuron
{
    double value;
    double k;
};

struct Network
{
    size_t layer_count;
    size_t* layers;
    Neuron** neurons;

    double* koefs;
};

Network* CreateBuyNetwork();
Network* CreateSellNetwork();
Network* CreatePriorityNetwork();

void DestroyNetwork(Network* net);

void RunNetwork(Network* net);

void RandomNetwork(Network* net);

Network* CopyNetwork(Network* net);
void EvolveNetwork(Network* net);

#endif // NEURAL_NETWORK_H