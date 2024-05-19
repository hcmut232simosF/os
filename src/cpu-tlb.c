/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* (done) TODO: update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */
  // Just flush it, I don't really care.
  tlb_flush_tlb_of(proc, mp);

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* (done) TODO: flush tlb cached*/
  if (mp == NULL) {
    return -1;
  }

  // Remove all entries with that PID from the array
  int m = 0;
  for (int n = 0; n < mp->tlb_entry_count; n++) {
    struct tlb_entry_struct *source = &mp->tlb_cache_entries[n];
    struct tlb_entry_struct *destination = &mp->tlb_cache_entries[m];
    bool survives = source->pid != proc->pid;
    if (n != m) {
      *destination = *source;
    }
    if (survives) {
      m++;
    }
  }
  mp->tlb_entry_count = m;

  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* (done) TODO: update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  /* By default using vmaid = 0 */
  int addr;
  int val = __alloc(proc, 0, reg_index, size, &addr);
  if (val != 0) { // allocation error
    return val;
  }

  for (uint32_t page = 0; page < DIV_ROUND_UP(size, PAGE_SIZE); page++) {
    tlb_cache_write(proc->tlb, proc->pid, addr / PAGE_SIZE + page, '\0');
  }

  return 0;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  /* (done) TODO: update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  struct vm_rg_struct *rg = get_symrg_byid(proc->mm, reg_index);
  if (rg == NULL) {
    // No region by that ID/index/whatever.
    return 1;
  }

  for (uint32_t page = rg->rg_start / PAGE_SIZE; page < (rg->rg_end + PAGE_SIZE - 1) / PAGE_SIZE; page++) {
    tlb_cache_write(proc->tlb, proc->pid, page, 0);
  }

  return (__free(proc, 0, reg_index) == 0) ? 0 : 1;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  /* (done) TODO: retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/

  BYTE data;
  int pgnum = PAGING_PGN((proc->regs[source] + offset));
  int frmnum = tlb_cache_read(proc->tlb, proc->pid, pgnum, &data);
  // NOTE: where's the error handling for tlb_cache_read??
  // The code the professors gave to us really is subpar.

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  if (frmnum >= 0) {
    proc->regs[destination] = data;
    return 0;
  }

  // Fall back to normal memory read
  int status = __read(proc, 0, source, offset, &data);
  //                           ^ don't be confused, __read does want the register.
  if (status == 0) {
    /* (done) TODO: update TLB CACHED with frame num of recent accessing page(s)*/
    /* by using tlb_cache_read()/tlb_cache_write()*/
    tlb_cache_write(proc->tlb, proc->pid, pgnum, data);
    proc->regs[destination] = data;
  }

  // Parrot __read status.
  return status;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  int pgnum = PAGING_PGN((proc->regs[destination] + offset));
  BYTE frmnum = tlb_cache_read(proc->tlb, proc->pid, pgnum, &data);

  /* (done) TODO: retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  val = __write(proc, 0, destination, offset, data);

  /* (done) TODO: update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  tlb_cache_write(proc->tlb, proc->pid, pgnum, data);

  return val;
}

//#endif
