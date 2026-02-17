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
        if(player->nickname == nickname) return player;

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
    if(!player) return player;

    if(player->password == password) return player;

    return NULL;
}

// Зарегестрироваться по никнейму и паролю
Player* RegisterPlayer(const char* nickname, const char* password)
{
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
        fprintf(file, "%s:%s:", player->nickname, player->password);
        player = player->next;
    }

    fclose(file);

    return 1;
}

// Загрузить последнее сохранение сервера
bool Load()
{
    // TODO
    return 1;
}