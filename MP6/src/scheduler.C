/*
 File: scheduler.C
 
 Author: Dhanraj Murali
 Date  : 04-11-23
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */
#ifndef NULL
#define NULL 0L
#endif
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
	//assert(false);

	size = 0;
	//ready = NULL;
	//initialize the size of the queue as zero
	Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	//assert(false);
	Thread *temp1;
	//assert(size<0);
	if(size > 0)
	{
		//retrieve the element at the top of the stack
		temp1 = ready.next_element();
		size = size - 1;//reduce the size of the queue as one element is popped out for execution
		Thread::dispatch_to(temp1);//dispatch the CPU to the thread element that is popped out of the ready queue
	}  
  
}

void Scheduler::resume(Thread * _thread) {
	//assert(false);
	add(_thread);
  
  
}

void Scheduler::add(Thread * _thread) {
	//assert(false);
	ready.add_to_last(_thread);//add the thread element to the back end/tail of the stack
	size = size + 1;//increment the size of the queue
}

void Scheduler::terminate(Thread * _thread) {
	//assert(false);
	int k = 0;
	Thread* temp_head;
	//traverse through the list to identify the thread id of the target thread.
	while(k<size)
	{
		temp_head = ready.next_element();
		if(_thread->ThreadId() != temp_head->ThreadId())
			ready.add_to_last(temp_head);//add the thread to the back end of the queue
		else
			size = size - 1;//decrement the size
		k++;
	}
	
	
}
