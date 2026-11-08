#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Declared as the named cell the exact siblings on this target use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and
 * the original re-reads the cell for its second store group because the store
 * that precedes it goes through the pointer the cell supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00DC
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, advances
 *           that object's unsigned byte 0x9 by one with a byte-width add
 *           (lbu / addiu 1 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0xA0 for equality. Only when that byte is 0xA0 does it
 *           re-read the pointer cell and increment the object's byte 0x2 with a
 *           byte-width add (lbu / addiu 1 / sb), advancing to the next byte-0x2
 *           handler step. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state. No jal in this payload
 *           reaches it: its address is entry 3 of the handler table based at
 *           0x801F0680, the word at 0x801F068C inside the reviewed textbin row
 *           T_801F065C that the byte-1 dispatcher
 *           dispatchStateByte1Table0680BmagicMagic05903_801EFE48 indexes.
 *           Bytes 0x801F00DC..0x801F012B are byte-identical (one payload cmp of
 *           both 0x50-byte spans) to the exact same-target sibling
 *           incrementWorkByte9AndAdvanceWorkByte2AtA0BmagicMagic05903_801EF570,
 *           whose body spelling this follows with this function's own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2AtA0BmagicMagic05903_801F00DC(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0xA0) {
    g_battle_work[2]++;
  }
}
