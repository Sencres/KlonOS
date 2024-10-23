#include <stddef.h>
#include <limine.h>
#include <pmm.h>

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 2
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 2
};

uint64_t hhdm_offset = 0;
uint64_t *freelist = NULL;

void init_pmm() {
    if (memmap_request.response == NULL || hhdm_request.response == NULL) {
        kpanic();
    }

    hhdm_offset = hhdm_request.response->offset;

    for (size_t i = 0; i < memmap_request.response->entry_count; ++i) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        for (size_t j = 0; j < (entry->length / PAGE_SIZE); ++j) {
            pmm_free((void *) (entry->base + j * PAGE_SIZE));
        }
    }
}

void *pmm_alloc() {
    void *ret = (void *) (((uintptr_t) freelist) - hhdm_offset);
    freelist = (uint64_t *) (*freelist);

    return ret;
}

void pmm_free(void *addr) {
    *((uint64_t *) (((uintptr_t) addr) + hhdm_offset)) = (uintptr_t) freelist;
    freelist = addr;
}