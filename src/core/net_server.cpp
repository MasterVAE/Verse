#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <assert.h>

#include "net_server.h"
#include "core.h"
#include "data_manager.h"
#include "net_request.h"

static void* HandleClient(void* arg);

size_t CLIENTS = 0;
static bool WORKING = true;

// Массив информации о потоках
static ThreadInfo THREAD_INFO[MAX_CLIENTS];

// Контроль TCP туннеля с пользователем
static void* HandleClient(void* arg) 
{
    ThreadInfo* info = (ThreadInfo*)arg;
    ClientData* client_data = info->data;
    int client_socket = client_data->client_socket;
    
    printf("[NET SERVER THREAD %lu] Client connected to pipe\n", info->num);
    
    char buffer[BUFFER_SIZE] = {0};
    
    while (WORKING) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read <= 0) 
        {
            printf("[NET SERVER THREAD %lu] Client disconnected\n", info->num);
            break;
        }
        
        buffer[bytes_read] = '\0';

        printf("[NET SERVER THREAD %lu] %lu:%s\n", info->num, strlen(buffer), buffer);

        // Обрабатываем запрос пользователя
        ParseRequest(info, buffer);
    }
    
    close(client_socket);
    
    // Освобождаем слот в массиве потоков
    info->in_use = false;
    if(info->player)    info->player->thread = NULL;
    info->player = NULL;
    CLIENTS--;
    
    printf("[NET SERVER THREAD %lu] Connection closed\n", info->num);
    
    free(client_data); // Освобождаем память структуры
    pthread_exit(NULL);
}




// Запуск сервера сетевых подключений
void* NetServerStartup(void* data) 
{
    printf("[NET SERVER] Net server starting up...\n");
    
    WORKING = true;
    
    // Инициализация информации о потоках
    for (size_t i = 0; i < MAX_CLIENTS; i++) 
    {
        THREAD_INFO[i].in_use = false;
        THREAD_INFO[i].num = i;
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
        
        // Отмечаем слот как занятый
        THREAD_INFO[thread_index].in_use = true;
        THREAD_INFO[thread_index].data = client_data;
        
        // Создаем поток для обработки клиента
        if (pthread_create(&THREAD_INFO[thread_index].thread, NULL,
                          HandleClient, (void*)&THREAD_INFO[thread_index]) != 0) 
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
    
    printf("[NET SERVER] Net server shutting down...\n");

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

ThreadInfo* GetThreads()
{
    return THREAD_INFO;
}