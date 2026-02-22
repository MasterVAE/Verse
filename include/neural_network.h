#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

enum NeuronFunc
{
    LINEAR,         // y = x
    SIGMOID,        // y = 1/(1 + e^-x)
    HYBERBOLOID     // y = 1/(1 - x) - 1
};

struct Neuron
{

};


struct Network
{
    
};

void CreateNetwork();
void SetupNetwork();
void DestroyNetwork();

#endif // NEURAL_NETWORK_H