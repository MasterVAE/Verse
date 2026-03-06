#include <assert.h>

#include "neural_network.h"
#include "lib.h"

static Network* CreateNetwork(size_t layer_count, size_t* layers);

static Network* CreateNetwork(size_t layer_count, size_t* layers)
{
    assert(layers);

    Network* net = (Network*)calloc(1, sizeof(Network));
    if(!net) return NULL;

    net->layer_count = layer_count;
    net->layers = (size_t*)calloc(layer_count, sizeof(size_t));
    net->neurons = (Neuron**)calloc(layer_count, sizeof(Neuron*));

    for(size_t i = 0; i < layer_count; i++)
    {
        net->layers[i] = layers[i];
        net->neurons[i] = (Neuron*)calloc(layers[i], sizeof(Neuron));  
    }

    size_t koefs_count = 0;
    for(size_t i = 1; i < layer_count; i++)
    {
        koefs_count += layers[i - 1] * layers[i];
    }

    net->koefs = (double*)calloc(koefs_count, sizeof(double));

    return net;
}

void DestroyNetwork(Network* net)
{
    if(!net) return;
    for(size_t i = 0; i < net->layer_count; i++)
    {
        free(net->neurons[i]);
    }

    free(net->neurons);
    free(net->layers);

    free(net);
}

Network* CreateBuyNetwork()
{
    size_t layer_count = 8;
    size_t layers[8] = {60, 80, 100, 120, 120, 100, 60, 1};

    return CreateNetwork(layer_count, layers);
}

Network* CreateSellNetwork()
{
    size_t layer_count = 8;
    size_t layers[8] = {60, 80, 100, 120, 120, 100, 60, 3};

    return CreateNetwork(layer_count, layers);
}

Network* CreatePriorityNetwork()
{
    size_t layer_count = 8;
    size_t layers[8] = {60, 80, 100, 120, 120, 100, 60, 2};

    return CreateNetwork(layer_count, layers);
}



// Запустить сеть
void RunNetwork(Network* net)
{
    assert(net);

    size_t koef = 0;

    
    for(size_t i = 1; i < net->layer_count; i++)
    {
        for(size_t j = 0; j < net->layers[i]; j++)
        {
            net->neurons[i][j].value = 0;
            for(size_t k = 0; k < net->layers[i - 1]; k++)
            {
                net->neurons[i][j].value += net->neurons[i - 1][k].value * net->koefs[koef++];
            }
            net->neurons[i][j].value = Sigmoid(net->neurons[i][j].k, net->neurons[i][j].value);
        }
    }
}


// Расставить случайные коэффициенты
void RandomNetwork(Network* net)
{
    assert(net);

    for(size_t i = 0; i < net->layer_count; i++)
    {
        for(size_t j = 0; j < net->layers[i]; j++)
        {
            net->neurons[i][j].k = ((double)(rand()%10000))/10000;
        }
    }

    size_t koefs_count = 0;
    for(size_t i = 1; i < net->layer_count; i++)
    {
        koefs_count += net->layers[i - 1] * net->layers[i];
    }

    for(size_t i = 0; i < koefs_count; i++)
    {
        net->koefs[i] = ((double)(rand()%10000))/10000;
    }
}


// Создать копию нейросети
Network* CopyNetwork(Network* net)
{
    assert(net);

    Network* new_net = CreateNetwork(net->layer_count, net->layers);
    if(!new_net) return NULL;

    for(size_t i = 0; i < net->layer_count; i++)
    {
        for(size_t j = 0; j < net->layers[i]; j++)
        {
            new_net->neurons[i][j].k = net->neurons[i][j].k;
        }
    }

    size_t koefs_count = 0;
    for(size_t i = 1; i < net->layer_count; i++)
    {
        koefs_count += net->layers[i - 1] * net->layers[i];
    }

    for(size_t i = 0; i < koefs_count; i++)
    {
        new_net->koefs[i] = net->koefs[i];
    }

    return new_net;
}


// Сделать небольшие изменения в сети
void EvolveNetwork(Network* net)
{
    assert(net);

    int rnd = rand() % 10;
    if(rnd == 0)
    {

    }

    // TODO
}
