#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "data_manager.h"
#include "core.h"

const char* PLAYERS_FILENAME = "players.data";
const char* BOTS_FILENAME = "bots.data";
const char* POSSIBLE_SYMBOLS = "abcdefghijklmnopqrstuvwxyz"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "0123456789"
                               "_!@#$%^&*()-+=";

static Server* server;

static bool CheckNicknameAndPassword(const char* nickname, const char* password);
static Player* CreatePlayer(const char* nickname, const char* password);

static Agent* CreateAgent();
static void DestroyAgent(void* agent);

static Lot* CreateLot(size_t amount, size_t price);
static Bot* CreateBot();

static bool SavePlayers();
static bool SaveBots();

static bool LoadPlayers();
static bool LoadBots();


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
    return SavePlayers() && SaveBots();
}






// Загрузить последнее сохранение сервера
bool Load()
{
    return LoadPlayers() && LoadBots();
}






// Инициилизировать структуру сервера
void CreateServer()
{
    Server* serv = (Server*)calloc(1, sizeof(Server));
    if(!serv) CoreShutdown();

    serv->players = ListCreate();
    serv->bots = ListCreate();
    serv->agents = ListCreate();
    serv->lots = ListCreate();

    serv->old_lots_count = 0;

    for(size_t i = 0; i < PRICE_ARRAY_COUNT; i++)
    {
        serv->cycled_list[i] = 0;
    };
    serv->cycled_list_index = 0;
    
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

    new_player->agent = CreateAgent();
    if(!new_player->agent)
    {
        free(new_player);
        return NULL;
    }

    ListAddElem(server->agents, new_player->agent);

    new_player->nickname = strdup(nickname);
    new_player->password = strdup(password);

    return new_player;
}



// Инициилизировать структуру агента
static Agent* CreateAgent()
{
    Agent* agent = (Agent*)calloc(1, sizeof(Agent));
    if(!agent) return NULL;

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

    agent->selling_lots = ListCreate();
    if(!agent->selling_lots)
    {
        free(agent->want_buy_lots);
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
static Lot* CreateLot(size_t amount, size_t price)
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

    lot->amount = amount;
    lot->price = price;
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

    size_t have_stocks = agent->stocks;
    ListElem* elem = agent->selling_lots->start;
    while(elem)
    {
        have_stocks -= ((Lot*)elem->value)->amount;
        elem = elem->next;
    };

    if(amount > have_stocks) return false;

    Lot* new_lot = CreateLot(amount, price);
    new_lot->owner = agent;
    agent->want_sell_lot = new_lot;

    ListAddElem(server->lots, new_lot);

    printf("[DATA MANAGER] Lot accepted\n");

    return true;
}

// Уничтожение структуры игрока
void DestroyPlayer(void* player_void)
{
    assert(player_void);

    Player* player = (Player*)player_void;
    free(player->nickname);
    free(player->password);

    ListDeleteElem(server->agents, player->agent, DestroyAgent);
    free(player);
}

// Сохранение данных игроков на диск
static bool SavePlayers()
{
    FILE* file = fopen(PLAYERS_FILENAME, "w+");
    if(!file) return false;

    size_t player_count = server->players->count;

    fprintf(file, "%lu ", player_count);


    ListElem* player_elem = server->players->start;
    while(player_elem)
    {
        Player* player = (Player*)player_elem->value;
        fprintf(file, "%s %s %lu %lu ", player->nickname, player->password, 
                                       player->agent->stocks, player->agent->money);

        player_elem = player_elem->next;
    }


    fclose(file);

    printf("[DATA MANAGER] Data saved\n");

    return true;
}

// Сохрнение ботов на диск
static bool SaveBots()
{
    // TODO
    return true;
}

// Загрузка данных игроков с диска
static bool LoadPlayers()
{
    FILE* file = fopen(PLAYERS_FILENAME, "r+");
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

        fscanf(file, "%lu %lu ", &player->agent->stocks, &player->agent->money);

        ListAddElem(server->players, player);
    }

    fclose(file);

    printf("[DATA MANAGER] Data loaded\n");

    return true;
}

// Загрузка ботов с диска
static bool LoadBots()
{
    for(size_t i = 0; i < START_BOTS_COUNT; i++)
    {
        ListAddElem(server->bots, CreateBot());
    }
    return true;
}


static Bot* CreateBot()
{
    Bot* new_bot = (Bot*)calloc(1, sizeof(Bot));
    if(!new_bot) return NULL;

    new_bot->agent = CreateAgent();
    if(!new_bot->agent)
    {
        free(new_bot);
        return NULL;
    }

    ListAddElem(server->agents, new_bot->agent);

    return new_bot;
}

// Удаление структуры бота
void DestroyBot(void* bot_void)
{
    assert(bot_void);

    Bot* bot = (Bot*)bot_void;
    ListDeleteElem(server->agents, bot->agent, DestroyAgent);
    free(bot);
}