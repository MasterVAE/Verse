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
bool Sell(Agent* agent, size_t amount, size_t price, size_t company);
bool Cancel(Agent* agent, size_t lot_id);
bool BuyPriority(Agent* agent, size_t priority);

void Save();
void Load();

const size_t PRICE_ARRAY_COUNT = 60;
static const size_t START_BOTS_COUNT = 100;
const size_t COMPANIES_COUNT = 5;

struct Lot;

// Структура для хранения данных о мире
struct Server
{
    List* players;
    List* bots;

    List* agents;

    size_t old_lots_count[COMPANIES_COUNT];
    Lot** old_lots[COMPANIES_COUNT];

    List* lots[COMPANIES_COUNT];

    double cycled_list[COMPANIES_COUNT][PRICE_ARRAY_COUNT];
    size_t cycled_list_index[COMPANIES_COUNT];

    Agent* goverment_agent;
};


// Структура для хранения информации о лоте
struct Lot
{
    size_t id;
    size_t company;

    size_t amount;
    size_t price;

    Agent* owner;

    List* agents_want;

    bool canceled;
};

// Структура для хранения информации о боте
struct Bot
{
    Agent* agent;
};

// Структура агента на бирже
struct Agent
{
    size_t money;
    size_t stocks[COMPANIES_COUNT];

    size_t expected_money;

    List* want_buy_lots;
    Lot* want_sell_lot[COMPANIES_COUNT];
    
    size_t priority;

    List* selling_lots;
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
void DestroyBot(void* player_void);

#endif // DATA_MANAGER_H