#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "net_server.h"

void* GameServerStartup(void* data);
void GameServerShutdown();

void SendPrices(ThreadInfo* info, size_t company);
void SendLots(ThreadInfo* info, size_t company);

void UpdateData();

#endif // NET_SERVER_H