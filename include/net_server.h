#ifndef NET_SERVER_H
#define NET_SERVER_H

void* NetServerStartup(void* data);
void NetServerShutdown();

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 2

#endif // NET_SERVER_H