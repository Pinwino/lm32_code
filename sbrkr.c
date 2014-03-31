#include <stdio.h>
#include <stdlib.h>
#include <pp-printf.h>
#define GGC_MIN_HEAPSIZE_DEFAULT 64
//#define align 8


// linker (standalone.ld) sets heap start and end
extern unsigned int _HEAP_START;
extern unsigned int _HEAP_END;
extern unsigned int aling;

static caddr_t heap = NULL;


// low level bulk memory allocator - used by malloc
caddr_t _sbrk ( int increment ) {
 
    caddr_t prevHeap;
    caddr_t nextHeap;
    
   // unsigned int  _HEAP_START;
	//unsigned int  _HEAP_END;

	//_HEAP_START = *_heap_start;// 0x00045628;
	//_HEAP_END = *_heap_end;  //0x00045628 + 0x000800;
    
    if (heap == NULL) {
        // first allocation
        pp_printf("first allocation\n");
        heap = (caddr_t)&_HEAP_START;
    }

    prevHeap = heap;
            
    // Always return data aligned on a 8 byte boundary 
   // nextHeap = (caddr_t)(((unsigned int)(heap + increment) + aling) & ~aling);
	nextHeap = (caddr_t)((unsigned int)(heap + increment));
    // get current stack pointer 
    register caddr_t stackPtr asm ("sp");
    pp_printf("_HEAP_START=0x%08x, _HEAP_END=0x%08x, stackPtr=0x%08x, nextHeap=0x%08x, prevHeap=0x%08x, increment=%d\n", &_HEAP_START, &_HEAP_END, stackPtr, nextHeap, prevHeap, increment);
    
    // Check enough space and there is no collision with stack coming the other way
    // if stack is above start of heap
    if ( (((caddr_t)&_HEAP_START < stackPtr) && (nextHeap > stackPtr)) || 
         (nextHeap >= (caddr_t)&_HEAP_END)) {
		pp_printf("error - no more memory\n");
		return NULL; // error - no more memory 
    } else {
        heap = nextHeap;
        return (caddr_t) prevHeap;    
    }    
}
