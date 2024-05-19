/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, BYTE *value)
{
    /* (done) TODO: the identify info is mapped to 
     *      cache line by employing:
     *      direct mapped, associated mapping etc.
     */
    if (mp == NULL) {
        return -1;
    }

    int raw_addr = pgnum * PAGE_SIZE;
    if (mp->tlb_cache_entries != NULL) {
        for (int n = 0; n < mp->tlb_entry_count; n++) {
            struct tlb_entry_struct *entry = &mp->tlb_cache_entries[n];
            bool entry_matches = (entry->page_number == pgnum)
                                 && (entry->pid == pid)
                                 && (entry->address == raw_addr);
            if (entry_matches && (TLBMEMPHY_read(mp, entry->address, value) == 0)) {
                return 0;
            }
        }
    }

    return -1;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, BYTE value)
{
    /* (done) TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */

    if (mp == NULL) {
        return -1;
    }

    int raw_addr = pgnum * PAGE_SIZE;
    TLBMEMPHY_write(mp, raw_addr, value);
    if (mp->tlb_entry_count >= mp->maxsz) {
        // Reset
        mp->tlb_entry_count = 0;
    }
    // Write to last element, increment array size
    struct tlb_entry_struct *entry = &mp->tlb_cache_entries[mp->tlb_entry_count];
    entry->pid = pid;
    entry->address = raw_addr;
    entry->page_number = pgnum;
    mp->tlb_entry_count++;

    return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
    if (mp == NULL)
        return -1;

    /* TLB cached is random access by native */
    *value = mp->storage[addr];

    return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
    if (mp == NULL)
        return -1;

    /* TLB cached is random access by native */
    mp->storage[addr] = data;

    return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
    /* (done) TODO: dump memphy contnt mp->storage 
     *     for tracing the memory content
     */

    if (mp == NULL) {
        return -1;
    }

    printf("==== TLB memphy dump ====\n");
	for (int i = 0; i < mp->maxsz; i++) {
        BYTE val = mp->storage[i];
        printf("\t%d: %02X\n", i, val);
    }
	printf("==== End of TLB memphy dump ====\n");

    return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;

   mp->tlb_cache_entries = malloc(mp->maxsz * sizeof(struct tlb_entry_struct));
   mp->tlb_entry_count = 0;

   mp->rdmflg = 1;

   return 0;
}

//#endif
