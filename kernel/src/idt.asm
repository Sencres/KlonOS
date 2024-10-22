global flush_idt
flush_idt:
    lidt [rdi]
    ret

%macro isr 1
    global isr_%1
    isr_%1:
        push 0
        push %1
        jmp common_stub
%endmacro

%macro isr_err 1
    global isr_%1
    isr_%1:
        push %1
        jmp common_stub
%endmacro

isr 0
isr 1
isr 2
isr 3
isr 4
isr 5
isr 6
isr 7
isr_err 8
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr 16
isr_err 17
isr 18
isr 19
isr 20
isr_err 21

extern handler
common_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov rdi, rsp
    call handler
    mov rsp, rax
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16
    iretq