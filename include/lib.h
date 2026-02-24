#ifndef LIB_H
#define LIB_H

#include <stdlib.h>

struct ListElem
{
    ListElem* next;
    ListElem* prev;

    void* value;
};

struct List
{
    ListElem* start;
    size_t count;
};

List* ListCreate();
void ListDelete(List* list, void (*destroyer)(void*));

void ListAddElem(List* list, void* value);
void ListDeleteElem(List* list, void* value, void (*destroyer)(void*));

bool ListContainsElem(List* list, void* value);

#endif // LIB_H