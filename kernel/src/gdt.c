#include <stdint.h>
#include <gdt.h>

// hacky but it works for now
void init_gdt() {
    asm volatile ("cli" ::: "memory");

    uint64_t gdt[5] = {
        0x0000000000000000, // null
        0x00AF9A000000FFFF, // kernel code
        0x00AF92000000FFFF, // kernel data
        0x00AFFA000000FFFF, // user code
    };

    struct gdtr gdtptr = {
        .size = 31,
        .offset = (uint64_t) &gdt
    };

    flush_gdt(&gdtptr);
}