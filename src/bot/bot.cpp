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

    for(size_t i = 0; i < server->old_lots_count; i++)
    {
        int buy = rand() % 2;
        if(buy)
        {
            Buy(bot->agent, server->old_lots[i]->id);
        }
    }

    int amount = rand() % (bot->agent->stocks + 1);
    int price = rand() % 5000;    
    
    Sell(bot->agent, amount, price);
}