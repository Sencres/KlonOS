#ifndef GDT_H
#define GDT_H

#pragma once

#include <stdint.h>

struct gdtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

extern void flush_gdt(void *);
void init_gdt();

#endif