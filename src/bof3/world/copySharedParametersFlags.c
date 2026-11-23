#include "bof3/world/area03004_internal.h"

/* Per-evaluation reload of the work cursor cell: the original emits
 * lui $v1,0x1f80 / lw $v1,0x44($v1) before each store group. */
#define AREA030_SCRATCH_CURSOR ((volatile u8*)D_1F800044)

/* @behavior Clear the work flags and return when shared signed halfword 0x80144006 is
 * nonpositive or byte 0x80143FC9 is zero. Otherwise copy the shared
 * coordinates, byte 0x2A, sequence fields, word pair and halfword pair to the
 * work record, then call projectQueueActiveWork.
 * @source 0x801D2AE0
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedParametersFlags(void) {
  volatile u8* work;
  volatile u8* sharedFields;

  sharedFields = (volatile u8*)0x80140000u;

  if ((*(s16*)(sharedFields + 0x4006u) < 1) || (sharedFields[0x3fc9u] == 0u)) {
    func_80196070();
    return;
  }

  work = AREA030_SCRATCH_CURSOR;
  *(volatile u32*)(work + 0x34u) = *(volatile u32*)(sharedFields + 0x3ffcu);
  *(volatile u32*)(work + 0x38u) = *(volatile u32*)(sharedFields + 0x4000u);
  work[0x2au] = sharedFields[0x3ff2u];

  work = AREA030_SCRATCH_CURSOR;
  work[0x49u] = sharedFields[0x4011u];
  work = AREA030_SCRATCH_CURSOR;
  work[0x4au] = sharedFields[0x4012u];
  work = AREA030_SCRATCH_CURSOR;
  work[0x4bu] = sharedFields[0x4013u];
  work = AREA030_SCRATCH_CURSOR;
  *(volatile u32*)(work + 0x50u) = *(volatile u32*)(sharedFields + 0x4018u);
  *(volatile u32*)(work + 0x54u) = *(volatile u32*)(sharedFields + 0x401cu);
  *(volatile u16*)(work + 0x58u) = *(volatile u16*)(sharedFields + 0x4020u);
  *(u16*)(work + 0x5au) = *(volatile u16*)(sharedFields + 0x4022u);
  projectQueueActiveWork();
}
