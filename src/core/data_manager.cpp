#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "data_manager.h"
#include "core.h"

const char* PLAYERS_FILENAME = "players.data";
const char* BOTS_FILENAME = "bots.data";
const char* WORLD_FILENAME = "world.data";
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

static void SavePlayers();
static void SaveBots();
static void SaveWorld();

static void LoadPlayers();
static void LoadBots();
static void LoadWorld();


static size_t LOT_ID = 10;

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
void Save()
{
    SavePlayers(); 
    SaveBots();
    SaveWorld();
}






// Загрузить последнее сохранение сервера
void Load()
{
    LoadPlayers();
    LoadBots();
    LoadWorld();

    printf("[DATA MANAGER] Data loaded\n");
}






// Инициилизировать структуру сервера
void CreateServer()
{
    Server* serv = (Server*)calloc(1, sizeof(Server));
    if(!serv) CoreShutdown();

    serv->players = ListCreate();
    serv->bots = ListCreate();
    serv->agents = ListCreate();
    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        serv->lots[i] = ListCreate();
    }

    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        serv->old_lots_count[i] = 0;
    }
    

    for(size_t j = 0; j < COMPANIES_COUNT; j++)
    {
        for(size_t i = 0; i < PRICE_ARRAY_COUNT; i++)
        {
            serv->cycled_list[j][i] = 0;
        };
        serv->cycled_list_index[j] = 0;
    }   
    

    serv->goverment_agent = CreateAgent();
    ListAddElem(serv->agents, serv->goverment_agent);
    
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

    agent->money = 10000;
    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        agent->stocks[i] = 0;
    }
    
    agent->expected_money = 0;

    agent->want_buy_lots = ListCreate();
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

    agent->priority = 0;

    return agent;
}





// Уничтожить структуру агента
static void DestroyAgent(void* agent_void)
{
    assert(agent_void);

    Agent* agent = (Agent*)agent_void;

    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {
        server->goverment_agent->stocks[i] += agent->stocks[i];
    }
    
    free(agent->want_buy_lots);
    free(agent);
}





// Инициализация лота
static Lot* CreateLot(size_t amount, size_t price)
{
    Lot* lot = (Lot*)calloc(1, sizeof(Lot));
    if(!lot) return NULL;

    lot->agents_want = ListCreate();
    if(!lot->agents_want)
    {
        free(lot);
        return NULL;
    }


    lot->amount = amount;
    lot->price = price;
    lot->owner = NULL;

    lot->id = LOT_ID++;

    lot->canceled = false;

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
    for(size_t j = 0; j < COMPANIES_COUNT; j++)
    {
        for(size_t i = 0; i < server->old_lots_count[j]; i++)
        {
            if(server->old_lots[j][i]->id == lot_number)
            {
                lot = server->old_lots[j][i];
            }
        }
    }

    if(!lot) return false;

    ListElem* elem = lot->agents_want->start;
    while(elem)
    {
        if(elem->value == agent) return false;
        elem = elem->next;
    }

    if(lot->price > agent->expected_money) return false;

    agent->expected_money -= lot->price;

    ListAddElem(agent->want_buy_lots, lot);

    ListAddElem(lot->agents_want, agent);

    return true;
}




// Выставить на продажу
bool Sell(Agent* agent, size_t amount, size_t price, size_t company)
{
    assert(agent);
    if(company >= COMPANIES_COUNT) return false;
    if(amount == 0) return false;
    if(amount > agent->stocks[company]) return false;
    if(agent->want_sell_lot[company]) return false;

    size_t have_stocks = agent->stocks[company];
    ListElem* elem = agent->selling_lots->start;
    while(elem)
    {
        have_stocks -= ((Lot*)elem->value)->amount;
        elem = elem->next;
    };

    if(amount > have_stocks) return false;

    Lot* new_lot = CreateLot(amount, price);
    new_lot->company = company;
    new_lot->owner = agent;
    agent->want_sell_lot[company] = new_lot;

    ListAddElem(server->lots[company], new_lot);


    return true;
}


// Отмена лота
bool Cancel(Agent* agent, size_t lot_id)
{
    assert(agent);

    if(lot_id < COMPANIES_COUNT)
    {
        if(!agent->want_sell_lot[lot_id]) return false;

        ListDeleteElem(server->lots[lot_id], agent->want_sell_lot[lot_id], DestroyLot);
        agent->want_sell_lot[lot_id] = NULL;

        return true;
    }
    
    for(size_t j = 0; j < COMPANIES_COUNT; j++)
    {
        for(size_t i = 0; i < server->old_lots_count[j]; i++)
        {
            Lot* lot = server->old_lots[j][i];
            if(lot->id == lot_id)
            {
                if(lot->owner == agent)
                {
                    lot->canceled = true;
                    return true;
                }
                else
                {
                    ListDeleteElem(lot->agents_want, agent, NULL);
                    ListDeleteElem(agent->want_buy_lots, lot, NULL);
                }
            }
        }
    }

    return false;
}


// Покупка приотритета
bool BuyPriority(Agent* agent, size_t priority)
{
    assert(agent);

    if(priority > agent->money) return false;
    
    agent->priority += priority;
    agent->money -= priority;

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
static void SavePlayers()
{
    FILE* file = fopen(PLAYERS_FILENAME, "w+");
    if(!file) return;

    size_t player_count = server->players->count;

    fprintf(file, "%lu ", player_count);


    ListElem* player_elem = server->players->start;
    while(player_elem)
    {
        Player* player = (Player*)player_elem->value;
        fprintf(file, "%s %s %lu ", player->nickname, player->password, 
                                       player->agent->money);
        for(size_t i = 0; i < COMPANIES_COUNT; i++)
        {
            fprintf(file, "%lu ", player->agent->stocks[i]);
        }

        player_elem = player_elem->next;
    }


    fclose(file);


    return;
}

// Сохрнение ботов на диск
static void SaveBots()
{
    FILE* file = fopen(BOTS_FILENAME, "w+");
    if(!file) return;

    fprintf(file, "%lu ", server->bots->count);

    ListElem* elem = server->bots->start;
    while(elem)
    {
        Bot* bot = (Bot*)elem->value;
        fprintf(file, "%lu ", bot->agent->money);
        for(size_t i = 0; i < COMPANIES_COUNT; i++)
        {
            fprintf(file, "%lu ", bot->agent->stocks[i]);
        }
        elem = elem->next;
    }

    fclose(file);
}


// Сохрнение мира на диск
static void SaveWorld()
{
    FILE* file = fopen(WORLD_FILENAME, "w+");
    if(!file) return;

    for(size_t k = 0; k < COMPANIES_COUNT; k++)
    {
        fprintf(file, "%lu ", server->goverment_agent->stocks[k]);
    }

    fclose(file);
}

// Загрузка данных игроков с диска
static void LoadPlayers()
{
    FILE* file = fopen(PLAYERS_FILENAME, "r+");
    if(!file) return;
    
    size_t player_count = 0;
    fscanf(file, "%lu ", &player_count);
    for(size_t i = 0; i < player_count; i++)
    {
        char nickname[100] = {0};
        char password[100] = {0};

        fscanf(file, "%99s %99s ", nickname, password);

        Player* player = CreatePlayer(nickname, password);
        if(!player) return;

        fscanf(file, "%lu ", &player->agent->money);

        for(size_t j = 0; j < COMPANIES_COUNT; j++)
        {
            fscanf(file, "%lu ", &player->agent->stocks[j]);
        }

        ListAddElem(server->players, player);
    }

    fclose(file);
}

// Загрузка ботов с диска
static void LoadBots()
{
    FILE* file = fopen(BOTS_FILENAME, "r+");
    if(file)
    {
        size_t bot_count = 0;
        fscanf(file, "%lu ", &bot_count);

        for(size_t i = 0; i < bot_count; i++)
        {
            Bot* bot = CreateBot();
            fscanf(file, "%lu ", &bot->agent->money);
            for(size_t j = 0; j < COMPANIES_COUNT; j++)
            {
                fscanf(file, "%lu ", &bot->agent->stocks[j]);
            }
            ListAddElem(server->bots, bot);
        }

        fclose(file);
        return;
    }

    for(size_t i = 0; i < START_BOTS_COUNT; i++)
    {
        ListAddElem(server->bots, CreateBot());
    }
}


// Загрузка мира с диска
static void LoadWorld()
{
    FILE* file = fopen(WORLD_FILENAME, "r+");
    if(file)
    {
        for(size_t i = 0; i < COMPANIES_COUNT; i++)
        {
            fscanf(file, "%lu ", &server->goverment_agent->stocks[i]);
        }

        fclose(file);
        return;
    }

    for(size_t i = 0; i < COMPANIES_COUNT; i++)
    {   
        server->goverment_agent->stocks[i] = 10000;
    }
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