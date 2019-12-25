#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs402.h"

#include "my402list.h"


int main(int argc, char *argv[])
{
	My402List list;
	My402ListElem *currElem = NULL;
	//Initialize
	My402ListInit(&list);
	printf("List Initialized\n");

	//Check if Empty
    int s = My402ListEmpty(&list);
    printf("Empty check: %d\n", s);

    //Check for length
    s = My402ListLength(&list);
    printf("Length: %d\n", s);

    //Append first element
   
    s = My402ListAppend(&list, (void *)10); 
    printf("Appended: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next: %d\n", (int)list.anchor.next->obj);
    printf("anchor->prev: %d\n", (int)list.anchor.prev->obj);

    s = My402ListEmpty(&list);
    printf("Empty check: %d\n", s);

    //Append

    s = My402ListAppend(&list, (void *)12); 
    printf("Appended: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next %d\n", (int)list.anchor.next->obj);
    printf("anchor->prev %d\n", (int)list.anchor.prev->obj);

    s = My402ListAppend(&list, (void *)13); 
    printf("Appended: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next %d\n", (int)list.anchor.next->obj);
    printf("anchor->prev %d\n", (int)list.anchor.prev->obj);

    s = My402ListEmpty(&list);
    printf("Empty check: %d\n", s);

    for (currElem=My402ListFirst(&list); currElem != NULL; currElem=My402ListNext(&list, currElem)){
        printf("*********");
        printf("%d\n", (int)currElem->obj);
    }

    
    //Prepend first element
    s = My402ListPrepend(&list, (void *)20); 
    printf("Prepended: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next %d\n", (int)list.anchor.next->obj);
    printf("anchor->prev %d\n", (int)list.anchor.prev->obj);

    s = My402ListEmpty(&list);
    printf("Empty check: %d\n", s);

    //Prepend
    s = My402ListPrepend(&list, (void *)22); 
    printf("Prepended: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next %d\n", (int)list.anchor.next->obj);
    printf("anchor->prev %d\n", (int)list.anchor.prev->obj);

    s = My402ListEmpty(&list);
    printf("Empty check: %d\n", s);

    for (currElem=My402ListFirst(&list); currElem != NULL; currElem=My402ListNext(&list, currElem)){
        printf("*********");
        printf("%d\n", (int)currElem->obj);
    }
	
    My402ListElem *t;

    //First Element
    t = My402ListFirst(&list); 
    printf("First: %d\n", (int)t->obj);

    //Last Element
    t = My402ListLast(&list);
    printf("Last: %d\n", (int)t->obj);

    //Next Element
    t = My402ListNext(&list, list.anchor.next); 
    printf("Next to firstElem: %d\n", (int)t->obj);

    //Next to last element
    t = My402ListNext(&list, list.anchor.prev); 
    if (t != NULL){
    	printf("lastElem->next: %d\n", (int)t->obj);
    }
    else{
    	printf("NULL\n");
    }

    //Prev element
    t = My402ListPrev(&list, list.anchor.prev); 
    printf("Prev to lastElem: %d\n", (int)t->obj);

    //Prev to first element
    t = My402ListPrev(&list, list.anchor.next); 
    if (t != NULL){
    	printf("firstElem->prev: %d\n", (int)t->obj);
    }
    else{
    	printf("NULL\n");
    }

    //Find element which is present
    t = My402ListFind(&list, (void *)12); 
    printf("Element found: %d\n", (int)t->obj);

    //Find element which is not present
    t = My402ListFind(&list, (void *)45); 
    if (t == NULL){
    	printf("Element not found\n");
    }
    else{
    	printf("ERROR\n");
    }

    //Insert after an element
    s = My402ListInsertAfter(&list, (void *)50, list.anchor.next);
    printf("Inserted: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next->next %d\n", (int)list.anchor.next->next->obj);


    //Insert before an element
    s = My402ListInsertBefore(&list, (void *)50, list.anchor.prev);
    printf("Inserted: %d\n", s);
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->prev->prev %d\n", (int)list.anchor.prev->prev->obj);


    //Unlink an element
    printf("Before unlinking: anchor->next %d\n", (int)list.anchor.next->obj);
    printf("Before unlinking: anchor->next->next %d\n", (int)list.anchor.next->next->obj);
	My402ListUnlink(&list, list.anchor.next);
	printf("Unlinked");
    printf("Length: %d\n", My402ListLength(&list));
    printf("anchor->next %d\n", (int)list.anchor.next->obj);

    //Unlink all the elements
    My402ListUnlinkAll(&list);
	printf("Unlinked All\n");
    printf("Length: %d\n", My402ListLength(&list));

    return(0);
}
