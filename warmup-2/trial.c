#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <sys/time.h>


pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Queue_Not_Empty_Cond = PTHREAD_COND_INITIALIZER;
int Queue_Amount_Guard = 0;

long int timeDifference(struct timeval time1, struct timeval time2){
	long int diff = 1000000*(time1.tv_sec - time2.tv_sec) + (time1.tv_usec - time2.tv_usec);
	return diff;
}

void *cal1()
{
	int i = 0;
	while(i < 5){
		pthread_mutex_lock(&Mutex);
		printf("HI\n");
		pthread_mutex_unlock(&Mutex);
		i++;
	}
	struct timeval currTime;
	struct timeval currTimeTwo;

	gettimeofday(&currTime, 0);
	//printf("Time is %ld", currTime.tv_sec);
	//printf("Time is %ld", currTime.tv_usec);
	usleep(50000);
	gettimeofday(&currTimeTwo, 0);
	//printf("Time is %ld", currTimeTwo.tv_sec);
	//printf("Time is %ld", currTimeTwo.tv_usec);
	long int diff = timeDifference(currTimeTwo, currTime);
	printf("Time difference is: %ld", diff );
	pthread_mutex_lock(&Mutex);
	while(!(Queue_Amount_Guard > 0)){
		pthread_cond_wait(&Queue_Not_Empty_Cond, &Mutex);
		printf("Condition met\n");
		break;
	}
	pthread_mutex_unlock(&Mutex);

	return NULL;
}

void *cal2()
{
	int i = 0;
	while(i < 10){
		pthread_mutex_lock(&Mutex);
		printf("Howdy\n");
		//pthread_cond_broadcast(&Queue_Not_Empty_Cond);
		//printf("Broadcast sent\n");
		pthread_mutex_unlock(&Mutex);
		i++;
	}
	Queue_Amount_Guard++;
	pthread_mutex_lock(&Mutex);
	pthread_cond_broadcast(&Queue_Not_Empty_Cond);
	printf("Broadcast sent oustide\n");
	pthread_mutex_unlock(&Mutex);
	return NULL;

}


int main(int argc,char *argv[])
{

pthread_t thread1,thread2;

int ret1,ret2;
ret1=pthread_create(&thread1,NULL,cal1,NULL);
usleep(5000);
ret2=pthread_create(&thread2,NULL,cal2,NULL);

printf("This is\n");

pthread_join(thread1,NULL);
pthread_join(thread2,NULL);

printf("ret1=%d\n",ret1);
printf("ret2=%d\n",ret2);

return 0;

}