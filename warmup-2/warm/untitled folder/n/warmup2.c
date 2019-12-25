#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <locale.h>
#include <pthread.h>
#include "cs402.h"
#include <signal.h>
#include <locale.h>
#include "my402list.h"

#define NUM_THREADS 5

/*Packet structure*/
typedef struct pack{
	 double lambda;
	 double rate;
	 long double timentsys;
	 long double timleasys;
	 long double timentQ1;
	 long double timleaQ1;
	 long double timentQ2;
	 long double timleaQ2;
	 long double timentS;
	 long double timleavS;
	 long double timinQ1;
	 long double timinQ2;
	 long double timinsys;
	 long double timinS;
	int pid;
	double mu;
	int npackarr;
	int numtokpack;
	int totaltokens;
}packet;

/*Global variables*/
FILE *fp;
char filename[100];
char commandline[100];
int tracedriven;
double emulationtime=0;
int tokencount =0;
My402List formQ1;
My402List formQ2;
int packtid=1;
double tokenrate;
int totaltoken;
double servicetimeS;
long double timeentq1;
long double timeentq2;
int q1=1;
//Assign the default values to the tfile values given in the variable

/*Global variables for command line*/
double globallambda = 1;
double globalmu = 0.35;
double globalrate = 1.5;
int globaltotaltokens = 10;
int  globalnpackarr = 20;
int globalnumtokpack =3;
int serverend;
int s1exit=0,s2exit=0;
/*Global variables for stats*/
long double averageInterarrival;
long double averagePacketService=0;
long double averageInQ1=0;
long double averageInQ2=0;
long double averageInS1=0;
long double averageInS2=0;
long double totalemualtiontime=0;
long double averageSysTime=0;
long double numdroppedpack = 0,numdroppedTok = 0;
long double averageSysTimeSqr;
double numpackservd = 0;
double numpackentrdQ1 = 0;

/*Global variable for control C*/
int cntC=0;
void setdefaultvalues(packet *pacptr)
{
	pacptr->lambda = 1;
	pacptr->rate = 1.5;
	pacptr->mu = 0.35;
	pacptr->npackarr= 20;
	pacptr->numtokpack= 3;
	pacptr->totaltokens= 10;
}
void Initialize(packet *pacptr)
{
	pacptr->timentsys = 0.0;
	pacptr->timleasys= 0.0;
	pacptr->timentQ1 = 0.0;
	pacptr->timleaQ1= 0.0;
	pacptr->timentQ2 = 0.0;
	pacptr->timleaQ2 = 0.0;
	pacptr->timentS = 0.0;
	pacptr->timleavS = 0.0;
	pacptr->timinQ1 = 0.0;
	pacptr->timinQ2 = 0.0;
	pacptr->timinsys = 0.0;
	pacptr->timinS = 0.0;
}
int id=0;
int tokenflag=0;
int packetexit=0;
/*global Pthread types */
pthread_t thread[NUM_THREADS];
pthread_mutex_t mutexappend;
pthread_cond_t cv;
sigset_t sig;



void *packetarrival(void *arg)
{
	
	
	struct timeval tv;
	
	long double interarrival=0;int tempflag=0;packtid =0;
	if (tracedriven == 1)
	{
		
		char buf[1024];
		memset(buf,0,sizeof(buf));
		fp = fopen(filename,"r");
		double prevtime=0;
		if (fp == NULL) 
		{
			perror("Error");
			exit(1);
		}
		
		packet *pacptr = NULL;
		fgets(buf,sizeof(buf),fp);
		sscanf(buf,"%d",&globalnpackarr);
		
		if(atoi(buf) == 0)
			{
				fprintf(stderr,"*****Aborted***** : input file is not in the right format\n");
				exit(1);
			}
		while(fgets(buf,sizeof(buf),fp)!= NULL)
		{	
			pacptr = (packet*)malloc(sizeof(packet));
			Initialize(pacptr);
			
			setdefaultvalues(pacptr);
			packtid++;
			sscanf(buf,"%lf\t%d\t%lf",&pacptr->lambda,&pacptr->numtokpack,&pacptr->mu);
			pacptr->mu =(1/(pacptr->mu/1000));
			pacptr->totaltokens = globaltotaltokens;
			pacptr->rate= globalrate;
			usleep((pacptr->lambda)*1000);
			gettimeofday(&tv,NULL);
			if (prevtime== 0)
			{
				
				pacptr->timentsys = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				interarrival = pacptr->timentsys;
				prevtime =  pacptr->timentsys;
			}
			else
			{
				
				pacptr->timentsys = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				interarrival = pacptr->timentsys-prevtime;
				prevtime=pacptr->timentsys;
				
			}
			averageInterarrival += interarrival;
			globalnumtokpack = pacptr->numtokpack;
			pacptr->pid = packtid;
			servicetimeS = (1/pacptr->mu)*1000;
			pthread_mutex_lock(&mutexappend);
					
			if(pacptr->numtokpack > pacptr->totaltokens)
			{
				fprintf(stdout,"%012.3Lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3Lfms, dropped\n", pacptr->timentsys,pacptr->pid,pacptr->numtokpack,interarrival);
				numdroppedpack++;
				pthread_mutex_unlock(&mutexappend);
				continue;
			}
			else
				fprintf(stdout,"%012.3Lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3Lfms\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,pacptr->pid,pacptr->numtokpack,interarrival);
			//Append to the list
			My402ListAppend(&formQ1,pacptr);
			q1=1;
			// Timestamp as it  Enters Q1
			gettimeofday(&tv,NULL);
			pacptr->timentQ1 =((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ;
			fprintf(stdout,"%012.3Lfms: p%d enters Q%d\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,pacptr->pid,q1);
			numpackentrdQ1++;
			if(pacptr->pid == globalnpackarr){
				pthread_mutex_unlock(&mutexappend);
				pthread_exit(NULL);
			}
			
			pthread_mutex_unlock(&mutexappend);
			
			/*Mutex Unlocked*/
		}
		fclose(fp);
	}
	
	else if (tracedriven == 0)
	{
		// All the code is of the length
		packet *pacptr = NULL;
		int count = 0;
		packtid=0;interarrival =(1/ globallambda)*1000;
		double prevtime=0;
		while(count < globalnpackarr)
		{
			pacptr = (packet*)malloc(sizeof(packet));
			Initialize(pacptr);
			count++;
			packtid++;
			//Copy the values of the file into variables sscanf(buf,"%lf\t%d\t%lf",&pacptr->lambda,&pacptr->numtokpack,&pacptr->mu);
			pacptr->lambda = globallambda;
			pacptr->numtokpack = globalnumtokpack;
			pacptr->mu = globalmu;
			averageInterarrival += interarrival;
			
			if (prevtime== 0){
				
				usleep(interarrival*1000);
				gettimeofday(&tv,NULL);
				pacptr->timentsys = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				interarrival = pacptr->timentsys;
				prevtime=pacptr->timentsys;
				
			}
			else
			{
				usleep(interarrival*1000);
				gettimeofday(&tv,NULL);
				pacptr->timentsys = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				interarrival = pacptr->timentsys - prevtime;
				prevtime=pacptr->timentsys;
			}
			globalnumtokpack = pacptr->numtokpack;
			pacptr->pid = packtid;
			pthread_mutex_lock(&mutexappend);
			if(pacptr->numtokpack > globaltotaltokens)
			{
				fprintf(stdout,"%012.3Lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3Lfms, dropped\n", pacptr->timentsys ,pacptr->pid,pacptr->numtokpack,interarrival);
				numdroppedpack++;
				tempflag =1;
				pthread_mutex_unlock(&mutexappend);
				continue;
				
			}
			else
				fprintf(stdout,"%012.3Lfms: p%d arrives, needs %d tokens, inter-arrival time = %.3Lfms\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,pacptr->pid,pacptr->numtokpack,interarrival);
			q1=1;
					
			//Append to the list
			My402ListAppend(&formQ1,(void*)pacptr);
			// Timestamp as it  Enters Q1
			gettimeofday(&tv,NULL);
			
			pacptr->timentQ1 =((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ;
			fprintf(stdout,"%012.3Lfms: p%d enters Q%d\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,packtid,q1);
			numpackentrdQ1++;
			
			if(pacptr->pid == globalnpackarr){
				pthread_mutex_unlock(&mutexappend);
				pthread_exit(NULL);
			}
			
			pthread_mutex_unlock(&mutexappend);
			

		
		}
		if(tempflag ==1)
			{
				pthread_mutex_lock(&mutexappend);
				serverend = globalnpackarr;
				packetexit =1;
				numdroppedpack = globalnpackarr;
				pthread_cond_broadcast(&cv);
				pthread_mutex_unlock(&mutexappend);
			}
				
	}
	packetexit =1;
	pthread_exit(NULL);
	
}

void *tokengenerator(void *arg)
{
	// It would generate tokens at the rate of R and then it would check if the token is enough for the current packet
	//packet *access= NULL;
	
	struct timeval tv;
	tokencount = 0;
	My402ListElem *elem1 = NULL;
	packet *packint = NULL;long double prevtime=0;long double tokeninterarrival =(1/globalrate)*1000; long double current_time=0.0;
	packint =(packet*) malloc(sizeof(packet));
	
	while (1)
	{
		
		
		
		
		if (prevtime== 0){
				
				usleep(tokeninterarrival*1000);
				gettimeofday(&tv,NULL);
				current_time = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				tokeninterarrival = current_time;
				prevtime=current_time;
				
			}
			else
			{
				usleep(tokeninterarrival*1000);
				gettimeofday(&tv,NULL);
				current_time = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				tokeninterarrival = current_time-prevtime;
				
				prevtime=((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
			}
		
		if(packetexit == 1 && My402ListLength(&formQ1) ==0)
		{
			tokenflag = 1;
			pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&mutexappend);
			break;
		}
		
		
		
		
		pthread_mutex_lock(&mutexappend);
		tokencount++;
		if(s1exit ==1 && s2exit ==1){
			pthread_mutex_unlock(&mutexappend);
				pthread_exit(NULL);}
		gettimeofday(&tv,NULL);
		id++;
		if(tokencount <= globaltotaltokens)
			fprintf(stdout,"%012.3Lfms: token t%d arrives, token bucket now has %d token\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,id,tokencount);
		else if(tokencount > globaltotaltokens){
			fprintf(stdout,"%012.3Lfms: token t%d arrives, dropped\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,id);
			numdroppedTok++;
			tokencount = globaltotaltokens;
		}
		
		
			
		struct timeval tv;
		elem1 = My402ListFirst(&formQ1);
		if (My402ListLength(&formQ1) !=0)
			memcpy(packint,elem1->obj,sizeof(packet));
		/*else {
			pthread_mutex_unlock(&mutexappend);
			break;
			}*/
		if(packint->pid > globalnpackarr /*packint->npackarr*/)
			{
				pthread_mutex_unlock(&mutexappend);
				break;
			}
		if (My402ListEmpty(&formQ1) == 1)
		{
			pthread_mutex_unlock(&mutexappend);
			continue;
		}
		else
		{
			if(tokencount < packint->numtokpack)
			{
				pthread_mutex_unlock(&mutexappend);
				continue;
			}
			q1=1;
			tokencount = tokencount - packint->numtokpack;
			My402ListUnlink(&formQ1,elem1);
			// Timestamp as it leaves Q1
			gettimeofday(&tv,NULL);
			packint->timleaQ1 = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
			packint->timinQ1 = packint->timleaQ1 - packint->timentQ1;
			fprintf(stdout,"%012.3Lfms: p%d leaves Q%d, time in Q%d = %.3Lfms, token bucket now has %d token\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,packint->pid,q1,q1,packint->timinQ1,tokencount);
			//Form Q2
			averageInQ1 = averageInQ1 + packint->timinQ1;
			
			q1= 2;
			My402ListAppend(&formQ2,packint);
			gettimeofday(&tv,NULL);
			timeentq2 = ((double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
			fprintf(stdout,"%012.3Lfms: p%d enters Q%d\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,packint->pid,q1);
			
			
			if(My402ListEmpty(&formQ2)==0)
			{
				gettimeofday(&tv,NULL);
				long double timeleaveq2 = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
				long double timeinq2= timeleaveq2 - timeentq2;
				fprintf(stdout,"%012.3Lfms: p%d leaves Q%d, time in Q%d = %.3Lfms, token bucket now has %d token\n", ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000 ,packint->pid,q1,q1,timeinq2,tokencount);
				averageInQ2 = averageInQ2 + timeinq2;
				serverend = packint->pid;
				pthread_cond_broadcast(&cv);
				
			}
			
			
			pthread_mutex_unlock(&mutexappend);
		}
		
	}
	pthread_mutex_lock(&mutexappend);
	tokenflag = 1;
	pthread_cond_broadcast(&cv);
	pthread_mutex_unlock(&mutexappend);
	
	pthread_exit(NULL);
}

void *server1(void* arg)
{
	// Functionalities that the server1 has to perform
	//It has to serve the packets present in the Q1
	// If Q is empty it has to sleep
	//If Q is not empty it has to wake up to serve the packet
	
	// Need to access the Q2
	My402ListElem *elem = NULL;
	packet *p = NULL;
	p = (packet*) malloc(sizeof(packet));
	int s1=1;struct timeval tv;
	while(1)
	{
		
		pthread_mutex_lock(&mutexappend);
		while (My402ListEmpty(&formQ2) == 1 ){
			if(tokenflag == 1)
			{
				pthread_mutex_unlock(&mutexappend);
				pthread_exit(NULL);
			}
			if(cntC ==1)
			{
				pthread_mutex_unlock(&mutexappend);
				s1exit = 1;
				pthread_exit(NULL);
			}
			if(serverend == globalnpackarr){
				pthread_mutex_unlock(&mutexappend);
				s1exit = 1;
				pthread_exit(NULL);
				}
			pthread_cond_wait(&cv,&mutexappend);}
		if(cntC ==1)
		{
			pthread_mutex_unlock(&mutexappend);
			s1exit = 1;
			pthread_exit(NULL);
		}
		elem = My402ListFirst(&formQ2);
		memcpy(p,elem->obj,sizeof(packet));
		gettimeofday(&tv,NULL);
		p->timentS = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
		fprintf(stdout,"%012.3Lfms: p%d begins service at S%d, requesting %.3lfms of service\n",((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000,p->pid,s1,((1/p->mu)*1000));
		numpackservd++;
		My402ListUnlink(&formQ2,My402ListFirst(&formQ2));
		pthread_mutex_unlock(&mutexappend);
		usleep((1/p->mu)*1000000);
		pthread_mutex_lock(&mutexappend);
		
		gettimeofday(&tv,NULL);
		p->timleavS = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
		p->timinS = p->timleavS - p->timentS;
		p->timinsys = p->timleavS -p->timentsys;
		fprintf(stdout,"%012.3Lfms: p%d departs from S%d, service time = %.3Lfms, time in system = %Lfms\n",((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000,p->pid,s1,p->timinS,p->timinsys);
		averageInS1 = averageInS1 + p->timinS;
		averageSysTime += p->timinsys;
		averageSysTimeSqr += p->timinsys*p->timinsys;
		if(p->pid == globalnpackarr)
		{
			pthread_mutex_unlock(&mutexappend);
			s1exit = 1;
			pthread_exit(NULL);
		}
		if(tokenflag ==1 &&My402ListLength(&formQ2) == 0)
		{
			pthread_mutex_unlock(&mutexappend);
			break;
		}
		pthread_mutex_unlock(&mutexappend);
	}
	if(cntC ==1)
	{
		pthread_mutex_unlock(&mutexappend);
		s1exit = 1;
		pthread_exit(NULL);
	}
	s1exit = 1;
	pthread_mutex_lock(&mutexappend);
	pthread_cond_broadcast(&cv);
	pthread_mutex_unlock(&mutexappend);
	pthread_exit(NULL);
	
}

void *server2(void* arg)
{
	// Functionalities that the server2 has to perform
	//It has to serve the packets present in the Q1
	// If Q is empty it has to sleep
	//If Q is not empty it has to wake up to serve the packet
	packet *p = NULL;
	p = (packet*) malloc(sizeof(packet));
	My402ListElem *elem = NULL;
	int s1=2;struct timeval tv;
	while(1)
	{
		
		pthread_mutex_lock(&mutexappend);
		while (My402ListEmpty(&formQ2) == 1){
			if(tokenflag == 1)
			{
				pthread_mutex_unlock(&mutexappend);
				pthread_exit(NULL);
			}
			if(cntC ==1)
			{
				pthread_mutex_unlock(&mutexappend);
				s2exit=1;
				pthread_exit(NULL);
			}
			if(serverend == globalnpackarr){
				pthread_mutex_unlock(&mutexappend);
				s2exit=1;
				pthread_exit(NULL);
				}
			pthread_cond_wait(&cv,&mutexappend);}
		if(cntC ==1)
		{
			pthread_mutex_unlock(&mutexappend);
			s2exit=1;
			pthread_exit(NULL);
		}
		elem = My402ListFirst(&formQ2);
		memcpy(p,elem->obj,sizeof(packet));
		gettimeofday(&tv,NULL);
		p->timentS = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
		fprintf(stdout,"%012.3Lfms: p%d begins service at S%d, requesting %.3lfms of service\n",((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000,p->pid,s1,(1/p->mu)*1000);
		numpackservd++;
		My402ListUnlink(&formQ2,elem);
		pthread_mutex_unlock(&mutexappend);
		usleep((1/p->mu)*1000000);
		pthread_mutex_lock(&mutexappend);
		gettimeofday(&tv,NULL);
		p->timleavS = ((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
		p->timinS = p->timleavS - p->timentS;
		p->timinsys = p->timleavS -p->timentsys;
		fprintf(stdout,"%012.3Lfms: p%d departs from S%d, service time = %.3Lfms, time in system = %.3Lfms\n",((long double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000,p->pid,s1,p->timinS,p->timinsys);
		averageInS2 = averageInS2 + p->timinS;
		averageSysTime += p->timinsys;
		averageSysTimeSqr += p->timinsys*p->timinsys;
		if(p->pid == globalnpackarr)
		{
			pthread_mutex_unlock(&mutexappend);
			s2exit=1;
			pthread_exit(NULL);
		}
		
		if(tokenflag ==1 &&My402ListLength(&formQ2) == 0)
		{
			pthread_mutex_unlock(&mutexappend);
			break;
		}
		if(cntC ==1)
		{
			pthread_mutex_unlock(&mutexappend);
			s2exit=1;
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&mutexappend);
	}
	s2exit=1;
	pthread_mutex_lock(&mutexappend);
	pthread_cond_broadcast(&cv);
	pthread_mutex_unlock(&mutexappend);
	pthread_exit(NULL);
}
void *controlC()
{
	int set;
	
	My402ListElem  *elem = NULL;
	My402ListElem  *elem1 = NULL;
	sigwait(&sig,&set);
	cntC =1;
	pthread_mutex_lock(&mutexappend);
	pthread_cancel(thread[0]);
	pthread_cancel(thread[1]);
	pthread_cond_broadcast(&cv);
	for(elem = My402ListFirst(&formQ1); elem != NULL ; elem= My402ListNext(&formQ1,elem))
	{
		if(My402ListEmpty(&formQ1) == 1)
			break;
		fprintf(stdout, "\nP%d removed from Q1",((packet*)(elem->obj))->pid);
		My402ListUnlink(&formQ1, elem);
		
	}
	for(elem1 = My402ListFirst(&formQ2); elem1 != NULL ; elem1= My402ListNext(&formQ2,elem1))
	{
		if(My402ListEmpty(&formQ2) == 1)
			break;
		fprintf(stdout, "\nP%d removed from Q2",((packet*)(elem1->obj))->pid);
		My402ListUnlink(&formQ2, elem1);
		
	}
	fprintf(stdout, "\n");
	pthread_mutex_unlock(&mutexappend);
	
	
	pthread_exit(NULL);
}
	
	
	

void calculateStats()
{
	double newnumpackets=0;
	double localnpackarr=0;
	localnpackarr = globalnpackarr;
	newnumpackets = globalnpackarr - numdroppedpack;
	
	if(newnumpackets ==0)
	{
		newnumpackets = globalnpackarr;
	}
	if(numpackentrdQ1 ==0)
	{
		numpackentrdQ1=1;
	}
	if(numpackservd ==0)
	{
		numpackservd=1;
	}
	if(cntC !=1)
	{	
		averagePacketService = averageInS1 + averageInS2;
		fprintf(stdout,"Statistics:\n\n");
		fprintf(stdout,"\taverage packet inter-arrival time = %.6gs\n",(double)averageInterarrival/(1000*(localnpackarr)));
		fprintf(stdout,"\taverage packet service time = %.6gs\n\n",(double)(averagePacketService/(1000*(newnumpackets))));
		fprintf(stdout,"\taverage number of packets in Q1 = %.6g\n",(double)(averageInQ1/totalemualtiontime));
		fprintf(stdout,"\taverage number of packets in Q2 = %.6g\n",(double)(averageInQ2/totalemualtiontime));
		fprintf(stdout,"\taverage number of packets at S1 = %.6g\n",(double)(averageInS1/totalemualtiontime));
		fprintf(stdout,"\taverage number of packets at S2 = %.6g\n\n",(double)(averageInS2/totalemualtiontime));
		fprintf(stdout,"\taverage time a packet spent in system = %.6gs\n",(double)(averageSysTime/(1000*(newnumpackets))));
		
		long double sd=0.0,var=0.0;
		var = averageSysTimeSqr/(1000000*(newnumpackets)) - averageSysTime/(1000*(newnumpackets)) * averageSysTime/(1000*(newnumpackets));
		if(var <0)
			var= var*-1;
		sd=sqrt(var);
		double probpack;
		double probtok;
		probpack = numdroppedpack/localnpackarr;
		probtok = numdroppedTok/id;
		fprintf(stdout,"\tstandard deviation for time spent in system = %.6gs\n\n",(double)sd);
		fprintf(stdout,"\ttoken drop probability = %.6g\n",probtok);
		fprintf(stdout,"\tpacket drop probability = %.6g\n",probpack);
	}
	else
	{
		averagePacketService = averageInS1 + averageInS2;
		fprintf(stdout,"Statistics:\n\n");
		fprintf(stdout,"\taverage packet inter-arrival time = %.6gs\n",(double)averageInterarrival/(1000*(numpackentrdQ1)));
		fprintf(stdout,"\taverage packet service time = %.6gs\n\n",(double)(averagePacketService/(1000*(numpackservd))));
		fprintf(stdout,"\taverage number of packets in Q1 = %.6g\n",(double)(averageInQ1/totalemualtiontime));
		fprintf(stdout,"\taverage number of packets in Q2 = %.6g\n",(double)(averageInQ2/totalemualtiontime));
		fprintf(stdout,"\taverage number of packets at S1 = %.6g\n",(double)(averageInS1/totalemualtiontime));
		fprintf(stdout,"\taverage number of packets at S2 = %.6g\n\n",(double)(averageInS2/totalemualtiontime));
		fprintf(stdout,"\taverage time a packet spent in system = %.6gs\n",(double)(averageSysTime/(1000*(numpackservd))));
		
		long double sd=0.0,var=0.0;
		var = averageSysTimeSqr/(1000000*(numpackservd)) - averageSysTime/(1000*(numpackservd)) * averageSysTime/(1000*(numpackservd));
		if(var <0)
			var= var*-1;
		sd=sqrt(var);
		double probpack;
		double probtok;
		probpack = numdroppedpack/localnpackarr;
		probtok = numdroppedTok/id;
		fprintf(stdout,"\tstandard deviation for time spent in system = %.6gs\n\n",(double)sd);
		fprintf(stdout,"\ttoken drop probability = %.6g\n",probtok);
		fprintf(stdout,"\tpacket drop probability = %.6g\n",probpack);
		
	}


}
/*The main thread for execution of the statement */
int main (int argc, char *argv[])
{
	
	if (argc < 2 && argc >13)
	{
		fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");
		return -1;
	}
	FILE *fpmain;
	sigemptyset(&sig);
	struct stat sb;
	sigaddset(&sig, SIGINT);
	pthread_sigmask(SIG_BLOCK, &sig,NULL);
	packet* pacptr = malloc(sizeof(packet));
	setdefaultvalues(pacptr);
	void *join;
	
	//fp = fopen(argv[2], "r");
	/*
	*/
	sprintf(commandline,"%s",*argv);
	
	memset(&formQ1,0,sizeof(My402List));
	My402ListInit(&formQ1);
	
    	
	/*This is the list for queue Q2*/
	memset(&formQ2,0,sizeof(My402List));
	My402ListInit(&formQ2);
	
	
	if (argc > 1 )
	{
		tracedriven = 0;	
		int i;
		for (i=1;argv[i] != NULL;i=i+2)
		{
			if(strcmp(argv[i],"-lambda") == 0)
			{
				if(argc >i+1)
				{
					if(argv[i+1][0] != '-')
					{
						sscanf(argv[i+1], "%lf", &globallambda);
						if ((1/globallambda) > 10)
						globallambda = 0.1;
					}
					else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				}
				else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
			}
			else if(strcmp(argv[i],"-mu") == 0)
			{
				if(argc >i+1)
				{
					if(argv[i+1][0] != '-')
					{
						sscanf(argv[i+1], "%lf", &globalmu);
						if ((1/globalmu) >10)
							globalmu =0.1;
					}
					else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				}
				else
				{
					fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
					return -1;
				}
					
			}	
			else if(strcmp(argv[i],"-r") == 0)
			{
				if(argc >i+1)
				{
					if(argv[i+1][0] != '-')
					{
						sscanf(argv[i+1], "%lf", &globalrate);
						if ((1/globalrate) >10)
							globalrate = 0.1;
					}
					else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				}
				else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				
			}	
			else if(strcmp(argv[i],"-B") == 0)
			{
				if(argc >i+1)
				{
					if(argv[i+1][0] != '-')
					{	
						globaltotaltokens = atoi(argv[i+1]);
					}
					else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				}
				else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
			}	
			else if(strcmp(argv[i],"-n") == 0)
			{
				if(argc >i+1)
				{
					if(argv[i+1][0] != '-')
					{
						globalnpackarr = atoi(argv[i+1]);
					}
					else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				}
				else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
			}
			else if(strcmp(argv[i],"-P") == 0)
			{
				if(argc >i+1)
				{
					if(argv[i+1][0] != '-')
					{
						globalnumtokpack = atoi(argv[i+1]);
					}
					else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
				}
				else
					{
						fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
						return -1;
					}
			}
			
			else if(strcmp(argv[i],"-t") == 0)
			{
				tracedriven =1;
				if(argv[i+1] ==NULL)
				{
					fprintf(stderr, "File Name not specified\n");
					exit(0);
				}
				sprintf(filename,"%s",argv[i+1]);
				
				fpmain = fopen(filename,"r");
				if (fpmain == NULL) 
				{
					perror("Error");
					exit(1);
				}
				sprintf(filename,"%s",argv[i+1]);
				if (stat(filename, &sb) == 0 && S_ISDIR(sb.st_mode))
				{
					fprintf(stderr,"Error: This is a directory \n");
					
					exit(1);
				}
				fclose(fpmain);
				
								
			}
			else
			{
				fprintf(stderr, "Malformed command!!.. Usage --> warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n");
				return -1;
			}	
		}
		
	}
	
	
	if(tracedriven == 1)
	{
		fprintf(stdout,"Emulation Parameters:\n");
		fprintf(stdout,"P = %d\n",globalnpackarr);
		fprintf(stdout,"r = %.2lf\n",globalrate);
		fprintf(stdout,"B = %d\n",globaltotaltokens);
		fprintf(stdout,"tsfile = %s\n\n",filename);
		
	}
	else if(tracedriven == 0)
	{
		fprintf(stdout,"Emulation Parameters:\n");
		fprintf(stdout,"lambda = %.2lf\n",globallambda);
		fprintf(stdout,"mu = %.2lf\n",globalmu);
		fprintf(stdout,"r = %.2lf\n",globalrate);
		fprintf(stdout,"B = %d\n",globaltotaltokens);
		fprintf(stdout,"P = %d\n\n",globalnpackarr);
	}
	struct timeval tv;
	gettimeofday(&tv,NULL);
	emulationtime = (double)tv.tv_sec*1000000+tv.tv_usec;
	//tokenrate = pacptr->rate;
	//totaltoken = pacptr->totaltokens;
	//Token Generator thread
	pthread_create(&thread[0],0,tokengenerator,0);
	//Packet arrival thread  
	pthread_create(&thread[1],0,packetarrival,0);
	//Server1 Thread
	pthread_create(&thread[2],0,server1,0);
	
	//Server2 thread
	pthread_create(&thread[3],0,server2,0);
	//Monitor Thread
	pthread_create(&thread[4],0,controlC,0);
	
	fprintf(stdout,"00000000.000ms: Emulation Begins\n");
		
	
	// Token Generation thread is created
	
	//pthread_create(&thread[1],0,tokengenerator,(void *)1);
	//Initialize the variables of the packet
	pthread_join(thread[0],&join);
	pthread_join(thread[1],&join);
	pthread_join(thread[2],&join);
	pthread_join(thread[3],&join);
	//pthread_join(thread[4],&join);
	//Create the threads for executing the sequence
	gettimeofday(&tv,NULL);
	totalemualtiontime = ((double)tv.tv_sec*1000000+tv.tv_usec - emulationtime)/1000;
	fprintf(stdout,"%012.3Lfms: Emulation ends\n",totalemualtiontime);
	calculateStats();
	/*Create the thread-1 packet arrival thread*/
	
	// Append it to the list
	
	// Create the structure
	//printf("Average Time in system %.6g ,,,,, %lf,,,,,,,%lf\n",,averageInQ1,totalemualtiontime);

	return 0;

}