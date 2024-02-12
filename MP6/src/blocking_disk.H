/*
	 File        : blocking_disk.H

	 Author      : Dhanraj Murali

	 Date        : 17-11-2023
	 Description : Implementation

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class BlockingDisk : public SimpleDisk
{
public:
	BlockingDisk(DISK_ID _disk_id, unsigned int _size);
	/* Creates a BlockingDisk device with the given size connected to the
	MASTER or SLAVE slot of the primary ATA controller.
	NOTE: We are passing the _size argument out of laziness.
	In a real system, we would infer this information from the
	disk controller. */

	/* DISK OPERATIONS */

	virtual void wait_until_ready();
	virtual bool is_ready();
	virtual void read(unsigned long _block_no, unsigned char *_buf);
	/* Reads 512 Bytes from the given block of the disk and copies them
	to the given buffer. No error check! */

	virtual void write(unsigned long _block_no, unsigned char *_buf);
	/* Writes 512 Bytes from the buffer to the given block on the disk. */
};

#endif

class MirroringDisk : public BlockingDisk
{

private:
	BlockingDisk *MASTER_Mirror;
	BlockingDisk *DEPENDENT_Mirror;

public:
	MirroringDisk(DISK_ID _disk_id, unsigned int _size);
	/* Creates a MirroringgDisk device with the given size connected to the
	MASTER and SLAVE slot of the primary ATA controller.
	NOTE: We are passing the _size argument out of laziness.
	In a real system, we would infer this information from the
	disk controller. */

	/* DISK OPERATIONS */

	virtual void read(unsigned long _block_no, unsigned char *_buf);
	/* Reads 512 Bytes from the given block of the disk and copies them
	to the given buffer. No error check! */

	virtual void write(unsigned long _block_no, unsigned char *_buf);
	/* Writes 512 Bytes from the buffer to the given block on the disk. */

	virtual void wait_until_ready();
	// rewrite wait to check MASTER and DEPENDENT

	virtual void issue_operation(DISK_OPERATION _op, unsigned long _block_n, DISK_ID disk_id);
	// update with disk id as argument
};
