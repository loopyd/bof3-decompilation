#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, already bound at 0x1F800044 by this
 * target's map. The symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's three separate reads each
 * re-materialise the base. The byte-A test consumes the value just stored in
 * its own register (andi 0xFF) instead of re-loading the cell, and only the
 * branch target re-reads it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00F0
 * @behavior Per-frame ramp step of the overlay work object. Adds 2 to the
 * object's byte at offset 0x9 with a byte-width read-modify-write (lbu /
 * addiu 2 / sb). Decrements the object's byte at offset 0xA by one with the
 * same width, then zero-extends the value just stored with andi 0xFF - no
 * second load - and compares it against 0x30. Only on that equality does it
 * re-read the scratchpad pointer cell and increment the dispatch state byte at
 * offset 0x2 (lbu / addiu 1 / sb), advancing this overlay's per-frame work
 * byte 2 handler sequence. Leaf with no frame; takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workDecrementByte(void) {
  g_battle_work[9] += 2;
  if ((g_battle_work[0xA] -= 1) == 0x30) {
    g_battle_work[2]++;
  }
}
