#include <unistd.h>
#include <stdio.h>
#include "smalloc.h" 

sm_container_ptr sm_first = 0x0 ;
sm_container_ptr sm_last = 0x0 ;
sm_container_ptr sm_unused_containers = 0x0 ;

// Task 2-1
void sm_container_split(sm_container_ptr hole, size_t size)
{
	sm_container_ptr itr = 0x0 ;
	sm_container_ptr remainder = hole->data + size ;

	remainder->data = ((void *)remainder) + sizeof(sm_container_t) ;
	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;
	remainder->status = Unused ;
	remainder->next = hole->next ;
	remainder->next_unused = hole->next_unused;
	hole->next = remainder ;
	hole->next_unused = 0x0;

	if (sm_unused_containers == hole) {
		sm_unused_containers = remainder;
	} else {	
		for (itr = sm_unused_containers; itr->next != 0x0; itr = itr->next_unused) {
			if (itr->next_unused == hole) {
				itr->next_unused = remainder;
			}
		}
	}

	if (hole == sm_last)
		sm_last = remainder ;
}

void * sm_retain_more_memory(int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	if (hole == 0x0)
		return 0x0 ;

	hole->data = ((void *) hole) + sizeof(sm_container_t) ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	hole->status = Unused ;

	return hole ;
}

// Task 2-1
void * smalloc(size_t size) 
{
	sm_container_ptr hole = 0x0 ;

	sm_container_ptr itr = 0x0 ;
	sm_container_ptr itr2 = 0x0 ;
	for (itr = sm_unused_containers; itr != 0x0 ; itr = itr->next_unused) {
		if (size == itr->dsize) {
			if (itr == sm_unused_containers) {
				sm_unused_containers = itr->next_unused;
				itr->next_unused = 0x0;
			} else {
				for (itr2 = sm_unused_containers; itr2 != 0x0; itr2->next_unused) {
					if (itr2->next_unused == itr) {
						itr2->next_unused = itr->next_unused;
						itr->next_unused = 0x0;
					}
				}
			}
			itr->status = Busy;
			return itr->data ;
		}
		else if (size + sizeof(sm_container_t) < itr->dsize) {
			if (hole != 0x0 && hole->dsize > itr->dsize)
				hole = itr;
			else if (hole == 0x0)
				hole = itr;
		}
	}
	if (hole == 0x0) {
		hole = sm_retain_more_memory(size) ;

		if (hole == 0x0)
			return 0x0 ;

		if (sm_first == 0x0) {
			sm_first = hole ;
			sm_last = hole ;
			sm_unused_containers = hole;
			hole->next = 0x0 ;
			hole->next_unused = 0x0;
		}
		else {
			sm_last->next = hole ;
			sm_last = hole ;
			for (itr = sm_unused_containers; itr != 0x0; itr = itr->next_unused) {
				if (itr->next_unused == 0x0) {
					itr->next_unused = hole;
					break;
				}
			}
			hole->next = 0x0 ;
			hole->next_unused = 0x0;
		}
	}
	sm_container_split(hole, size) ;
	hole->dsize = size ;
	hole->status = Busy ;
	return hole->data ;
}


// Task 2-1
void sfree(void * p)
{
	sm_container_ptr itr ;
	sm_container_ptr itr_unused = 0x0 ;
	sm_container_ptr free ;
	int unused_flag = 0;
	int free_flag = 0;
	for (itr = sm_first ; itr->next != 0x0 ; itr = itr->next) {
		if (itr == sm_unused_containers) {
			if (free_flag == 1) {
				free->next_unused = sm_unused_containers;
				sm_unused_containers = free;
				break;
			}
			unused_flag = 1;
			itr_unused = itr;
		}
		if (itr_unused != 0x0 && itr == itr_unused->next_unused) itr_unused = itr;
		if (itr->data == p) {
			free = itr;
			itr->status = Unused ;
			free_flag = 1;
			if (unused_flag == 1) {
				free->next_unused = itr_unused->next_unused;
				itr_unused->next_unused = free;
				break;
			}
		}
	}
	
	// Task 2-2
	for (itr = sm_first ; itr->next != 0x0 ; itr = itr->next) {
		if (itr->next_unused == itr->next) {
			itr->dsize += itr->next->dsize + sizeof(sm_container_t);
			itr->next_unused = itr->next->next_unused;
			itr->next = itr->next->next;
		}
		if (itr->next == 0x0) break;
	}
}

void print_sm_containers()
{
	sm_container_ptr itr ;
	int i = 0 ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_first ; itr != 0x0 ; itr = itr->next, i++) {
		char * s ;
		printf("%3d:%p:%s:", i, itr->data, itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		for (s = (char *) itr->data ;
			 s < (char *) itr->data + (itr->dsize > 8 ? 8 : itr->dsize) ;
			 s++) 
			printf("%02x ", *s) ;
		printf("\n") ;
	}
	printf("=======================================================\n") ;

}

void print_sm_uses() {
	sm_container_ptr itr;
	int i = 0;
	size_t total_size = 0;
	size_t busy_size = 0;
	size_t unused_size = 0;

	for (itr = sm_first; itr != 0x0; itr = itr->next, i++) {
		char * s;
		total_size += itr->dsize;
		if (itr->status == Unused)
			unused_size += itr->dsize;
		else
			busy_size += itr->dsize;
	}

	fprintf(stderr, "Total memory retained by smalloc so far : %d\n", (int) total_size);
	fprintf(stderr, "Total memory allocated by smalloc at this moment : %d\n", (int) busy_size);
	fprintf(stderr, "Total memory retained by smalloc but not currently allocated : %d\n", (int) unused_size);
}
