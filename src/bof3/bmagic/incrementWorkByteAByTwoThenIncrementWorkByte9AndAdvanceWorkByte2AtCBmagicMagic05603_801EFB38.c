#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * address (lui $vN,0x1F80 / lw $vN,68($vN)) before every access, and each store
 * through the loaded pointer can alias the cell itself, so every later read
 * spells the named cell again instead of reusing the first loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFB38
 * @behavior First entry of the byte-0x9 timer sequence reached through word 0
 *           of the in-image code-pointer run at 0x801F0764, dispatched from
 *           outside this overlay. It reads the scratchpad work-object pointer
 *           cell at 0x1F800044, widens that object's unsigned byte 0xA by two
 *           with a byte-width add (lbu / addiu 2 / sb), then re-reads the cell
 *           and advances the object's unsigned byte 0x9 by one (lbu / addiu 1 /
 *           sb). The byte it just stored, truncated with andi 0xFF of the
 *           register rather than reloaded, is compared against 0xC; only when
 *           it equals 0xC does the function re-read the cell once more and
 *           increment the object's byte 0x2 (lbu / addiu 1 / sb), advancing to
 *           the next byte-0x2 handler step. The later sequence entries
 *           0x801EFBA4 and 0x801EFBF4 continue the same byte-0x9 timer with
 *           stops at 0x1E and 0x2E. The function is a leaf with no frame; it
 *           takes no arguments, returns nothing and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAByTwoThenIncrementWorkByte9AndAdvanceWorkByte2AtCBmagicMagic05603_801EFB38(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  work[0xA] += 2;
  counter = g_battle_work[9] + 1;
  g_battle_work[9] = counter;
  if (counter == 0xC) {
    g_battle_work[2]++;
  }
}
