#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdlib.h>

#include "net_request.h"

void LogIn          (ThreadInfo* info, TreeNode** arguments);
void Register       (ThreadInfo* info, TreeNode** arguments);

void Shutdown       (ThreadInfo* info, TreeNode** arguments);

void SendMessage    (ThreadInfo* info, TreeNode** arguments);
void SendMessageAll (ThreadInfo* info, TreeNode** arguments);

struct Command
{
    const char* command;
    size_t argc;
    void (*command_func)(ThreadInfo* info, TreeNode** arguments); 
};

const Command COMMANDS[] = 
{
    {"login",           2, LogIn},
    {"register",        2, Register},
    {"shutdown",        0, Shutdown},
    {"sendmessage",     2, SendMessage},
    {"sendmessageall",  1, SendMessageAll}
};

const size_t COMMANDS_COUNT = sizeof(COMMANDS)/sizeof(COMMANDS[0]);

#endif // COMMANDS_H