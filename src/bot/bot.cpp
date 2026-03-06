#include <assert.h>
#include <stdio.h>

#include "bot.h"
#include "data_manager.h"


static Server* server = NULL;

// Работа бота
void BotThink(Bot* bot)
{
    assert(bot);

    if(!server) server = GetServer();

    for(size_t k = 0; k < COMPANIES_COUNT; k++)
    {
        for(size_t i = 0; i < server->old_lots_count[k]; i++)
        {
            Network* net = bot->buy_net;
            for(size_t l = 0; l < net->layers[0]; l++)
            {
                net->neurons[0][l].value = 1;
            }

            // TODO input values

            RunNetwork(bot->buy_net);
            if(bot->buy_net->neurons[bot->buy_net->layer_count - 1][0].value > 0.5)
            {
                Buy(bot->agent, server->old_lots[k][i]->id);
            }
        }
    }

    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        Network* net = bot->sell_net;
        for(size_t l = 0; l < net->layers[0]; l++)
        {
            net->neurons[0][l].value = 1;
        }
        // TODO input values



        RunNetwork(net);
        if(net->neurons[net->layer_count - 1][0].value > 0.5)
        {
            Sell(bot->agent, 
                (size_t)(net->neurons[net->layer_count - 1][1].value * bot->agent->stocks[i]), 
                (size_t)(1 / (1 - net->neurons[net->layer_count - 1][2].value)), 
                i);
        }
        
    }

    Network* net = bot->priority_net;

    RunNetwork(net);


    if(net->neurons[net->layer_count - 1][0].value > 0.5)
    {
        BuyPriority(bot->agent, 
                    (size_t)(net->neurons[net->layer_count - 1][1].value * bot->agent->money));
    }
}