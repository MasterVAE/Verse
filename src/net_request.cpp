#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>

#include "net_request.h"
#include "net_server.h"
#include "core.h"

#include "commands.h"

// PARSE REQUEST ON THREAD
void ParseRequest(ThreadInfo* info, const char* buffer)
{
    assert(info);
    assert(buffer);

    // TODO
}
