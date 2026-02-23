#include <stdlib.h>
#include "neural_network.h"

void SetupNeuron(Neuron* neuron, NeuronFunc func, size_t koeff_count)
{
    Neuron* neuron = (Neuron*)calloc(1, sizeof(Neuron));
    if(!neuron) return;

    neuron->koeff_count = koeff_count;
    neuron->koeffs = (double*)calloc(koeff_count, sizeof(double));
    if(!neuron->koeffs)
    {
        free(neuron);
        return;
    }

}

Network* CreateNetwork(NetworkType type)
{
    Network* network = (Network*)calloc(1, sizeof(Network));
    if(!network) return NULL;

    network->type = type;
    for(size_t i = 0; i < layers_count; i++)
    {
        size_t neuron_count = type == NETWORK_BUY ? neuron_count_buy[i] : neuron_count_sell[i];

        network->layers[i] = (Neuron*)calloc(neuron_count, sizeof(Neuron));
        for(size_t j = 0; j < neuron_count; j++)
        {
            size_t koeff_count = i == 0 ? 0 : (type == NETWORK_BUY ? neuron_count_buy[i - 1] : neuron_count_sell[i - 1];)
            SetupNeuron(network->layers[i][j], SIGMOID, koeff_count);
        }
                                        
    }
    // TODO
}

void DestroyNetwork(Network* network);

void RunNetwork(Network* network); 