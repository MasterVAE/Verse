#include <stdio.h>

#include "game_server.h"

static bool WORKING = true;

void* GameServerStartup(void* data)
{
    printf("[GAME SERVER] Game server starting up...\n");

    WORKING = true;

    printf("[GAME SERVER] Game server started up\n");

    while(WORKING)
    {
        // TODO
    }

    printf("[GAME SERVER] Game server shutting down...\n");

    printf("[GAME SERVER] Game server shuted down\n");

    return NULL;
}

void GameServerShutdown()
{
    WORKING = false;
}