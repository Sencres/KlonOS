#ifndef IDT_H
#define IDT_H

#pragma once

#include <stdint.h>

struct interrupt_descriptor {
    uint16_t offset_lo;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset_mid;
    uint32_t offset_hi;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct cpu_status {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t vector_num;
    uint64_t err_code;
    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_rflags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
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