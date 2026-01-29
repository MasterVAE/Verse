#ifndef NET_SERVER_H
#define NET_SERVER_H

void* NetServerStartup(void* data);
void NetServerShutdown();

#define PORT 5432
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

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
    bool in_use;
};

void SendAll(const char* message);
void SendUser(const char* username, const char* message);

#endif // NET_SERVER_H