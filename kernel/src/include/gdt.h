#ifndef GDT_H
#define GDT_H

#pragma once

#include <stdint.h>

#define GDT_VALID_SEGMENT (1 << 7)
#define GDT_DPL_USER (0b11 << 5)
#define GDT_CODE_DATA_SELECTOR (1 << 4)
#define GDT_EXECUTABLE (1 << 3)
#define GDT_READ_WRITE (1 << 1)

#define GDT_PAGE_GRANULITY (1 << 7)
#define GDT_LONG_MODE (1 << 5)

struct gdt_descriptor_t {
    uint16_t limit;
    uint16_t base_lo;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags;
    uint8_t base_hi;
} __attribute__((packed));

struct gdtr_t {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

extern void flush_gdt(void *);

void init_gdt();

#endif