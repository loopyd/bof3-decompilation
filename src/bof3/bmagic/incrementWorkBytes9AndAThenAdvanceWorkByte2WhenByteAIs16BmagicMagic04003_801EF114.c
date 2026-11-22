#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The byte stores in front of each later
 * access may alias the cell, so the body re-materialises the cell address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for every group of accesses to it and the
 * plain named cell supplies those reloads by itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF114
 * @behavior Per-frame step of an overlay work-object handler that is reached
 * through the code-pointer run based at 0x801EEC04, where its address is the
 * run's entry 4 (0x801EEC80, 0x801EEDFC, 0x801EEEFC, 0x801EF074, this entry,
 * then 0x801EF180 and 0x801EF1D0). It reads the scratchpad work-object pointer
 * cell at 0x1F800044, increments the object's unsigned byte 0x09 by one with a
 * byte-width add
 * (lbu / addiu 1 / sb), then re-reads the cell and increments byte 0x0A the
 * same way while keeping the stored value in a byte local. That local carries
 * the counter: the truncated stored byte is compared against 0x10 with an
 * equality test (andi 0xFF of the stored register, no second load of the
 * field), and only when it is 0x10 does the body re-read the cell and
 * increment the object's byte 0x02, advancing to the next handler step. The
 * function is a leaf with no frame, takes no arguments, returns nothing and
 * reads or writes no other state. The only reference to its address is that
 * run word, which 0x801EEC20 copies as data instead of calling through it, so
 * the entry is published for a caller outside the payload and its
 * parameter roles stay structural. Its two leading access groups and its
 * 0x6C length equal the body at 0x801EF334 of emi/bmagic/magic086/03; that
 * body differs from this one after the byte 0x0A store, where it tests a
 * re-read byte 0x09 and reuses the pointer it already holds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytes9AndAThenAdvanceWorkByte2WhenByteAIs16BmagicMagic04003_801EF114(void) {
  u8 *work9;
  u8 *workA;
  u8 shade;

  work9 = g_battle_work;
  work9[9]++;
  workA = g_battle_work;
  shade = workA[0xA] + 1;
  workA[0xA] = shade;
  if (shade == 0x10) {
    g_battle_work[2]++;
  }
}
