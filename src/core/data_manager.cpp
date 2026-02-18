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

bool DeletePlayer(ThreadInfo* thread)
{
    assert(thread);

    Player* player = thread->player;
    if(!player) return false;

    if(server->players == player)
    {
        server->players = player->next;
    }

    if(player->prev)
    {
        player->prev->next = player->next;
    }
    if(player->next)
    {
        player->next->prev = player->prev;
    }

    free(player->nickname);
    free(player->password);
    free(player);
    thread->player = NULL;

    return true;
}

bool LogOutPlayer(ThreadInfo* thread)
{
    assert(thread);

    Player* player = thread->player;
    if(!player) return false;

    player->thread = NULL;
    thread->player = NULL;

    return true;
}

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
        fprintf(file, "%lu %lu ", player->money, player->stocks);
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

        fscanf(file, "%lu %lu ", &player->money, &player->stocks);
    }

    printf("[DATA MANAGER] Data loaded\n");

    return true;
}

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

Server* GetServer()
{
    return server;
}

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

static Player* CreatePlayer(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);

    Player* new_player = (Player*)calloc(1, sizeof(Player));
    if(!new_player) return NULL;

    new_player->nickname = strdup(nickname);
    new_player->password = strdup(password);

    new_player->money = 0;
    new_player->stocks = 0;

    new_player->next = server->players;
    if(server->players)
    {
        server->players->prev = new_player;
    }
    server->players = new_player;

    return new_player;
}