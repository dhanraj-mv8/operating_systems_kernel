/*
    File: simple_fifo.H

    Author: Dhanraj Murali
            Texas A&M University
    Date  : 04-11-23

                 
*/

#ifndef SIMPLE_FIFO_H                  // include file only once
#define SIMPLE_FIFO_H

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */
#ifndef NULL
#define NULL 0L
#endif
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "thread.H"
#include "console.H"
//#include <cstddef>
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/


class Simple_FIFO {

private: 
	Simple_FIFO *next_ptr;
	Thread *head;	
		
	
public: 

	Simple_FIFO()
	{
		head = NULL;
		next_ptr = NULL;
	}
	//constructor
	Simple_FIFO(Thread *th2)
	{
		head = th2;
		next_ptr = NULL;
	}
	//constructor with parameters
	void add_to_last(Thread *temp_th)
	{
		if(head == NULL)
		{
			head = temp_th;
			//if head is null meaning queue is empty then add the new thread to top of queue
		}
		else if(next_ptr == NULL)
		{
			next_ptr = new Simple_FIFO(temp_th);
			//if queue has only one element then add the new thread to the next of head
		}
		else
		{
			next_ptr->add_to_last(temp_th);
			//if queue has many elements then recursively call the function 
		}
		Console::puts("FIFO - Addition\n");
		
	}	
	Thread* next_element()
	{
		if(head == NULL){
			return NULL;
		}
		
		Thread *temp_head = head;
		Simple_FIFO *old_th;
		
		if(next_ptr == NULL)
		{
			head = NULL;
		}
		else
		{	
			//Simple_FIFO = old_th;
			//simple swap operation
			head = next_ptr -> head;
			old_th = next_ptr;
			next_ptr = next_ptr->next_ptr;
			//delete temp_head;
		}
		Console::puts("FIFO - Removal\n");
		
		return temp_head;
	}
};

#endif
