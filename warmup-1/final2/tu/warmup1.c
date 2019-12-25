#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "cs402.h"

#include "my402list.h"

/* ----------------------- main() ----------------------- */
typedef struct tagMy402Transaction {
    char type;
    int timestamp;
    char *desc;
    int cents;
}Transaction;


static
void BubbleForward(My402List *pList, My402ListElem **pp_elem1, My402ListElem **pp_elem2)
{
    My402ListElem *elem1=(*pp_elem1), *elem2=(*pp_elem2);
    void *obj1=elem1->obj, *obj2=elem2->obj;
    My402ListElem *elem1prev=My402ListPrev(pList, elem1);
    My402ListElem *elem2next=My402ListNext(pList, elem2);

    My402ListUnlink(pList, elem1);
    My402ListUnlink(pList, elem2);
    if (elem1prev == NULL) {
        (void)My402ListPrepend(pList, obj2);
        *pp_elem1 = My402ListFirst(pList);
    } else {
        (void)My402ListInsertAfter(pList, obj2, elem1prev);
        *pp_elem1 = My402ListNext(pList, elem1prev);
    }
    if (elem2next == NULL) {
        (void)My402ListAppend(pList, obj1);
        *pp_elem2 = My402ListLast(pList);
    } else {
        (void)My402ListInsertBefore(pList, obj1, elem2next);
        *pp_elem2 = My402ListPrev(pList, elem2next);
    }
}


static
void BubbleSortForwardList(My402List *pList, int num_items)
{
    My402ListElem *elem=NULL;
    int i=0;

    if (My402ListLength(pList) != num_items) {
        fprintf(stderr, "List length is not %1d in BubbleSortForwardList().\n", num_items);
        exit(1);
    }
    for (i=0; i < num_items; i++) {
        int j=0, something_swapped=FALSE;
        My402ListElem *next_elem=NULL;

        for (elem=My402ListFirst(pList), j=0; j < num_items-i-1; elem=next_elem, j++) {
            next_elem=My402ListNext(pList, elem);
            int cur_val= (int)((Transaction *)(elem->obj))->timestamp, next_val = (int)((Transaction *)(next_elem->obj))->timestamp;

            if (cur_val > next_val){
                BubbleForward(pList, &elem, &next_elem);
                something_swapped = TRUE;
            }
        }
        if (!something_swapped) break;
    }
}


void parseLine(My402List *list, char *str){
    char *ptr;
    const char delim[]= "\t";
    int count = 0;
    char c;
    My402ListElem *currElem=NULL;

    for (int i = 0; str[i] != '\0'; i++) {
      c = (char)str[i];
      if (c == '\t'){
         ++count;
      }
    }

    if(count == 3){
        Transaction *t = (Transaction *)malloc(sizeof(Transaction));
        ptr=strtok(str,delim);
        t->type = *ptr;
        
        ptr = strtok (NULL, delim);
        if (strlen(ptr) > 11){
            fprintf(stderr, "Bad timestamp\n");
            exit(-1);
        }
        t->timestamp = atoi(ptr);
        
        ptr = strtok (NULL, delim);
        if(atof(ptr) < 0){
            fprintf(stderr, "Negative value\n");
            exit(-1);
        }

        int val = 0;
        int pos = 0;
        char ch;

        while(ptr[pos]!= '.'){
            ch = ptr[pos];
            val = val*10 + atoi(&ch);
            pos++;
        }
        pos++;

        while(ptr[pos] != '\0'){
            ch = ptr[pos];
            val = val*10 + atoi(&ch);
            pos++;
        }

        if(val > 1000000000){
            fprintf(stderr, "Input amount too big\n");
            exit(-1);
        }

        t->cents = val;

        ptr = strtok (NULL, delim);

        t->desc = strdup(ptr);
        if(ptr == NULL){
            fprintf(stderr, "Empty Description\n");
            exit(-1);
        }

        for (currElem=My402ListFirst(list); currElem != NULL; currElem=My402ListNext(list, currElem)){
            if((int)((Transaction *)(currElem->obj))->timestamp == t->timestamp){
                fprintf(stderr, "Already present\n");
                exit(-1);
            }
        }
        My402ListAppend(list, (void *)t);
    }
    else{
        fprintf(stderr, "Malformed text file\n");
        exit(-1);
    }
}

void printAll(My402List *list){
    My402ListElem *currElem=NULL;

    for (currElem=My402ListFirst(list); currElem != NULL; currElem=My402ListNext(list, currElem)){
        printf("%d\n", (int)((Transaction *)(currElem->obj))->timestamp);
    }
}

My402List *readFrom(char *file, My402List *list){
    int MAX_LENGTH = 1023;
    FILE *fp;
    char str[MAX_LENGTH];
    str[MAX_LENGTH] = '\0';

    if(file){
        fp = fopen(file , "r");
        if(fp == NULL) {
           perror("Error opening file");
        }
    }
    else{
        fp = stdin;
    }
    while(!feof(fp))
    if(fgets(str, MAX_LENGTH, fp)!=NULL) {
       parseLine(list, str);
    }
    fclose(fp);
    return list;
}

void sortList(My402List *list){
    BubbleSortForwardList(list, My402ListLength(list));
}

void printTransactions(My402List *list){
    My402ListElem *currElem=NULL;
    int balance = 0;
    printf("+-----------------+--------------------------+----------------+----------------+\n");
    printf("|       Date      | Description              |         Amount |        Balance |\n");
    printf("+-----------------+--------------------------+----------------+----------------+\n");
    for (currElem=My402ListFirst(list); currElem != NULL; currElem=My402ListNext(list, currElem)){
        char line[80];
        line[0] = '\0';

        time_t t = ((Transaction *)(currElem->obj))->timestamp;
        char buf[26];
        char date[11];
        strcpy(buf, ctime(&t));
        buf[24] = '\0';
        memcpy(date, &buf, 11);
        date[10] = '\0';
        char year[6] = {' ', buf[20], buf[21], buf[22], buf[23], '\0'};
        strcat(date, year);
        strcat(line, "| ");
        strcat(line, date);
        strcat(line, " | ");

        char *des = ((Transaction *)(currElem->obj))->desc;
        char dess[24];
        strncpy(dess, des, 24);

        if(strlen(des) > 24){
            dess[24] = '\0';
        }
        else{
            dess[strlen(dess)-1] = ' ';

            int length = 24;
            int l = strlen(dess);

            while(l<length) {
                dess[l] = ' ';
                l++;
            }
            dess[l]= '\0';
        }

        strcat(line, dess);
        strcat(line, " | ");

        int c = ((Transaction *)(currElem->obj))->cents;
        char type = ((Transaction *)(currElem->obj))->type;

        if(type == '-'){
            balance = balance - c;
        }
        else{
            balance = balance + c;
        }

        int cents = c%100;
        c = c/100;

        char bufs[13];

        if(c >= 10000000){
            snprintf(bufs, 13, "?,???,???.??");
        }

        else if(c >= 1000000){
            char afterSecondCommaChar[4];
            char beforeSecondCommaChar[4];
            char centsChar[4];

            int beforeFirstComma = c/1000000;
            int afterFirstComma = c%1000000;

            int beforeSecondComma = afterFirstComma/1000;
            int afterSecondComma = afterFirstComma%1000;
            
            if(beforeSecondComma < 10){
                snprintf(beforeSecondCommaChar, 4, "00%d", beforeSecondComma);
            }
            else if (beforeSecondComma < 100){
                snprintf(beforeSecondCommaChar, 4, "0%d", beforeSecondComma);
            }
            else{
                snprintf(beforeSecondCommaChar, 4, "%d", beforeSecondComma);
            }

            if(afterSecondComma < 10){
                snprintf(afterSecondCommaChar, 4, "00%d", afterSecondComma);
            }
            else if (afterSecondComma < 100){
                snprintf(afterSecondCommaChar, 4, "0%d", afterSecondComma);
            }
            else{
                snprintf(afterSecondCommaChar, 4, "%d", afterSecondComma);
            }

            if(cents == 0){
                snprintf(centsChar, 4, "00");
            }
            else if(cents < 10){
                snprintf(centsChar, 4, "0%d", cents);
            }
            else{
                snprintf(centsChar, 4, "%d", cents);
            }

            snprintf(bufs, 13, "%d,%s,%s.%s", beforeFirstComma, beforeSecondCommaChar, afterSecondCommaChar, centsChar);

        }

        else if (c >= 1000){
            char afterSecondCommaChar[4];
            char centsChar[4];
            int beforeSecondComma = c/1000;
            int afterSecondComma = c%1000;
            //printf("afterSecondComma:%d", afterSecondComma);
            if(afterSecondComma < 10){
                snprintf(afterSecondCommaChar, 4, "00%d", afterSecondComma);
            }
            else if (afterSecondComma < 100){
                snprintf(afterSecondCommaChar, 4, "0%d", afterSecondComma);
            }
            else{
                snprintf(afterSecondCommaChar, 4, "%d", afterSecondComma);
            }

            if(cents == 0){
                snprintf(centsChar, 4, "00");
            }
            else if(cents < 10){
                snprintf(centsChar, 4, "0%d", cents);
            }
            else{
                snprintf(centsChar, 4, "%d", cents);
            }
            snprintf(bufs, 13, "%d,%s.%s", beforeSecondComma, afterSecondCommaChar, centsChar);
        }
        else{
            if(cents == 0){
                snprintf(bufs, 13, "%d.00", c);
            }
            else{
                if(cents < 10){
                    snprintf(bufs, 13, "%d.0%d", c, cents);
                }
                else{
                    snprintf(bufs, 13, "%d.%d", c, cents);
                }
            }
        }

        char bufss[15];

        int target = 12 - strlen(bufs), i = 0;

        while(i < target) {
            bufss[i] = ' '; 
            i++;
        }
        bufss[i]= '\0';

        if(type == '-'){
            strcat(line, "(");
            strcat(line, bufss);
            strcat(line, bufs);
            strcat(line, ")");
        }
        else{
            strcat(line, " ");
            strcat(line, bufss);
            strcat(line, bufs);
            strcat(line, " ");
        }

        strcat(line, " | ");
        c = balance;
        if(balance<0){
            c = c*(-1);
        }
        int centsTwo = c%100;
        c = c/100;

        char buff[13];
        if(c >= 10000000){
            snprintf(buff, 13, "?,???,???.??");
        }
        else if(c >= 1000000){
            char afterSecondCommaChar[4];
            char beforeSecondCommaChar[4];
            char centsChar[4];

            int beforeFirstComma = c/1000000;
            int afterFirstComma = c%1000000;

            int beforeSecondComma = afterFirstComma/1000;
            int afterSecondComma = afterFirstComma%1000;
            
            if(beforeSecondComma < 10){
                snprintf(beforeSecondCommaChar, 4, "00%d", beforeSecondComma);
            }
            else if (beforeSecondComma < 100){
                snprintf(beforeSecondCommaChar, 4, "0%d", beforeSecondComma);
            }
            else{
                snprintf(beforeSecondCommaChar, 4, "%d", beforeSecondComma);
            }

            if(afterSecondComma < 10){
                snprintf(afterSecondCommaChar, 4, "00%d", afterSecondComma);
            }
            else if (afterSecondComma < 100){
                snprintf(afterSecondCommaChar, 4, "0%d", afterSecondComma);
            }
            else{
                snprintf(afterSecondCommaChar, 4, "%d", afterSecondComma);
            }

            if(centsTwo == 0){
                snprintf(centsChar, 4, "00");
            }
            else if(centsTwo < 10){
                snprintf(centsChar, 4, "0%d", centsTwo);
            }
            else{
                snprintf(centsChar, 4, "%d", centsTwo);
            }

            snprintf(buff, 13, "%d,%s,%s.%s", beforeFirstComma, beforeSecondCommaChar, afterSecondCommaChar, centsChar);

        }

        else if (c >= 1000){
            char afterSecondCommaChar[4];
            char centsChar[4];
            int beforeSecondComma = c/1000;
            int afterSecondComma = c%1000;
            //printf("afterSecondComma:%d", afterSecondComma);
            if(afterSecondComma < 10){
                snprintf(afterSecondCommaChar, 4, "00%d", afterSecondComma);
            }
            else if (afterSecondComma < 100){
                snprintf(afterSecondCommaChar, 4, "0%d", afterSecondComma);
            }
            else{
                snprintf(afterSecondCommaChar, 4, "%d", afterSecondComma);
            }

            if(centsTwo == 0){
                snprintf(centsChar, 4, "00");
            }
            else if(centsTwo < 10){
                snprintf(centsChar, 4, "0%d", centsTwo);
            }
            else{
                snprintf(centsChar, 4, "%d", centsTwo);
            }
            snprintf(buff, 13, "%d,%s.%s", beforeSecondComma, afterSecondCommaChar, centsChar);
        }
        else{
            if(centsTwo == 0){
                snprintf(buff, 13, "%d.00", c);
            }
            else{
                if(centsTwo < 10){
                    snprintf(buff, 13, "%d.0%d", c, centsTwo);
                }
                else{
                    snprintf(buff, 13, "%d.%d", c, centsTwo);
                }
            }
        }

        target = 12 - strlen(buff), i = 0;
        char tryu[15];
        while(i < target) {
            tryu[i] = ' '; 
            i++;
        }
        tryu[i]= '\0';

        if(balance < 0){
            strcat(line, "(");
            strcat(line, tryu);
            strcat(line, buff);
            strcat(line, ")");
        }
        else{
            strcat(line, " ");
            strcat(line, tryu);
            strcat(line, buff);
            strcat(line, " ");
        }
        strcat(line, " |");

        printf("%s\n", line);
    }

    
    printf("+-----------------+--------------------------+----------------+----------------+\n");
}


int main(int argc, char *argv[])
{
    My402List *list= (My402List *)malloc(sizeof(My402List));
    My402ListInit(list);

    char *file = argv[2];
    list = readFrom(file, list);
    sortList(list);
    printTransactions(list);
    My402ListUnlinkAll(list);
    free(list);
    return(0);

}