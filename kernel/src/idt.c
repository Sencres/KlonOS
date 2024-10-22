#include <stddef.h>
#include <idt.h>

struct interrupt_descriptor idt[256];

void set_idt_entry(uint8_t vector, void *handler, uint16_t selector, uint8_t gate_type) {
    uint64_t handler_addr = (uint64_t) handler;

    idt[vector].offset_lo = handler_addr & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = 0;
    idt[vector].flags = 0x80 | (gate_type & 0xF);
    idt[vector].offset_mid = (handler_addr >> 16) & 0xFFFF;
    idt[vector].offset_hi = (handler_addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

extern void write_string(char *);
struct cpu_status *handler(struct cpu_status *ctx) {
    switch (ctx->vector_num) {
    case 0:
        write_string("Division by zero");
        break;
    case 8:
        write_string("Double fault");
        break;
    case 13:
        write_string("General protection fault");
        break;
    case 14:
        write_string("Page fault");
        break;
    default:
        write_string("Other exception");
    }

    return ctx;
}

void init_idt() {
    asm volatile ("cli" ::: "memory");

    set_idt_entry(0, isr_0, 0x0008, 0xF);
    set_idt_entry(1, isr_1, 0x0008, 0xF);
    set_idt_entry(2, isr_2, 0x0008, 0xF);
    set_idt_entry(3, isr_3, 0x0008, 0xF);
    set_idt_entry(4, isr_4, 0x0008, 0xF);
    set_idt_entry(5, isr_5, 0x0008, 0xF);
    set_idt_entry(6, isr_6, 0x0008, 0xF);
    set_idt_entry(7, isr_7, 0x0008, 0xF);
    set_idt_entry(8, isr_8, 0x0008, 0xF);
    set_idt_entry(10, isr_10, 0x0008, 0xF);
    set_idt_entry(11, isr_11, 0x0008, 0xF);
    set_idt_entry(12, isr_12, 0x0008, 0xF);
    set_idt_entry(13, isr_13, 0x0008, 0xF);
    set_idt_entry(14, isr_14, 0x0008, 0xF);
    set_idt_entry(16, isr_16, 0x0008, 0xF);
    set_idt_entry(17, isr_17, 0x0008, 0xF);
    set_idt_entry(18, isr_18, 0x0008, 0xF);
    set_idt_entry(19, isr_19, 0x0008, 0xF);
    set_idt_entry(20, isr_20, 0x0008, 0xF);
    set_idt_entry(21, isr_21, 0x0008, 0xF);

    struct idtr idtptr = {
        .size = sizeof(idt) - 1,
        .offset = (uint64_t) &idt
    };

    flush_idt(&idtptr);

    asm volatile ("sti" ::: "memory");
}