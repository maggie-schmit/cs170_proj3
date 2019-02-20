#include <stdlib.h>   /* for exit() */
#include <stdio.h>    /* standard buffered input/output */
#include <setjmp.h>   /* for performing non-local gotos with setjmp/longjmp */
#include <sys/time.h>   /* for setitimer */
#include <signal.h>   /* for sigaction */
#include <unistd.h>   /* for pause */
#include <pthread.h>
#include <stdint.h>
#include <vector>
#include <queue>
#include <assert.h>
#include <unordered_map>

using namespace std;
/* number of milliseconds to go off */
#define INTERVAL 50
#define main_function sleep(1)

//use this ex to test: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/ptexit.htm

// when you add 1 to any pointer, c interprets this as add sizeof(pointer base type) to the pointer
// so (char *)c + 1 increments the address of c by 1
// but (int *)c + 1 increments the address of c by 4 b/c 4 is the sizeof(int)
// so, when working with a stack logically if a cell of the stack holds a pointer to a function its size
// must be pointer size which is sizeof(void *)
// therefore, even if logically the type void ** doesn't REALLY mean anything for stackptr
// making it this type will turn out to be useful because
// ((void **)this->stackptr)-- will move the stack pointer down 4 bytes of a 32 bit machine and 8 bytes on a 64 bit device like your mac
// ((void *)this->stackptr)--


//pthread_create(): https://linux.die.net/man/3/pthread_create
struct Thread {
  unsigned long int thread_id = 0;
  //void* is any untyped region
  int* stack = NULL;
  int** stackptr = NULL;
  void* progcounter = NULL;
  // vector<uint64_t *> registers;

  // keeps track of how many milliseconds the thread has run
  int time_run = 0;

  bool isMain = false;

  // saves the enviornment of the thread
  jmp_buf thread_env;

  // int status = -1; /* 1 is running, 0 is ready to run, -1 is exited*/
  //https://forums.whirlpool.net.au/archive/1689677
  enum status_codes {
    RUNNING = 1,
    READYTORUN = 0,
    EXITED = -1
  } status;

  //constructor to create Thread class
  Thread(){
    stack = (int*)malloc(32767);
  }
  // deconstructor, later need to do something w freeing registers (maybe)
  ~Thread(){
    free(stack);
  }

};

// jmp_buf buf;

static int ptr_mangle(int p){
    unsigned int ret;
    asm(" movl %1, %%eax;\n"
        " xorl %%gs:0x18, %%eax;"
        " roll $0x9, %%eax;"
        " movl %%eax, %0;"
    : "=r"(ret)
    : "r"(p)
    : "%eax"
    );
    return ret;
}

unordered_map<int, Thread *> threads; // track threads by id
queue<Thread *> thread_queue; // track order of threads to execute
unordered_map<int, Thread*>::iterator threads_it;

bool initialized = false;
bool isWrapped = 0;
void scheduler(int);


int init(){
  // assert(initialized == false);
  // printf("in init function\n");
  Thread *main = new Thread;
  main->thread_id = 0;
  main->status = Thread::RUNNING;
  main->isMain = true;
  threads[main->thread_id] = main;
  threads_it = threads.find(main->thread_id);
  // main->stackptr = ((void **)((char *)main->stack));

  // printf("pushing main thread onto queue\n");
  thread_queue.push(main);

  main->stackptr = (int **)(((char *)main->stack)+32767);


  // timer magicks
  struct itimerval it_val;
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_handler = scheduler;
  act.sa_flags = SA_NODEFER;

  if(sigaction(SIGALRM, &act, NULL) == -1) {
    perror("Unable to catch SIGALRM");
    exit(1);
  }
  // printf("set up timer\n");
  it_val.it_value.tv_sec = INTERVAL/1000;
  it_val.it_value.tv_usec = (INTERVAL*1000) % 1000000;
  it_val.it_interval = it_val.it_value;
  if(setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    perror("error calling setitimer()");
    exit(1);
  }

  // printf("creating main thread\n");
  // // create a new thread that will be used to represent the 'main' thread of execution which called 'init'
  // printf("init setjmp value: %d\n", main->thread_env);


  // (*(main->stackptr)) = (int *) pthread_exit;  // sets the return function, idk if this is correct
  return setjmp(main->thread_env);


}


void swap_in_thread(Thread *old_thread) {

  if(setjmp(old_thread->thread_env) != 0){
    // perror("Something went wrong with setjmp!\n");
    return;

  }

  // if(old_thread->status == Thread::EXITED){
  //     threads.erase(old_thread->thread_id);
  //     // printf("erased succusefully\n");
  // }else{
  //
  // }
  if(old_thread->status != Thread::EXITED){
    old_thread->status = Thread::READYTORUN;
  }
  thread_queue.front()->status=Thread::RUNNING;


  longjmp(thread_queue.front()->thread_env, 1);


}

void scheduler(int sig){
  if (sig == SIGALRM){
    // printf("Caught SIGALARM, Existing\n");

    // printf("thread_count: %d\n", thread_queue.size());
    // printf("curthread: %d\n", thread_queue.front()->thread_id);
    Thread *old_thread = thread_queue.front();

    if(thread_queue.front()->isMain || thread_queue.front()->status != Thread::EXITED){
      // printf("pushed thread\n");
      thread_queue.push(thread_queue.front());
    }
    thread_queue.pop();




    // printf("nextthread: %d\n", thread_queue.front()->thread_id);

    if (thread_queue.size() == 0) {
      exit(0);
    }else{

      swap_in_thread(old_thread);
    }
  }
};


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
  // printf("pthread_create called\n");
  Thread* new_thread = new Thread;
  new_thread->status = Thread::READYTORUN;
  new_thread->thread_id = 1;

  if (initialized == false){
    // printf("pthread_create: initializing, first invocation of the library\n");
    initialized = true;
    if (init() != 0) {
      // printf("does it get here?\n");
      return new_thread->thread_id;
    }
  }

  // get thread_id from iterator
  // if(threads_it->thread_id < 128){
  //   new_thread->thread_id = (*threads_it->thread_id) +1;
  // }else{
  //   new_thread->thread_id = 1;
  // }

  //
  // // make iterator point to new_thread;
  // new_thread->thread_id = threads.find(new_thread->thread_id);

  int new_id = threads_it->second->thread_id +1;

  if(new_id >= 128){
    new_id = 1;
  }

  bool isWrapped = false;
  while(threads.find(new_id) != threads.end()){
    // this means that there is currently a thread at new_id
    if(threads.find(new_id)->second->status != Thread::EXITED){
      new_id++;
    }else{
      break;
    }
    if(new_id >= 128 && isWrapped){
      perror("128 THREADS ARE CURRENTLY RUNNING; CANNOT CREATE MORE\n");
      return -1;
    }else if (new_id >= 128){
      isWrapped = true;
      new_id = 1;
    }
  }
  new_thread->thread_id = new_id;	

  // add new_thread to threads
  threads[new_thread->thread_id] = new_thread;
  *thread = new_thread->thread_id;

  // make the iterator point to the new thread
  threads_it = threads.find(new_thread->thread_id);





  //do stuff with stack here
  new_thread->progcounter = (void *)start_routine; //progcounter points to address of the function
  new_thread->stackptr = (int **)(((char *)new_thread->stack)+32767);

  (*(new_thread->stackptr)) = (int *)arg; //pretends stack is an array of pointers
  new_thread->stackptr = (int**)((char*)new_thread->stackptr-4*sizeof(char));

  (*(new_thread->stackptr)) = (int *) pthread_exit;
  new_thread->stackptr = (int**)((char*)new_thread->stackptr-4*sizeof(char));


  setjmp(new_thread->thread_env);

  (new_thread->thread_env)->__jmpbuf[4] = ptr_mangle((int) new_thread->stackptr);
  (new_thread->thread_env)->__jmpbuf[5] = ptr_mangle((int) new_thread->progcounter);   // this is what sets jmpbuf to jump to the thread function, otherwise it would go back to where setjmp was set





  thread_queue.push(new_thread);
  scheduler(SIGALRM);
  return 0;
};


void pthread_exit(void *retval){
  thread_queue.front()->status = Thread::EXITED;


  scheduler(SIGALRM);
  // printf("hello\n");
}

//THIS FUNCTION IS FINISHED
pthread_t pthread_self(void){
  return thread_queue.front()->thread_id;
  // return NULL;
}
