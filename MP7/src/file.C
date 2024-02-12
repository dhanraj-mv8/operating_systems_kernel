/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

//definition of the block size as 512, can be modified to higher value
#define TEMP_SIZE 512
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    
    fileSystem1 = _fs;
    fileId1 = _id;
    
    //identify the inode of the target file id and store it
    //modify the attributes
    iNode_id1 = fileSystem1->LookupFile(fileId1);
    iNode_id1->fs = fileSystem1;
    iNode_id1->fileLength1 = 0;
    
    //update current position by initializing it to 0
    currentPosition1 = 0;
    
    //read the file from the file's block and store in cache
    fileSystem1->disk->read(iNode_id1->blockNumber1,block_cache);
    
    //assert(false);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    
    //write onto the file if at all anything is stored in block cache
    fileSystem1->disk->write(iNode_id1->blockNumber1,block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    
    Console::puts("Current Position = "); 
    Console::puti(currentPosition1);
    Console::puts("\n");
    
    unsigned int temp_block_number = iNode_id1->blockNumber1;
    unsigned int temp_size = _n;
    int looper = 0;
    
    //read characters 1 by 1 by looping 
    while(looper < temp_size)
    {
    	//loop until eof is reached
    	if(!EoF())
    	{
    		
    		assert(currentPosition1 + looper < TEMP_SIZE);
    		//read the file and store it in the block cache
    		_buf[looper] = block_cache[currentPosition1];
    		//update current position of the header
    		currentPosition1++;
    	}
    	  	
    	looper++;
    }
    Console::puts("Reached End of Read. Well done!\n");
    
    return looper;
    //assert(false);
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    unsigned int temp_block_number = iNode_id1->blockNumber1;
    
    int looper = 0;
    //loop 1 by 1
    while(looper < _n)
    {
    	//buf is stored onto the block cache
    	assert(currentPosition1 + looper < TEMP_SIZE);
    	block_cache[currentPosition1] = _buf[looper];
    	currentPosition1++;//current position to be updated
    	
    	looper++;
    }
    //file length updated by adding the n to the file length which was initialized as 0
    iNode_id1->fileLength1+=_n;
    Console::puts("Reached End of Write. Well done!\n");
    return looper;
    
    assert(false);
}

void File::Reset() {
    Console::puts("resetting file\n");
    //set current position to 0
    currentPosition1 = 0;
    //assert(false);
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    
    //end of file is reached if current position is greater than or equal to 512
    if(currentPosition1 >= TEMP_SIZE)
    	return true;
    return false;
    //assert(false);
}
