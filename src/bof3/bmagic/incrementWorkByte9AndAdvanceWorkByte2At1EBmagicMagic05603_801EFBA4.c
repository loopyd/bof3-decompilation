#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) for both of its reads, and the
 * second read follows the byte-0x9 store through the loaded pointer, so the
 * plain named cell supplies that reload by itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFBA4
 * @behavior Per-frame step of an overlay work-object handler, dispatched from
 *           outside this overlay: it reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once, advances that object's unsigned byte 0x9
 *           by one with a byte-width add (lbu / addiu 1 / sb), then tests the
 *           truncated byte it just stored (andi 0xFF of the stored register, no
 *           second load of the cell) against 0x1E for equality. Only when that
 *           byte is 0x1E does it re-read the pointer cell and increment the
 *           object's byte 0x2 with a byte-width add (lbu / addiu 1 / sb),
 *           advancing to the next byte-0x2 handler step. The function is a leaf
 *           with no frame; it takes no arguments, returns nothing and reads or
 *           writes no other state. Its address is word 1 of the in-image
 *           code-pointer run based at 0x801F0764, whose three entries
 *           (0x801EFB38, this function and 0x801EFBF4) form one byte-0x9 timer
 *           sequence stopping at 0xC, 0x1E and 0x2E, so this entry is published
 *           for a caller outside the payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At1EBmagicMagic05603_801EFBA4(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x1E) {
    g_battle_work[2]++;
  }
}
