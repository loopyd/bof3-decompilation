#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Declared as the named cell the exact siblings on this target use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and
 * the original re-reads the cell for its second store group because the store
 * that precedes it goes through the pointer the cell supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFCDC
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, advances
 *           that object's unsigned byte 0x9 by one with a byte-width add
 *           (lbu / addiu 1 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0x10 for equality. Only when that byte is 0x10 does it
 *           re-read the pointer cell and increment the object's byte 0x2 with a
 *           byte-width add (lbu / addiu 1 / sb), advancing to the next byte-0x2
 *           handler step. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state. Its address is entry 1
 *           of the eight-address code-pointer run at 0x801F035C inside the
 *           reviewed textbin row D_801F02A4, and no in-image jal reaches it, so
 *           it is published for a consumer outside the demonstrated set. The
 *           body follows the exact emi/bmagic/magic040/03 sibling
 *           incrementWorkByte9AndAdvanceWorkByte2At20BmagicMagic04003_801EF180,
 *           whose instructions match but for the discriminator immediate this
 *           function writes as 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At10BmagicMagic07903_801EFCDC(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x10) {
    g_battle_work[2]++;
  }
}
