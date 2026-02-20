#include "lib.h"
#include <assert.h>



static ListElem* ListCreateElem()
{
    ListElem* elem = (ListElem*)calloc(1, sizeof(ListElem));
    if(!elem) return NULL;

    elem->next = NULL;
    elem->prev = NULL;
    elem->value = NULL;

    return elem;
};

List* ListCreate()
{
    List* list = (List*)calloc(1, sizeof(List));
    if(!list) return NULL;

    list->start = NULL;
    list->count = 0;

    return list;
}

void ListDelete(List* list, void (*destroyer)(void*))
{
    assert(list);

    ListElem* elem = list->start;
    while(elem)
    {
        ListElem* next = elem->next;
        if(destroyer)
        {
            destroyer(elem->value);
        }
        free(elem);
        elem = next;
    }

    free(list);
}

void ListAddElem(List* list, void* value)
{
    assert(list);

    ListElem* new_elem = ListCreateElem();
    if(!new_elem) return;

    new_elem->value = value;

    ListElem* start = list->start;
    new_elem->next = start;

    if(start)
    {
        start->prev = new_elem;
    }
    list->start = new_elem;
    list->count++;
}
void ListDeleteElem(List* list, void* value, void (*destroyer)(void*))
{
    assert(list);

    ListElem* elem = list->start;
    while(elem)
    {
        if(elem->value == value)
        {
            if(elem->next)  elem->next->prev = elem->prev;
            if(elem->prev)  elem->prev->next = elem->next;
            if(elem == list->start) list->start = elem->next;
            destroyer(elem->value);
            free(elem);
            break;
        }
        elem = elem->next;
    }
    list->count--;
}