#ifndef _COLLECTIONS_H
#define _COLLECTIONS_H
#include <types.h>

typedef struct linked_list_item linked_list_item;

typedef struct linked_list_item{
    void* item;
    linked_list_item* prev;
    linked_list_item* next;
} linked_list_item;

typedef struct linked_list{
    uint64 count;
    linked_list_item* first;
    linked_list_item* last;
} linked_list;

linked_list_item* linked_list_add(linked_list* list, void* item);
linked_list_item* linked_list_insert(linked_list* list, void* item, uint64 idx_after);
linked_list_item* linked_list_insert_after(linked_list* list, void* item, linked_list_item* after);
uint8 linked_list_remove_at(linked_list* list, uint64 idx);
uint8 linked_list_remove_item(linked_list* list, linked_list_item* item);
linked_list_item* linked_list_get(linked_list* list, uint64 idx);


#endif