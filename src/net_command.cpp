#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <assert.h>

#include "commands.h"
#include "data_manager.h"
#include "core.h"

void LogIn(ThreadInfo* info, TreeNode** arguments)
{
    assert(info);
    assert(arguments);

    if(arguments[0]->type != NODE_STRING) return;
    if(arguments[1]->type != NODE_STRING) return;

    if(info->player) return;

    info->player = LogInPlayer(arguments[0]->value.string, arguments[1]->value.string);

    const char* answer = "SUCCESS\n";
    if(!info->player) answer = "FAIL\n";

    send(info->data->client_socket, answer, strlen(answer), 0);
}
void Register(ThreadInfo* info, TreeNode** arguments)
{
    assert(info);
    assert(arguments);

    if(arguments[0]->type != NODE_STRING) return;
    if(arguments[1]->type != NODE_STRING) return;

    if(info->player) return;

    info->player = RegisterPlayer(arguments[0]->value.string, arguments[1]->value.string);
    
    const char* answer = "SUCCESS\n";
    if(!info->player) answer = "FAIL\n";

    send(info->data->client_socket, answer, strlen(answer), 0);
}

void Shutdown(ThreadInfo* info, TreeNode** arguments)
{
    assert(info);

    CoreShutdown();
}

void SendMessage(ThreadInfo* info, TreeNode** arguments)
{
    assert(info);
    assert(arguments);

    if(arguments[0]->type != NODE_STRING) return;
    if(arguments[1]->type != NODE_STRING) return;

    if(!info->player) return;

    Player* p = FindPlayerByNickname(arguments[0]->value.string);
    
    if(!p || !p->thread) return;

    send(p->thread->data->client_socket, arguments[1]->value.string, strlen(arguments[1]->value.string), 0);
};

void SendMessageAll(ThreadInfo* info, TreeNode** arguments)
{
    assert(info);
    assert(arguments);

    if(arguments[0]->type != NODE_STRING) return;

    if(!info->player) return;

    SendAll(arguments[0]->value.string);
}