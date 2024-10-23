#ifndef IDT_H
#define IDT_H

#pragma once

#include <stdint.h>

#define IDT_ENTRY_PRESENT (1 << 7)
#define IDT_DPL_USER (0b11 << 5)

#define IDT_INTERRUPT_GATE 0b1110
#define IDT_TRAP_GATE 0b1111

struct idt_descriptor_t {
    uint16_t offset_lo;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset_mid;
    uint32_t offset_hi;
    uint32_t reserved;
} __attribute__((packed));

struct idtr_t {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct interrupt_frame_t {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi;
    uint64_t rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t iret_rip, iret_cs, iret_rflags, iret_rsp, iret_ss;
} __attribute__((packed));

#define ISR(N) extern void isr_##N()
ISR(0); ISR(1); ISR(2); ISR(3); ISR(4);
ISR(5); ISR(6); ISR(7); ISR(8); ISR(10);
ISR(11); ISR(12); ISR(13); ISR(14); ISR(16);
ISR(17); ISR(18); ISR(19); ISR(20); ISR(21);

extern void flush_idt(void *);
void set_idt_entry(uint8_t, void *, uint16_t, uint8_t);
struct cpu_status *handler(struct cpu_status *);
void init_idt();

#endif