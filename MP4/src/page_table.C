#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //assert(false);

    PageTable::kernel_mem_pool = _kernel_mem_pool;
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;

    Console::puts("Initialized Paging System\n");
}


PageTable::PageTable()
{
   //assert(false);
   
    unsigned long page_dir_pages;
    unsigned long page_table_pages;

    //Page Directory creation - obtained from process memory
    page_directory = (unsigned long *)(process_mem_pool->get_frames(1)*PAGE_SIZE);
    //Page Table creation for shared address space between 2MB to 4MB but stored in process memory pool this time
    unsigned long * free_mapped_pt = (unsigned long *)(process_mem_pool->get_frames
                                                        (1)*PAGE_SIZE);
	unsigned int frame_count_shared = shared_size/PAGE_SIZE;
	
	//register vm pool
    int i= 0;
    while(i < 5) 
    {
        temp_vm_pool[i] = NULL;
        i++;
    }
    vm_pool_count = 0;

    unsigned long temp_addr = 0;
    
	/* Page directory and page table created in process memory pool.
	 * Shared by all address spaces.
	 * 
	 */
    unsigned long temp_frame_count = ( PageTable::shared_size / PAGE_SIZE);
    i = 0;
    //Create Page Table for shared address space and mark all as present and write enabled
	
    while(i < temp_frame_count) 
    {
        free_mapped_pt[i] = temp_addr | 3 ;
        temp_addr += PAGE_SIZE;
        i++;
    }
	//Above page table will be stored as first entry in the page directory page
	//Rest will be marked as unused, i.e left alone with write bit set, all pages so far are kernel mode so LSB+2 is not set
	
    page_directory[0] = (unsigned long)free_mapped_pt | 3;

    temp_addr = 0;
    
    i = 1;
    while(i< temp_frame_count) 
    {
        page_directory[i] = temp_addr | 2;
        i++;
    }
        //LSB - Present(1)/Absent(0)
	//LSB + 1 - Write(1)/Read(0)
	//LSB + 2 - User(1)/Kernel(0)
	
	//recursive lookup by storing the page directory in the last entry of the page table
	page_directory[temp_frame_count - 1] = (unsigned long)(page_directory) | 3;
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   //assert(false);

    current_page_table = this;
    write_cr3((unsigned long)page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //assert(false);

    paging_enabled = 1;

    write_cr0(read_cr0() | 0x80000000);
    
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    //assert(false);
    Console::puts("HANDLING FAULTS");
    //Read the faulting address from CR2 in order to handle the page fault for that
	
    unsigned long fault_address = read_cr2();
    //Split the bits as 10 (PDE) | 10 (PTE) | 12 (Offset)
	
    unsigned long dir_offset   = fault_address >> 22;
    unsigned long tab_offset   = (fault_address >> 12) & 0x3FF;
	//NULL PT pointer, to point to the page table in consideration of the faulting address.
	
    unsigned long * page_table = NULL;
    unsigned long error_code = _r->err_code;


    bool flag = false;

    unsigned long *temp_page_dir = (unsigned long*)(0xFFFFF000);

		
	//to check if error code is 0.
    if ((error_code & 1) == 0 ) 
    {
     VMPool ** temp_pool = current_page_table->temp_vm_pool;
     int i = 0;
     while (i < current_page_table->vm_pool_count) 
     {
         if (temp_pool[i] != NULL) 
         {
                if (temp_pool[i]->is_legitimate(fault_address)) 
                {
                    flag  = true;
                    break;
                }
            }
            i++;
     }
    //if the flag is false, which means the faulting address is not legitimate i.e part of any of the allocated regions
    if(flag)
    {
        if ((temp_page_dir[dir_offset] & 0x1 ) == 0) 
        {  
            //Page Table is faulting in directory
	    //Get frame for dir_offset page table and update page directory
						
            temp_page_dir[dir_offset] = (unsigned long) ((process_mem_pool->get_frames(1)*PAGE_SIZE) | 3);

            page_table = (unsigned long *)((dir_offset << 12) | (0xFFC00000) );
		int cnt = 0;
            while(cnt <1024) 
            {
            	//page marked as user page
                page_table[i] = 0 | 0X4 ; 
                cnt++;
            }
            //For the required offset, get a frame from process pool
					
	     page_table[tab_offset] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE);
            page_table[tab_offset] = page_table[tab_offset] | 3 ;
		//Get frame for the tab_offset, mark as present and write
        } 
        else 
        {
            //page is faulting in page table
            page_table = (unsigned long *)((dir_offset << 12) | (0xFFC00000) );
            //page reuqested from pocess memory pool to be allocated for missing page
            page_table[tab_offset] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE);
            //page marked as write and present
            page_table[tab_offset] = page_table[tab_offset] | 3;
        }
    

    Console::puts("handled page fault\n");
  }


  
     }
}

void PageTable::register_pool(VMPool* _vm_pool)
{

	 if (vm_pool_count >= 5) 
	 {
		Console::puts("VM POOL is full");
	 }
	 else 
	 {
	 	temp_vm_pool[vm_pool_count++] = _vm_pool;
		Console::puts("registered VM pool\n");
		
	 } 


}

void PageTable::free_page(unsigned long _page_no)
{
  //directory address resolution
 unsigned long dir_offset = _page_no >> 22;//directory offset (first 10 bits) moved to the LSB
 dir_offset = (0xFFC00000) | (dir_offset << 12);
 unsigned long tab_offset = (_page_no >> 12) & 0x3FF; // table offset moved to he lSB by right shift and then the leading 10 bits corresp to directory offset is removed

 //table address resolution
 unsigned long * page_table = (unsigned long*)(dir_offset);

 unsigned long target_frame = page_table[tab_offset] / Machine::PAGE_SIZE;
//release required frame
 process_mem_pool->release_frames(target_frame); 

 page_table[tab_offset] = 2;  
//flush tlb
} 
