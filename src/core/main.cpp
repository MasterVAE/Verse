#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "core.h"

int main() 
{   
    srand(time(NULL));
    return CoreStartup();
}
