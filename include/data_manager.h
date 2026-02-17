#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "net_server.h"

Player* FindPlayerByNickname(const char* nickname);
Player* LogInPlayer(const char* nickname, const char* password);
Player* RegisterPlayer(const char* nickname, const char* password);
bool LogOutPlayer(ThreadInfo* thread);
bool DeletePlayer(ThreadInfo* thread);

bool Save();
bool Load();

#endif // DATA_MANAGER_H