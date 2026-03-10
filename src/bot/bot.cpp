#include <assert.h>
#include <stdio.h>

#include "bot.h"
#include "data_manager.h"

static void BotThink(Bot* bot);


static Server* server = NULL;

void* BotsThink(void* argv)
{
    if(!server) server = GetServer();

    ListElem* elem = server->bots->start;
    while(elem)
    {
        BotThink((Bot*)elem->value);
        elem = elem->next;
    }

    return NULL;
}


// Работа бота
static void BotThink(Bot* bot)
{
    assert(bot);

    // Buying
    pthread_mutex_lock(&server->mutex);
    Lot* lots[COMPANIES_COUNT] = {0};
    size_t lots_count[COMPANIES_COUNT] = {0};
    for(size_t k = 0; k < COMPANIES_COUNT; k++)
    {
        lots[k] = (Lot*)calloc(server->old_lots_count[k], sizeof(Lot));
        lots_count[k] = server->old_lots_count[k];

        for(size_t i = 0; i < server->old_lots_count[k]; i++)
        {
            lots[k][i].amount = server->old_lots[k][i]->amount;
            lots[k][i].price = server->old_lots[k][i]->price;
            lots[k][i].id = server->old_lots[k][i]->id;
        }
    }
    pthread_mutex_unlock(&server->mutex);

    for(size_t k = 0; k < COMPANIES_COUNT; k++)
    {
        for(size_t i = 0; i < server->old_lots_count[k]; i++)
        {   
            Lot* lot = &lots[k][i];

            size_t id = lots[k][i].id;

            Network* net = bot->buy_net;

            net->neurons[0][0].value = 1;
            net->neurons[0][1].value = 0;
            net->neurons[0][2].value = Sigmoid(net->neurons[0][2].k, bot->agent->expected_money);
            net->neurons[0][3].value = Sigmoid(net->neurons[0][3].k, bot->agent->stocks[k]);
            net->neurons[0][4].value = Sigmoid(net->neurons[0][4].k, bot->agent->priority);
            net->neurons[0][5].value = Sigmoid(net->neurons[0][5].k, ((double)lot->price)/lot->amount);
            for(size_t l = 0; l < 60; l++)
            {
                size_t index = (server->cycled_list_index[k] + l) % 60;

                net->neurons[0][6 + l].value = server->cycled_list[k][index];
            }


            RunNetwork(net);
            if(net->neurons[net->layer_count - 1][0].value > 0.5)
            {
                Buy(bot->agent, id);    
            }

        }
    }

    for(size_t k = 0; k < COMPANIES_COUNT; k++)
    {
        free(lots[k]);
    }

    // Selling
    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        Network* net = bot->sell_net;
        net->neurons[0][0].value = 1;
        net->neurons[0][1].value = 0;
        net->neurons[0][2].value = Sigmoid(net->neurons[0][2].k, bot->agent->expected_money);
        net->neurons[0][3].value = Sigmoid(net->neurons[0][3].k, bot->agent->stocks[i]);
        net->neurons[0][4].value = Sigmoid(net->neurons[0][4].k, bot->agent->priority);
        for(size_t l = 0; l < 60; l++)
        {
            size_t index = (server->cycled_list_index[i] + l) % 60;
            net->neurons[0][5 + l].value = server->cycled_list[i][index];
        }


        RunNetwork(net);
        if(net->neurons[net->layer_count - 1][0].value > 0.5)
        {
            Sell(bot->agent, 
                (size_t)(net->neurons[net->layer_count - 1][1].value * bot->agent->stocks[i]), 
                (size_t)(1 / (1 - net->neurons[net->layer_count - 1][2].value)), 
                i);
        }
        
    }

    // Network* net = bot->priority_net;

    // RunNetwork(net);


    // if(net->neurons[net->layer_count - 1][0].value > 0.5)
    // {
    //     BuyPriority(bot->agent, 
    //                 (size_t)(net->neurons[net->layer_count - 1][1].value * bot->agent->money));
    // }
}