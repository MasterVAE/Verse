#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

size_t CLIENTS = 0;

// Структура для передачи данных в поток
struct ClientData 
{
    int client_socket;
    sockaddr_in client_addr;
    bool autorized;
    bool active;
};

// Функция для обработки клиента в отдельном потоке
void* HandleClient(void* arg) 
{
    ClientData* data = (ClientData*)arg;

    int client_socket = data->client_socket;
    sockaddr_in client_addr = data->client_addr;
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);
    
    printf("[Thread] Client %s:%d connected\n", client_ip, client_port);
    
    char buffer[BUFFER_SIZE] = {0};
    const char* welcome = "Welcome! Type 'exit' to quit.\n";
    send(client_socket, welcome, strlen(welcome), 0);
    
    while (1) 
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read <= 0) 
        {
            printf("[Thread] Client %s:%d disconnected\n", client_ip, client_port);
            break;
        }
        
        buffer[bytes_read] = '\0';
        
        // Убираем символы новой строки
        char* p = buffer;
        while (*p) 
        {
            if (*p == '\n' || *p == '\r') *p = '\0';
            p++;
        }
        
        printf("[Thread] %s:%d: %s\n", client_ip, client_port, buffer);
        
        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) 
        {
            const char* bye = "Goodbye!\n";
            send(client_socket, bye, strlen(bye), 0);
            break;
        }
        
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "Echo: %s\n", buffer);
        send(client_socket, response, strlen(response), 0);
    }
    
    close(client_socket);
    free(data); // Освобождаем память структуры
    
    printf("[Thread] Connection %s:%d closed\n", client_ip, client_port);
    pthread_exit(NULL);

    CLIENTS--;
}

int main() 
{
    int server_fd;
    sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) 
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("Multi-threaded server listening on port %d\n", PORT);
    printf("Maximum clients: %d\n", MAX_CLIENTS);
    
    pthread_t threads[MAX_CLIENTS];
    
    while (1) 
    {
        printf("\n[Main] Waiting for connection (%d active)...\n", CLIENTS);
        
        // Принимаем новое подключение
        int client_socket = accept(server_fd, (sockaddr*)&address, &addrlen);
        if (client_socket < 0) 
        {
            perror("accept failed");
            continue;
        }
        
        // Создаем структуру данных для потока
        ClientData* data = (ClientData*)malloc(sizeof(ClientData));
        data->client_socket = client_socket;
        data->client_addr = address;
        data->active = true;
        data->autorized = false;
        
        // Создаем поток для обработки клиента
        if (pthread_create(&threads[CLIENTS % MAX_CLIENTS], NULL, 
                          HandleClient, (void*)data) != 0) 
        {
            perror("pthread_create failed");
            free(data);
            close(client_socket);
            continue;
        }
        
        // Отсоединяем поток (автоматически освобождает ресурсы при завершении)
        pthread_detach(threads[CLIENTS % MAX_CLIENTS]);
        
        CLIENTS++;
        printf("[Main] New client accepted. Total connections: %d\n", CLIENTS);
    }
    
    close(server_fd);
    return 0;
}