#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "net_server.h"
#include "core.h"

static void* HandleClient(void* arg);

size_t CLIENTS = 0;
static bool WORKING = true;

// Структура для хранения информации о потоке
struct ThreadInfo 
{
    pthread_t thread;
    bool in_use;
};

// Массив информации о потоках
static ThreadInfo THREAD_INFO[MAX_CLIENTS];

// Структура для передачи данных в поток
struct ClientData 
{
    int client_socket;
    sockaddr_in client_addr;
    int thread_index;  // Индекс в массиве THREAD_INFO
};

static void* HandleClient(void* arg) 
{
    ClientData* data = (ClientData*)arg;
    int client_socket = data->client_socket;
    sockaddr_in client_addr = data->client_addr;
    int thread_index = data->thread_index;
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);
    
    printf("[NET SERVER THREAD] Client %s:%d connected\n", client_ip, client_port);
    
    char buffer[BUFFER_SIZE] = {0};
    const char* welcome = "Welcome! Type 'exit' to quit.\n";
    send(client_socket, welcome, strlen(welcome), 0);
    
    while (WORKING) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read <= 0) {
            printf("[NET SERVER THREAD] Client %s:%d disconnected\n", client_ip, client_port);
            break;
        }
        
        buffer[bytes_read] = '\0';
        
        printf("[NET SERVER THREAD] %s:%d: %s\n", client_ip, client_port, buffer);
        
        if (strncmp(buffer, "exit", 4) == 0 || strncmp(buffer, "quit", 4) == 0) 
        {
            const char* bye = "Goodbye!\n";
            send(client_socket, bye, strlen(bye), 0);
            break;
        }
        
        if (strncmp(buffer, "shutdown", 4) == 0) 
        {
            const char* bye = "Goodbye!\n";
            send(client_socket, bye, strlen(bye), 0);
            CoreShutdown();
            break;
        }
        
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "Echo: %s\n", buffer);
        send(client_socket, response, strlen(response), 0);
    }
    
    close(client_socket);
    
    // Освобождаем слот в массиве потоков
    THREAD_INFO[thread_index].in_use = false;
    CLIENTS--;
    
    printf("[NET SERVER THREAD] Connection %s:%d closed\n", client_ip, client_port);
    
    free(data); // Освобождаем память структуры
    pthread_exit(NULL);
}

void* NetServerStartup(void* data) 
{
    printf("[NET SERVER] Net server starting up...\n");
    
    WORKING = true;
    
    // Инициализация информации о потоках
    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        THREAD_INFO[i].in_use = false;
    }
    
    int server_fd;
    sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket failed");
        CoreShutdown();
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    {
        perror("setsockopt failed");
        close(server_fd);
        CoreShutdown();
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) 
    {
        perror("bind failed");
        close(server_fd);
        CoreShutdown();
    }
    
    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        close(server_fd);
        CoreShutdown();
    }
    
    printf("[NET SERVER] Net server started up\n");
    printf("\n[NET SERVER] Multi-threaded server listening on port %d\n", PORT);
    printf("[NET SERVER] Maximum clients: %d\n", MAX_CLIENTS);
    
    while (WORKING) 
    {
        printf("\n[NET SERVER] Waiting for connection (%lu active)...\n", CLIENTS);
        
        // Принимаем новое подключение
        int client_socket = accept(server_fd, (sockaddr*)&address, &addrlen);
        if (client_socket < 0) 
        {
            perror("accept failed");
            continue;
        }
        
        // Ищем свободный слот для потока
        int thread_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) 
        {
            if (!THREAD_INFO[i].in_use) 
            {
                thread_index = i;
                break;
            }
        }
        
        if (thread_index == -1) 
        {
            printf("[NET SERVER] Too many users. Connection rejected\n");
            const char* msg = "Server is full. Try again later.\n";
            send(client_socket, msg, strlen(msg), 0);
            close(client_socket);
            continue;
        }
        
        // Создаем структуру данных для потока
        ClientData* client_data = (ClientData*)malloc(sizeof(ClientData));
        client_data->client_socket = client_socket;
        client_data->client_addr = address;
        client_data->thread_index = thread_index;
        
        // Отмечаем слот как занятый
        THREAD_INFO[thread_index].in_use = true;
        
        // Создаем поток для обработки клиента
        if (pthread_create(&THREAD_INFO[thread_index].thread, NULL,
                          HandleClient, (void*)client_data) != 0) 
        {
            perror("pthread_create failed");
            THREAD_INFO[thread_index].in_use = false;
            free(client_data);
            close(client_socket);
            continue;
        }
        
        // Отсоединяем поток
        pthread_detach(THREAD_INFO[thread_index].thread);
        
        CLIENTS++;
        printf("[NET SERVER] New client accepted. Total connections: %lu\n", CLIENTS);
    }
    
    printf("[NET SERVER] Net server shutting down...");

    // Ожидание завершения всех потоков при shutdown
    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        if (THREAD_INFO[i].in_use) 
        {
            pthread_join(THREAD_INFO[i].thread, NULL);
        }
    }
    
    close(server_fd);

    printf("[NET SERVER] Net server shutted down...");

    return NULL;
}

void NetServerShutdown() 
{
    WORKING = false;
}