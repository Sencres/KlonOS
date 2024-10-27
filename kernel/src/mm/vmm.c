#include <stdint.h>
#include <limine.h>
#include <pmm.h>
#include <vmm.h>

extern uint8_t kernel_size[];
extern struct limine_memmap_response *memmap_response;
extern uint64_t hhdm_offset;
extern uint64_t kernel_phys_addr;
extern uint64_t kernel_virt_addr;

static uint64_t *pml4;

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

        table[index] = ((uintptr_t) new_table) | flags;
    }

    return (uint64_t *) (table[index] & ~0xFFF);
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
        return (uint64_t *) (table[index] & ~0xFFF);
    }

    return NULL;
}

void unmap(uint64_t addr) {
    uint64_t lvl4_index = (addr >> 39) & 0x1FF;
    uint64_t lvl3_index = (addr >> 30) & 0x1FF;
    uint64_t lvl2_index = (addr >> 21) & 0x1FF;
    uint64_t lvl1_index = (addr >> 12) & 0x1FF;

    uint64_t *cur_table = next_table_addr(VMM_PTRS_PHYS_TO_VIRT(pml4), lvl4_index);

    if (cur_table == NULL) {
        return;
    }

    cur_table = next_table_addr(VMM_PTRS_PHYS_TO_VIRT(cur_table), lvl3_index);

    if (cur_table == NULL) {
        return;
    }

    cur_table = next_table_addr(VMM_PTRS_PHYS_TO_VIRT(cur_table), lvl2_index);

    if (cur_table == NULL) {
        return;
    }

    VMM_PTRS_PHYS_TO_VIRT(cur_table)[lvl1_index] = 0;
    asm volatile (
        "invlpg (%0)"
        : 
        : "r" (addr)
        : "memory"
    );
}

void init_vmm() {
    pml4 = pmm_alloc();
    memset(VMM_PTRS_PHYS_TO_VIRT(pml4), 0, PAGE_SIZE);

    for (size_t i = 0; i < memmap_response->entry_count; ++i) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        for (size_t page_index = 0; page_index < entry->length; page_index += PAGE_SIZE) {
            map(VMM_PHYS_TO_VIRT(entry->base + page_index), entry->base + page_index, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
        }
    }

    for (size_t i = 0; i < ((((size_t) kernel_size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)); i += PAGE_SIZE) {
        map(kernel_virt_addr + i, kernel_phys_addr + i, VMM_TABLE_ENTRY_READ_WRITE | VMM_TABLE_ENTRY_PRESENT);
    }

    load_cr3(pml4);
}