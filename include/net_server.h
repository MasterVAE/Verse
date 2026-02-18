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

// Структура для хранения данных о мире
struct Server
{
    Player* players;
    double data[DATA_COUNT];
    size_t data_start;
};


// Структура для хранения информации о лоте
struct Lot
{
    bool isSell;

    size_t amount;
    size_t price;

    Agent* owner;
};

// Структура для хранения информации о боте
struct Bot
{
    Agent* agent;
};

// Структура агента на бирже
struct Agent
{
    bool isPlayer;

    Player* player;
    Bot* bot;

    size_t money;
    size_t stocks;

    size_t expected_money;
    size_t expected_stocks;
};

// Структура для хранения информации о игроке
struct Player
{
    char* nickname;
    char* password;

    ThreadInfo* thread;

    Player* next;
    Player* prev;

    Agent* agent;
};

ThreadInfo* GetThreads();

#endif // NET_SERVER_H