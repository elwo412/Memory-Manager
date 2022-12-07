#include "vmm.h"

// Memory Manager implementation
// Implement all other functions here...

v_Page *create_page_entry(int wb, int frame_number, int v_page_num, int resident){
	v_Page *frame = (v_Page *)malloc(sizeof(v_Page));
	frame->wb = wb;
	frame->modified = wb;
	frame->referenced = 1;
	frame->frame_number = frame_number;
	frame->virt_page = v_page_num;
	frame->resident = resident;
	frame->next = NULL;

	return frame;
}

void vmmu_init(int num_frames, Table_Stack *g_page_map, enum policy_type policy){
	//set up policy type logic, etc.
	g_page_map->resident_count = 0;
	g_page_map->maxLength = num_frames;
	g_page_map->pt = policy;
}

v_Page* get_frame(int virt_page, Table_Stack *g_page_map, int f_type, int *existing_rw){
	//if there are resident frames in the page map
	if (g_page_map->resident_count) {
		v_Page *frame;
		frame = g_page_map->head;
		
		// search for frame with virt_page number
		while (frame->next != NULL && frame->virt_page != virt_page) {
			frame = (v_Page *)frame->next;
		}
		//frame was not found in residents
		if (frame->next == NULL && frame->virt_page != virt_page){
			
			//must evict to add since out of physical mem
			if (g_page_map->resident_count == g_page_map->maxLength)
				return NULL;

			printf("ADDING FRAME\n");
			return add_frame(virt_page, g_page_map, f_type);
		}

		*existing_rw = 1;
		//frame was found
		frame->referenced = 1;
		if (f_type){
			frame->wb = f_type;
			frame->modified = f_type;
		}
		

		/*
		if (g_page_map->pt == MM_THIRD){ //NOT SURE IF CORRECT AT ALL
			//put frame back to tail
			if (frame == g_page_map->head){
				if (g_page_map->resident_count == 2) {
					g_page_map->head = frame->next;
					g_page_map->tail = frame;
					g_page_map->tail->next = NULL;
					g_page_map->head->next = frame;
				}
				else if (g_page_map->resident_count > 2){
					g_page_map->head = (v_Page *)g_page_map->head->next;
					v_Page *n_1 = frame;
					while (n_1->next != g_page_map->tail) n_1 = n_1->next;
					n_1->next = frame;
					g_page_map->tail = frame;
					g_page_map->tail->next = NULL;
				}
			}
			else {
				if (frame != g_page_map->tail){
					v_Page *n_1 = g_page_map->head;
					while (n_1->next != g_page_map->tail){
						if (n_1->next == frame) n_1->next = frame->next;
						n_1 = n_1->next;
					} 
					n_1->next = frame;
					g_page_map->tail = frame;
					g_page_map->tail->next = NULL;
				}
			}
		}
		*/

		return frame;
	}

	//completely empty linked list
	printf("Setting head to virt page:  %d\n", virt_page);
	v_Page *frame = (v_Page *)malloc(sizeof(v_Page));
	g_page_map->head = create_page_entry(f_type, 0, virt_page, 1);
	g_page_map->resident_count += 1;
	g_page_map->tail = g_page_map->head;
	return g_page_map->head;
}

v_Page* add_frame(int virt_page, Table_Stack *g_page_map, int f_type){

	// Create a new frame and allocate memory for it
	v_Page *new_frame = create_page_entry(f_type, g_page_map->resident_count, virt_page, 1);

	// If this is the first frame in the table stack
	if (g_page_map->resident_count == 1){

		g_page_map->head->next = (struct v_Page *)new_frame;
		g_page_map->tail = new_frame;
		//new_frame->prev = (struct v_Page *)g_page_map->head;
		new_frame->next = NULL;

		g_page_map->resident_count++;

		return new_frame;
	}

	//new_frame->prev = (struct v_Page *)g_page_map->tail;
	g_page_map->tail->next = (struct v_Page *)new_frame;
	g_page_map->tail = new_frame;
	g_page_map->tail->next = NULL;

	g_page_map->resident_count++;

	// Return the new frame (which is also the current tail)
	return g_page_map->tail;
}

int evict(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type){
	*page_buf = *g_page_map->head;
	if (g_page_map->head->next == NULL) {
		//If there exists only one frame
		v_Page *new_frame = create_page_entry(f_type, 0, virt_page, 1);
		g_page_map->head = new_frame;
		g_page_map->head->next = NULL;
		g_page_map->tail = g_page_map->head;
		printf("Evicted Virt Page:          %d\n", (*page_buf).virt_page);
		printf("Added Virt Page:            %d\n", virt_page);
		return (*page_buf).virt_page;
	}
	if (g_page_map->pt == MM_FIFO) evict_FIFO(page_buf, virt_page, g_page_map, f_type);
	else evict_THIRD(page_buf, virt_page, g_page_map, f_type);
}

int evict_FIFO(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type){

	//for frame counts > 1
	g_page_map->head = (v_Page *)g_page_map->head->next;
	//g_page_map->head->prev = NULL;
	
	v_Page *new_frame = create_page_entry(f_type, page_buf->frame_number, virt_page, 1);
	//new_frame->prev = (struct v_Page *)g_page_map->tail;
	g_page_map->tail->next = (struct v_Page *)new_frame;
	g_page_map->tail = new_frame;
	g_page_map->tail->next = NULL;

	return (*page_buf).virt_page;
	
}

v_Page *find_eviction_candidate(v_Page *cpage, Table_Stack *g_page_map){
	if (cpage == NULL) cpage = g_page_map->head;
	printf("r: %d | m: %d | w: %d | VIRT PAGE %d\n", cpage->referenced, cpage->modified, cpage->wb, cpage->virt_page);

	if (cpage->referenced == 0 && cpage->modified == 0 && cpage->wb == 0){
		return cpage;
	}
	if (cpage->referenced == 1 && cpage->modified == 0 && cpage->wb == 0){
		cpage->referenced = 0;
		mprotect(g_page_map->vm_ptr+cpage->virt_page*g_page_map->page_size, g_page_map->page_size, PROT_NONE);
		cpage = (v_Page *)cpage->next;
		return find_eviction_candidate(cpage, g_page_map);
	}
	if (cpage->referenced == 0 && cpage->modified == 1 && cpage->wb == 0){
		cpage->modified = 0;
		cpage = (v_Page *)cpage->next;
		return find_eviction_candidate(cpage, g_page_map);
	}
	if (cpage->referenced == 0 && cpage->modified == 0 && cpage->wb == 1){
		return cpage;
	}
	if (cpage->referenced == 1 && cpage->modified == 0 && cpage->wb == 1){
		cpage->referenced = 0;
		mprotect(g_page_map->vm_ptr+cpage->virt_page*g_page_map->page_size, g_page_map->page_size, PROT_NONE);
		cpage = (v_Page *)cpage->next;
		return find_eviction_candidate(cpage, g_page_map);
	}
	if (cpage->referenced == 0 && cpage->modified == 1 && cpage->wb == 1){
		cpage->modified = 0;
		cpage = (v_Page *)cpage->next;
		return find_eviction_candidate(cpage, g_page_map);
	}
	if (cpage->referenced == 1 && cpage->modified == 0 && cpage->wb == 1){
		cpage->referenced = 0;
		mprotect(g_page_map->vm_ptr+cpage->virt_page*g_page_map->page_size, g_page_map->page_size, PROT_NONE);
		cpage = (v_Page *)cpage->next;
		return find_eviction_candidate(cpage, g_page_map);
	}
	if (cpage->referenced == 1 && cpage->modified == 1 && cpage->wb == 1){
		cpage->referenced = 0;
		mprotect(g_page_map->vm_ptr+cpage->virt_page*g_page_map->page_size, g_page_map->page_size, PROT_NONE);
		cpage = (v_Page *)cpage->next;
		return find_eviction_candidate(cpage, g_page_map);

	}
	printf("r: %d | m: %d | w: %d\n", cpage->referenced, cpage->modified, cpage->wb);
	printf("ERROR ERROR\n");
	raise(SIGINT);
}

int evict_THIRD(v_Page *page_buf, int virt_page, Table_Stack *g_page_map, int f_type){
	
	//for frame counts > 1
	v_Page *found = NULL;
	v_Page *clock_head;

	/*
	//search for case where R=0 and M=0
	for (int i = 0; i < 3; i++) {
		clock_head = g_page_map->head;
		while (clock_head->next != NULL && (clock_head->referenced != 0 || clock_head->modified != 0)){
			if (clock_head->referenced){ clock_head->referenced = 0; mprotect(g_page_map->vm_ptr+clock_head->virt_page*g_page_map->page_size, g_page_map->page_size, PROT_NONE); printf("[DEBUG] Changed R=0 for v_p=%d (R=%d, M=%d)\n", clock_head->virt_page, clock_head->referenced, clock_head->modified);}
			else if (clock_head->modified && i > 0){ clock_head->modified = 0; printf("[DEBUG] Changed M=0 for v_p=%d (R=%d, M=%d)\n", clock_head->virt_page, clock_head->referenced, clock_head->modified);}
			

			clock_head = (v_Page *)clock_head->next;
		}
		printf("[DEBUG] PASS %d\n", i);
		if (!clock_head->referenced && !clock_head->modified) found = clock_head;
		if (found) break;
	}
	*/
	found = find_eviction_candidate(g_page_map->head, g_page_map);
	printf("[DEBUG] Found v_p=%d (R=%d, M=%d)\n", found->virt_page, found->referenced, found->modified);
	
	//evict found
	*page_buf = *found;
	if (found == g_page_map->head){
		g_page_map->head = (v_Page *)g_page_map->head->next;
	} else {
		clock_head = g_page_map->head;
		while ((v_Page *)clock_head->next != found){
			clock_head = (v_Page *)clock_head->next;
		}
		clock_head->next = found->next;
		if (found == g_page_map->tail) g_page_map->tail = clock_head;
	}
	
	v_Page *new_frame = create_page_entry(f_type, page_buf->frame_number, virt_page, 1);
	//new_frame->prev = (struct v_Page *)g_page_map->tail;
	g_page_map->tail->next = (struct v_Page *)new_frame;
	g_page_map->tail = new_frame;
	g_page_map->tail->next = NULL;

	return (*page_buf).virt_page;
	
}












