#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "cs402.h"
#include "my402list.h"

typedef struct tagMy402Packet {
    int packet_number;
    struct timeval arrival_time;
    struct timeval enter_time_Q1;
    struct timeval departure_time_Q1; // convert to struct timeval and use inbuilt functions
    struct timeval time_in_Q1;
    struct timeval enter_time_Q2;
    struct timeval departure_time_Q2;
    struct timeval time_in_Q2;
    int lambda;
    int service_time;
    int tokens_required;
}Packet;

pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Queue_Not_Empty_Cond = PTHREAD_COND_INITIALIZER;
int Queue_Amount_Guard = 0;
My402List *Q1, *Q2;

long int timeValToLongInt(struct timeval time){
	long int converted_time = time.tv_sec * 1000000 + time.tv_usec;
	return converted_time;
}

void printAll(My402List *list){
    My402ListElem *currElem=NULL;

    for (currElem=My402ListFirst(list); currElem != NULL; currElem=My402ListNext(list, currElem)){
        printf("%d\n", (int)((Packet *)(currElem->obj))->packet_number);
    }
}

void *packet_arrival(){
	int i = 0;
	
	while(i < 2){
		i++;
		struct timeval packet_arrival_time;
		gettimeofday(&packet_arrival_time, 0);
		int time_between_packets = 0;
		int time_to_sleep = 0;

		Packet *p = (Packet *)malloc(sizeof(Packet));
		p->packet_number = i;
		p->arrival_time = packet_arrival_time;
		p->lambda = 0.5;
		p->service_time = 100;
		p->tokens_required = 3;

		My402ListAppend(Q1, (void *)p);
		struct timeval enter_time_Q1;
		gettimeofday(&enter_time_Q1, 0);

		p->enter_time_Q1 = enter_time_Q1;

		time_between_packets = (int)1000000/p->lambda;
		time_to_sleep = time_between_packets - (timeValToLongInt(p->enter_time_Q1) - timeValToLongInt(p->arrival_time));

		usleep(time_to_sleep);

		printf("Packet Added\n");


	}


	printAll(Q1);
	return(0);
}

int main(int argc,char *argv[])
{
Q1 = (My402List *)malloc(sizeof(My402List));
My402ListInit(Q1);
//My402ListInit(Q2);
//create four threds and join them 
pthread_t packet_arrival_thread;

pthread_create(&packet_arrival_thread,NULL,packet_arrival,NULL);

pthread_join(packet_arrival_thread,NULL);

//Run this once all four threads are dead
//Print statistics

return 0;

}