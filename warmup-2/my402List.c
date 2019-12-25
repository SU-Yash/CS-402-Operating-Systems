#include <stdio.h>
#include <stdlib.h>
#include "my402list.h"

int My402ListLength(My402List *list){
	return list->num_members;
}

int My402ListInit(My402List *list){
	list->num_members = 0;
	list->anchor.obj = NULL;
	list->anchor.next = &list->anchor;
	list->anchor.prev = &list->anchor;
	return 0;
}

int My402ListEmpty(My402List *list){
	if (list->num_members == 0){
		return TRUE;
	}
	return FALSE;
}

int My402ListAppend(My402List *list, void *elem){
	My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
	newElem->obj = elem;
	//When to return FALSE
	if(My402ListEmpty(list)){
		newElem->next = &list->anchor;
		newElem->prev = &list->anchor;
		list->anchor.next = newElem;
		list->anchor.prev = newElem;
		list->num_members = list->num_members + 1;
	}
	else{
		My402ListElem *lastElem;
		lastElem = list->anchor.prev;
		lastElem->next = newElem;
		newElem->prev = lastElem;

		list->anchor.prev = newElem;
		newElem->next = &list->anchor;
		list->num_members = list->num_members + 1;
	}
	return TRUE;
}

int My402ListPrepend(My402List *list, void *elem){
	My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
	newElem->obj = elem;
	//When to return FALSE
	if(My402ListEmpty(list)){
		newElem->next = &list->anchor;
		newElem->prev = &list->anchor;
		list->anchor.next = newElem;
		list->anchor.prev = newElem;
		list->num_members = list->num_members + 1;
	}
	else{
		My402ListElem *firstElem;
		firstElem = list->anchor.next;
		firstElem->prev = newElem;
		newElem->next = firstElem;

		list->anchor.next = newElem;
		newElem->prev = &list->anchor;
		list->num_members = list->num_members + 1;
	}
	return TRUE;
}

My402ListElem* My402ListFirst(My402List *list){
	if(My402ListEmpty(list)){
		return NULL;
	}
	return list->anchor.next;
}

My402ListElem* My402ListLast(My402List *list){
	if(My402ListEmpty(list)){
		return NULL;
	}
	return list->anchor.prev;
}

My402ListElem* My402ListNext(My402List *list, My402ListElem *elem){
	if(My402ListLast(list) == elem){
		return NULL;
	}
	return elem->next;
}

My402ListElem* My402ListPrev(My402List *list, My402ListElem *elem){
	if(My402ListFirst(list) == elem){
		return NULL;
	}
	return elem->prev;
}

My402ListElem* My402ListFind(My402List *list, void *obj){
	My402ListElem *currElem=NULL;

        for (currElem=My402ListFirst(list);
                currElem != NULL;
                currElem=My402ListNext(list, currElem)) {
            if(currElem->obj == obj){
            	return currElem;
            }
     	}
     	return NULL;
}

int My402ListInsertAfter(My402List *list, void *data, My402ListElem *elem){
	if(elem == NULL){
		return My402ListAppend(list, data);
	}
	else{
		My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));

		My402ListElem *nextElem;
		nextElem = elem->next;

		newElem->obj = data;
		nextElem->prev = newElem;
		elem->next = newElem;

		newElem->next = nextElem;
		newElem->prev = elem;

		list->num_members = list->num_members + 1;
		return TRUE;
	}
}

int My402ListInsertBefore(My402List *list, void *obj, My402ListElem *elem){
	if(elem == NULL){
		My402ListPrepend(list, obj);
		return TRUE;
	}

	else{
		My402ListElem *prevElem;
		prevElem = elem->prev;
		
		My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
		newElem->obj = obj;
		
		elem->prev = newElem;
		prevElem->next = newElem;

		newElem->prev = prevElem;
		newElem->next = elem;
		list->num_members = list->num_members + 1;
		return TRUE;
	}
}

void My402ListUnlink(My402List *list, My402ListElem *elem){
	My402ListElem *prevElem, *nextElem;
	prevElem = elem->prev;
	nextElem = elem->next;

	prevElem->next = nextElem;
	nextElem->prev = prevElem;
	list->num_members = list->num_members - 1;

	free(elem);
}

void My402ListUnlinkAll(My402List *list){
	My402ListElem *currElem, *nextElem;
	currElem=My402ListFirst(list);

	while(currElem != NULL){
		nextElem=My402ListNext(list, currElem);
		My402ListUnlink(list, currElem);
		currElem = nextElem;
	}
	list->anchor.next = NULL;
	list->anchor.prev = NULL;
}

