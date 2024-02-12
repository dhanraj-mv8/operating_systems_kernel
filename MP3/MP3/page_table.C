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

#define PT_Count 1

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
	PageTable::shared_size = _shared_size;
	PageTable::kernel_mem_pool = _kernel_mem_pool;
	PageTable::process_mem_pool = _process_mem_pool;
	
	Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   	//assert(false);
   	//Page Directory creation
	page_directory = (unsigned long *)(kernel_mem_pool -> get_frames(PT_Count)*PAGE_SIZE);
	
	//Page Table creation for shared address space between 2MB to 4MB
	unsigned long *PT_Free_Map = (unsigned long *)(kernel_mem_pool ->get_frames(PT_Count)*PAGE_SIZE);
	
	
	/* Above defined memory pool belongs to kernel (2MB to 4MB).
	 * Shared by all address spaces.
	 * 
	 */	
	unsigned int frame_count_shared = PageTable::shared_size/PAGE_SIZE;
	int count = 0;
	unsigned long temp_addr = 0;

	//Create Page Table for shared address space and mark all as present and write enabled
	while(count < frame_count_shared)
	{
		PT_Free_Map[count] = temp_addr | 3;
		// Above makes the frame in PT as Present and Write.
		count++;
		temp_addr = temp_addr + PAGE_SIZE;
	}
	
	//Above page table will be stored as first entry in the page directory page
	//Rest will be marked as unused, i.e left alone with write bit set, all pages so far are kernel mode so LSB+2 is not set
	temp_addr = 0;
	count = 0;

	while(count < frame_count_shared)
	{
		if(count == 0)
		{
			page_directory[count] = (unsigned long)PT_Free_Map | 3;
			//temp_addr = temp_addr + PAGE_SIZE;
			//count++;
		}
		else
		{
			page_directory[count] = temp_addr | 2;
			//temp_addr = temp_addr + PAGE_SIZE;
			//count++;
		}
		count+=1;
	}
	//LSB - Present(1)/Absent(0)
	//LSB + 1 - Write(1)/Read(0)
	//LSB + 2 - User(1)/Kernel(0)
	
	Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   
   write_cr3((unsigned long)page_directory);
   //Write CR3(PTBR) with Page Directory address.
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //assert(false);   
   paging_enabled = 1;
   unsigned long temp = read_cr0();
   temp = temp | 0x80000000;
   //Setting MSB as 1 to make Paging bit in CR0 set which will enable paging
   
   write_cr0(temp);
   //Writing the same to CR0 register.

   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  	//assert(false);
  	//Read the faulting address from CR2 in order to handle the page fault for that
	unsigned long fault_address = read_cr2();
	unsigned long error = _r->err_code;
	
	//Split the bits as 10 (PDE) | 10 (PTE) | 12 (Offset)
	unsigned long dir_offset = fault_address >> 22; // Shifting to the left by 22 push the PDE offset to the LSB
	unsigned long tab_offset = fault_address & 0x3FF000;//0x3FF000 - 1111111111000000000000 , this removes offset and PDE from the address so we get PTE offset
	tab_offset = tab_offset >> 12; // Shift it by 12 bits to LSB
	unsigned long frame_offset = fault_address & 0XFFF;//0xFFF - 111111111111, this removes 1st 20 bits of the fault address to get actual offset

	//Reading PTBR from CR3
	unsigned long *temp_page_dir = (unsigned long*)read_cr3();
	
	//NULL PT pointer, to point to the page table in consideration of the faulting address.
	unsigned long *temp_pt = NULL;
	
	//to check if error code is 0.	
	if((error & 1) == 0)
	{
		if((temp_page_dir[dir_offset] & 1) == 0) 
		{
			
			//Page Table is faulting in directory
			//Get frame for dir_offset page table and update page directory
			temp_page_dir[dir_offset] = (unsigned long) (kernel_mem_pool->get_frames(PT_Count)*PAGE_SIZE);
			temp_page_dir[dir_offset] = temp_page_dir[dir_offset] | 3;
			temp_pt = (unsigned long*) (temp_page_dir[dir_offset] & 0xFFFFF000);
			//initialize page table & mark as user mode and write enabled but absent
			int cnt = 0;
			while(cnt<=1023)
			{
				temp_pt[cnt] = 0 | 6;
				cnt++;
			}
			//For the required offset, get a frame from process pool
			temp_pt[tab_offset] = (PageTable::process_mem_pool->get_frames(PT_Count)*PAGE_SIZE);
			temp_pt[tab_offset] = temp_pt[tab_offset] | 7;
			//Get frame for the tab_offset, mark as present
			
			
		}
		else
		{
			
			//Page is faulting in page table
			temp_pt = (unsigned long*) (temp_page_dir[dir_offset] & 0xFFFFF000);//Get to the PDE entry of the page table and get the startign address of the page table
			temp_pt[tab_offset] = (PageTable::process_mem_pool->get_frames(PT_Count)*PAGE_SIZE);//Get frame for the reqd PTE.
			temp_pt[tab_offset] = temp_pt[tab_offset] | 3;
			//Get frame tab_offset in page table and mark it as write and present
		}
	}
	Console::puts("handled page fault\n");
	
	
	
	
}

