#include <stdint.h>
#include <gdt.h>

static struct gdt_descriptor_t gdt[] = {
    {}, // null descriptor
    {
        .limit = 0xFFFF,
        .base_lo = 0x0000,
        .base_mid = 0x00,
        .access = GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_EXECUTABLE | GDT_READ_WRITE,
        .flags = (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F,
        .base_hi = 0x00
    },
    {
        .limit = 0xFFFF,
        .base_lo = 0x0000,
        .base_mid = 0x00,
        .access = GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_READ_WRITE,
        .flags = (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F,
        .base_hi = 0x00
    },
    {
        .limit = 0xFFFF,
        .base_lo = 0x0000,
        .base_mid = 0x00,
        .access = GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_DPL_USER | GDT_EXECUTABLE | GDT_READ_WRITE,
        .flags = (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F,
        .base_hi = 0x00
    },
    {
        .limit = 0xFFFF,
        .base_lo = 0x0000,
        .base_mid = 0x00,
        .access = GDT_VALID_SEGMENT | GDT_CODE_DATA_SELECTOR | GDT_DPL_USER | GDT_READ_WRITE,
        .flags = (GDT_PAGE_GRANULITY | GDT_LONG_MODE) | 0x0F,
        .base_hi = 0x00
    },
    {}, {} // tss
};

void init_gdt() {
    asm volatile ("cli" : : : "memory");

    struct gdtr_t gdtr = {
        .size = sizeof(gdt) - 1,
        .offset = (uint64_t) &gdt
    };

    flush_gdt(&gdtr);
}