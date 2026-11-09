#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0D4
 * @behavior Per-frame ramp step of the overlay work object. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and adds 2 to the
 *           object byte at offset 0x9 with a byte-width read-modify-write
 *           (lbu / addiu 2 / sb). The value just stored is zero-extended with
 *           andi 0xFF - no second load - and compared against 0x10, which is
 *           materialised with addiu $v0,$zero,0x10. Only when that byte has
 *           ramped to 0x10 does it re-read the pointer cell to clear the work
 *           object byte at offset 0xA with sb $zero, then re-read the cell once
 *           more to increment the dispatch state byte at offset 0x2 (lbu /
 *           addiu 1 / sb), which advances this overlay's per-frame work byte 2
 *           handler sequence. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9ClearWorkByteAAndAdvanceWorkByte2At10BmagicMagic03903_801EF0D4(void) {
  if ((g_battle_work[9] += 2) == 0x10) {
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
  }
}
