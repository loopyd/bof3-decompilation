#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound as g_battle_work by this target's
 * map. The symbolic address folds into each use (lui $vN,0x1F80 /
 * lw $vN,68($vN)), and the original re-reads the cell for its second store
 * group because the store that precedes it goes through the pointer the cell
 * supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0F8
 * @behavior Per-frame step of an overlay work-object handler. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and
 *           advances the work object's unsigned byte 0x9 by one with a
 *           byte-width add (lbu / addiu 1 / sb), then tests that same stored
 *           register truncated to a byte (andi 0xFF, with no second load of
 *           the cell) against 0x10 for equality. Only when the byte reaches
 *           0x10 does it re-read the pointer cell and increment the work
 *           object's byte 0x2 with a byte-width add (lbu / addiu 1 / sb),
 *           stepping the overlay to its next byte-0x2 handler. The re-read is
 *           required because the preceding store goes through the pointer the
 *           cell supplied. Leaf with no stack frame: takes no arguments,
 *           returns nothing and touches no other state. The payload holds no
 *           jal to this address; the overlay's byte-1 dispatcher at 0x801EF00C
 *           calls it as entry 2, zero-based, of the nine-word handler run
 *           (0x801EF050, 0x801EF0B8, 0x801EF0F8, 0x801EF148, 0x801EF18C,
 *           0x801EF404, 0x801EF47C, 0x801EF628, 0x801EF6E0) that this target's
 *           map leaves as the textbin row T_801EFC2C. A payload cmp proved
 *           bytes 0x801EF0F8..0x801EF147 byte-identical to the 80-byte spans at
 *           0x801EF59C of emi/bmagic/magic088/03, 0x801EF588 of magic062/03,
 *           0x801EFFCC of magic067/03, 0x801EFCDC and 0x801EFF08 of
 *           magic079/03, 0x801EF630 of magic082/03 and 0x801EFD00 of
 *           magic137/03, so statement order, operand forms, offset-local
 *           spelling and declaration style follow those sources with this
 *           function's own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementByte9Advance2(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x10) {
    g_battle_work[2]++;
  }
}
