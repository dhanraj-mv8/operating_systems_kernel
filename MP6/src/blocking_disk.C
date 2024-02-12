/*
     File        : blocking_disk.c

     Author      : Dhanraj Murali
     Modified    : 17-11-2023

     Description : Implementation

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "simple_disk.H"
#include "scheduler.H"
#include "thread.H"

extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
	
	DISK_ID new_id = _disk_id;
	unsigned int new_size = _size;

}


/*--------------------------------------------------------------------------*/
/* BLOCKING_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::wait_until_ready()
{

	if(!SimpleDisk::is_ready())//check if io is ready or not ready
	{
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());//move the thread to the back end of the queue
		SYSTEM_SCHEDULER->yield();//thread yields control and next thread in ready queue gets executed
	}
	
}
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
	// -- REPLACE THIS!!!
	SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
	// -- REPLACE THIS!!!
	SimpleDisk::write(_block_no, _buf);
}


bool BlockingDisk::is_ready() {
  	return ((Machine::inportb(0x1F7) & 0x08) != 0);
}

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

MirroringDisk::MirroringDisk(DISK_ID _disk_id, unsigned int _size):BlockingDisk(_disk_id, _size)
{
	MASTER_Mirror = new BlockingDisk(DISK_ID::MASTER, _size);
	DEPENDENT_Mirror = new BlockingDisk(DISK_ID::DEPENDENT, _size);
}



//disk id is sent as input argument and then checked
void MirroringDisk::issue_operation(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id1)
{
	Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
	Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
	Machine::outportb(0x1F3, (unsigned char)_block_no);
		         /* send low 8 bits of block number */
	Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
		         /* send next 8 bits of block number */
	Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
		         /* send next 8 bits of block number */
	unsigned int disk_no = disk_id1 == DISK_ID::MASTER ? 0 : 1;//input arg is checked against
	Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_no << 4));
		         /* send drive indicator, some bits, 
		            highest 4 bits of block no */

	Machine::outportb(0x1F7, (_op == DISK_OPERATION::READ) ? 0x20 : 0x30);

}

//derived from simple disk as reference
void MirroringDisk::read(unsigned long _block_no, unsigned char * _buf)
{
	//issue_operation(DISK_OPERATION::READ, _block_no);
	issue_operation(DISK_OPERATION::READ, _block_no,DISK_ID::MASTER);//operation issued for master
	//issue_operation(DISK_OPERATION::READ, _block_no);
	issue_operation(DISK_OPERATION::READ, _block_no,DISK_ID::DEPENDENT);//operation issued for dependent

	wait_until_ready();

	int i;
	unsigned short tmpw;
	for (i = 0; i < 256; i++)
	{
		tmpw = Machine::inportw(0x1F0);
		_buf[i*2]   = (unsigned char)tmpw;
		_buf[i*2+1] = (unsigned char)(tmpw >> 8);
	}
}

void MirroringDisk::write(unsigned long _block_no, unsigned char * _buf)
{
  	//WRITE is done in both the MASTER and DEPENDENT blocks
  	//SimpleDisk::write(_block_no, _buf);
	MASTER_Mirror->write(_block_no, _buf);
	DEPENDENT_Mirror->write(_block_no, _buf);
}

void MirroringDisk::wait_until_ready()
{
	//WAIT is checked against both MASTER and SLAVE and even if one is ready the execution will be done
	if(!MASTER_Mirror->is_ready() && !DEPENDENT_Mirror->is_ready() )
	{
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());//move the thread to the back end of the queue
		SYSTEM_SCHEDULER->yield();//thread yields control and next thread in ready queue gets executed
	}
	
}

