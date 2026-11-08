#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of its three reads, and
 * each of the two later reads follows a byte store through the loaded pointer,
 * so the plain named cell supplies those reloads by itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF504
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 * scratchpad work-object pointer cell at 0x1F800044 once and advances that
 * object unsigned byte 0x9 by one with a byte-width add (lbu / addiu 1 / sb),
 * then re-reads the cell and adds two to the same object byte 0xA with a
 * byte-width add (lbu / addiu 2 / sb). It tests the byte it just stored there,
 * truncated to 0xFF in the stored register rather than re-loaded, against 0x20
 * for equality, and only on that equality re-reads the pointer cell and
 * increments the object byte 0x2 with a byte-width add (lbu / addiu 1 / sb),
 * advancing to the next byte-0x2 handler step. The function is a leaf with no
 * frame; it takes no arguments, returns nothing and reads or writes no other
 * state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9Add2ToWorkByteAThenAdvanceWorkByte2At20BmagicMagic05903_801EF504(void) {
  u8 *work;
  u8 count;
  u8 level;

  work = g_battle_work;
  count = work[9] + 1;
  work[9] = count;
  work = g_battle_work;
  level = work[0xA] + 2;
  work[0xA] = level;
  if (level == 0x20) {
    g_battle_work[2]++;
  }
}
