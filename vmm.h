#ifndef VMM_H
#define VMM_H

#include "interface.h"

// Declare your own data structures and functions here...


typedef struct {
	void *virtual_addr;
	int present;
} p_Frame;

typedef struct {
	void *physical_addr;
	int offset;
	int protection_bit; //define later
	int modified; // 1 if write/store to a page
	int referenced; // 1 if read/write to a page

} v_Page;

typedef struct {
	v_Page *first;
	v_Page *last;
	int count;

} v_Page_Table; 









#endif
