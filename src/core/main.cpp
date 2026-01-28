#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "server.h"

int main() 
{   
    printf("Core starting...\n");

    LaunchServer(NULL);

    pthread_t thread;
    
    if (pthread_create(&thread, NULL, LaunchServer, NULL) != 0) 
    {
        fprintf(stderr, "Creating thread for net server failed\n");
        return EXIT_FAILURE;
    }

    pthread_detach(thread);

    while(1);

    return EXIT_SUCCESS;
}