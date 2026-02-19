#ifndef NET_SERVER_H
#define NET_SERVER_H


void* NetServerStartup(void* data);
void NetServerShutdown();

#define PORT 5432
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

const size_t DATA_COUNT = 60;

struct Player;
struct Agent;
struct Bot;

// Структура для передачи данных в поток
struct ClientData 
{
    int client_socket;
};

// Структура для хранения информации о потоке
struct ThreadInfo 
{
    size_t num;
    pthread_t thread;
    ClientData* data;
    Player* player;
    bool in_use;
};

ThreadInfo* GetThreads();

#endif // NET_SERVER_H