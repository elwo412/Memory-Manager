#include "interface.h"
#include "vmm.h"
#include <errno.h>

// Interface implementation
// Implement APIs here...

void *vm_ptr;
int PAGE_SIZE;
Table_Stack *page_table;

void mm_init(enum policy_type policy, void *vm, int vm_size, int num_frames, int page_size)
{
	printf("DETAILS\npolicy type: %d\nStart of memory at address %p\nVM Size: %d\n# of Frames: %d\nPage Size: %d\n-----------------\n\n", policy, vm, vm_size, num_frames, page_size);
	vm_ptr = vm;
	PAGE_SIZE = page_size;

	page_table = (Table_Stack *)malloc(sizeof(Table_Stack));

	vmmu_init(num_frames, page_table, policy);

	//Setting up memory protections
	//printf("Page 1 ends at memory address %p\n", vm+page_size);
	mprotect(vm, vm_size,  PROT_NONE);
	struct sigaction sig_a;
	memset(&sig_a, 0, sizeof(struct sigaction));
	sigemptyset(&sig_a.sa_mask);
	sig_a.sa_sigaction = sigsegv_handler;
	sig_a.sa_flags = SA_SIGINFO | SA_ONSTACK;

	sigaction(SIGSEGV, &sig_a, NULL);
}

void sigsegv_handler(int signal, siginfo_t* siginfo, void* context){
	printf("Virtual mem access attempt: %p\n", siginfo->si_addr);
	void *offset = (void *)((siginfo->si_addr - vm_ptr) % PAGE_SIZE);
	void *v_mem_start = (void *)(siginfo->si_addr-offset);
	int virt_page = (v_mem_start-(void *)vm_ptr)/PAGE_SIZE;
	printf("Virtual mem page:           %p\n", v_mem_start);
	printf("Virtual page #:             %d\n", virt_page);

	//decode whether read or write operation
	int fault_type = (((ucontext_t *)context)->uc_mcontext.gregs[19] & 2) / 2; // 0 if read | 1 if write
	printf("[DEBUG] ORIGINAL FAULT TYPE: %lld   (aka take-out later)\n", ((ucontext_t *)context)->uc_mcontext.gregs[19]);

	//find physical frame
	v_Page *phys_frame = get_frame(virt_page, page_table, fault_type);
	void *phy_addr;
	int evicted_page = -1;
	int write_back = 0;

	if (phys_frame == NULL) {
		printf("Physical Frame Not Found\n");
		v_Page page_buf;
		evict(&page_buf, virt_page, page_table, fault_type);
		evicted_page = page_buf.virt_page;
		write_back = page_buf.wb;
		printf("Evicted Virtual Page #:     %d\n", evicted_page);
		void *evicted_page_mem = vm_ptr + evicted_page*PAGE_SIZE;
		mprotect(evicted_page_mem, PAGE_SIZE, PROT_NONE);
		phy_addr = (page_table->tail->frame_number)*PAGE_SIZE+offset;
	}
	else {
		phy_addr = (phys_frame->frame_number)*PAGE_SIZE+offset;
		printf("Physical Frame Loc: %p\n", phy_addr);
	}

	int ret;
	if (fault_type) ret = mprotect(v_mem_start, PAGE_SIZE, PROT_WRITE); // PROT_NONE | PROT_READ ?
	else ret = mprotect(v_mem_start, PAGE_SIZE, PROT_READ);
	if (ret < 0){
		perror("mprotect failed with error:");
		printf("More info: failed with error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	fault_type += ((ucontext_t *)context)->uc_mcontext.gregs[19] & 1;
	printf("Fault Value = %d\n", fault_type);

#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
	mm_logger(virt_page, fault_type, evicted_page, write_back, (int)phy_addr);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"

	/*
	unsigned int phy_addr;
	int evicted_page = -1;
	if (phys_frame == NULL) {
		//need to evict a frame
		//evict_method will return frame number evicted, so:
		//evicted_page = frame_number
		//phy_addr = frame_number * PAGE_SIZE + offset
		evicted_page = evict(virt_page);
		phy_addr = evicted_page*PAGE_SIZE + offset;
		printf("RETURNED NULL");
	}
	else phy_addr = (phys_frame->frame_number)*PAGE_SIZE+offset;


	printf("Physical_addr: %p", phy_addr);

	//v_Page page;
	//page.physical_addr = siginfo->si_addr-offset;

	int bit_op = (((ucontext_t *)context)->uc_mcontext.gregs[19] & 2) / 2; // 0 if read | 1 if write
	printf("\nREAD?Write: %d\n", bit_op);
	//raise(SIGINT);

	//query the size requested
	int ret;
	if (bit_op) ret = mprotect(siginfo->si_addr-offset, PAGE_SIZE, PROT_NONE | PROT_WRITE);
	else ret = mprotect(siginfo->si_addr-offset, PAGE_SIZE, PROT_NONE | PROT_WRITE);
	if (ret < 0){
		perror("mprotect failed with error:");
		printf("More info: failed with error %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int fault_type = bit_op;
	int write_back = 0;
	//memset(phy_addr, 1, sizeof(int));
	mm_logger(virt_page, fault_type, evicted_page, write_back, phy_addr);
	*/
}









