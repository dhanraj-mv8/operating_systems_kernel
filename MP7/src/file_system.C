/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    
    size = 0;//initial size is 0
    nInode1 = 0;//free inode counter
    
    disk = NULL;//initialize to null
    free_blocks = NULL;//initialize to null
    inodes = NULL;//initialize to null
    
    BLOCK_SIZE1 = SimpleDisk::BLOCK_SIZE;
    //assert(false);
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    
    unsigned char *dummy = new unsigned char[BLOCK_SIZE1];
    
    //remove inodes bitmap and data blocks bimap stored in block 0&1 respectively
    memcpy(dummy,free_blocks,BLOCK_SIZE1);
    memcpy(dummy,inodes,BLOCK_SIZE1);
    
    disk->write(0,(unsigned char *)dummy);//delete inodes list
    disk->write(1,(unsigned char *)dummy);//delete data blocks list
    //assert(false);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

int FileSystem::GetFreeBlock()
{
	int x =0;
	while(x < BLOCK_SIZE1)
	{	//iterates until it finds a free block
		if(free_blocks[x] == 0)
		{
			return x;
		}
		x++;
	}
	//returns the number of the free block
	return x;
}

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    disk = _disk;
    BLOCK_SIZE1 = disk->BLOCK_SIZE;
    
    //initialise and declate new data structure to store free data blocks and inode maps
    free_blocks = new unsigned char[BLOCK_SIZE1];
    unsigned char *free_inodes = new unsigned char[BLOCK_SIZE1];
    
    //read the free blocks and inode bitmap to memory
    disk->read(0,free_inodes);
    disk->read(0,free_blocks);
    
    inodes = (Inode*)free_inodes;
    
    return true;
    
    //assert(false);
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
       
    disk = _disk;
    size = _size;
    
    //identify the inode
    inodeMaps = (sizeof(Inode) * (size/BLOCK_SIZE1))/BLOCK_SIZE1;
    
    int looper = 0;
    
    while(looper < BLOCK_SIZE1)
    {
    	if(looper <= inodeMaps)
    		free_blocks[looper] = 0xFF;//mark as used
    	else
    	    	free_blocks[looper] = 0;//mark free
    	looper++;
    }
    
    //update both inode bitmap and free block bitmap
    disk->write(0,(unsigned char *) inodes);
    disk->write(1,free_blocks);
    
    return true;    
    
    //assert(false);
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    
    int id1 = _file_id;
    int looper = 0;
    
    //iterate until max inodes allowed
    while(looper < MAX_INODES)
    {
    	//inode needs to be valid by checking status bit and the id of the inode matches with target id that we are looking for
    	if(inodes[looper].status && inodes[looper].id == id1)
    	{
    		return &inodes[looper];//returns the inode if id match is found
    	}
    	looper++;
    }
    //satisfy compiler by below
    return NULL;
    
    
    //assert(false);
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    
    int id1 = _file_id;
    bool result = false;
    
    //lookup the target id, and if already a file exists with that id then return false
    
    if(LookupFile(id1) != NULL)
    {
    	result = false;
    }
    else
    {
    	//get one free block number using below function
    	int dummy_block_number = GetFreeBlock();
    	
    	//create new inode
    	Inode *dummy_inode = new Inode();
    	
    	//set the free block as used in the free block bitmap
    	free_blocks[dummy_block_number] = 0xFF;
    	
    	//set inode attributes
    	dummy_inode->id = id1;
    	dummy_inode->status = true;
    	dummy_inode->blockNumber1 = dummy_block_number;
    	
    	//check nInode1 counter if it has reached MAXINODES allowed	
    	if(MAX_INODES == nInode1)
    	{
    		result = false;
    	}
    	else
    	{
    		
    		//store the newly created inode at the position equaling current count of inodes
    		inodes[nInode1] = *dummy_inode;
    		nInode1++;//increment inode counter
    		result = true;
    	}
    	
    }
    
       
    return result;
    //assert(false);
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
       
       int id1 = _file_id;
       //first identify the inode of the target id and store it temporarily
       Inode *dummy_inode = LookupFile(id1);
       
       if(dummy_inode != NULL)
       {
       	//inode invalidated by modifying status as false
       	dummy_inode->status = false;
       	free_blocks[dummy_inode->blockNumber1] = 0;//mark it as free in free block
       	nInode1--;//decrement inode counter
       	return true;
       }
       
       
       return false;
}
