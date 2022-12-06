#include "vmm.h"

// Memory Manager implementation
// Implement all other functions here...

Table_Stack g_page_map;

void vmmu_init(int num_frames){
	//set up policy type logic, etc.
	g_page_map.maxLength = num_frames;
}

v_Page* get_frame(int virt_page){
	if (g_page_map.count) {
		v_Page *frame;
		frame = g_page_map.head;
		while (frame->next != NULL && frame->virt_page != virt_page) {
			printf("GOOD\n");
			frame = (v_Page *)frame->next;
		}
		if (frame->next == NULL){
			//frame was not found, thus evict
			return NULL;
		} else {
			return frame;
		}
	}
	else {
		//empty linked list
		v_Page frame;
		g_page_map.head = &frame;
		g_page_map.head->frame_number = 0;
		g_page_map.head->virt_page = virt_page;
		g_page_map.count += 1;
		printf("%d\n", g_page_map.head->frame_number);
		return g_page_map.head;
	}
}

int evict(int virt_page){
	//policy type conditional needed (FIFO for now)
	if (g_page_map.head->next == NULL) {
		v_Page frame;
		int evicted_v_page = g_page_map.head->virt_page;
		g_page_map.head = &frame;
		g_page_map.head->frame_number = 0;
		g_page_map.head->virt_page = virt_page;
		printf("RETURNING EVICTED VIRT PAGE: %d\n", evicted_v_page);
		return evicted_v_page;
	} else {
		//need to update for frame counts > 1
		g_page_map.head = (v_Page *)g_page_map.head->next;
		return -2;
	}
}












