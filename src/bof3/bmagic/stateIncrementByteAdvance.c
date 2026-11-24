#include "bof3/bof3.h"

/* Scratchpad battle work-object pointer cell. This target's map already binds
 * 0x1F800044 as g_battle_work, so the cell keeps that name here; the symbolic
 * address folds into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)) and the
 * original re-reads the cell for its second store group because the store that
 * precedes it goes through the pointer the cell supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF588
 * @behavior Per-step handler of the overlay's work-object state machine. It
 *           reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           advances that object's unsigned byte 0x9 by one with a byte-width
 *           add (lbu / addiu 1 / sb), then tests the truncated byte it just
 *           stored (andi 0xFF of the stored register, no second load of the
 *           cell) against 0x10 for equality. Only when that byte is 0x10 does
 *           it re-read the pointer cell and increment the object's byte 0x2
 *           with a byte-width add (lbu / addiu 1 / sb), advancing the byte-0x2
 *           step index that this overlay's 0x801EFBBC dispatcher reads.
 *           Leaf with no frame; takes no arguments, returns nothing and reads
 *           or writes no other state. No jal in this payload reaches it: its
 *           address is entry 2, zero-based, of the handler run based at
 *           0x801F0C10 (word 0x801F0C18), the table the overlay's 0x801EF4C4
 *           dispatcher indexes on work byte 0x1, and the lifted neighbour
 *           0x801EF5D8 is entry 3 of that same run. Bytes
 *           0x801EF588..0x801EF5D7 are byte-identical (80 bytes) to the exact
 *           sibling emi/bmagic/magic088/03@0x801EF59C, whose body spelling
 *           this follows with this function's own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateIncrementByteAdvance(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x10) {
    g_battle_work[2]++;
  }
}
