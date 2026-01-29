#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "net_server.h"

struct Player
{
    const char* nickname;
    const char* password;

    ThreadInfo* thread;
    bool online;

    Player* next;
};

Player* FindPlayerByNickname(const char* nickname);
Player* LogIn(const char* nickname, const char* password);
Player* Register(const char* nickname, const char* password);

void Save(const char* filename);
void Load(const char* filename);

#endif // DATA_MANAGER_H