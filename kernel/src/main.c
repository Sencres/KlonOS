#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <font.h>
#include <gdt.h>
#include <idt.h>
#include <pmm.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

[[noreturn]] void kpanic() {
    asm volatile ("cli" : : : "memory");
    for (;;) {
        asm volatile ("hlt" : : : "memory");
    }
}

size_t linenum = 0;
struct limine_framebuffer *framebuffer;

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len++]);
    return len;
}

void write_char(uint8_t chr, size_t x, size_t y) {
    uintptr_t addr = ((uintptr_t) framebuffer->address);

    for (size_t row = y * 13; row < (y + 1) * 13; ++row) {
        for (size_t column = 8 * x; column < 8 * (x + 1); ++column) {
            if ((letters[chr - 32][12 - (row - (13 * y))] & (1 << (8 - (column - (8 * x))))) == (1 << (8 - (column - (8 * x))))) {
                ((uint32_t *) addr)[row * framebuffer->pitch / (framebuffer->bpp / 8) + column] = 0xFFFFFF;
            }
        }
    }
}

void write_string(const char *str) {
    for (size_t i = 0; i < strlen(str); ++i) {
        write_char(str[i], i, linenum);
    }

    linenum += 1;
}

void kmain() {
    if (!LIMINE_BASE_REVISION_SUPPORTED) {
        kpanic();
    }

    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1) {
        kpanic();
    }

    framebuffer = framebuffer_request.response->framebuffers[0];

    write_string("Hello kernel World!");

    init_gdt();
    write_string("Loaded GDT");

    init_idt();
    write_string("Loaded IDT");

    init_pmm();
    write_string("Initialized PMM");

    kpanic();
}