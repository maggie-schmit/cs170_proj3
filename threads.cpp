/*
 * CS170 - Operating Systems
 * Project 2 Solution
 * Author: me myself and i
 */


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <queue>
#include <semaphore.h>
#include <unordered_map>
#include <map>
/*
 * these could go in a .h file but i'm lazy
 * see comments before functions for detail
 */
void signal_handler(int signo);
void the_nowhere_zone(void);
static int ptr_mangle(int p);
void pthread_exit_wrapper();
void lock();
void unlock();

/*
 *Timer globals
 */
static struct timeval tv1,tv2;
static struct itimerval interval_timer = {0}, current_timer = {0}, zero_timer = {0};
static struct sigaction act;



/*
 * Timer macros for more precise time control
 */

#define PAUSE_TIMER setitimer(ITIMER_REAL,&zero_timer,&current_timer)
#define RESUME_TIMER setitimer(ITIMER_REAL,&current_timer,NULL)
#define START_TIMER current_timer = interval_timer; setitimer(ITIMER_REAL,&current_timer,NULL)
#define STOP_TIMER setitimer(ITIMER_REAL,&zero_timer,NULL)
/* number of ms for timer */
#define INTERVAL 50
#define SEM_VALUE_MAX 65536

/*
 * Thread Control Block definition
 */
typedef struct {
	/* pthread_t usually typedef as unsigned long int */
	pthread_t id;
	/* jmp_buf usually defined as struct with __jmpbuf internal buffer
	   which holds the 6 registers for saving and restoring state */
	jmp_buf jb;
	/* stack pointer for thread; for main thread, this will be NULL */
	char *stack;

	bool blocked = false;
	bool blocker = false;
} tcb_t;

/*
 * Additional Semaphore Struct definition
 */
typedef struct {
	// sem_t *mysem;
	unsigned int sem_id;
	//stores the current value
	unsigned cur_val;
	//a pointer to a queue for threads that are waiting
	int* thread_queue_ptr;

	/*queue for threads that are waiting*/
	std::queue<tcb_t> wait_pool;
	//a flag that indicates whether the semaphore is initialized
	bool flag_init = false;
} mysem_t;

/*
 * Globals for thread scheduling and control
 */

//keep track of semaphore
typedef std::unordered_map<unsigned int, mysem_t> semaphore_map;

/* queue for pool thread, easy for round robin */
static std::queue<tcb_t> thread_pool;

/*queue for threads that are waiting*/
std::queue<tcb_t> wait_pool;

/* keep separate handle for main thread */
static tcb_t main_tcb;
static tcb_t garbage_collector;

/* for assigning id to threads; main implicitly has 0 */
static unsigned long id_counter = 1;
/* we initialize in pthread_create only once */
static int has_initialized = 0;


void lock(){
	// we don't want to be interrupted
	STOP_TIMER;
}

void unlock(){
	RESUME_TIMER;
}


/*
 * init()
 *
 * Initialize thread subsystem and scheduler
 * only called once, when first initializing timer/thread subsystem, etc...
 */
void init() {
	/* on signal, call signal_handler function */
	act.sa_handler = signal_handler;
	/* set necessary signal flags; in our case, we want to make sure that we intercept
	   signals even when we're inside the signal_handler function (again, see man page(s)) */
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;

	/* register sigaction when SIGALRM signal comes in; shouldn't fail, but just in case
	   we'll catch the error  */
	if(sigaction(SIGALRM, &act, NULL) == -1) {
		perror("Unable to catch SIGALRM");
		exit(1);
	}

	/* set timer in seconds */
	interval_timer.it_value.tv_sec = INTERVAL/1000;
	/* set timer in microseconds */
	interval_timer.it_value.tv_usec = (INTERVAL*1000) % 1000000;
	/* next timer should use the same time interval */
	interval_timer.it_interval = interval_timer.it_value;

	/* create thread control buffer for main thread, set as current active tcb */
	main_tcb.id = 0;
	main_tcb.stack = NULL;

	/* front of thread_pool is the active thread */
	thread_pool.push(main_tcb);

	/* set up garbage collector */
	garbage_collector.id = 128;
	garbage_collector.stack = (char *) malloc (32767);

	/* initialize jump buf structure to be 0, just in case there's garbage */
	memset(&garbage_collector.jb,0,sizeof(garbage_collector.jb));
	/* the jmp buffer has a stored signal mask; zero it out just in case */
	sigemptyset(&garbage_collector.jb->__saved_mask);

	/* garbage collector 'lives' in the_nowhere_zone */
	garbage_collector.jb->__jmpbuf[4] = ptr_mangle((uintptr_t)(garbage_collector.stack+32759));
	garbage_collector.jb->__jmpbuf[5] = ptr_mangle((uintptr_t)the_nowhere_zone);

	/* Initialize timer and wait for first sigalarm to go off */
	START_TIMER;
	pause();
}



/*
 * pthread_create()
 *
 * create a new thread and return 0 if successful.
 * also initializes thread subsystem & scheduler on
 * first invocation
 */
int pthread_create(pthread_t *restrict_thread, const pthread_attr_t *restrict_attr,
                   void *(*start_routine)(void*), void *restrict_arg) {

	/* set up thread subsystem and timer */
	if(!has_initialized) {
		has_initialized = 1;
		init();
	}

	/* pause timer while creating thread */
    PAUSE_TIMER;

	/* create thread control block for new thread
	   restrict_thread is basically the thread id
	   which main will have access to */
	tcb_t tmp_tcb;
	tmp_tcb.id = id_counter++;
	*restrict_thread = tmp_tcb.id;

	/* simulate function call by pushing arguments and return address to the stack
	   remember the stack grows down, and that threads should implicitly return to
	   pthread_exit after done with start_routine */

	tmp_tcb.stack = (char *) malloc (32767);

	*(int*)(tmp_tcb.stack+32763) = (int)restrict_arg;
	*(int*)(tmp_tcb.stack+32759) = (int)pthread_exit_wrapper;

	/* initialize jump buf structure to be 0, just in case there's garbage */
	memset(&tmp_tcb.jb,0,sizeof(tmp_tcb.jb));
	/* the jmp buffer has a stored signal mask; zero it out just in case */
	sigemptyset(&tmp_tcb.jb->__saved_mask);

	/* modify the stack pointer and instruction pointer for this thread's
	   jmp buffer. don't forget to mangle! */
	tmp_tcb.jb->__jmpbuf[4] = ptr_mangle((uintptr_t)(tmp_tcb.stack+32759));
	tmp_tcb.jb->__jmpbuf[5] = ptr_mangle((uintptr_t)start_routine);

	/* new thread is ready to be scheduled! */
	thread_pool.push(tmp_tcb);

    /* resume timer */
    RESUME_TIMER;

    return 0;
}



/*
 * pthread_self()
 *
 * just return the current thread's id
 * undefined if thread has not yet been created
 * (e.g., main thread before setting up thread subsystem)
 */
pthread_t pthread_self(void) {
	if(thread_pool.size() == 0) {
		return 0;
	} else {
		return (pthread_t)thread_pool.front().id;
	}
}



/*
 * pthread_exit()
 *
 * pthread_exit gets returned to from start_routine
 * here, we should clean up thread (and exit if no more threads)
 */
void pthread_exit(void *value_ptr) {

	/* just exit if not yet initialized */
	if(has_initialized == 0) {
		exit(0);
	}

	/* stop the timer so we don't get interrupted */
	STOP_TIMER;

	if(thread_pool.front().id == 0) {
		/* if its the main thread, still keep a reference to it
	       we'll longjmp here when all other threads are done */
		main_tcb = thread_pool.front();
		if(setjmp(main_tcb.jb)) {
			/* garbage collector's stack should be freed by OS upon exit;
			   We'll free anyways, for completeness */
			free((void*) garbage_collector.stack);
			exit(0);
		}
	}

	/* Jump to garbage collector stack frame to free memory and scheduler another thread.
	   Since we're currently "living" on this thread's stack frame, deleting it while we're
	   on it would be undefined behavior */
	printf("ABOUT TO GARBAGE COLLECT (unless it's a blocker)\n");
	if(! thread_pool.front().blocker){
		printf("GARBAGE COLLECTING!\n");
		longjmp(garbage_collector.jb,1);
	}
	printf("no garbage collection today\n");
}


int pthread_join(pthread_t thread, void **value_ptr){
	// set that this pthread is blocked
	STOP_TIMER;
	thread_pool.front().blocked = true;
	printf("size is: %d\n", thread_pool.size());
	if(thread_pool.front().id != 0){
		if( setjmp(thread_pool.front().jb) != 0){
			perror("something went wrong with setjmp\n");
			return 1;
		}
	}
	printf("thread id of calling thread is: %d\n", thread_pool.front().id);
	printf("thread id of joining thread is: %d\n", thread);

	// check if thread is exited already
	printf("checking if thread is exited\n");
	bool exited = false;
	pthread_t curr_front = thread_pool.front().id;

	while(thread_pool.front().id != thread ){
		thread_pool.push(thread_pool.front());
		thread_pool.pop();
		if(thread_pool.front().id == curr_front){
			// wrapped around to the calling thread
			// this means that thread is already exited
			exited = true;
			break;
		}
	}

	if(exited){
		printf("thread is exited!\n");
		thread_pool.front().blocked = false;
		return ESRCH;
	}
	printf("thread is not exited; start on actual join stuff\n");

	// if we get down here, then thread is at the front of the queue
	// jump to thread but make sure that thread jumps here when it exits
	START_TIMER;

	thread_pool.front().blocker = true;
	printf("ABOUT TO LONGJMP TO THREAD! YAY!\n");
	longjmp(thread_pool.front().jb,1);
	printf("we got past that longjmp yo\n");

	// make sure that thread is at front
	while(thread_pool.front().id != thread ){
		thread_pool.push(thread_pool.front());
		thread_pool.pop();
	}

	int return_value = thread_pool.front().jb->__jmpbuf[4];
	longjmp(garbage_collector.jb,1);

	STOP_TIMER;
	// make normal thread not blocked
	while(thread_pool.front().id != curr_front){
		thread_pool.push(thread_pool.front());
		thread_pool.pop();
	}
	// old thread is now at front
	thread_pool.front().blocked = false;
	START_TIMER;


	return return_value;
}


//TODO: sem_init, sem_destroy, sem_wait, sem_post
//this is useful: https://os.itec.kit.edu/downloads/sysarch09-mutualexclusionADD.pdf

//global to declare current semaphore??

// int sem_init (sem_t *sem, int pshared, unsigned value ){
//
// 	unsigned long sem_id_count = 0;
//
// 	mysem_t cur_sem;
// 	cur_sem.sem_id = sem_id_count;
// 	sem_id_count++;
//
// 	// cur_sem.mysem = *sem;
// 	if (value < SEM_VALUE_MAX){
// 		cur_sem.cur_val = value;
// 	} else {
// 		//return error bc value should be less than sem value max
// 		return -1;
// 	}
//
// 	if (pshared != 0){
// 		//return error bc pshared should always be 0
// 		return -1;
// 	}
//
// 	cur_sem.flag_init = true;
// 	*sem->__align = &cur_sem;
// 	// *sem = tmp_sem.mysem;
// 	semaphore_map[cur_sem.sem_id] = cur_sem;
//
// 	return 0;
// }
//
// int sem_destroy(sem_t *sem){
//
// 	return 0;
// }
//
// //idk need to call block whatever
// int sem_wait(sem_t *sem){
// 	mysem_t cur_sem;
//
// 	for (semaphore_map::const_iterator it = std::map.begin(); it != std::map.end(); ++it) {
//   		if ((it->second) == (*sem->__align)){
//   			cur_sem = it->second;
//   		}
//   			// return it->first;
//   		else{
//   			printf("shit semaphore (wait) not in the map???\n");
//   		}
// 	}
//
//
// 	if(cur_sem.cur_val > 0){
// 		cur_sem.cur_val = cur_sem.cur_val - 1;
// 		return 0;
// 	} else if (cur_sem.cur_val < 0){
// 		return -1;
// 	}
//
// 	if (cur_sem.cur_val == 0){
// 		//not sure if correct....
// 		// thread_pool.front().blocked = true;
// 		(cur_sem.wait_pool).push(thread_pool.front());
// 	}
//
// 	return 0;
//
// }
//
// int sem_post(sem_t *sem){
// 	mysem_t cur_sem;
//
// 	for (semaphore_map::const_iterator it = std::map.begin(); it != std::map.end(); ++it) {
//   		if ((it->second).mysem == (*sem->__align)){
//   			cur_sem = it->second;
//   		}
//   			// return it->first;
//   		else{
//   			printf("shit semaphore (post) not in the map???\n");
//   		}
// 	}
//
// 	cur_sem.cur_val = cur_sem.cur_val + 1;
// 	if (cur_sem.cur_val > 0){
// 		(cur_sem.wait_pool).pop();
// 		thread_pool.push((cur_sem.wait_pool).front());
// 	} else if (cur_sem.cur_val < 0){
// 		return -1;
// 	}
//
//
// 	return 0;
// }


/*
 * signal_handler()
 *
 * called when SIGALRM goes off from timer
 */
void signal_handler(int signo) {

	/* if no other thread, just return */
	if(thread_pool.size() <= 1) {
		return;
	}

	/* Time to schedule another thread! Use setjmp to save this thread's context
	   on direct invocation, setjmp returns 0. if jumped to from longjmp, returns
	   non-zero value. */
	if(setjmp(thread_pool.front().jb) == 0) {
		/* switch threads */
		thread_pool.push(thread_pool.front());
		thread_pool.pop();
		/* resume scheduler and GOOOOOOOOOO */
		// check if the front thread is blocked.
		// If it IS blocked, then we want to push it to the back and call another thread
		while(thread_pool.front().blocked == true || thread_pool.front().id == 0){
			// printf("blocked! push to next thread!\n");
			// printf("thread_id of blocked thread is: %d\n", thread_pool.front().id);
			thread_pool.push(thread_pool.front());
			thread_pool.pop();
		}
		printf("jumping to thread: %d\n", thread_pool.front().id);
		longjmp(thread_pool.front().jb,1);
	}

	/* resume execution after being longjmped to */
	return;
}


/*
 * the_nowhere_zone()
 *
 * used as a temporary holding space to safely clean up threads.
 * also acts as a pseudo-scheduler by scheduling the next thread manually
 */
void the_nowhere_zone(void) {
	/* free stack memory of exiting thread
	   Note: if this is main thread, we're OK since
	   free(NULL) works */
	free((void*) thread_pool.front().stack);
	thread_pool.front().stack = NULL;

	/* Don't schedule the thread anymore */
	thread_pool.pop();

	/* If the last thread just exited, jump to main_tcb and exit.
	   Otherwise, start timer again and jump to next thread*/
	if(thread_pool.size() == 0) {
		longjmp(main_tcb.jb,1);
	} else {
		START_TIMER;
		longjmp(thread_pool.front().jb,1);
	}
}



/*
 * ptr_mangle()
 *
 * ptr mangle magic; for security reasons
 */
int ptr_mangle(int p)
{
    unsigned int ret;
    __asm__(" movl %1, %%eax;\n"
        " xorl %%gs:0x18, %%eax;"
        " roll $0x9, %%eax;"
        " movl %%eax, %0;"
    : "=r"(ret)
    : "r"(p)
    : "%eax"
    );
    return ret;
}

void pthread_exit_wrapper()
{
  unsigned int res;
  asm("movl %%eax, %0\n":"=r"(res));
  pthread_exit((void *) res);
	printf("done with exit\n");
}
