#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) for both of its reads, and the
 * second read follows the byte-0x9 store through the loaded pointer, so the
 * plain named cell supplies that reload by itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF180
 * @behavior Per-frame step of an overlay work-object handler, dispatched from
 * outside this overlay: it reads the scratchpad work-object pointer cell at
 * 0x1F800044 once, advances that object's unsigned byte 0x9 by one with a
 * byte-width add (lbu / addiu 1 / sb), then tests the truncated byte it just
 * stored (andi 0xFF of the stored register, no second load of the cell)
 * against 0x20 for equality. Only when that byte is 0x20 does it re-read the
 * pointer cell and increment the object's byte 0x2 with a byte-width add
 * (lbu / addiu 1 / sb), advancing to the next byte-0x2 handler step. The
 * function is a leaf with no frame; it takes no arguments, returns nothing and
 * reads or writes no other state. Its address is word 5 of the in-image
 * code-pointer run based at 0x801EEC04, whose only in-image consumer
 * (0x801EEC20) copies just the run's first three entries into its own frame,
 * so this entry is published for a caller outside the payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At20BmagicMagic04003_801EF180(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x20) {
    g_battle_work[2]++;
  }
}
