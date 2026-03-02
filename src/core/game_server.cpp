#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#include "game_server.h"
#include "net_server.h"
#include "data_manager.h"
#include "bot.h"

static void SendGameData(ThreadInfo* info, double seconds_till_next_tick);
static void Tick();

static const double UPDATES_PER_SECOND = 5;
static const double TIME_FOR_TICK = 20;


static bool WORKING = true;

static ThreadInfo* THREAD_INFO;
static Server* server;

int comparat(const void* a, const void* b);
int comparat(const void* a, const void* b)
{
    Lot* lot_a = *(Lot**)a;
    Lot* lot_b = *(Lot**)b;

    float cen_a = (float)lot_a->price / (float)lot_a->amount;
    float cen_b = (float)lot_b->price / (float)lot_b->amount;

    return cen_a > cen_b;
}

// Запуск сервера контроля мира
void* GameServerStartup(void* data)
{
    printf("[GAME SERVER] Game server starting up...\n");

    WORKING = true;
    THREAD_INFO = GetThreads();

    server = GetServer();

    printf("[GAME SERVER] Game server started up\n");

    size_t counter = 0;

    while(WORKING)
    {
        usleep(1000000 / UPDATES_PER_SECOND);
        counter++;
        if((double)counter >= TIME_FOR_TICK * UPDATES_PER_SECOND)
        {
            counter = 0;
            Tick();
        }

        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            ThreadInfo* info = THREAD_INFO + i;
            if(!info->in_use) continue;

            SendGameData(info, TIME_FOR_TICK - (double)counter/UPDATES_PER_SECOND);
        }
    }


    printf("[GAME SERVER] Game server shutting down...\n");

    DestroyServer();

    printf("[GAME SERVER] Game server shuted down\n");

    return NULL;
}

// Выключение сервера контроля мира
void GameServerShutdown()
{
    WORKING = false;
}





// Разослать по потокам данные о мире
static void SendGameData(ThreadInfo* info, double seconds_till_next_tick)
{
    char buffer[10240] = {0};
    size_t shift = 0;

    sprintf(buffer, "004 ");
    shift = strlen(buffer);

    // Рассылка данных залогиненного игрока
    Player* player = info->player;
    if(!player)
    {
        sprintf(buffer + shift, "0 ");
        shift = strlen(buffer);
    }
    else
    {
        sprintf(buffer + shift, "1 %s %lu %lu ", player->nickname, player->agent->stocks, player->agent->money);
        shift = strlen(buffer);
    }

    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        if(player && player->agent->want_sell_lot[i])
        {
            sprintf(buffer + shift, "1 %lu %lu ", player->agent->want_sell_lot[i]->amount, player->agent->want_sell_lot[i]->price);
            shift = strlen(buffer);
        }
        else
        {
            sprintf(buffer + shift, "0 ");
            shift = strlen(buffer);
        }
    }

    sprintf(buffer + shift, "%lf ", seconds_till_next_tick);
    shift = strlen(buffer);

    // Рассылка лотов

    // size_t count = server->old_lots_count;
    // if(count > 8) count = 8;
 
    // sprintf(buffer + shift, "%lu ", count);
    // shift = strlen(buffer);

    // for(size_t i = 0; i < count; i++)
    // {
    //     Lot* lot = server->old_lots[i];

    //     sprintf(buffer + shift, "%lu %d %lu %lu ", lot->id, info->player ? lot->owner == info->player->agent : 0, lot->amount, lot->price);
    //     shift = strlen(buffer);
    // }

    // Рассылка 60 тиков
    // sprintf(buffer + shift, "%lu ", PRICE_ARRAY_COUNT);
    // shift = strlen(buffer);

    // for(size_t i = 0; i < PRICE_ARRAY_COUNT; i++)
    // {
    //     size_t j = server->cycled_list_index + i +1;
    //     if(j >= PRICE_ARRAY_COUNT) j -= PRICE_ARRAY_COUNT;
    //     sprintf(buffer + shift, "%lf ", server->cycled_list[j]);
    //     shift = strlen(buffer);
    // }

    sprintf(buffer + shift, "\n");

    send(info->data->client_socket, buffer, strlen(buffer), 0); 
}


size_t ticks = 0;

// Один игровой тик (каждые 20 секунд)
static void Tick()
{
    // Запуск ботов
    {
        ListElem* elem = server->bots->start;
        while(elem)
        {
            BotThink((Bot*)elem->value);
            elem = elem->next;
        }
    }

    // Обработка покупки лотов
    for(size_t k = 0; k < COMPANIES_COUNT; k++)
    {
        for(size_t i = 0; i < server->old_lots_count[k]; i++)
        {
            Lot* lot = server->old_lots[k][i];
            if(lot->agents_want->count > 0)
            {
                size_t highest_priority = 0;
                size_t priority_count = 0;

                ListElem* elem = lot->agents_want->start;
                while(elem)
                {
                    Agent* agent = (Agent*)elem->value;
                    size_t priority = agent->priority;
                    if(priority == highest_priority) priority_count++;
                    else if(priority > highest_priority)
                    {
                        highest_priority = priority;
                        priority_count = 1;
                    }
                    elem = elem->next;
                }

                size_t winner = rand() % priority_count;
                Agent* buyer = NULL;
                size_t count = 0;

                elem = lot->agents_want->start;
                while(elem)
                {
                    Agent* agent = (Agent*)elem->value;
                    size_t priority = agent->priority;
                    if(priority == highest_priority) 
                    {
                        if(count == winner)
                        {
                            buyer = agent;
                            break;
                        }
                        count++;
                    }

                    elem = elem->next;
                }

                buyer->money -= lot->price;
                buyer->stocks[lot->company] += lot->amount;
                lot->owner->money += lot->price;
                lot->owner->stocks[lot->company] -= lot->amount;

                ListDeleteElem(lot->owner->selling_lots, lot, DestroyLot);

                for(size_t j = i + 1; j < server->old_lots_count[k]; j++)
                {
                    server->old_lots[k][j - 1] = server->old_lots[k][j];
                }

                server->old_lots_count[k]--;
                i--;
            }
            else if(lot->canceled)
            {
                ListDeleteElem(lot->owner->selling_lots, lot, DestroyLot);

                for(size_t j = i + 1; j < server->old_lots_count[k]; j++)
                {
                    server->old_lots[k][j - 1] = server->old_lots[k][j];
                }

                server->old_lots_count[k]--;
                i--;
            }
            else if(i >= 8)
            {
                ListDeleteElem(lot->owner->selling_lots, lot, DestroyLot);
            }
        }
        if(server->old_lots_count[k] > 8) server->old_lots_count[k] = 8;
    }
    

    // Обработка новых лотов
    {
        for(size_t k = 0; k < COMPANIES_COUNT; k++)
        {
            size_t lot_count = server->lots[k]->count;
            server->old_lots_count[k] += lot_count;

            server->old_lots[k] = (Lot**)realloc(server->old_lots[k], server->old_lots_count[k] * sizeof(Lot*));

            ListElem* elem = server->lots[k]->start;
            for(size_t i = server->old_lots_count[k] - lot_count; 
                i < server->old_lots_count[k]; 
                i++, elem = elem->next)
            {
                server->old_lots[k][i] = (Lot*)elem->value;
                ListAddElem(server->old_lots[k][i]->owner->selling_lots, server->old_lots[k][i]);
            }

            ListDelete(server->lots[k], NULL);
            server->lots[k] = ListCreate();
        }
    }

    {
        ListElem* elem = server->agents->start;
        {
            while(elem)
            {
                Agent* agent = (Agent*)elem->value;

                for(size_t i = 0; i < COMPANIES_COUNT; i++)
                {
                    agent->want_sell_lot[i] = NULL;
                }
                agent->expected_money = agent->money;

                elem = elem->next;
            }
        }

        for(size_t i = 0; i < COMPANIES_COUNT; i++)
        {
            if(server->old_lots_count[i] > 0)
            {
                qsort(server->old_lots[i], server->old_lots_count[i], sizeof(Lot**), comparat);
            }
        }
    }


    // Обновление таблицы цен
    {
        for(size_t k = 0; k < COMPANIES_COUNT; k++)
        {
            double price = 0;
            if(server->old_lots_count[k] > 0)
            {
                price = (float)server->old_lots[k][0]->price/(float)server->old_lots[k][0]->amount;
            }
            else
            {
                price = server->cycled_list[k][server->cycled_list_index[k]];
            }

            server->cycled_list_index[k]++;
            if(server->cycled_list_index[k] >= PRICE_ARRAY_COUNT)
            {
                server->cycled_list_index[k] -= PRICE_ARRAY_COUNT;
            }
            server->cycled_list[k][server->cycled_list_index[k]] = price;
        }
    }

    // Снижение приоритета
    {
        ListElem* elem = server->agents->start;
        while(elem)
        {
            Agent* agent = (Agent*)elem->value;
            agent->priority /= 2;
            
            elem = elem->next;
        }

    }

    // Выдача государтвенных лотов
    {
        Agent* agent = server->goverment_agent;

        for(size_t i = 0; i < COMPANIES_COUNT; i++)
        {
            if(agent->stocks[i] >= 10)
            {
                // TODO constant
                Sell(agent, 10, 1000, i);
            }
            else if(agent->stocks[i] > 0)
            {
                Sell(agent, agent->stocks[i], 100 * agent->stocks[i], i);
            }
        }
    }
    
    // Выплата дивидентов (раз в день)
    {
        if(ticks >= 24 * 60)
        {
            ticks = 0;

            ListElem* elem = server->agents->start;
            {
                while(elem)
                {
                    Agent* agent = (Agent*)elem->value;

                    for(size_t i = 0; i < COMPANIES_COUNT; i++)
                    {
                        // TODO разные дивиденды
                        agent->money += agent->stocks[i] * (i + 1);
                    }
                
                    elem = elem->next;
                }
            }

            printf("[GAME MANAGER] Dividents\n");
        }
    }

    // Рассылка запроса тика
    
    for(size_t i = 0; i < MAX_CLIENTS; i++)
    {
        ThreadInfo* info = THREAD_INFO + i;
        if(!info->in_use) continue;
        
        char buffer[] = "013 Tick";
        send(info->data->client_socket, buffer, strlen(buffer), 0); 
    }



    Save();
}