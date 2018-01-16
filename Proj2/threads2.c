//-----------------------
//
//	EC440 Operating Systems
//	Project 3
//	Jenna Zhu	
//
//-----------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <semaphore.h>

//register 4
#define SP_REG 4 
//register 5
#define PC_REG 5 

//amount of time allowed for a thread (50ms)
#define THREADTIMER 50000
//byte size of each thread stack
#define STACKSIZE 32787
//total number of threads that can be created
#define THREADCOUNT 128
//total number of semaphores that can be created
	//THREADCOUNT * 2
#define SEMAPHORECOUNT 256

#define EXITED 0
#define EMPTY 1
#define READY 2
#define RUNNING 3
#define UNUSED 4
#define BLOCKED 5


//use as flag to signify if first call has been made
int first = 0;
//indexing for the total number of threads
int threadindex = 0;
//number of threads running
int runthreads = 0;
//the next thread
int next = 0;
//current thread
int current = 0;
//semaphore indexing
int semindex = 1;
//the next semaphore 
int nextsem =  0;

int ptr_mangle(int p)
{
        unsigned int ret;
        asm(" movl %1, %%eax;\n"
        " xorl %%gs:0x18, %%eax;\n"
        " roll $0x9, %%eax;\n"
        " movl %%eax, %0;\n"
        : "=r"(ret)
        : "r"(p)
        : "%eax"
        );
        return ret;
}

//TCB
struct threading{
	int status;
	void *stack;
	pthread_t threadID;
	jmp_buf tstate;
	void *status_exit;
	pthread_t threadjoin[THREADCOUNT];
};

//create your own semaphore structure that stores:
 	//the current value, a pointer to a queue for threats that are waiting, 
	//and a flag that indicates whether the semaphore is initialized
struct semaphore{
	sem_t *sem_ptr;
	//the queue for a semaphore
	int semqueue[SEMAPHORECOUNT];
	int semval;
	int semID;
	int semcount;
	//this is used as a marker for the beginning/head of the semaphore's queue
	int sembegin;
	//this is used as a marker for the end/tail of the semaphore's queue
	int semend;
	//to see if this current semaphore has been used or not
	// 0 - has been used
	// 1 - good to go!
	int sem_initflag;
};

//make array to hold threads with maximum thread amount
static struct threading threads[THREADCOUNT];
void schedule();

static struct semaphore semaphores[50];

//return the thread ID of the calling thread
pthread_t pthread_self(){
	return threads[current].threadID;
}

//-------Proj 3---------//
void lock(){
//thread cannot be interrupted
//calling lock twice without invoking unlock is UNDEF
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGALRM);
	sigprocmask(SIG_BLOCK, &signal_set, NULL);
}

void unlock(){
//thread resumes normal state and scheduled when SIGALARM is received
//thread only calls unlock when previously locked
	sigset_t signal_set;
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}

int pthread_join(pthread_t thread, void **value_ptr){
//postpones exec of thread that called until target thread terminates
//need to correctly handle exit code of threads that terminate

	lock();

	int index = 0;

	while(threads[index].threadID != thread && index < THREADCOUNT){

		index++;
	
	}

	if(threads[index].status != EXITED){

		threads[current].status = BLOCKED;

		int threadindex = 0;

		for(threadindex = 0; threadindex < THREADCOUNT; threadindex++){

			if(threads[threadindex].threadID == thread){

				int joinindex = 0;

				for(joinindex = 0; joinindex < THREADCOUNT; joinindex++){

					if(threads[threadindex].threadjoin[joinindex] == 0){

						threads[threadindex].threadjoin[joinindex] = threads[current].threadID;
						break;

					}
					
				}

			}

		}
	}

	schedule();
	
	if(value_ptr != NULL){
	//on successful pthread_join call return with a non-NULL value_ptr argv, 
	//value passed to pthread_exit by terminating thread will be made 
	//available in location referenced by value_ptr

		*value_ptr = threads[thread].status_exit;	
	}

	//if thread is blocked, cannot be selected by scheduler
	unlock();
	return 0;
}

int sem_init (sem_t *sem, int pshared, unsigned value){
//pshared always equals 0 - semaphore pointed to by *sem is shared between threads of a process
	lock();

	int initindex;

	//while your index is less than max semophore count 128
	//and while you have not reached a semaphore that has not been initialized before
	while(initindex < SEMAPHORECOUNT && semaphores[initindex].sem_initflag != 1){

		initindex++;

	}

	//saves the reference to this semaphore to __align for other semaphore functions
	sem->__align = initindex;
	//changes flag to show that semaphore has already been initialized
	semaphores[initindex].sem_initflag = 0;
	//creates a head "pointer" for queue
	semaphores[initindex].sembegin = 0;
	//creates a tail "pointer" for queue
	semaphores[initindex].semend = 0;
	semaphores[initindex].semval = value;

	unlock();
	return 0;
}

int sem_wait(sem_t *sem){
	//adding blocked threads to semaphore queue
//decrements (locks) semaphore pointed to by *sem
	lock();

	int waitindex = sem->__align;

	//loop through your semaphore array
//	while(index < semindex){


		//sembegin = semaphores[index].queue[0];
		//semend = semaphores[index].queue[0];

		//if semaphore value > 0, proceed to decrement and returns
		if(semaphores[waitindex].semval > 0){

			semaphores[waitindex].semval--;
			unlock();

		}

		//if semaphore value = 0, block until possible to decrement (sval > 0)

				

		//if there are no blocked threads in this semaphore's queue/semaphore is not being used
		if(semaphores[waitindex].semqueue[semaphores[waitindex].semend] == 0){

					threads[current].status = BLOCKED;

					//add this current blocked thread's ID into the queue
					semaphores[waitindex].semqueue[semaphores[waitindex].sembegin] = threads[current].threadID;
					//semend += 4;
					//increment count of blocked threads in semaphore queue
					semaphores[waitindex].semcount++;
					//move the pointer of the end of the queue "forward"
					semaphores[waitindex].semend++;
				}

				else{

					unlock();
					return -1;
				}

			

//			index++;
//		}

//semaphore value will never be below zero
	unlock();
	return 0;

}

int sem_post(sem_t *sem){
//increments (unlocks) semaphore pointed to by *sem
//if semaphore value > 0, another thread blocked in sem_wait will be "woken up" and is locked
	lock();

	int postindex = sem->__align;

	//while(index < semindex){

		if(semaphores[postindex].semcount != 0 && semaphores[postindex].sem_ptr == sem){

			semaphores[postindex].semval++;

			if(semaphores[postindex].semqueue[semaphores[postindex].sembegin]){

			}
		}

	//	index++;
	//}

//when thread is woken up and takes lock, value of semaphore = 0
	unlock();
	return 0;
}

int sem_destroy(sem_t *sem){
//using a destroyed semaphore is UNDEF til semaphore is reinitialized using sem_init
//destroying a semaphore that other threads are blocked on (in sem_wait) is UNDEF

	lock();

//	int findsema = 0;

//	while(findsema < semindex){
		//find the semaphore


		int findsema = sem->__align;

		if(semaphores[findsema].sem_ptr == sem && semaphores[findsema].sem_initflag == 0){

			semaphores[findsema].semcount = 0;
			unlock();
			return 0;
		}
		else{

			unlock();
			return -1;
		}

//		findsema++;
//	}

return 0;
}


//---------------------//

void next_run(){

	//printf("next thread reached \n");
	int count = 1;
	int threadcounter = 0;

	while(threadcounter < THREADCOUNT){
		threadcounter++;
		current++;


		if(current == 128){
			//printf("Round Robin %d \n", count);
			count++;
			current = 0;
		}

		//find next READY thread
		if(threads[current].status == READY){
			return;
		}
	}
	return;
}

void schedule()
{
	//printf("schedule called \n");
	if (setjmp(threads[current].tstate) == 0)
	{
		//printf("schedule called\n");
		lock();
		next_run();
		longjmp(threads[current].tstate, 1);
		unlock();
	}
	else

	{
		unlock();
		return;
	}
}

void alarm_handler(int signo)
{
	schedule();
}

//alarm signals/interrupt 
void alarm_set()
{
	struct sigaction sigact;
	sigact.sa_handler = alarm_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_NODEFER;
	sigaction(SIGALRM, &sigact, NULL);

	if (ualarm(THREADTIMER, THREADTIMER) < 0)
	{
		perror("schedule"); 
	}
}

//terminates calling thread
//ignore value passed in as first argument (value_ptr) and clean up all info of terminated thread
//exit with exit status of 0 after last thread has been terminated
//as if called exit() with a zero argument at thread termination time
void pthread_exit(void *value_ptr){

	lock();

	int index = 0;
	int joinedindex = 0;

	//clear up resources
	threads[current].status = EXITED;
	threads[current].status_exit = value_ptr;

	/*int threadcheck = threads[current].checkjoined;

	if( (threadcheck != -1) && (threads[threadcheck].status == BLOCKED) ){

		threads[threadcheck].status = threads[threadcheck].prevstatus;
		//threadindex--; 

	}*/

	for(joinedindex = 0; joinedindex < THREADCOUNT; joinedindex++){

		index = threads[current].threadjoin[joinedindex];
		threads[index].status = READY;

	}

		threadindex--;

	unlock();
	schedule();
	__builtin_unreachable();
}

void pthread_exit_wrapper(){
	unsigned int res;
	asm("movl %%eax, %0\n":"=r"(res));
	pthread_exit((void *) res);
} 

//int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
//creates new thread within a process
//on success, store ID of created thread in location reference by /thread/
//attr is always NULL
//thread created, execute start_routine with /arg/ as sole argument
//if start_routine returns, effect like implicit call to pthread_exit() using return of s_r as exit status
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){

	lock();

	//check to see if this is the first time pthread_create was called
	if(first == 0){
		//printf("reached first in pthread_create \n");
		threads[next].threadID = threadindex;
		threads[next].status = READY;
		//threads[next].stack = malloc(STACKSIZE * sizeof(void*));
		setjmp(threads[next].tstate);
		//index to move to next thread
		threadindex++;
		//start the alarm/timer
		alarm_set();
		first = 1;
	}

	//moving on to next threads until 128
	if(next < 128)
	{
		//printf("made next stack and traversal \n");
		next++;
		threads[next].threadID = threadindex;
		*thread = threads[next].threadID;
		threads[next].status = READY;
		threads[next].stack = malloc(STACKSIZE * sizeof(void*));
		threads[next].status_exit = NULL;

		//move to the top of the stack!!!!!!!! wheeeeeeeeeee
		void *trav = threads[next].stack + STACKSIZE;
		//move down the stack
		trav -= 4;
		*((unsigned long int*)trav) = (unsigned long int)arg;

		trav -= 4;
		*((unsigned long int*)trav) = (unsigned long int)pthread_exit_wrapper;

		setjmp(threads[next].tstate);

		//store SP and PC register values into jmpbuf
		threads[next].tstate[0].__jmpbuf[SP_REG] = ptr_mangle((unsigned long int)trav);
		threads[next].tstate[0].__jmpbuf[PC_REG] = ptr_mangle((unsigned long int)start_routine);
		threadindex++;

		unlock();
		schedule();
		return 0;
}
	//if error return -1
	else{

		unlock();
		return -1;
}

}