// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

/*******************************************************************************
 * Tests used to verify Supervisor mode 
 ******************************************************************************/
#include "simple_system_common.h"
#include <stdint.h>

void as_supervisor(void) {
  puts("Supervisor\n");
  uint32_t sstatus;
  __asm__ volatile("csrr %0, sstatus": "=r" (sstatus));
  puts("SSTATUS read: ");
  puthex(sstatus);
  puts("\n");
  
  if ((sstatus & (1<<18)) != 0) {
    puts("SUM bit preserved\n");
  }
  
  puts("Executing SFENCE.VMA: ");
  __asm__ volatile("sfence.vma");
  puts("OK\n");

  sim_halt();
}

int main(void) {
  puts("Test supervisor mode\n");

  // Go to supervisor mode
  uint32_t mstatus;
  uint32_t ptr_fn = (uintptr_t) &as_supervisor;
  __asm__ volatile("csrr %0, mstatus": "=r" (mstatus));
  mstatus |= 1<<11; // 01 Supervisor MPP
  mstatus |= 1<<18; // SUM
  mstatus &= ~(1<<7);
  __asm__ volatile("csrw mstatus, %0" :: "r" (mstatus));
  __asm__ volatile("csrw mepc, %0" :: "r" (ptr_fn));

  mstatus = 0;
  ptr_fn = 0;
  __asm__ volatile("csrr %0, mstatus": "=r" (mstatus));
  __asm__ volatile("csrr %0, mepc": "=r" (ptr_fn));
  
  puts("MSTATUS after write: "),
  puthex(mstatus);
  puts("\nMEPC after write: ");
  puthex(ptr_fn);
  puts("\n");
  
  if ((mstatus & (1<<11)) == 0) {
    puts("\nMPP WAS NOT UPDATED\n");
  }

  __asm__ volatile("mret");

  puts("Still alive\n");
  return 0;
}
