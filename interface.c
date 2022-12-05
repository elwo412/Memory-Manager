#include "interface.h"
#include "vmm.h"
#include <errno.h>

// Interface implementation
// Implement APIs here...

void *vm_ptr;
int PAGE_SIZE;

v_Page_Table pageTable;

void mm_init(enum policy_type policy, void *vm, int vm_size, int num_frames, int page_size)
{
	printf("DETAILS\npolicy type: %d\nStart of memory at address %p\nVM Size: %d\n# of Frames: %d\nPage Size: %d\n-----------------\n\n", policy, vm, vm_size, num_frames, page_size);
	vm_ptr = vm;
	PAGE_SIZE = page_size;

	printf("Page 1 ends at memory address %p\n", vm+page_size);
	mprotect(vm, vm_size,  PROT_NONE);
	struct sigaction sig_a;
	memset(&sig_a, 0, sizeof(struct sigaction));
	sigemptyset(&sig_a.sa_mask);
	sig_a.sa_sigaction = sigsegv_handler;
	sig_a.sa_flags = SA_SIGINFO | SA_ONSTACK;

	sigaction(SIGSEGV, &sig_a, NULL);
}

void sigsegv_handler(int signal, siginfo_t* siginfo, void* context){
	printf("Attempt to access memory at address %p\n", siginfo->si_addr);
	void *offset = (siginfo->si_addr - vm_ptr) % PAGE_SIZE;
	printf("Aligning to %p\n", siginfo->si_addr-offset);

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

	printf("\n\n");

	int page = ((void *)(siginfo->si_addr-offset)-(void *)vm_ptr)/PAGE_SIZE;
	int fault_type = bit_op;
	int evicted_page = -1;
	int write_back = 0;
	unsigned int phy_addr = (void *)(siginfo->si_addr)-(void *)vm_ptr;
	mm_logger(page, fault_type, evicted_page, write_back, phy_addr);
}









