#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>

#include "net_request.h"
#include "net_server.h"
#include "core.h"
#include "data_manager.h"
#include "protocol.h"


#define RESPONSE(response)                                                  \
{                                                                           \
    send(info->data->client_socket, response, strlen(response), 0);         \
    return;                                                                 \
}

#define TEXT(a) #a

// Обработать запрос на потоке
void ParseRequest(ThreadInfo* info, const char* buffer)
{
    assert(info);
    assert(buffer);

    int request_number = 0;
    
    if(!sscanf(buffer, "%d", &request_number))
    {
        RESPONSE("301 Incorrect request\n");
    }

    if(request_number >= 300)
    {
        fprintf(stderr, "Error: %d\n", request_number);
    }

    if(request_number >= 200) return;

    switch(request_number)
    {
        case(PROTOCOL_CLIENT_CONNECTION_CHECK):
        {
            RESPONSE("200 Connection successful\n");
        }
        case(PROTOCOL_CLIENT_LOGIN):
        {
            char login[100] = {0};
            char password[100] = {0};

            if(sscanf(buffer + 3, "%99s %99s", login, password) < 2)
            {
                RESPONSE("301 Incorrect request\n");
            }

            Player* player = LogInPlayer(login, password);
            if(player)
            {
                info->player = player;
                player->thread = info;

                RESPONSE("202 Login successful\n");
            }
            else
            {
                RESPONSE("302 Login error\n");
            }

            return;
        }
        case(PROTOCOL_CLIENT_REGISTRATION):
        {
            char login[100] = {0};
            char password[100] = {0};

            if(sscanf(buffer + 4, "%99s %99s", login, password) < 2)
            {
                RESPONSE("301 Incorrect request\n");
            }

            Player* player = RegisterPlayer(login, password);
            if(player)
            {
                info->player = player;
                player->thread = info;

                RESPONSE("202 Registration successful\n");
            }
            else
            {
                RESPONSE("302 Login error\n");
            }

            return;
        }
        case(PROTOCOL_CLIENT_SHUTDOWN):
        {       
            CoreShutdown();

            RESPONSE("207 Successful shutdown\n");

            return;
        }
        case(PROTOCOL_CLIENT_LOGOUT):
        {
            if(LogOutPlayer(info)) RESPONSE("208 Successful logout\n");
            RESPONSE("308 Failure logout\n");
        }
        default:
        {
            RESPONSE("301 Incorrect request\n");
        }
    }
}
