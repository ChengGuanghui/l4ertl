#include <l4/sys/task.h>
#include <l4/re/env.h>
#include <l4/re/c/mem_alloc.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/util/util.h>

#include <stdio.h>
#include <sysmemory.h>
#include <config.h>
#include <tlsf.h>
#include <processor.h>
#include <l4/sysmem.h>

#include <string.h>
#include <stdio.h>
#include <sched.h>

#define SYSTEM_MEMORY_SIZE 8*1024*1024
#define MIN_SIZE 12
static unsigned int HEADER_SIZE;
static list_header_t list;

static int allocate_memory(void ** virt_addr, unsigned long virt_base,
        unsigned long size_in_bytes, unsigned long flags)
{
  int r;
  l4re_ds_t ds;

  /* Allocate a free capability index for our data space */
  ds = l4re_util_cap_alloc();
  if (l4_is_invalid_cap(ds))
    return -L4_ENOMEM;

  /* Allocate memory via a dataspace */
  if ((r = l4re_ma_alloc(size_in_bytes, ds, flags)))
  {
    printf("Memory allocation failed.\n");
    return r;
  }

  /* Make the dataspace visible in our address space */
  *virt_addr = (void *)virt_base;
  if ((r = l4re_rm_attach(virt_addr, size_in_bytes,
    L4RE_RM_SEARCH_ADDR, ds, 0, L4_PAGESHIFT)))
  {
    printf("Memory mapping failed.\n");
    return r;
  }
  
  /* Done, virtual address is in virt_addr */
  return 0;
}

//----------//
// init_vmm //
//----------//
extern void *vcpu_vmm_vbase;
extern unsigned long vcpu_vmm_size;

static unsigned long init_vmm(void) {

  //check the CONFIG_KDYNAMIC_MEMORY and the init_vmm()
  //make sure the there are enough memory space.
  //I think in this place we should make sure the system will have CONFIG_KDYNAMIC_MEMORY memory at least.
  //I don't know how much memory is CONFIG_KDYNAMIC_MEMORY
  //But in this step we should check the memory pool from the dm_phys component
  int vbase = 0x91000000;
  void * l4ertl_mm_addr;
  unsigned long l4ertl_mm_size;
  
  vcpu_vmm_vbase = (void *)vbase;

  l4ertl_mm_size= l4_round_page(SYSTEM_MEMORY_SIZE);
  vcpu_vmm_size = l4ertl_mm_size;

  //Allocate the memory which is  continous/pinned physical memory for the PaRTiKle
  if(allocate_memory(&l4ertl_mm_addr, vbase, l4ertl_mm_size, L4RE_MA_CONTINUOUS | L4RE_MA_PINNED) > 0)
  {
     printf("L4eRTL Memory Allocation failed.\n");
     return 0;
  }

  //Memory pool for PaRTiKle
  printf("\nSetting up the Memory Pool for PaRTiKle (%d kbytes between 0x%x and 0x%x)\n", SYSTEM_MEMORY_SIZE/1024, (unsigned int)l4ertl_mm_addr, (unsigned int)(l4ertl_mm_size + l4ertl_mm_addr - 1));

  list_t *r;

	{ // required by the list used to store the free memory
		list_t l;
		HEADER_SIZE = ((unsigned int)&l.mem.ptr[0] -
			(unsigned int) &l);
		list.head = NULL;
	}

	//l4ertl_mm_addr
	r = (list_t *)l4ertl_mm_addr;
	
	//l4ertl_mm_size
	r -> size = l4ertl_mm_size - HEADER_SIZE;
	SET_FREE_BLOCK (r);

	r -> mem.free_ptr.next = r -> mem.free_ptr.prev =
		r -> prev_phys = r -> next_phys = NULL;

	list.head = r;

	return r -> size;
}

//----------------//
// create_app_map //
//----------------//

void create_app_map (void) {
}

//------------//
// alloc_region //
//------------//
static void *alloc_region (unsigned long from, unsigned long to, unsigned long size) {
	list_t *aux, *pos = NULL, *new;
	unsigned int new_size;

	if (!size) return (void *) NULL;

	if (size < MIN_SIZE) size = MIN_SIZE;

	aux = list.head;

	for ( ; aux ;  aux = aux -> mem.free_ptr.next){
		if (aux->size >= size) {
			pos = aux;
          		break;
      		}
	}

	aux = pos;

	if (!aux) return (void *) NULL;

	if (aux -> mem.free_ptr.next)
		aux -> mem.free_ptr.next -> mem.free_ptr.prev = aux -> mem.free_ptr.prev;

	if (aux -> mem.free_ptr.prev)
		aux -> mem.free_ptr.prev -> mem.free_ptr.next = aux -> mem.free_ptr.next;

	if (list.head == aux)
		list.head = aux -> mem.free_ptr.next;

	SET_USED_BLOCK (aux);
	aux -> mem.free_ptr.next = NULL;
	aux -> mem.free_ptr.prev = NULL;

	new_size = GET_BLOCK_SIZE(aux) - size - HEADER_SIZE;
	if (((int) new_size) >= (int)MIN_SIZE) {
		new = (list_t *) (((char *) aux) + (unsigned long) HEADER_SIZE +
                      (unsigned long) size);
		new -> size = new_size;
		new -> mem.free_ptr.prev = NULL;
		new -> mem.free_ptr.next = NULL;

		SET_FREE_BLOCK (new);
		new -> prev_phys = aux;
		aux -> next_phys = new;

		new -> next_phys = aux -> next_phys;
		if (aux -> next_phys)
			aux -> next_phys -> prev_phys = new;

		aux -> size = size;
		SET_USED_BLOCK (aux);
		 // the new block is indexed inside of the list of free blocks
		new -> mem.free_ptr.next = list.head;
		if (list.head)
			list.head -> mem.free_ptr.prev = new;
		list.head = new;

	} 

	return (void *) &aux -> mem.ptr [0];
}


//----------------//
// dealloc_region //
//----------------//

static void dealloc_region (void *ptr, unsigned long size) {
	list_t *b = (list_t *) ((char *)ptr - HEADER_SIZE), *b2;

	if (!ptr || !IS_USED_BLOCK(b))
		return;

	SET_FREE_BLOCK (b);
	b -> mem.free_ptr.next = NULL;
	b -> mem.free_ptr.prev = NULL;
	if (b -> prev_phys) {
		b2 = b -> prev_phys;
		if (!IS_USED_BLOCK (b2)) {
			b2 -> size = GET_BLOCK_SIZE(b2) + GET_BLOCK_SIZE (b) + HEADER_SIZE;
			if (b2 -> mem.free_ptr.next)
				b2 -> mem.free_ptr.next -> mem.free_ptr.prev = b2 -> mem.free_ptr.prev;

			if (b2 -> mem.free_ptr.prev)
			b2 -> mem.free_ptr.prev -> mem.free_ptr.next = b2 -> mem.free_ptr.next;

			if (list.head == b2)
				list.head = b2 -> mem.free_ptr.next;
			SET_FREE_BLOCK (b2);
			b2 -> mem.free_ptr.next = NULL;
			b2 -> mem.free_ptr.prev = NULL;

			if (b -> next_phys)
				b -> next_phys -> prev_phys = b2;
			b2 -> next_phys = b -> next_phys;

			b = b2;
    		}
  	}

	if (b -> next_phys){
		b2 = b -> next_phys;

		if (!IS_USED_BLOCK (b2)) {
			b -> size += GET_BLOCK_SIZE(b2) + HEADER_SIZE;

			if (b2 -> mem.free_ptr.next)
				b2 -> mem.free_ptr.next -> mem.free_ptr.prev = b2 -> mem.free_ptr.prev;

			if (b2 -> mem.free_ptr.prev)
				b2 -> mem.free_ptr.prev -> mem.free_ptr.next = b2 -> mem.free_ptr.next;

			if (list.head == b2)
				list.head = b2 -> mem.free_ptr.next;

			b2 -> mem.free_ptr.next = NULL;
			b2 -> mem.free_ptr.prev = NULL;

			if (b2 -> next_phys)
			b2 -> next_phys -> prev_phys = b;
			b -> next_phys = b2 -> next_phys;
		}

		b -> mem.free_ptr.next = list.head;

		if (list.head)
			list.head -> mem.free_ptr.prev = b;
		list.head = b;
  	}
}

//------------//
// ualloc //
//------------//
#define VCPU_VMM_LOG2_SIZE 20
#define VCPU_VMM_SIZE 0x100000
extern l4_cap_idx_t vcpu_task;

asmlinkage void *ualloc_sys (int size) {
  
        //Because ualloc_sys needs to map the kernel memroy to user space. But in the L4 the page mapping
        //is based on 2^n PAGE. For memory saving the size of ualloc_sys is 1024*1024*1024 = 0x100000 = 1M
        l4_msgtag_t tag;
        void* vmm_vbase;
#if 1
        vmm_vbase = (char *)alloc_region(0, 0, VCPU_VMM_SIZE);
        printf("thread: 0x%x\n", (unsigned long)current_thread);
        printf("vmm_vbase = 0x%x\n", (unsigned int)vmm_vbase);
        vmm_vbase = (char *)(l4_trunc_page((unsigned long)vmm_vbase));
        memset(vmm_vbase, 0, VCPU_VMM_SIZE);
        tag = l4_task_map(vcpu_task, L4_BASE_TASK_CAP, l4_fpage((l4_addr_t)vmm_vbase, VCPU_VMM_LOG2_SIZE, L4_FPAGE_RWX), (l4_addr_t)(vmm_vbase));
        if(l4_error(tag))
          printf("VMM Mapping error. %s\n", l4sys_errtostr(l4_error(tag)));
        else
          printf("VMM Mapping at 0x%x size 0x%x\n", (unsigned int)vmm_vbase, VCPU_VMM_SIZE);

        return vmm_vbase;
#else
	return alloc_region (0, 0, size);
#endif
}

//-----------//
// ufree //
//-----------//
asmlinkage void ufree_sys (void *ptr, int size) {
	dealloc_region (ptr, (unsigned long)memory_pool);
}

//-------------//
// init_sysmem //
//-------------//

int init_sysmem () {
	unsigned long freemem = init_vmm ();

	if (CONFIG_KDYNAMIC_MEMORY > freemem) return -1;	
	
	if (!(memory_pool = alloc_region
        	(0, 0xffffffff, CONFIG_KDYNAMIC_MEMORY))) return -1;

	// Setting up TLSF with the largest free area, memory_pool will be
	// zeroed by this function as well
	printf ("\nSetting up the dynamic memory manager (%d kbytes at 0x%x)\n",
		CONFIG_KDYNAMIC_MEMORY/1024, 
		(unsigned int)memory_pool);
 
	if (init_memory_pool (CONFIG_KDYNAMIC_MEMORY, memory_pool) == 0) return -1;

	return freemem;
}
