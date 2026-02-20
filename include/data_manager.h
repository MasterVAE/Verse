#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "net_server.h"
#include "lib.h"

struct Server;

Player* FindPlayerByNickname(const char* nickname);
Player* LogInPlayer(const char* nickname, const char* password);
Player* RegisterPlayer(const char* nickname, const char* password);
bool LogOutPlayer(ThreadInfo* thread);
bool DeletePlayer(ThreadInfo* thread);

Server* GetServer();
void CreateServer();
void DestroyServer();

bool Buy(Agent* agent, size_t lot_number);
bool Sell(Agent* agent, size_t amount, size_t price);

bool Save();
bool Load();

struct Lot;

// Структура для хранения данных о мире
struct Server
{
    List* players;

    List* agents;

    size_t old_lots_count;
    Lot** old_lots;

    List* lots;
};


// Структура для хранения информации о лоте
struct Lot
{
    size_t id;

    bool isSell;

    size_t amount;
    size_t price;

    Agent* owner;

    size_t agents_want_count;
    Agent** agents_want;
};

// Структура для хранения информации о боте
struct Bot
{
    Agent* agent;

    // TODO
};

// Структура агента на бирже
struct Agent
{
    bool isPlayer;

    Player* player;
    Bot* bot;

    size_t money;
    size_t stocks;

    size_t expected_money;

    size_t want_buy_lots_count;
    Lot** want_buy_lots;
    Lot* want_sell_lot;
};

// Структура для хранения информации о игроке
struct Player
{
    char* nickname;
    char* password;

    ThreadInfo* thread;

    Agent* agent;
};

void DestroyLot(void* lot_void);
void DestroyPlayer(void* player_void);

#endif // DATA_MANAGER_H