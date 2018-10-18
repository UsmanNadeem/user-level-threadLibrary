#include <iostream>
#include <stdlib.h>
#include <ucontext.h>
#include <list>
#include <stdarg.h> 
#include <signal.h>
#include <string.h>
#include <unistd.h>
// #################################################
class Thread;

bool timer = false;  // turn on for timer based context switch.
unsigned ALARM_TIME = 1;

bool INTERRUPT = true;
std::list<Thread*> ready_to_run;
Thread* current_thread;
static int id = 0;
#define STACKSIZE 1024*50
#define SIZE_TO_COPY 1024*1
// #################################################

void handle_error(std::string message) {
  std::cout << message << std::endl;
  exit(-1);
}
void TURN_INTERRUPT_OFF() {INTERRUPT = false;}
void TURN_INTERRUPT_ON() {INTERRUPT = true;}

class Thread {
public:
	ucontext_t* context;
	int id;
	std::list<Thread*> waiting_list;
	Thread (){
		this->id = 0;
		context = new ucontext_t;
	};
	Thread(ucontext_t* _new_ctx,int _id) {
		context = _new_ctx;

		greg_t *regs = context->uc_mcontext.gregs;
		void* old_sp = (void*)regs[REG_ESP];
		void* new_sp = malloc(STACKSIZE);
		if (new_sp == 0) handle_error("Error in malloc");
		regs[REG_ESP] = ((greg_t)new_sp+STACKSIZE-1-SIZE_TO_COPY);
		memcpy((void*)regs[REG_ESP], old_sp, SIZE_TO_COPY);
		
		this->id = _id;
	}
	~Thread(){ 
		delete context;
	}
};

void run_next() {
	if (ready_to_run.size() <= 0) return;
	TURN_INTERRUPT_OFF();
	Thread* old_thread = current_thread;ready_to_run.push_back(old_thread);
	current_thread = ready_to_run.front();ready_to_run.pop_front();
	TURN_INTERRUPT_ON();
	if (swapcontext(old_thread->context, current_thread->context) == -1) handle_error("Error in context_switch");
}

void context_switch(int){
	if (!timer) return;
	if (INTERRUPT) {run_next();}
	alarm(ALARM_TIME);
}

void mythread_yield() {
	if (!timer) run_next();
}

void mythread_init() {
	if (timer) {
		signal(SIGALRM, context_switch);
		alarm(ALARM_TIME);
	}
	ucontext_t *context = new ucontext_t;
	if (getcontext(context) != 0) handle_error("Error during init\n");
	current_thread = new Thread(context, 0);
}

int mythread_fork() {
	int return_val = ++id;
	ucontext_t *context = new ucontext_t;
	
	if (getcontext(context) != 0) handle_error("Error during fork\\getcontext");

	if(current_thread->id == return_val) return 0;
	
	TURN_INTERRUPT_OFF();	
	Thread* old_thread = current_thread;
	ready_to_run.push_back(old_thread);
	greg_t *regs = old_thread->context->uc_mcontext.gregs;
	current_thread = new Thread(context, return_val);
	TURN_INTERRUPT_ON();

	if (swapcontext(old_thread->context, current_thread->context) == -1) handle_error("Error in fork\\swapcontext");
	
	return return_val;
}

void mythread_exit() {
	TURN_INTERRUPT_OFF();
	for (std::list<Thread*>::iterator it=current_thread->waiting_list.begin(); it != current_thread->waiting_list.end(); ++it)
		ready_to_run.push_back(*it);
	delete current_thread;
	if (ready_to_run.size() == 0) exit(0);
	current_thread = ready_to_run.front();ready_to_run.pop_front();
	TURN_INTERRUPT_ON();
	if (setcontext(current_thread->context) == -1) handle_error("Error in Exit");
}

int mythread_join(int threadid) {
	if (threadid == current_thread->id  ||  ready_to_run.size() <= 0) return -1;

	TURN_INTERRUPT_OFF();
	bool found = false;
	for (std::list<Thread*>::iterator it=ready_to_run.begin(); it != ready_to_run.end(); ++it) {
		if ((*it)->id == threadid){
			(*it)->waiting_list.push_back(current_thread);
			found = true;
			break;
		}
	}
	if (found == false) return -1;
	Thread* old_thread = current_thread;
	current_thread = ready_to_run.front();ready_to_run.pop_front();
	TURN_INTERRUPT_ON();

	if (swapcontext(old_thread->context, current_thread->context) == -1) handle_error("Error in join");
}
