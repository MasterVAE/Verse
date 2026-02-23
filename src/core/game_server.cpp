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

static void SendGameData(ThreadInfo* info, double seconds_till_next_tick);
static void Tick();


static const double UPDATED_PER_SECOND = 5;
static const double TIME_FOR_TICK = 20;

static bool WORKING = true;

static ThreadInfo* THREAD_INFO;
Server* server;

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
        usleep(1000000 / UPDATED_PER_SECOND);
        counter++;
        if(counter >= TIME_FOR_TICK * UPDATED_PER_SECOND)
        {
            counter = 0;
            Tick();
        }

        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            ThreadInfo* info = THREAD_INFO + i;
            if(!info->in_use) continue;

            SendGameData(info, TIME_FOR_TICK - counter/UPDATED_PER_SECOND);
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

    sprintf(buffer + shift, "%lf ", seconds_till_next_tick);
    shift = strlen(buffer);

    // Рассылка лотов
    sprintf(buffer + shift, "%lu ", server->old_lots_count);
    shift = strlen(buffer);

    for(size_t i = 0; i < server->old_lots_count; i++)
    {
        Lot* lot = server->old_lots[i];

        sprintf(buffer + shift, "%lu %lu %lu ", lot->id, lot->amount, lot->price);
        shift = strlen(buffer);
    }

    // Рассылка 60 тиков
    sprintf(buffer + shift, "%lu ", PRICE_ARRAY_COUNT);
    shift = strlen(buffer);

    for(size_t i = 0; i < PRICE_ARRAY_COUNT; i++)
    {
        size_t j = server->cycled_list_index + i +1;
        if(j >= PRICE_ARRAY_COUNT) j -= PRICE_ARRAY_COUNT;
        sprintf(buffer + shift, "%lf ", server->cycled_list[j]);
        shift = strlen(buffer);
    }

    sprintf(buffer + shift, "\n");

    send(info->data->client_socket, buffer, strlen(buffer), 0); 
}


size_t ticks = 0;

// Один игровой тик (каждые 60 секунд)
static void Tick()
{
    // Запуск ботов

    // Обработка покупки лотов
    for(size_t i = 0; i < server->old_lots_count; i++)
    {
        Lot* lot = server->old_lots[i];
        if(lot->agents_want_count > 0)
        {
            int min = 0;
            int max = (int)lot->agents_want_count - 1;
            int winner = (rand() % (max - min + 1)) + min;

            Agent* buyer = lot->agents_want[winner];
            buyer->money -= lot->price;
            buyer->stocks += lot->amount;
            lot->owner->money += lot->price;
            lot->owner->stocks -= lot->amount;

            ListDeleteElem(lot->owner->selling_lots, lot, DestroyLot);

            for(size_t j = i + 1; j < server->old_lots_count; j++)
            {
                server->old_lots[j - 1] = server->old_lots[j];
            }

            server->old_lots_count--;
            i--;
        }
        else if(i >= 8)
        {
            ListDeleteElem(lot->owner->selling_lots, lot, DestroyLot);
        }
    }
    if(server->old_lots_count > 8) server->old_lots_count = 8;
    

    // Обработка новых лотов
    {
        size_t lot_count = server->lots->count;
        server->old_lots_count += lot_count;

        server->old_lots = (Lot**)realloc(server->old_lots, server->old_lots_count * sizeof(Lot*));

        ListElem* elem = server->lots->start;
        for(size_t i = server->old_lots_count - lot_count; 
            i < server->old_lots_count; 
            i++, elem = elem->next)
        {
            server->old_lots[i] = (Lot*)elem->value;
            ListAddElem(server->old_lots[i]->owner->selling_lots, server->old_lots[i]);
        }

        ListDelete(server->lots, NULL);
        server->lots = ListCreate();
    }

    {
        ListElem* elem = server->agents->start;
        {
            while(elem)
            {
                Agent* agent = (Agent*)elem->value;

                agent->want_sell_lot = NULL;
                agent->expected_money = agent->money;

                elem = elem->next;
            }
        }

        if(server->old_lots_count > 0)
        {
            qsort(server->old_lots, server->old_lots_count, sizeof(Lot**), comparat);
        }
    }


    // Обновление таблицы цен
    {
        double price = 0;
        if(server->old_lots_count > 0)
        {
            price = (float)server->old_lots[0]->price/(float)server->old_lots[0]->amount;
        }
        else
        {
            price = server->cycled_list[server->cycled_list_index];
        }

        server->cycled_list_index++;
        if(server->cycled_list_index >= PRICE_ARRAY_COUNT)
        {
            server->cycled_list_index -= PRICE_ARRAY_COUNT;
        }
        server->cycled_list[server->cycled_list_index] = price;
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

                    agent->money += agent->stocks;

                    elem = elem->next;
                }
            }

            printf("[GAME MANAGER] Dividents\n");
        }
    }
    Save();
}