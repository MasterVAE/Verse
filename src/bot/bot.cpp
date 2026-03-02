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
            int buy = rand() % 2;
            if(buy)
            {
                Buy(bot->agent, server->old_lots[k][i]->id);
            }
        }
    }

    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        int amount = rand() % ((int)bot->agent->stocks[i] + 1);
        int price = rand() % 5000;    
        
        Sell(bot->agent, (size_t)amount, (size_t)price, i);
    }

    BuyPriority(bot->agent, rand() % 100);
}