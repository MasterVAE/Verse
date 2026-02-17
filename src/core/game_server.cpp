#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "game_server.h"
#include "net_server.h"
#include "data_manager.h"

void SendGameData(ThreadInfo* info);
void Tick();

static bool WORKING = true;

static ThreadInfo* THREAD_INFO;

// Запуск сервера контроля мира
void* GameServerStartup(void* data)
{
    printf("[GAME SERVER] Game server starting up...\n");

    WORKING = true;
    THREAD_INFO = GetThreads();

    printf("[GAME SERVER] Game server started up\n");

    size_t counter = 0;

    while(WORKING)
    {
        usleep(100000);
        counter++;
        if(counter >= 600)
        {
            counter = 0;
            Tick();
        }

        for(size_t i = 0; i < MAX_CLIENTS; i++)
        {
            ThreadInfo* info = THREAD_INFO + i;
            if(!info->in_use) continue;

            SendGameData(info);
        }
    }

    printf("[GAME SERVER] Game server shutting down...\n");

    printf("[GAME SERVER] Game server shuted down\n");

    return NULL;
}

void GameServerShutdown()
{
    WORKING = false;
}

void SendGameData(ThreadInfo* info)
{
    char buffer[1024] = {0};
    size_t i = 0;

    sprintf(buffer, "004 ");
    i = strlen(buffer);

    Player* player = info->player;
    if(!player)
    {
        sprintf(buffer + i, "0 ");
        i = strlen(buffer);
    }
    else
    {
        sprintf(buffer + i, "1 %s ", player->nickname);
        i = strlen(buffer);
    }

    send(info->data->client_socket, buffer, strlen(buffer), 0); 
}

void Tick()
{
    Save();
}