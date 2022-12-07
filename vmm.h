#ifndef VMM_H
#define VMM_H

#include "interface.h"

// Declare your own data structures and functions here...

typedef struct {
	struct v_Page *prev, *next;
	int frame_number;
	int offset;
	int resident;
	int protection_bit; //define later
	int wb;
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
	int resident_count;
	v_Page *head; //head -> tail = resident pages
	v_Page *tail; //tail.next -> end = non-resident pages
	v_Page *end;
	enum policy_type pt;

} Table_Stack; 


void vmmu_init(int num_frames, Table_Stack *page_table, enum policy_type policy);
v_Page* get_frame(int virt_page, Table_Stack *g_page_map, int f_type);
int evict(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type);
int evict_FIFO(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type);
int evict_THIRD(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type);
v_Page* add_frame(int virt_page, Table_Stack *g_page_map, int f_type);
v_Page *create_page_entry(int wb, int frame_number, int v_page_num, int resident);








#endif
