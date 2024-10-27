#include <stdint.h>
#include <limine.h>
#include <pmm.h>
#include <vmm.h>

extern uint8_t kernel_size[];
extern struct limine_memmap_response *memmap_response;
extern uint64_t hhdm_offset;
extern uint64_t kernel_phys_addr;
extern uint64_t kernel_virt_addr;

static uint64_t *pml4 = NULL;
static struct vmm_region_t *head = NULL;

void load_cr3(void *val) {
    asm volatile (
        "movq %0, %%cr3"
        : 
        : "r" ((uintptr_t) val)
        : "memory"
    );
}

uint64_t *next_table_addr_new(uint64_t *table, size_t index, uint8_t flags) {
    if (!(table[index] & VMM_TABLE_ENTRY_PRESENT)) {
        uint64_t *new_table = pmm_alloc();
        memset(VMM_PTRS_PHYS_TO_VIRT(new_table), 0, PAGE_SIZE);

        table[index] |= (uintptr_t) new_table;
    }

    table[index] |= flags;
    return (uint64_t *) (table[index] & VMM_ADDR_MASK);
}

void map(uint64_t virt_addr, uint64_t phys_addr, uint8_t flags) {
    uint64_t lvl4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t lvl3_index = (virt_addr >> 30) & 0x1FF;
    uint64_t lvl2_index = (virt_addr >> 21) & 0x1FF;
    uint64_t lvl1_index = (virt_addr >> 12) & 0x1FF;

    uint64_t *cur_table = next_table_addr_new(VMM_PTRS_PHYS_TO_VIRT(pml4), lvl4_index, flags);
    cur_table = next_table_addr_new(VMM_PTRS_PHYS_TO_VIRT(cur_table), lvl3_index, flags);
    cur_table = next_table_addr_new(VMM_PTRS_PHYS_TO_VIRT(cur_table), lvl2_index, flags);

    VMM_PTRS_PHYS_TO_VIRT(cur_table)[lvl1_index] = phys_addr | flags;
}

uint64_t *next_table_addr(uint64_t *table, size_t index) {
    if (table != NULL && (table[index] & VMM_TABLE_ENTRY_PRESENT) && (table[index] & VMM_TABLE_ENTRY_READ_WRITE)) {
        return (uint64_t *) (table[index] & VMM_ADDR_MASK);
    }

    return NULL;
}

uint64_t unmap(uint64_t addr) {
    uint64_t lvl4_index = (addr >> 39) & 0x1FF;
    uint64_t lvl3_index = (addr >> 30) & 0x1FF;
    uint64_t lvl2_index = (addr >> 21) & 0x1FF;
    uint64_t lvl1_index = (addr >> 12) & 0x1FF;

    uint64_t *cur_table = next_table_addr(VMM_PTRS_PHYS_TO_VIRT(pml4), lvl4_index);

    if (cur_table == NULL) {
        return 0;
    }

    cur_table = next_table_addr(VMM_PTRS_PHYS_TO_VIRT(cur_table), lvl3_index);

    if (cur_table == NULL) {
        return 0;
    }

    cur_table = next_table_addr(VMM_PTRS_PHYS_TO_VIRT(cur_table), lvl2_index);

    if (cur_table == NULL) {
        return 0;
    }

    uint64_t ret = VMM_PTRS_PHYS_TO_VIRT(cur_table)[lvl1_index];

    VMM_PTRS_PHYS_TO_VIRT(cur_table)[lvl1_index] = 0;
    asm volatile (
        "invlpg (%0)"
        : 
        : "r" (addr)
        : "memory"
    );

    return ret;
}

void *vmm_alloc(size_t num_bytes, uint8_t flags) {
    size_t aligned_num_bytes = VMM_PAGE_ALIGN_UP(num_bytes);

    struct vmm_region_t *prev_node = head;
    struct vmm_region_t *cur_node = head->next;

    while (cur_node != NULL) {
        if (prev_node->base + aligned_num_bytes < cur_node->base) {
            struct vmm_region_t *this_node = VMM_ALLOC_PHYS_TO_VIRT(pmm_alloc());
            this_node->base = prev_node->base;
            this_node->length = aligned_num_bytes;
            this_node->flags = flags;

            prev_node->next = this_node;
            this_node->next = cur_node;

            cur_node->base = prev_node->base + aligned_num_bytes;
            
            for (size_t i = 0; i < aligned_num_bytes; i += PAGE_SIZE) {
                map(this_node->base + i, (uintptr_t) pmm_alloc(), VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
            }

            return (void *) this_node->base;
        }

        prev_node = cur_node;
        cur_node = cur_node->next;
    }

    kpanic();
    __builtin_unreachable();
}

void vmm_free(void *addr) {
    struct vmm_region_t *prev_node = head;
    struct vmm_region_t *cur_node = head->next;

    while (cur_node != NULL) {
        if (cur_node->base == (uintptr_t) addr) {
            struct vmm_region_t *next_node = cur_node->next;

            prev_node->length += cur_node->length;
            prev_node->next = next_node;

            for (size_t i = 0; i < cur_node->length; i += PAGE_SIZE) {
                uint64_t phys_addr = unmap(cur_node->base + i);
                pmm_free((void *) phys_addr);
            }

            return;
        }

        prev_node = cur_node;
        cur_node = cur_node->next;
    }

    kpanic();
}

void init_vmm_regions() {
    head = VMM_ALLOC_PHYS_TO_VIRT(pmm_alloc());
    head->base = hhdm_offset;
    head->length = ((size_t) kernel_size) - hhdm_offset;
    head->flags = VMM_REGION_EXEC | VMM_REGION_WRITE;
    
    struct vmm_region_t *kernel_node = VMM_ALLOC_PHYS_TO_VIRT(pmm_alloc());
    kernel_node->base = kernel_virt_addr;
    kernel_node->length = (size_t) kernel_size;
    kernel_node->flags = VMM_REGION_EXEC | VMM_REGION_WRITE;
    head->next = kernel_node;
}

void init_vmm() {
    pml4 = pmm_alloc();
    memset(VMM_PTRS_PHYS_TO_VIRT(pml4), 0, PAGE_SIZE);

    for (size_t i = 0; i < memmap_response->entry_count; ++i) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        for (size_t page_index = 0; page_index < VMM_PAGE_ALIGN_UP(entry->length); page_index += PAGE_SIZE) {
            map(VMM_PHYS_TO_VIRT(VMM_PAGE_ALIGN_DOWN(entry->base) + page_index), VMM_PAGE_ALIGN_DOWN(entry->base) + page_index, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
        }
    }

    for (size_t i = 0; i < VMM_PAGE_ALIGN_UP((size_t) kernel_size); i += PAGE_SIZE) {
        map(VMM_PAGE_ALIGN_DOWN(kernel_virt_addr) + i, VMM_PAGE_ALIGN_DOWN(kernel_phys_addr) + i, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
    }

    load_cr3(pml4);
    init_vmm_regions();
}