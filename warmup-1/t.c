#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct tagMy402Transaction {
    char type;
    char *timestamp;
    char *desc;
    int cents;
} Transaction;

int main(int argc, char *argv[]){
	Transaction *t = (Transaction *)malloc(sizeof(Transaction));
	char *ptr = "1230728833";

    time_t tt = atoi(ptr);
    char buf[26];
    strcpy(buf, ctime(&tt));
    /* buf[25] is guaranteed to be '\0' and buf[24] is guaranteed to be '\n' */

    buf[24] = '\0';
    char temp[12];
    char year[5] = {buf[20], buf[21], buf[22], buf[23]};
    //printf("%c\n", buf[25]);
    //printf("%c\n", buf[24]);

    t->timestamp = buf;
    printf("%s\n", t->timestamp);
    //strncat(temp, year, strlen(year));
    //printf("%s\n", temp);
    /*strncpy(temp, buf, 10);
    printf("%s\n", buf);
    printf("%s\n", temp);
    printf("%s\n", year);

	
	printf("%s\n", temp);*/
	//t->timestamp =  temp;
	//printf("%s\n", t->timestamp);


	t->type = '-';



	return 0;
}