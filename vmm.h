#ifndef VMM_H
#define VMM_H

#include "interface.h"

// Declare your own data structures and functions here...

typedef struct {
	struct v_Page *prev, *next;
	int frame_number;
	int offset;
	int protection_bit; //define later
	int modified; // 1 if write/store to a page
	int referenced; // 1 if read/write to a page
	int virt_page;
	void *v_mem_addr;

} v_Page;

typedef struct {
	int dirty_bit;
	int resident;
	int physical_page_number;

} Table_Element; 

typedef struct {
	int maxLength;
	int count;
	v_Page *head;
	v_Page *tail;
	enum policy_type pt;

} Table_Stack; 


void vmmu_init(int num_frames, Table_Stack *page_table);
v_Page* get_frame(int virt_page, Table_Stack *page_table);
int evict(int virt_page, Table_Stack *page_table);








#endif
