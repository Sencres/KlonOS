#ifndef VMM_H
#define VMM_H

#pragma once

#include <stddef.h>
#include <stdint.h>

#define VMM_TABLE_ENTRY_READ_WRITE (1 << 1)
#define VMM_TABLE_ENTRY_PRESENT (1 << 0)

#define VMM_PHYS_TO_VIRT(ADDR) ((ADDR) + hhdm_offset)
#define VMM_PTRS_PHYS_TO_VIRT(ADDR) ((uint64_t *) (((uintptr_t) (ADDR)) + hhdm_offset))

extern void *memset(void *, int, size_t);

void load_cr3(void *);
uint64_t *next_table_addr_new(uint64_t *, size_t, uint8_t);
void map(uint64_t, uint64_t, uint8_t);
uint64_t *next_table_addr(uint64_t *, size_t);
void unmap(uint64_t);
void init_vmm();

#endif