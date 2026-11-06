#include "bof3/bof3.h"

/* @source 0x8014832E @kind unknown */
extern volatile u8 D_8014832E;

/* @source 0x801F2D44
 * @behavior Overlay step handler: loads the work record published at the
 *           scratchpad pointer cell 0x1F800044, copies the record byte at
 *           +0x06 into the shared byte at 0x8014832E, then advances the
 *           record's dispatch byte at +0x01 by one. Takes no arguments and
 *           returns nothing. The address is entry 3 of the per-frame handler
 *           table at 0x801F3E68, which this overlay's entry point 0x801F2C04
 *           indexes with the same +0x01 work byte that this body advances.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkByte6To832EAndIncrementWorkByte1World03Area14413_801F2D44(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  D_8014832E = work[6];
  work[1]++;
}
