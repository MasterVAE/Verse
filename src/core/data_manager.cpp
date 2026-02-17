#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "data_manager.h"

const char* FILENAME = "server.data";

static Player* players;

// Поиск игрока по никнейму
Player* FindPlayerByNickname(const char* nickname)
{
    assert(nickname);

    Player* player = players;
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

    if(!strcmp(player->password, password)) return player;
    return NULL;
}

// Зарегестрироваться по никнейму и паролю
Player* RegisterPlayer(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);

    //TODO check if nickname is correct

    Player* player = LogInPlayer(nickname, password);
    if(player) return player;

    Player* new_player = (Player*)calloc(1, sizeof(Player));
    if(!new_player) return NULL;

    new_player->nickname = strdup(nickname);
    new_player->password = strdup(password);

    new_player->next = players;
    if(players)
    {
        players->prev = new_player;
    }
    players = new_player;

    return new_player;
}

bool DeletePlayer(ThreadInfo* thread)
{
    assert(thread);

    Player* player = thread->player;
    if(!player) return false;

    if(players == player)
    {
        players = player->next;
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

// Сохранить данные сервера
bool Save()
{
    FILE* file = fopen(FILENAME, "w+");
    if(!file) return false;

    Player* player = players;
    while(player)
    {
        fprintf(file, "%s %s ", player->nickname, player->password);
        player = player->next;
    }

    fclose(file);

    return 1;
}

// Загрузить последнее сохранение сервера
bool Load()
{
    FILE* file = fopen(FILENAME, "w+");
    if(!file) return false;
    
    bool flag = true;
    while(flag)
    {
        char nickname[100] = {0};
        char password[100] = {0};

        if(fscanf(file, "%99s %99s ", nickname, password) < 2)
        {
            flag = false;
        }
        else
        {
            Player* new_player = (Player*)calloc(1, sizeof(Player));
            if(!new_player) return false;
 
            new_player->nickname = strdup(nickname);
            new_player->password = strdup(password);

            new_player->next = players;
            if(players)
            {
                players->prev = new_player;
            }
            players = new_player;
        }
    }

    Save();

    return true;
}