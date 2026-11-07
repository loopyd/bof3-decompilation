#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) for both of its reads, and the
 * second read follows the byte-0x9 store through the loaded pointer, so the
 * plain named cell supplies that reload by itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF570
 * @behavior Per-frame step of an overlay work-object handler. The dispatcher
 * at 0x801EF1E0 indexes the byte-0x1 state table based at 0x801F066C, and this
 * function is entry 3 of that run of code pointers, so it runs while the work
 * object's state byte 0x1 reads 3. It reads the scratchpad work-object pointer
 * cell at 0x1F800044 once, advances that object's unsigned byte 0x9 by one
 * with a byte-width add (lbu / addiu 1 / sb), then tests the truncated byte it
 * just stored (andi 0xFF of the stored register, no second load of the cell)
 * against 0xA0 for equality. Only when that byte is 0xA0 does it re-read the
 * pointer cell and increment the object's byte 0x2 with a byte-width add
 * (lbu / addiu 1 / sb), advancing to the next byte-0x2 handler step. The
 * function is a leaf with no frame; it takes no arguments, returns nothing and
 * reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2AtA0BmagicMagic05903_801EF570(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0xA0) {
    g_battle_work[2]++;
  }
}
