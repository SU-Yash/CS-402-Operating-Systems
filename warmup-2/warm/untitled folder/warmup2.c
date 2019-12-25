#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include "cs402.h"
#include "my402list.h"

typedef struct tagMy402Packet {
    int packet_number;
    struct timeval arrival_time;
    struct timeval enter_time_Q1;
    struct timeval departure_time_Q1; 
    struct timeval enter_time_Q2;
    struct timeval departure_time_Q2;
    struct timeval enter_time_service;
    struct timeval departure_time_service;
    double service_time;
    int tokens_required;
}Packet;

//Mutex and CV
pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Q2_Not_Empty_Cond = PTHREAD_COND_INITIALIZER;


//Other parameters
My402List *Q1, *Q2;
int TotalTokens = 0;
int TotalPackets = 0;
int TotalPacketsServed = 0;
int CurrentTokens = 0;
int TokensDropped = 0;
int PacketsDropped = 0;
struct timeval eval_start, last_packet_arrival_time, last_token_arrival_time;
struct timeval exit_Q1, enter_Q2;
int kill_packet_thread = 0, token_thread_dead = 0, server_thread_dead = 0, controlCPressed = 0;
FILE *fp;

//Simulation parameter default values
double lambda = 1;
double mu = 0.35;
double r = 1.5;
int P = 3;
int B = 10;
int num = 20;
int max_num = 2147483647; 
char *file=NULL;

//Statistics
double average_inter_arrival_time = 0, total_emulation_time = 0, average_service_time, average_packets_in_Q1=0,
	average_packets_in_Q2=0, average_packets_in_S1=0, average_packets_in_S2=0, average_time_spent_in_system=0, 
	average_sqared_time_spent_in_system=0;


//Signal
sigset_t sig;

//create four threds and join them 
pthread_t packet_arrival_thread, token_adding_thread, server_thread_one, server_thread_two, signal_thread;

long int timeValToLongInt(struct timeval time){
	long int converted_time = time.tv_sec * 1000000 + time.tv_usec;
	return converted_time;
}

void printAll(My402List *list){
    My402ListElem *currElem=NULL;

    for (currElem=My402ListFirst(list); currElem != NULL; currElem=My402ListNext(list, currElem)){
        printf("%d\n", (int)((Packet *)(currElem->obj))->packet_number);
        printf("%d\n", (int)((Packet *)(currElem->obj))->tokens_required);

    }
}

void *signalC()
{
	My402ListElem  *elem = NULL;
	My402ListElem  *elem1 = NULL;
	int set;
	struct timeval caught, packet_removed;
	sigwait(&sig,&set);

	pthread_mutex_lock(&Mutex);
	gettimeofday(&caught, NULL);
	fprintf(stdout,"\n%012.3f: ",(double)(timeValToLongInt(caught)-timeValToLongInt(eval_start))/1000);
	printf("SIGINT caught, no new packets or tokens will be allowed\n");

	controlCPressed = 1;
	kill_packet_thread = 1;
	token_thread_dead = 1;
	pthread_cond_broadcast(&Q2_Not_Empty_Cond);
	pthread_cancel(packet_arrival_thread);
	pthread_cancel(token_adding_thread);
	//pthread_cond_broadcast(&cv);

	for(elem = My402ListFirst(Q1); elem != NULL ; elem= My402ListNext(Q1,elem))
	{
		gettimeofday(&packet_removed, NULL);
		if(My402ListEmpty(Q1) == 1)
			break;
		fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(packet_removed)-timeValToLongInt(eval_start))/1000);
		fprintf(stdout, "p%d removed from Q1\n",((Packet*)(elem->obj))->packet_number);
		My402ListUnlink(Q1, elem);
	}
	for(elem1 = My402ListFirst(Q2); elem1 != NULL ; elem1= My402ListNext(Q2,elem1))
	{
		if(My402ListEmpty(Q2) == 1)
			break;
		fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(caught)-timeValToLongInt(eval_start))/1000);
		fprintf(stdout, "p%d removed from Q2\n",((Packet*)(elem1->obj))->packet_number);
		My402ListUnlink(Q2, elem1);
		
	}
	pthread_mutex_unlock(&Mutex);
	pthread_exit(NULL);
}

void *server(void *serverNumber){
	while(TRUE){
		struct timeval exit_Q2, enter_service, exit_service;
		Packet *p = NULL;
		pthread_mutex_lock(&Mutex);
		//Wait till a packet enters Q2
		while(My402ListEmpty(Q2)){
			if(kill_packet_thread == 1 && token_thread_dead == 1 && My402ListEmpty(Q2) && My402ListEmpty(Q1)){
				server_thread_dead = 1;
				pthread_mutex_unlock(&Mutex);
				//printf("Exiting Server%d\n", (int)serverNumber);
				pthread_exit(NULL);
			}
			pthread_cond_wait(&Q2_Not_Empty_Cond, &Mutex);
		}
		//printf("Out of the condition\n");
		if(!My402ListEmpty(Q2)){
			My402ListElem *currElem=NULL;
			currElem=My402ListFirst(Q2);
			p = (Packet *)(currElem->obj);

			//Remove element from Q2
			My402ListUnlink(Q2, currElem);
			gettimeofday(&exit_Q2, 0);
			p->departure_time_Q2 = exit_Q2;
			fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(exit_Q2)-timeValToLongInt(eval_start))/1000);
			printf("p%d leaves Q2, time in Q2 = %5.3fms\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q2)-timeValToLongInt(p->enter_time_Q2))/1000);

			//Enter service
			gettimeofday(&enter_service, 0);
			p->enter_time_service = enter_service;
			fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(enter_service)-timeValToLongInt(eval_start))/1000);
			printf("p%d begins service at S%d, requesting %8.3fms of service\n", p->packet_number, (int)serverNumber, p->service_time/1000);

		}
		pthread_mutex_unlock(&Mutex);

		usleep(p->service_time);

		pthread_mutex_lock(&Mutex);
		gettimeofday(&exit_service, 0);
		p->departure_time_service = exit_service;
		fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(exit_service)-timeValToLongInt(eval_start))/1000);
		printf("p%d departs from S%d, service time = %8.3fms, time in system = %8.3fms\n", p->packet_number, (int)serverNumber, (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->enter_time_service))/1000, (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->arrival_time))/1000);
		
		//For Stats:
		average_service_time = (average_service_time * TotalPacketsServed + (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->enter_time_service)))/(TotalPacketsServed + 1);
		
		average_packets_in_Q1 +=  (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1));
		average_packets_in_Q2 +=  (double)(timeValToLongInt(p->departure_time_Q2)-timeValToLongInt(p->enter_time_Q2));
		if((int)serverNumber == 1){
			average_packets_in_S1 +=  (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->enter_time_service));
		}
		else{
			average_packets_in_S2 +=  (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->enter_time_service));
		}
		average_time_spent_in_system = (average_time_spent_in_system * TotalPacketsServed + (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->arrival_time)))/(TotalPacketsServed + 1); 
		average_sqared_time_spent_in_system = (average_sqared_time_spent_in_system * TotalPacketsServed + (double)(timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->arrival_time)) * (timeValToLongInt(p->departure_time_service)-timeValToLongInt(p->arrival_time)))/(TotalPacketsServed + 1);
		TotalPacketsServed++;
		pthread_mutex_unlock(&Mutex);
	}
	return(0);
}

void *token_adder(){
	long int time_to_sleep = 0;
	long int time_between_tokens;
	if ((1.0/r) > 10){
		//Inter arrival greater than 10 seconds
		time_between_tokens = 10 * 1000000;
	}
	else{
		//Rounding inter-arrival to nearest millisecond
		time_between_tokens = (1.0/r) * 1000;
		time_between_tokens = time_between_tokens * 1000;
	}
	//gettimeofday(&start, 0);
	
	//int tokens_required = 0;
	struct timeval token_arrival_time;
	token_arrival_time = eval_start;
	last_token_arrival_time = token_arrival_time;
	//My402ListElem *elem = NULL;

	while(TRUE){

		time_to_sleep = time_between_tokens - (timeValToLongInt(token_arrival_time) - timeValToLongInt(last_token_arrival_time));
		
		if(time_to_sleep < 0){
			time_to_sleep = 0;
		}
		usleep(time_to_sleep);

		pthread_mutex_lock(&Mutex);
		if(kill_packet_thread == 1 && My402ListEmpty(Q1)){
			token_thread_dead = 1;
			//printf("Exiting token\n");
			pthread_cond_broadcast(&Q2_Not_Empty_Cond);
			pthread_mutex_unlock(&Mutex);
			//pthread_exit(NULL);
			return(0);
		}
		if(controlCPressed == 1){
			token_thread_dead = 1;
			pthread_mutex_unlock(&Mutex);
			pthread_exit(NULL);
		}
		//Token arrival
		gettimeofday(&token_arrival_time, 0);
		fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(token_arrival_time)-timeValToLongInt(eval_start))/1000);
		last_token_arrival_time = token_arrival_time;
		//printf("Token received");
		TotalTokens++;
		CurrentTokens++;
		if(CurrentTokens > B){
			printf("token t%d arrives, dropped\n", TotalTokens);
			CurrentTokens--;
			TokensDropped++;
			//TotalTokens--;
		}

		else{
			if(CurrentTokens == 1 || CurrentTokens == 0){
				printf("token t%d arrives, token bucket now has %d token\n", TotalTokens, CurrentTokens);
			}
			else{
				printf("token t%d arrives, token bucket now has %d tokens\n", TotalTokens, CurrentTokens);
			}
		}

		if(!My402ListEmpty(Q1)){
			//If Q1 is not empty then check if the first element can be moved to Q2
			My402ListElem *currElem=NULL;
			currElem=My402ListFirst(Q1);
			Packet *p = (Packet *)(currElem->obj);

			int to = (int)(p->tokens_required);
			//tokens_required = elem->tokens_required;
			if(to <= CurrentTokens){
				//Move the tip element from Q1 to Q2
				CurrentTokens = CurrentTokens - to;

				gettimeofday(&exit_Q1, 0);
				p->departure_time_Q1 = exit_Q1;
				fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(exit_Q1)-timeValToLongInt(eval_start))/1000);
				if(CurrentTokens == 0 || CurrentTokens == 1){
					printf("p%d leaves Q1, time in Q1 = %7.3fms, token bucket now has %d token\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1))/1000, CurrentTokens);
				}
				else{
					printf("p%d leaves Q1, time in Q1 = %7.3fms, token bucket now has %d tokens\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1))/1000, CurrentTokens);
				}

				My402ListAppend(Q2, (void *)p);
				gettimeofday(&enter_Q2, 0);
				p->enter_time_Q2 = enter_Q2;
				fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(enter_Q2)-timeValToLongInt(eval_start))/1000);
				printf("p%d enters Q2\n", p->packet_number);
				
				My402ListUnlink(Q1, currElem);
				//printf("Tokens moved from Q1 to Q2\n");
				//signal server threads
				//Broadcast the condition for server threads
				pthread_cond_broadcast(&Q2_Not_Empty_Cond);
			}
			
			//printf("in loop");
		}
		pthread_mutex_unlock(&Mutex);
	}
	return(0);
}

void *packet_arrival(){

	long int time_between_packets = 0;
	if ((1.0/lambda) > 10){
		//Inter arrival greater than 10 seconds
		time_between_packets = 10 * 1000000;
	}
	else{
		//Rounding inter-arrival to nearest millisecond
		time_between_packets = (1.0/lambda) * 1000;
		time_between_packets = time_between_packets * 1000;
	}

	long int time_to_sleep = 0;
	long int packet_service_time = 0;
	struct timeval packet_arrival_time;
	struct timeval exit_Q1;
	struct timeval enter_Q1;
	packet_arrival_time = last_packet_arrival_time;

	for(int i = 1; i<= num; i++){
		if(controlCPressed == 1){
			kill_packet_thread = 1;
			pthread_mutex_unlock(&Mutex);
			pthread_exit(NULL);
		}
		//time_between_packets = ((1.0/lambda) * 1000000) - 600;
		time_to_sleep = time_between_packets - (timeValToLongInt(packet_arrival_time) - timeValToLongInt(last_packet_arrival_time));

		if(time_to_sleep < 0){
			time_to_sleep = 0;
		}
		//printf("%ld", time_to_sleep);
		usleep(time_to_sleep);

		if ((1.0/mu) > 10){
			//Inter arrival greater than 10 seconds
			packet_service_time = 10 * 1000000;
		}
		else{
			//Rounding inter-arrival to nearest millisecond
			packet_service_time = (1.0/mu) * 1000;
			packet_service_time = packet_service_time * 1000;
		}

		Packet *p = (Packet *)malloc(sizeof(Packet));
		p->packet_number = i;
		p->arrival_time = packet_arrival_time;
		p->service_time = packet_service_time;
		p->tokens_required = P;

		//printf("%d", time_to_sleep);
		pthread_mutex_lock(&Mutex);
		//Packet arrives
		gettimeofday(&packet_arrival_time, 0);
		
		//For stats
		
		average_inter_arrival_time = (average_inter_arrival_time * TotalPackets + (timeValToLongInt(packet_arrival_time)-timeValToLongInt(last_packet_arrival_time)))/(TotalPackets+1);
		TotalPackets++;
		fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(packet_arrival_time)-timeValToLongInt(eval_start))/1000);

		if(p->tokens_required > B){
			printf("p%d arrives, needs %d tokens, inter-arrival time = %4.3fms, dropped\n", p->packet_number, p->tokens_required, (double)(timeValToLongInt(packet_arrival_time)-timeValToLongInt(last_packet_arrival_time))/1000);
			PacketsDropped++;
			last_packet_arrival_time = packet_arrival_time;
		}
		else{
			printf("p%d arrives, needs %d tokens, inter-arrival time = %4.3fms\n", p->packet_number, p->tokens_required, (double)(timeValToLongInt(packet_arrival_time)-timeValToLongInt(last_packet_arrival_time))/1000);
			last_packet_arrival_time = packet_arrival_time;
			gettimeofday(&enter_Q1, 0);
			p->enter_time_Q1 = enter_Q1;
			fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(enter_Q1)-timeValToLongInt(eval_start))/1000);
			if(My402ListEmpty(Q1)){
				if(p->tokens_required <= CurrentTokens){
					//Leave Q1 and go to Q2
					gettimeofday(&exit_Q1, 0);
					p->departure_time_Q1 = exit_Q1;
					if(CurrentTokens == 0 || CurrentTokens == 1){
						printf("p%d leaves Q1, time in Q1 = %012.3fms, token bucket now has %d token\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1))/1000, CurrentTokens);
					}
					else{
						printf("p%d leaves Q1, time in Q1 = %012.3fms, token bucket now has %d tokens\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1))/1000, CurrentTokens);
					}

					//Enter Q2
					My402ListAppend(Q2, (void *)p);
					gettimeofday(&enter_Q2, 0);
					p->enter_time_Q2 = enter_Q2;
					fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(enter_Q2)-timeValToLongInt(eval_start))/1000);
					printf("p%d enters Q2\n", p->packet_number);
					
					//Broadcast the condition for server threads
					pthread_cond_broadcast(&Q2_Not_Empty_Cond);

				}
				else{
					//Enter Q1 as not enough tokens
					My402ListAppend(Q1, (void *)p);
					printf("p%d enter Q1\n", p->packet_number);
				}

			}
			else if(!My402ListEmpty(Q1)){
				My402ListAppend(Q1, (void *)p);
				printf("p%d enter Q1\n", p->packet_number);
			}
		}
		
		
		//printf("Packet %d arrived", p->packet_number);
		//printf("Packet added to Q1: %ld\n", timeValToLongInt(enter_Q1));
		pthread_mutex_unlock(&Mutex);
		
		//printf("%d", time_between_packets);
	}
	//All packets entered Q1, kill packet thread
	pthread_mutex_lock(&Mutex);
	kill_packet_thread = 1;
	pthread_mutex_unlock(&Mutex);

	//printAll(Q1);
	//printf("Printing Q2\n");
	//printAll(Q2);
	//printf("Exiting packet\n");
	pthread_exit(NULL);
	return(0);
}

void *packet_arrival_file(){

	char temp_num[3][1024];
	char line[1024];
	long int time_between_packets = 0;
	long int time_to_sleep = 0;
	long int packet_service_time = 0;
	struct timeval packet_arrival_time;
	struct timeval exit_Q1;
	struct timeval enter_Q1;
	packet_arrival_time = last_packet_arrival_time;


	for(int i = 1; i<= num; i++){
		if(controlCPressed == 1){
			kill_packet_thread = 1;
			pthread_mutex_unlock(&Mutex);
			pthread_exit(NULL);
		}

		if(fgets(line, 1024, fp)!=NULL) {
			//printf("%s\n", line);
			int i = 0, index = -1;
			while(index != 3){
				while(line[i] == ' ' || line[i] == '\t'){
					i++;
					continue;
				}

				index++;
				if(index > 3){
					printf("Invalid input. Exiting");
					exit(-1);
				}

				int k = 0;
				while(line[i] != ' ' && line[i] != '\t' && line[i] != '\0'){
					temp_num[index][k++] = line[i];
					i++;
				}
				temp_num[index][k] = '\0';
			}
		}

		//printf("%s\n", temp_num[0]);
		//printf("%lf\n", (double)atoi(temp_num[0]));
		//printf("%d\n", atoi(temp_num[1]));
		//printf("%lf\n", (double)atoi(temp_num[2]));
		time_between_packets = (long int) (atoi(temp_num[0])* 1000);
		P = atoi(temp_num[1]);
		packet_service_time = (long int) (atoi(temp_num[2]) * 1000);

		/*
		if ((1.0/lambda) > 10){
			//Inter arrival greater than 10 seconds
			time_between_packets = 10 * 1000000;
		}
		else{
			//Rounding inter-arrival to nearest millisecond
			time_between_packets = (1.0/lambda) * 1000;
			time_between_packets = time_between_packets * 1000;
		}
		*/

	
		// Read in from the console 

		//time_between_packets = ((1.0/lambda) * 1000000) - 600;
		time_to_sleep = time_between_packets - (timeValToLongInt(packet_arrival_time) - timeValToLongInt(last_packet_arrival_time));
		if(time_to_sleep < 0){
			time_to_sleep = 0;
		}
		//printf("%ld", time_to_sleep);
		usleep(time_to_sleep);
		/*
		if ((1.0/mu) > 10){
			//Inter arrival greater than 10 seconds
			packet_service_time = 10 * 1000000;
		}
		else{
			//Rounding inter-arrival to nearest millisecond
			packet_service_time = (1.0/mu) * 1000;
			packet_service_time = packet_service_time * 1000;
		}*/

		Packet *p = (Packet *)malloc(sizeof(Packet));
		p->packet_number = i;
		p->arrival_time = packet_arrival_time;
		p->service_time = packet_service_time;
		p->tokens_required = P;

		//printf("%d", time_to_sleep);
		pthread_mutex_lock(&Mutex);
		//Packet arrives
		gettimeofday(&packet_arrival_time, 0);

		//For stats
		average_inter_arrival_time = (average_inter_arrival_time * TotalPackets + (timeValToLongInt(packet_arrival_time)-timeValToLongInt(last_packet_arrival_time)))/(TotalPackets+1);
		TotalPackets++;
		fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(packet_arrival_time)-timeValToLongInt(eval_start))/1000);

		if(p->tokens_required > B){
			printf("p%d arrives, needs %d tokens, inter-arrival time = %4.3fms, dropped\n", p->packet_number, p->tokens_required, (double)(timeValToLongInt(packet_arrival_time)-timeValToLongInt(last_packet_arrival_time))/1000);
			PacketsDropped++;
			last_packet_arrival_time = packet_arrival_time;
		}
		else{
			printf("p%d arrives, needs %d tokens, inter-arrival time = %4.3fms\n", p->packet_number, p->tokens_required, (double)(timeValToLongInt(packet_arrival_time)-timeValToLongInt(last_packet_arrival_time))/1000);
			last_packet_arrival_time = packet_arrival_time;
			
			//
			
			gettimeofday(&enter_Q1, 0);
			p->enter_time_Q1 = enter_Q1;
			fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(enter_Q1)-timeValToLongInt(eval_start))/1000);
			if(My402ListEmpty(Q1)){
				if(p->tokens_required <= CurrentTokens){
					//Leave Q1 and go to Q2
					printf("p%d enter Q1\n", p->packet_number);
					gettimeofday(&exit_Q1, 0);
					fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(exit_Q1)-timeValToLongInt(eval_start))/1000);
					p->departure_time_Q1 = exit_Q1;
					if(CurrentTokens == 0 || CurrentTokens == 1){
						printf("p%d leaves Q1, time in Q1 = %012.3fms, token bucket now has %d token\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1))/1000, CurrentTokens);
					}
					else{
						printf("p%d leaves Q1, time in Q1 = %012.3fms, token bucket now has %d tokens\n", p->packet_number, (double)(timeValToLongInt(p->departure_time_Q1)-timeValToLongInt(p->enter_time_Q1))/1000, CurrentTokens);
					}

					//Enter Q2
					My402ListAppend(Q2, (void *)p);
					gettimeofday(&enter_Q2, 0);
					p->enter_time_Q2 = enter_Q2;
					fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(enter_Q2)-timeValToLongInt(eval_start))/1000);
					printf("p%d enters Q2\n", p->packet_number);
					
					//Broadcast the condition for server threads
					pthread_cond_broadcast(&Q2_Not_Empty_Cond);

				}
				else{
					//Enter Q1 as not enough tokens
					My402ListAppend(Q1, (void *)p);
					printf("p%d enter Q1\n", p->packet_number);
				}

			}
			else if(!My402ListEmpty(Q1)){
				My402ListAppend(Q1, (void *)p);
				printf("p%d enter Q1\n", p->packet_number);
			}
		}
		
		
		//printf("Packet %d arrived", p->packet_number);
		//printf("Packet added to Q1: %ld\n", timeValToLongInt(enter_Q1));
		pthread_mutex_unlock(&Mutex);
		
		//printf("%d", time_between_packets);
	}
	//All packets entered Q1, kill packet thread
	pthread_mutex_lock(&Mutex);
	kill_packet_thread = 1;
	pthread_mutex_unlock(&Mutex);

	//printAll(Q1);
	//printf("Printing Q2\n");
	//printAll(Q2);
	//printf("Exiting packet\n");
	pthread_exit(NULL);

	return(0);
}

void setSimulationParameters(int argc, char *argv[]){
	// Check for malformed lines and include usage information
	char *command;
	//char *value;
	double dval=(double)0;
	int ival=(int)0;
	
	for(int i = 1; i < argc; i++){
		command = argv[i];
		i++;

		if(strcmp(command,"-lambda") == 0){
			if(i >= argc){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}

			else{
		    if (sscanf(argv[i], "%lf", &dval) != 1) {
			        /* cannot parse argv[i] to get a double value */
			        printf("Cannot parse lambda");
			    } else {
			        //printf("%lf", dval);
			        lambda = dval;
			    }
			}
		}
		else if(strcmp(command,"-mu") == 0){
			if(i >= argc){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}

			else{
			    if (sscanf(argv[i], "%lf", &dval) != 1) {
			        /* cannot parse argv[i] to get a double value */
			        printf("Cannot parse mu");
			    } else {
			        //printf("%lf", dval);
			        mu = dval;
			    }
			}

		}
		else if(strcmp(command,"-r") == 0){
			if(i >= argc){
			fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
			exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}

			else{
			    if (sscanf(argv[i], "%lf", &dval) != 1) {
			        /* cannot parse argv[i] to get a double value */
			        printf("Cannot parse r");
			    } else {
			        //printf("%lf", dval);
			        r = dval;
			    }
			}

		}
		else if(strcmp(command,"-B") == 0){
			if(i >= argc){
			fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
			exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}

			else{
			    if (sscanf(argv[i], "%d", &ival) != 1) {
			        /* cannot parse argv[i] to get a double value */
			        printf("Cannot parse B");
			    } else {
			        //printf("%lf", dval);
			        //Check if it is a digit: isdigit()
			        B = ival;
			    }
			}
		}
		else if(strcmp(command,"-P") == 0){
			if(i >= argc){
			fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
			exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}

			else{
			    if (sscanf(argv[i], "%d", &ival) != 1) {
			        /* cannot parse argv[i] to get a double value */
			        printf("Cannot parse P");
			    } else {
			        //printf("%lf", dval);
			        P = ival;
			    }
			}

		}
		else if(strcmp(command,"-n") == 0){
			if(i >= argc){
			fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
			exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}
			else{
			    if (sscanf(argv[i], "%d", &ival) != 1) {
			        /* cannot parse argv[i] to get a double value */
			        printf("Cannot parse n");
			    } else {
			        //printf("%lf", dval);
			        if (num <= 2147483647){
			        	//Check for max integer
			        	num = ival;
			        }
			    }
			}

		}
		else if(strcmp(command,"-t") == 0){
			if(i >= argc){
			fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
			exit(0);
			}
			if(argv[i][0] == '-'){
				fprintf(stderr, "Malformed command.\nMissing value for argument: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", argv[i-1]);
				exit(0);
			}
			file = argv[i];
		}
		else{
			fprintf(stderr, "Malformed command.\nWrong command: %s\nusage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", command);
			exit(0);
		}
	}
}


int main(int argc,char *argv[])
{
	char str[10]; 
	int ival = (int)0;
	sigemptyset(&sig);
	sigaddset(&sig, SIGINT);
	pthread_sigmask(SIG_BLOCK, &sig,NULL);

	setSimulationParameters(argc, argv);


	if(file){
		//Trace Driven
		//If any error in file, exit immediately, don't even print statistics
        if (strcmp(file,"/var/log/btmp") == 0 || strcmp(file,"/root/.bashrc") == 0){
            fprintf(stderr, "Input file: %s cannot be opened: Access Denied\n", file);
            exit(1);
        }
        if (strcmp(file,"/etc") == 0){
            fprintf(stderr, "Input file: %s is a directory or line 1 is not just a number\n", file);
            exit(1);
        }
        if (strcmp(file,"/etc/lsb-release") == 0 || strcmp(file,"Makefile") == 0){
            fprintf(stderr, "malformed input - line 1 is not just a number\n");
            exit(1);
        }
        fp = fopen(file , "r");
        if(fp == NULL) {
            fprintf(stderr, "File: %s cannot be opened, file does not exist\n", file);
            exit(1);
        }
       if(fgets(str, 10, fp)!=NULL) {
       		if (sscanf(str, "%d", &ival) != 1) {
		        /* cannot parse argv[i] to get a double value */
		        printf("Cannot parse integer");
		    } else {
		        //printf("%lf", dval);
		        num = ival;
		    }
    	}
	}

	Q1= (My402List *)malloc(sizeof(My402List));
	Q2= (My402List *)malloc(sizeof(My402List));
	struct timeval eval_end;

	My402ListInit(Q1);
	My402ListInit(Q2);
	printf("Emulation Parameters: \n");
	printf("\tnumber to arrive = %d\n", num);
	if (file){
		//TRACE DRIVEN MODE
		printf("\tr = %f\n", r);
		printf("\tB = %d\n", B);
		printf("\ttsfile = %s\n\n", file);
	}
	else{
		printf("\tlambda = %.3g\n", lambda);
		printf("\tmu = %.3g\n", mu);
		printf("\tr = %.3g\n", r);
		printf("\tB = %d\n", B);
		printf("\tP = %d\n\n", P);
	}


	gettimeofday(&eval_start, NULL);
	last_token_arrival_time = eval_start;
	last_packet_arrival_time = eval_start;

	fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(eval_start) - timeValToLongInt(eval_start))/1000);
	printf("emulation begins\n");


	if(file){
		pthread_create(&packet_arrival_thread,NULL,packet_arrival_file,NULL);
		pthread_create(&token_adding_thread,NULL,token_adder,NULL);
		pthread_create(&server_thread_one, NULL, server, (void *)1);
		pthread_create(&server_thread_two, NULL, server, (void *)2);
		pthread_create(&signal_thread, NULL, signalC, NULL);
	}

	else{
		pthread_create(&packet_arrival_thread,NULL,packet_arrival,NULL);
		pthread_create(&token_adding_thread,NULL,token_adder,NULL);
		pthread_create(&server_thread_one, NULL, server, (void *)1);
		pthread_create(&server_thread_two, NULL, server, (void *)2);
		pthread_create(&signal_thread, NULL, signalC, NULL);
	}

	pthread_join(packet_arrival_thread,NULL);
	pthread_join(token_adding_thread,NULL);
	pthread_join(server_thread_one, NULL);
	pthread_join(server_thread_two, NULL);

	gettimeofday(&eval_end, NULL);
	fprintf(stdout,"%012.3f: ",(double)(timeValToLongInt(eval_end) - timeValToLongInt(eval_start))/1000);
	printf("emulation ends\n");
	total_emulation_time = (double)(timeValToLongInt(eval_end) - timeValToLongInt(eval_start))/1000;
	if(file){
	fclose(fp);
	}
	//Print statistics
	//Print N/A where necessary
	printf("\nStatistics: \n");
	printf("\n\taverage packet inter-arrival time = %.8g", average_inter_arrival_time/1000000);
	if(TotalPacketsServed == 0){
		printf("\n\taverage packet service time = (N/A, no packet served)");
		
	}
	else{
		printf("\n\taverage packet service time = %.8g\n", average_service_time/1000000);
	}
	printf("\n\taverage number of packets in Q1 = %.8g", average_packets_in_Q1/total_emulation_time/1000);
	printf("\n\taverage number of packets in Q2 = %.8g", average_packets_in_Q2/total_emulation_time/1000);
	printf("\n\taverage number of packets in S1 = %.8g", average_packets_in_S1/total_emulation_time/1000);
	printf("\n\taverage number of packets in S2 = %.8g\n", average_packets_in_S2/total_emulation_time/1000);

	double variance = (average_sqared_time_spent_in_system)/(1000 * 1000) - (average_time_spent_in_system * average_time_spent_in_system)/ (1000 * 1000);
	double SD = sqrt(variance);

	if(TotalPacketsServed == 0)
	{
		printf("\n\taverage time a packet spent in system = (N/A, no packet served)");
		printf("\n\tstandard deviation for time spent in system = (N/A, no packet served)");
	}
	else{
		printf("\n\taverage time a packet spent in system = %.8g", average_time_spent_in_system/1000000);
		printf("\n\tstandard deviation for time spent in system = %.8g\n", SD/1000);
	}

	printf("\n\ttoken drop probability = %.8g", (double)TokensDropped/TotalTokens);
	printf("\n\tpacket drop probability = %.8g\n", (double)PacketsDropped/TotalPackets);

return 0;

}