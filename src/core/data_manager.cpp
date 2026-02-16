#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "data_manager.h"

static Player* players;

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

Player* LogInPlayer(const char* nickname, const char* password)
{
    assert(nickname);
    assert(password);
    
    Player* player = FindPlayerByNickname(nickname);
    if(!player) return player;

    if(player->password == password) return player;

    return NULL;

}
Player* RegisterPlayer(const char* nickname, const char* password)
{
    Player* player = LogInPlayer(nickname, password);
    if(player) return player;

    Player* new_player = (Player*)calloc(1, sizeof(Player));
    if(!new_player) return NULL;

    new_player->nickname = nickname;
    new_player->password = password;

    new_player->next = players;
    players = new_player;

    return new_player;
}

void Save(const char* filename)
{
    // TODO
    assert(filename);
}

void Load(const char* filename)
{
    // TODO
    assert(filename);
}