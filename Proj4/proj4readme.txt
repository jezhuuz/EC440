EC440 Operating Systems
Project 4 - Thread Local Storage
README

Project goal: understand memory management and provide protected memory regions for threads.
	- implement a library that provides protected memory regions for threads 
	- threads share same memory address space, so we need a way to protect data from being overwritten by other threads
	- have each thread possess a protected storage area that only this thread can R/W
			---> local storage


int tls_create (unsigned int size): creates a local storage area (LSA) for the current executing thread
	- size is the amount of bytes it can hold
	- returns 0 on success, -1 (error) if thread already has a local storage (size > 0 bytes)

int tls_write(unsigned int offset, unsigned int length, char *buffer): reads length bytes, starting from memory location pointed to by buffer,
	and writes to local storage area of current executing thread, starting at position offset
	- returns 0 on success, -1 (error) if function is asked to write more data than the LSA can hold
		(offset + length > size of LSA) or if current thread has no LSA

int tls_read(unsigned int offset, unsigned int length, char *buffer): reads length bytes from local storage area of currently executing thread,
	starting at position offset, and writes them into the memory location pointed to by buffer
	- returns 0 on success, -1 (error) if it is asked to read past th end of the LSA 
		(offset + length > size of LSA) or if current thread has no LSA

int tls_destroy(): function frees a previously allocated local storage area of the currently executing thread
	- returns 0 on success, -1 (error) when thread does not have a local storage area

int tls_clone(pthread_t tid): clones local storage area of a target thread identified by tid
	- when tls is cloned, content is NOT copied
		- storage areas of both threads initially refer to same memory location
	- only when one thread writes to its own LSA, then the TLS library creates a private copy of the region that is written
		- CoW (copy-on-write): save memory space and avoid unnecessary copy operations
	- returns - on success, -1 (error) when target thread has no LSA or current executing thread already has a LSA

***when a thread attempts to read from or write to any thread local storage area without using appropriate tls_read and tls_write functions,
	then this thread should be terminated (by calling pthread_exit on its behalf). Remaining threads continue to run unaffected.***


1) whenever a thread calls tls_read or tls_write, temporarily unprotect this thread's local storage area
	- when thread A is executing read or write, B can intterupt thread A to access A's local storage without A being terminated

2) relax sharing requirement for tls_clone
	- B clones local storage of thread B (size 2*page_size (page_size = 4096 bytes and can be edtermined by calling getpagesize())
	- thread A writes one byte at the beginning of its own local storage 
	- allow the entire first page (first 4096 of the local storage) to be copied
	- second page still remains shared between thread A and B

- possible that more than two threads share the same local storage
	- multiple threads can tls_clone the LSA of the same target thread and these threads
		would then point to the same memory region (pages)
		- when one thread writes to its storage, only this thread gets its own copy	
		- remaining threads would still share the same region


