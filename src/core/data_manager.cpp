#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "data_manager.h"
#include "core.h"

const char* FILENAME = "server.data";
const char* POSSIBLE_SYMBOLS = "abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "0123456789"
                               "_!@#$%^&*()-+=";

static Server* server;

static bool CheckNicknameAndPassword(const char* nickname, const char* password);
static Player* CreatePlayer(const char* nickname, const char* password);

static Agent* CreateAgent(bool isPlayer);
static void DestroyAgent(void* agent);

static Lot* CreateLot(bool isSell, size_t amount, size_t price);


static size_t LOT_ID = 0;

// Поиск игрока по никнейму
Player* FindPlayerByNickname(const char* nickname)
{
    assert(nickname);

    ListElem* player_elem = server->players->start;
    while(player_elem)
    {
        if(!strcmp(((Player*)(player_elem->value))->nickname, nickname)) return (Player*)(player_elem->value);

        player_elem = player_elem->next;
    }
    
    return NULL;
}






// Залогиниться по никнейму и паролю
Player* LogInPlayer(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);
    
    Player* player = FindPlayerByNickname(nickname);
    if(!player) return NULL;

    if(!strcmp(player->password, password) && player->thread == NULL) return player;
    return NULL;
}






// Зарегестрироваться по никнейму и паролю
Player* RegisterPlayer(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);

    if(!CheckNicknameAndPassword(nickname, password)) return NULL;

    Player* player = LogInPlayer(nickname, password);
    if(player) return player;

    player = FindPlayerByNickname(nickname);
    if(player) return NULL;

    player = CreatePlayer(nickname, password);

    ListAddElem(server->players, player);

    return player;
}







// Разлогиниться игроку
bool LogOutPlayer(ThreadInfo* thread)
{
    assert(thread);

    Player* player = thread->player;
    if(!player) return false;

    player->thread = NULL;
    thread->player = NULL;

    return true;
}







// Сохранить данные сервера
bool Save()
{
    FILE* file = fopen(FILENAME, "w+");
    if(!file) return false;

    size_t player_count = server->players->count;

    fprintf(file, "%lu ", player_count);


    ListElem* player_elem = server->players->start;
    while(player_elem)
    {
        Player* player = (Player*)player_elem->value;
        fprintf(file, "%s %s ", player->nickname, player->password);

        player_elem = player_elem->next;
    }


    fclose(file);

    printf("[DATA MANAGER] Data saved\n");


    // TODO save money
    return 1;
}






// Загрузить последнее сохранение сервера
bool Load()
{
    FILE* file = fopen(FILENAME, "r+");
    if(!file) return false;
    
    size_t player_count = 0;
    fscanf(file, "%lu ", &player_count);
    for(size_t i = 0; i < player_count; i++)
    {
        char nickname[100] = {0};
        char password[100] = {0};

        fscanf(file, "%99s %99s ", nickname, password);

        Player* player = CreatePlayer(nickname, password);
        if(!player) return false;

        ListAddElem(server->players, player);
    }

    printf("[DATA MANAGER] Data loaded\n");

    // TODO load money

    return true;
}






// Инициилизировать структуру сервера
void CreateServer()
{
    Server* serv = (Server*)calloc(1, sizeof(Server));
    if(!serv) CoreShutdown();

    serv->players = ListCreate();
    serv->agents = ListCreate();
    serv->lots = ListCreate();

    serv->old_lots_count = 0;
    

    server = serv;
}







// Уничтожить структуру сервера
void DestroyServer()
{
    ListDelete(server->players, DestroyPlayer);
    free(server);
    server = NULL;
}






// Получить структуру сервера (из других единиц трансляции)
Server* GetServer()
{
    return server;
}






// Проверить никнейм и пароль на корректность
static bool CheckNicknameAndPassword(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);

    size_t len_nickname = strlen(nickname);
    size_t len_password = strlen(password);

    if(len_nickname < 3 || len_nickname > 30) return false;
    if(len_password < 8 || len_password > 30) return false;

    for(size_t i = 0; i < len_nickname; i++)
    {
        if(!strchr(POSSIBLE_SYMBOLS, nickname[i])) return false;
    }

    for(size_t i = 0; i < len_password; i++)
    {
        if(!strchr(POSSIBLE_SYMBOLS, password[i])) return false;
    }

    return true;
}






// Инициилизировать структуру игрока
static Player* CreatePlayer(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);

    Player* new_player = (Player*)calloc(1, sizeof(Player));
    if(!new_player) return NULL;

    new_player->agent = CreateAgent(true);
    if(!new_player->agent)
    {
        free(new_player);
        return NULL;
    }

    new_player->nickname = strdup(nickname);
    new_player->password = strdup(password);

    return new_player;
}



// Инициилизировать структуру агента
static Agent* CreateAgent(bool isPlayer)
{
    Agent* agent = (Agent*)calloc(1, sizeof(Agent));
    if(!agent) return NULL;

    agent->isPlayer = isPlayer;
    agent->player = NULL;
    agent->bot = NULL;
    agent->money = 1000;
    agent->stocks = 10;
    agent->expected_money = 0;
    agent->want_buy_lots_count = 0;
    agent->want_buy_lots = (Lot**)calloc(1, sizeof(Lot*));
    if(!agent->want_buy_lots)
    {
        free(agent);
        return NULL;
    }

    return agent;
}

// Уничтожить структуру агента
static void DestroyAgent(void* agent_void)
{
    assert(agent_void);

    Agent* agent = (Agent*)agent_void;
    free(agent->want_buy_lots);

    free(agent);
}



// Инициализация лота
static Lot* CreateLot(bool isSell, size_t amount, size_t price)
{
    Lot* lot = (Lot*)calloc(1, sizeof(Lot));
    if(!lot) return NULL;

    lot->agents_want = (Agent**)calloc(1, sizeof(Agent*));
    if(!lot->agents_want)
    {
        free(lot);
        return NULL;
    }

    lot->agents_want_count = 0;

    lot->amount = 0;
    lot->price = 0;
    lot->owner = NULL;

    lot->id = LOT_ID++;

    return lot;
}




// Уничтожение структура лота
void DestroyLot(void* lot_void)
{
    assert(lot_void);

    Lot* lot = (Lot*)lot_void;

    free(lot->agents_want);
    free(lot);
}






// Запрос на покупку лота
bool Buy(Agent* agent, size_t lot_number)
{
    assert(agent);

    Lot* lot = NULL;
    for(size_t i = 0; i < server->old_lots_count; i++)
    {
        if(server->old_lots[i]->id == lot_number)
        {
            lot = server->old_lots[i];
        }
    }

    if(!lot) return false;

    if(lot->price > agent->expected_money) return false;

    agent->expected_money -= lot->price;

    agent->want_buy_lots_count++;
    agent->want_buy_lots = (Lot**)realloc(agent->want_buy_lots, 
                                          sizeof(Lot*) * agent->want_buy_lots_count);
    agent->want_buy_lots[agent->want_buy_lots_count - 1] = lot;

    lot->agents_want_count++;
    lot->agents_want = (Agent**)realloc(lot->agents_want, lot->agents_want_count * sizeof(Agent*));
    lot->agents_want[lot->agents_want_count - 1] = agent;

    return true;
}




// Выставить на продажу
bool Sell(Agent* agent, size_t amount, size_t price)
{
    assert(agent);
    if(amount == 0) return false;
    if(amount > agent->stocks) return false;
    if(agent->want_sell_lot) return false;

    Lot* new_lot = CreateLot(true, amount, price);
    new_lot->owner = agent;
    agent->want_sell_lot = new_lot;

    return true;
}


void DestroyPlayer(void* player_void)
{
    assert(player_void);

    Player* player = (Player*)player_void;
    free(player->nickname);
    free(player->password);

    DestroyAgent(player->agent);
    free(player);
}