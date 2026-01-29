#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "net_server.h"

Player* FindPlayerByNickname(const char* nickname);
Player* LogInPlayer(const char* nickname, const char* password);
Player* RegisterPlayer(const char* nickname, const char* password);

void Save(const char* filename);
void Load(const char* filename);

#endif // DATA_MANAGER_H