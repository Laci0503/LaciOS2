#include <collections.h>
#include <config.h>

linked_list_item* linked_list_add(linked_list* list, void* item){
    if(list==NULL)return NULL;
    linked_list_item* newitem = malloc(sizeof(linked_list_item));
    if(newitem==NULL)return NULL;
    newitem->item=item;
    newitem->next=NULL;
    list->count++;
    if(list->first==NULL){
        list->first=newitem;
        newitem->prev=NULL;
        list->last=newitem;
        return newitem;
    }
    list->last->next=newitem;
    newitem->prev=list->last;
    list->last=newitem;
    return newitem;
}

linked_list_item* linked_list_insert(linked_list* list, void* item, uint64 idx_after){
    linked_list_item* after = linked_list_get(list, idx_after);
    return linked_list_insert_after(list, item, after);
}

linked_list_item* linked_list_insert_after(linked_list* list, void* item, linked_list_item* after){
    if(list==NULL || after==NULL)return NULL;
    linked_list_item* newitem = malloc(sizeof(linked_list_item));
    if(newitem==NULL)return NULL;
    newitem->item=item;
    newitem->prev=after;
    list->count++;
    linked_list_item* nextitem = after->next;
    after->next=newitem;
    if(nextitem==NULL){ // utolsó után insertel
        newitem->next=NULL;
        list->last=newitem;
        return newitem;
    }
    newitem->next=nextitem;
    nextitem->prev=newitem;
    return newitem;
}

uint8 linked_list_remove_at(linked_list* list, uint64 idx){
    linked_list_item* item = linked_list_get(list, idx);
    return linked_list_remove_item(list, item);
}

uint8 linked_list_remove_item(linked_list* list, linked_list_item* item){
    if(list==NULL || item==NULL)return 0;
    if(item->prev!=NULL){
        item->prev->next=item->next;
    }else{ // első item
        list->first=item->next;
    }
    if(item->next!=NULL){
        item->next->prev=item->prev;
    }else{ // utolsó item
        list->last=item->prev;
    }
    free(item);
    list->count--;
    return 1;
}

linked_list_item* linked_list_get(linked_list* list, uint64 idx){
    if(list==NULL)return NULL;
    if(idx>=list->count)return NULL;
    if(idx<list->count >> 1){ // Az első felében van
        linked_list_item* item = list->first;
        for(uint64 i=0;i<idx;i++){
            item=item->next;
        }
        return item;
    }else{ // A második felében van
        linked_list_item* item = list->last;
        for(uint64 i=list->count-1;i!=idx;i--){
            item=item->prev;
        }
        return item;
    }
}