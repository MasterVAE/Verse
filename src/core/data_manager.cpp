#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "data_manager.h"
#include "core.h"

const char* FILENAME = "server.data";
const char* POSSIBLE_SYMBOLS = "abcdefghijklmopqrstuvwxyz"
                               "ABCDEFGHIJKLMOPQRSTUVWXYZ"
                               "0123456789"
                               "_!@#$%^&*()-+=";

static Server* server;

static size_t PlayerCount();
static bool CheckNicknameAndPassword(const char* nickname, const char* password);
static Player* CreatePlayer(const char* nickname, const char* password);

static Agent* CreateAgent(bool isPlayer);
static void DestroyAgent(Agent* agent);






// Поиск игрока по никнейму
Player* FindPlayerByNickname(const char* nickname)
{
    assert(nickname);

    Player* player = server->players;
    while(player)
    {
        if(!strcmp(player->nickname, nickname)) return player;

        player = player->next;
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

    return CreatePlayer(nickname, password);
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







// Количество игроков
static size_t PlayerCount()
{
    size_t i = 0;

    Player* player = server->players;
    while(player)
    {
        i++;
        player = player->next;   
    }

    return i;
}






// Сохранить данные сервера
bool Save()
{
    FILE* file = fopen(FILENAME, "w+");
    if(!file) return false;

    size_t player_count = PlayerCount();

    fprintf(file, "%lu ", player_count);
    Player* player = server->players;
    while(player)
    {
        fprintf(file, "%s %s ", player->nickname, player->password);
        player = player->next;
    }

    fclose(file);

    printf("[DATA MANAGER] Data saved\n");

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
    }

    printf("[DATA MANAGER] Data loaded\n");

    return true;
}






// Инициилизировать структуру сервера
void CreateServer()
{
    Server* serv = (Server*)calloc(1, sizeof(Server));
    if(!serv) CoreShutdown();

    serv->players = NULL;
    serv->data_start = 0;
    
    for(size_t i = 0; i < DATA_COUNT; i++)
    {
        serv->data[i] = 0;
    }

    server = serv;
}







// Уничтожить структуру сервера
void DestroyServer()
{
    Player* player = server->players;

    while(player)
    {
        Player* to_free = player;
        player = player->next;

        free(to_free->nickname);
        free(to_free->password);
        free(to_free);
    }
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

    new_player->next = server->players;
    if(server->players)
    {
        server->players->prev = new_player;
    }
    server->players = new_player;

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
    agent->money = 0;
    agent->stocks = 0;
    agent->expected_money = 0;
    agent->expected_stocks = 0;
    agent->want_buy_lots_count = 0;
    agent->want_buy_lots = (Lot**)calloc(1, sizeof(Lot*));
    if(!agent->want_buy_lots)
    {
        free(agent);
        return NULL;
    }

    agent->prev = NULL;
    agent->next = server->agents;
    if(server->agents)
    {
        server->agents->prev = agent;
    }
    server->agents = agent;

    return agent;
}

// Уничтожить структуру агента
static void DestroyAgent(Agent* agent)
{
    assert(agent);

    free(agent->want_buy_lots);

    if(agent->prev)
    {
        agent->prev->next = agent->next;
    }
    if(agent->next)
    {
        agent->next->prev = agent->prev;
    }
    if(server->agents == agent)
    {
        server->agents = agent->next;
    }

    free(agent);
}