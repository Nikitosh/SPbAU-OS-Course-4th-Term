#include "paging.h"
                   
#include "memory.h"
#include "buddy_allocator.h"
#include "memory_map.h"

void init_entry(pte_t *entry, uint32_t flags)
{
	if (pte_present(*entry))
		return;
   	*entry = pa(allocate_empty_page(0)) | flags;
}                          

void make_paging(uint64_t physical_address, uint64_t virtual_address, uint64_t size, pte_t *pml4)
{
	for (; size > 0; physical_address += BIG_PAGE_SIZE, virtual_address += BIG_PAGE_SIZE, size -= BIG_PAGE_SIZE)
	{
		pte_t *pml4e = (pte_t*) pml4 + pml4_i(virtual_address);
		init_entry(pml4e, PTE_PRESENT | PTE_WRITE);
		pte_t *pdpte = (pte_t*) va(pte_phys(*pml4e) << 12) + pml3_i(virtual_address);
		init_entry(pdpte, PTE_PRESENT | PTE_WRITE);
		pte_t *pde = (pte_t*) va(pte_phys(*pdpte) << 12) + pml2_i(virtual_address);
		*pde = physical_address | PTE_PRESENT | PTE_WRITE | PTE_LARGE;
		flush_tlb_addr(virtual_address);
	}
}

void init_after_canonical_hole_mapping(pte_t *pml4)
{
	make_paging(PHYSICAL_BASE, HIGH_BASE, get_memory_size(), pml4);
}
                                      	
void init_kernel_mapping(pte_t *pml4)
{
	make_paging(PHYSICAL_BASE, KERNEL_BASE, KERNEL_SIZE, pml4);
}

void init_paging()
{
	pte_t *pml4 = (pte_t*) allocate_empty_page(0);
	init_after_canonical_hole_mapping(pml4);
	init_kernel_mapping(pml4);
	store_pml4(pa(pml4));
}