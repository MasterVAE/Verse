#ifndef NET_SERVER_H
#define NET_SERVER_H


void* NetServerStartup(void* data);
void NetServerShutdown();

#define PORT 5432
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

struct Player;

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

struct Player
{
    const char* nickname;
    const char* password;

    ThreadInfo* thread;
    bool online;

    Player* next;
};

void SendAll(const char* message);

#endif // NET_SERVER_H