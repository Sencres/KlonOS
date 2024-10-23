#ifndef PMM_H
#define PMM_H

#pragma once

#define PAGE_SIZE 0x1000

extern void kpanic();

void init_pmm();
void *pmm_alloc();
void pmm_free(void *);

#endif