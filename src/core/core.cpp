#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "net_server.h"
#include "game_server.h"
#include "data_manager.h"
#include "core.h"

static bool WORKING = true;

pthread_t THREADS[2];

// Запуск ядра
int CoreStartup()
{
    printf("[CORE] Core starting up...\n");

    CreateServer();

    WORKING = true;

    // Запуск сервера подключений в отдельном потоке
    if (pthread_create(&THREADS[0], NULL, NetServerStartup, NULL) != 0) 
    {
        fprintf(stderr, "Creating thread for net server failed\n");
        return EXIT_FAILURE;
    }
    pthread_detach(THREADS[0]);

    // Запуск игрового сервера в отдельном потоке
    if (pthread_create(&THREADS[1], NULL, GameServerStartup, NULL) != 0) 
    {
        fprintf(stderr, "Creating thread for game server failed\n");
        return EXIT_FAILURE;
    }
    pthread_detach(THREADS[1]);

    Load();

    printf("[CORE] Core started\n");

    while(WORKING);

    printf("[CORE] Core shutting down...\n");

    NetServerShutdown();
    GameServerShutdown();

    printf("[CORE] Core shut down\n");

    return EXIT_SUCCESS;
}


// Отключение ядра
void CoreShutdown()
{
    WORKING = false;
}

