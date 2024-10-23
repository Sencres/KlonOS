#include <stdint.h>
#include <gdt.h>

uint64_t gdt[4] = {
    0x0000000000000000, // null
    0x00AF9A000000FFFF, // kernel code
    0x00AF92000000FFFF, // kernel data
    0x00AFFA000000FFFF, // user code
};

void init_gdt() {
    asm volatile ("cli" : : : "memory");

    struct gdtr gdtptr = {
        .size = 31,
        .offset = (uint64_t) &gdt
    };

    flush_gdt(&gdtptr);
}