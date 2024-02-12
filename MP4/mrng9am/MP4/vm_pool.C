/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

#define GB * (0x1 << 30)
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    //assert(false);
    
    region_size = _size;
    region_start_address = _base_address;
    
    frames = _frame_pool;
    page_table = _page_table;
    
    //define the start address and size
    start_address[0] = region_start_address;
    //size[0] = region_size;
    
    //define free address and free size
    free_address = region_start_address;
    free_size = region_size;
    //initialize current pool count
    current_region = 0;
    page_table ->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    //assert(false);
    //before allocating need to check if the requested size is available by checking the size of the free region
	assert(_size < free_size);
    unsigned long temp_address;
    unsigned long temp_size = _size;
    
    //identify the frame requirement for requested size
    unsigned long temp_frame_count = temp_size / Machine::PAGE_SIZE;
    unsigned long temp_last_frame = temp_size % Machine::PAGE_SIZE;
    
    temp_frame_count = temp_last_frame > 0 ? temp_frame_count + 1 : temp_frame_count;
    
    //identify the start address for the new allocation and update the arrays
    if(current_region != 0)
    {
    	temp_address = start_address[current_region - 1] + size[current_region - 1];
    	start_address[current_region] = temp_address;
	
    }
    else
    {
    	temp_address = region_start_address + (2 * Machine::PAGE_SIZE);
    	start_address[current_region] = temp_address;

    }
    free_address = temp_address + ((temp_frame_count)*Machine::PAGE_SIZE);
    free_size = free_size - _size;
    size[current_region] = temp_frame_count * Machine::PAGE_SIZE;
    current_region = current_region + 1;
    Console::puts("Allocated region of memory.\n");
    return temp_address;
}

void VMPool::release(unsigned long _start_address) {
    //assert(false);
    int temp_current_region;
    int temp_address = _start_address;
    //identify the region index
    int looper = 0;
    while(1)
    {
    	if(start_address[looper] != temp_address)
    		looper++;
    	else
    	{
    		temp_current_region = looper;
    		break;
    	}
    }
    
 
    //free pages one by one
    looper = 0;
    
    //update free size
    free_address = start_address[temp_current_region];
    free_size = free_size + size[temp_current_region];
    while(looper < ((size[temp_current_region]) / Machine::PAGE_SIZE))
    {
    	page_table->free_page(temp_address);
    	temp_address = temp_address + Machine::PAGE_SIZE;
    	looper++;
    }
    
    //remove the pool from the list
   // left shift all contents starting from the target pool
    while(temp_current_region <  current_region - 1)
    {
    	start_address[temp_current_region] = start_address[temp_current_region + 1];
    	size[temp_current_region] = size[temp_current_region + 1];
    	temp_current_region++;
    }
    
    
    current_region = current_region - 1;
    
    //flush TLB by loading the PTBR
    page_table -> load();
    //write_cr3((unsigned long)read_cr3);
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    //assert(false);
    if((_address >= region_start_address))
    {
    	if(_address < region_start_address + region_size)
    	{
    		return true;
    	}
    
    }
    Console::puts("Checked whether address is part of an allocated region.\n");
    return false;
}

