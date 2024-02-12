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
    
    //register vm pool
    page_table ->register_pool(this);
    //define the start address and size
    //start_address[0] = region_start_address;
    //size[0] = region_size;
    allocator = (vm_pool_regions*) region_start_address;
    //define free size
    //free_address = region_start_address;
    free_size = region_size;
    //initialize current pool count
    current_region = 0;
    
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
    		//if vm pool is already having entries
	    allocator[current_region].start_address = allocator[current_region - 1].start_address + allocator[current_region].size;
	    allocator[current_region].size = temp_frame_count * Machine::PAGE_SIZE;
	    temp_address = allocator[current_region].start_address;
	    //update current region
	    current_region++;
	    //update free size
	    free_size -= _size;
	    Console::puts("Allocated region of memory 1.\n");
	    return temp_address;
    }
    else
    {
    		//if this is the first allocation request
    		allocator[0].start_address = region_start_address +  Machine::PAGE_SIZE;
	    allocator[0].size = temp_frame_count * Machine::PAGE_SIZE;
	    //update the count
	    current_region++;
	    //update free size
	    free_size -= _size;
	    Console::puts("Allocated region of memory.\n");
	    return region_start_address +  Machine::PAGE_SIZE;
	    
    }

    Console::puts("Allocated region of memory.\n");
    
}

void VMPool::release(unsigned long _start_address) {
    
    //identify the region index
    int looper = 0;
    
    //to identify the vm pool of the provided address
    while(allocator[looper].start_address != _start_address)
    {
    	looper++;
    }
    /*
	if(looper > 0)
	{
		Console::puts("hier");
	}*/
	unsigned long temp_all = ((allocator[looper].size) / (Machine::PAGE_SIZE));
	
	//free pages for the identified set
	int j = 0;
	while(j < temp_all)
	{
		page_table->free_page(_start_address);
		_start_address += Machine::PAGE_SIZE;
		j++;
	}
	
	//left shifting contents to make sure the entry is removed
	j =1;
	while(j < current_region)
	{
		allocator[j] = allocator[j+1];
		j++;
	}
	//update pool count 
	current_region--;
	
	//flush tlb here
	page_table-> load();
	
	//update free size
	free_size -= allocator[looper].size;
	Console::puts("Released region of memory\n");
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

