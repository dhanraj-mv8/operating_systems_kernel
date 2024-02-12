/*
 File: ContFramePool.C
 
 Author: Dhanraj Murali - 734003894
 Date  : 12-Sep-2023
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
ContFramePool* ContFramePool::head;
ContFramePool* ContFramePool::prev;
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

//#define FRAME_SIZE 4096

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/


ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no) {
    // we use 2 bits to save state of frame, so 1 byte can store frame state information of upto 8/2=4 frames, so we divide frame_no by 4 identify which byte the given frame's info is stored  
    unsigned int bitmap_index = _frame_no / 4;
    unsigned char mask = 0x3 << (_frame_no%4)*2;
    //right shift 
    unsigned char check = (bitmap[bitmap_index] & mask)>>(_frame_no%4)*2;

	/* 
	 * change bitmap[bitmap_index] to denote 2 bits
	 * Free == 01
	 * Used == 10
	 * HoS == 11
	*/
	    
    if(check == 0x3)
	    return FrameState::HoS;
    else if(check == 0x2)
	    return FrameState::Used;
    else if(check == 0x1)
	    return FrameState::Free;
    else
    {
	    Console::puts("State not valid");
	    assert(false);
    }
    return FrameState::Used;
}


void ContFramePool::set_state(unsigned long _frame_no, FrameState _state) {
	unsigned int bitmap_index = _frame_no / 4;
	unsigned int shift =( _frame_no%4)*2;
	unsigned char mask = 0x3 << shift;
	/* 
	 * change bitmap[bitmap_index] to denote 2 bits
	 * Free == 11
	 * Used == 01
	 * HoS == 10
	*/
 	switch(_state) {
   	case FrameState::Free:
 	bitmap[bitmap_index] = (bitmap[bitmap_index] & (~mask)) | (0x1<<shift);
      	break;
	case FrameState::Used:
        bitmap[bitmap_index] = (bitmap[bitmap_index] & (~mask)) | (0x2<<shift);
        break;
	case FrameState::HoS:
	bitmap[bitmap_index] = (bitmap[bitmap_index] & (~mask)) | (0x3<<shift);
        break;
    }
}







ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    // Bitmap must fit in a single frame
    //assert(_n_frames < FRAME_SIZE * 4);
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    

    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }
    
    // Everything ok. Proceed to mark all frame as free.
    for(int fno = 0; fno < _n_frames; fno++) {
        set_state(fno, FrameState::Free);
    }
    
    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
        set_state(0, FrameState::Used);
        nFreeFrames--;
    }

    /* Linked List update for creating frame pools for user, system and kernel*/
    //ContFramePool *temp;
    if(head == NULL)
    {
	    head  = this;
	    prev = NULL;
    }
    else
    {
	    prev->next = this;
	    prev = this;
    }
    next = NULL;
    Console::puts("Frame Pool Initialized");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
	assert(nFreeFrames >0);
	unsigned int frame_no = 0;
	int _free_frame_count =0;
	int success_flag=0;
	
	//iterate through the bitmap until you find the first free frame, then enter a 2nd loop to find if there are contiguous free frames of size nframes. if such frame series is found then mark flag as 1 and return the frame no in terms of the physical memory.
 	for(unsigned int f = 0; f<nframes; f++)
	{
		if(get_state(f)==FrameState::Free)
		{
			frame_no=f;
			_free_frame_count=0;
			for(unsigned int h = 0;h < _n_frames;h++)
			{
				if(get_state(h+frame_no) ==FrameState::Free && h+frame_no<nframes)
				{
					_free_frame_count++;
					//break;
				}
			}
				if(_free_frame_count==_n_frames)
				{
					success_flag=1;
					break;
				}
			}
		}
		//check for success flag, if 1 then mark the first free frame from above as HoS and rest as used, if not then throw output.
		if(success_flag==1)
		{
			set_state(frame_no,FrameState::HoS);
			for(int m=frame_no+1;m<_n_frames+frame_no;m++)
			{
				set_state(m,FrameState::Used);
			}
			nFreeFrames-=_n_frames;
		}
		else
			Console::puts("Oops! Unable to provide required frames due to shortage!");
		return frame_no+base_frame_no;

}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
	//assert to check if the given input base frame belongs to the frame pool by comparing it with the first and last frame numbers.
	assert((_base_frame_no>base_frame_no) && ((_base_frame_no+_n_frames)<=(base_frame_no+nframes)));
	//if assertion is successful then proceed to mark first frame as HoS and rest as used
    	for(int fno = _base_frame_no; fno < _base_frame_no + _n_frames; fno++){
    		if(fno==_base_frame_no)
    			set_state(fno - base_frame_no, FrameState::HoS);
        	else
        		set_state(fno - base_frame_no, FrameState::Used);

}
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
	ContFramePool *obj1 = head; 
	
	//find the frame pool of the provided frame by comparing the frame number with the first and last frame numbers of the considered pool. if not then traverse the list until u reach.
	while(!(_first_frame_no>=obj1->base_frame_no && _first_frame_no < obj1->base_frame_no+obj1->nframes))
	{
		if(obj1->next != NULL)
		obj1 = obj1 -> next;
	}
	//assert to check if the above loop does not provide a frame pool object, meaning if the given frame num doesnt belong to any frame pool
	assert(obj1!=NULL)
	
	//bitmap index starts from 0, so subtract frame num from base frame no of the pool to identify bitmap index and then set state.
	unsigned long tmpframeno = _first_frame_no - obj1->base_frame_no;
	while(obj1->get_state(tmpframeno) != FrameState::Free)
	{
		obj1 -> set_state(tmpframeno, FrameState::Free);
                tmpframeno++;
                obj1 -> nFreeFrames++;
	}
}




unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	
	int n = FRAME_SIZE * 4;
	return(_n_frames/ n + (_n_frames % n > 0 ? 1 : 0));
}
