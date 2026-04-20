// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

/*******************************************************************************
 * Tests used to verify Supervisor mode 
 ******************************************************************************/
#include "simple_system_common.h"
#include <stdint.h>

void supervisor_step2(void) {
  puts("Supervisor exit\n");
  sim_halt();
}

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

  uintptr_t sepc = (uintptr_t) &supervisor_step2;
  __asm__ volatile("csrw sepc, %0" :: "r" (sepc));
  puts("Executing SRET: ");
  __asm__ volatile("sret");
  puts("SHOULD NOT BE HERE\n");
  
  sim_halt();
}

void as_supervisor_from_sret(void) {
  puts("Called from SRET\n");
  as_supervisor();
}

void machine_exit(void) {
  puts("Machine interrupt, exit\n");
  sim_halt();
}

int main(void) {
  puts("Test supervisor mode\n");

  // Go to supervisor mode
  uint32_t mstatus;
  uint32_t mepc = (uintptr_t) &machine_exit;
  uint32_t sepc   = (uintptr_t) &as_supervisor;
  __asm__ volatile("csrr %0, mstatus": "=r" (mstatus));
  mstatus |= 1<<11; // 01 Supervisor MPP
  mstatus |= 1<<18; // SUM
  mstatus |= 1<<22; // TSR
  mstatus |= 1<<8;  // SPP (S-Mode)
  mstatus &= ~(1<<7);
  __asm__ volatile("csrw mstatus, %0" :: "r" (mstatus));
  __asm__ volatile("csrw mepc, %0" :: "r" (mepc));
  __asm__ volatile("csrw sepc, %0" :: "r" (sepc));

  mstatus = 0;
  mepc = 0;
  sepc = 0;
  __asm__ volatile("csrr %0, mstatus": "=r" (mstatus));
  __asm__ volatile("csrr %0, mepc": "=r" (mepc));
  __asm__ volatile("csrr %0, sepc": "=r" (sepc));
  
  puts("MSTATUS after write: "),
  puthex(mstatus);
  puts("\nMEPC after write: ");
  puthex(mepc);
  puts("\nSEPC after write: ");
  puthex(sepc);
  puts("\n");
  
  if ((mstatus & (1<<11)) == 0) {
    puts("\nMPP WAS NOT UPDATED\n");
  }
  
  if ((mstatus & (1<<22)) != 0) {
    puts("\nTSR is enabled\n");
  }
  
  puts("SRET from Machine\n");
  __asm__ volatile("sret");
  
  puts("SRET executed, now executing MRET\n");

  __asm__ volatile("mret");

  puts("Still alive\n");
  return 0;
}
