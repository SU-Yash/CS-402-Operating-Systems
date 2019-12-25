#include<pthread.h>
#include <signal.h>
#include <sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

int x = 0;
sigset_t set;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void long_running_process(){
	while(1){
	pthread_mutex_lock(&m);
	printf("Waiting");
	usleep(100);
	pthread_mutex_unlock(&m);
	}
}

void *monitor(void *arg){
	int sig = 0;
	while(1){
		sigwait(&set, &sig);
		pthread_mutex_lock(&m);
		printf("%d\n", sig);
		pthread_exit(NULL);
		pthread_mutex_unlock(&m);
	}
}

int main(){
	pthread_t thr;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigprocmask(SIG_BLOCK, &set, 0);

	pthread_create(&thr, 0, monitor, NULL);
	pthread_join(thr, NULL);
	//long_running_process();
	return 0;
}
