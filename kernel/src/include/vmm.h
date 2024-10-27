#ifndef VMM_H
#define VMM_H

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <pmm.h>

#define VMM_ADDR_MASK ((uint64_t) 0x000FFFFFFFFFF000)

#define VMM_PHYS_TO_VIRT(ADDR) ((ADDR) + hhdm_offset)
#define VMM_PTRS_PHYS_TO_VIRT(ADDR) ((uint64_t *) (((uintptr_t) (ADDR)) + hhdm_offset))
#define VMM_ALLOC_PHYS_TO_VIRT(ADDR) ((void *) (((uintptr_t) (ADDR)) + hhdm_offset))

#define VMM_PAGE_ALIGN_UP(ADDR) (((ADDR) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define VMM_PAGE_ALIGN_DOWN(ADDR) ((ADDR) & ~(PAGE_SIZE - 1))

#define VMM_TABLE_ENTRY_READ_WRITE (1 << 1)
#define VMM_TABLE_ENTRY_PRESENT (1 << 0)

#define VMM_REGION_NONE 0
#define VMM_REGION_WRITE (1 << 0)
#define VMM_REGION_EXEC (1 << 1)
#define VMM_REGION_USER (1 << 2)

struct vmm_region_t {
    uint64_t base;
    size_t length;
    uint8_t flags;
    struct vmm_region_t *next;
};

extern void *memset(void *, int, size_t);

void load_cr3(void *);
uint64_t *next_table_addr_new(uint64_t *, size_t, uint8_t);
void map(uint64_t, uint64_t, uint8_t);
uint64_t *next_table_addr(uint64_t *, size_t);
uint64_t unmap(uint64_t);
void *vmm_alloc(size_t, uint8_t);
void vmm_free(void *);
void init_vmm_regions();
void init_vmm();

#endif