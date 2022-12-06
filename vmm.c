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
			printf("VIRT PAGE #: %d\n", frame->virt_page);
			frame = (v_Page *)frame->next;
		}
		printf("VIRT PAGE #: %d\n", frame->virt_page);
		//frame was not found in residents
		if (frame->next == NULL && frame->virt_page != virt_page){
			
			//must evict to add since out of physical mem
			if (g_page_map->resident_count == g_page_map->maxLength)
				return NULL;

			printf("ADDING FRAME\n");
			return add_frame(virt_page, g_page_map, f_type);
		}

		//frame was found
		frame->modified = f_type;
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

v_Page* add_frame(int virt_page, Table_Stack *g_page_map, int f_type){

	v_Page *new_frame = (v_Page *)malloc(sizeof(v_Page));
	new_frame->virt_page = virt_page;
	new_frame->frame_number = g_page_map->resident_count;
	new_frame->modified = f_type;
	new_frame->resident = 1;

	if (g_page_map->resident_count == 1){

		g_page_map->head->next = (struct v_Page *)new_frame;
		g_page_map->tail = new_frame;
		new_frame->prev = (struct v_Page *)g_page_map->head;
		new_frame->next = NULL;

		g_page_map->resident_count++;

		return new_frame;
	}

	new_frame->prev = (struct v_Page *)g_page_map->tail;
	g_page_map->tail->next = (struct v_Page *)new_frame;
	g_page_map->tail = new_frame;
	g_page_map->tail->next = NULL;

	g_page_map->resident_count++;


	return g_page_map->tail;
}

int evict(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type){
	//policy type conditional needed (FIFO for now)
	if (g_page_map->head->next == NULL) {
		v_Page *new_frame = (v_Page *)malloc(sizeof(v_Page));
		*page_buf = *g_page_map->head;
		g_page_map->head = new_frame;
		g_page_map->head->frame_number = 0;
		g_page_map->head->virt_page = virt_page;
		g_page_map->head->modified = f_type;
		g_page_map->head->resident = 1;
		g_page_map->head->next = NULL;
		g_page_map->tail = g_page_map->head;
		printf("Evicted Virt Page:          %d\n", (*page_buf).virt_page);
		printf("Added Virt Page:            %d\n", virt_page);
		return (*page_buf).virt_page;
	} else {
		//for frame counts > 1
		*page_buf = *g_page_map->head;
		g_page_map->head = (v_Page *)g_page_map->head->next;
		g_page_map->head->prev = NULL;

		//free(g_page_map->head);
		
		printf("GOOD HERE\n");
		v_Page *new_frame = (v_Page *)malloc(sizeof(v_Page));
		if (new_frame == NULL) perror("Unable to allocate memory\n");
		new_frame->virt_page = virt_page;
		new_frame->frame_number = page_buf->frame_number;
		new_frame->modified = f_type;
		new_frame->resident = 1;

		new_frame->prev = (struct v_Page *)g_page_map->tail;
		g_page_map->tail->next = (struct v_Page *)new_frame;
		g_page_map->tail = new_frame;
		g_page_map->tail->next = NULL;

		return (*page_buf).virt_page;
	}
}












