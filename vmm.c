#include "vmm.h"

// Memory Manager implementation
// Implement all other functions here...

void vmmu_init(int num_frames, Table_Stack *g_page_map){
	//set up policy type logic, etc.
	g_page_map->resident_count = 0;
	g_page_map->maxLength = num_frames;
}

v_Page* get_frame(int virt_page, Table_Stack *g_page_map, int f_type){
	if (g_page_map->resident_count) {
		v_Page *frame;
		frame = g_page_map->head;
		
		// search for frame with virt_page number
		while (frame->next != NULL && frame->virt_page != virt_page) {
			frame = (v_Page *)frame->next;
		}

		if (frame->next == NULL){
			//frame was not found, thus evict
			return NULL;
		}

		//frame was found
		return frame;
	}

	//completely empty linked list
	printf("Setting head to virt page:  %d\n", virt_page);
	v_Page *frame = g_page_map->head;
	frame->frame_number = 0;
	frame->virt_page = virt_page;
	frame->resident = 1;
	frame->next = NULL;
	frame->modified = f_type;
	g_page_map->resident_count += 1;
	g_page_map->tail = g_page_map->head;
	g_page_map->end = NULL;
	return g_page_map->head;
}

int evict(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type){
	//policy type conditional needed (FIFO for now)
	if (g_page_map->head->next == NULL) {
		v_Page frame;
		*page_buf = *g_page_map->head;
		*g_page_map->head = frame;
		g_page_map->head->frame_number = 0;
		g_page_map->head->virt_page = virt_page;
		g_page_map->head->modified = f_type;
		g_page_map->head->next = NULL;
		g_page_map->tail = g_page_map->head;
		printf("Evicted Virt Page:          %d\n", (*page_buf).virt_page);
		printf("Added Virt Page:            %d\n", virt_page);
		return (*page_buf).virt_page;
	} else {
		//need to update for frame counts > 1
		printf("ERROR\n");
		raise(SIGINT);
		g_page_map->head = (v_Page *)g_page_map->head->next;
		return -2;
	}
}












