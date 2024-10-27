#include <stddef.h>
#include <limine.h>
#include <pmm.h>

extern struct limine_memmap_response *memmap_response;
extern uint64_t hhdm_offset;
uintptr_t freelist = 0;

void init_pmm() {
    for (size_t i = 0; i < memmap_response->entry_count; ++i) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        for (size_t page_index = 0; page_index < entry->length; page_index += PAGE_SIZE) {
            pmm_free((void *) (entry->base + page_index));
        }
    }
}

void *pmm_alloc() {
    if (freelist == 0) {
        kpanic();
    }

    void *ret = (void *) freelist;
    freelist = *((uint64_t *) (freelist + hhdm_offset));

    return ret;
}

void pmm_free(void *addr) {
    *((uint64_t *) (((uintptr_t) addr) + hhdm_offset)) = freelist;
    freelist = (uintptr_t) addr;
}