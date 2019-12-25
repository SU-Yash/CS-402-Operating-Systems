#include <stdio.h>
#include "my402List.h"

int  My402ListLength(My402List *list){
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
		return 1;
	}
	return 0;
}

