#include "bof3/bof3.h"

/* Local helper at 0x801F08AC. Its own entry takes no arguments and keeps a
 * 0x20-byte frame; this caller discards its result. */
extern void func_801F08AC(void); /* @source 0x801F08AC @kind function */

/* Scratchpad work-object pointer cell, shared by this overlay's step handlers.
 * The row lives in this target's map. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0158
 * @behavior Overlay step handler: invokes the target-local helper at
 *           0x801F08AC, then advances the work object's byte 9 by one with a
 *           byte-width read-modify-write (lbu / addiu 1 / sb) and tests the
 *           truncated value it just stored (andi 0xFF of the stored register,
 *           no second load of the byte). Only when that byte reached 0x10 it
 *           reads the scratchpad pointer cell at 0x1F800044 a second time and
 *           increments the work object's state byte at offset 0x2, advancing
 *           the overlay's work byte 2 handler sequence. Takes no arguments
 *           and returns nothing; its 0x18-byte frame exists only to keep the
 *           return address across the call. The pointer word at 0x801F0CD8,
 *           index 12 of the handler run based at 0x801F0CA8, names this span.
 *           The statement order, operand forms, offsets and declaration style
 *           follow the exact sibling incrementWorkAdvanceWork4,
 *           whose byte span this body repeats except for the compared literal
 *           (0x78 there, 0x10 here); the frame and leading call follow the
 *           exact siblings at 0x801F01C0 and 0x801F0224. A byte scan of the
 *           shipped payloads found no byte-identical twin of this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIncrementAdvance(
    void) {
  func_801F08AC();
  if ((g_battle_work[9] += 1) == 0x10) {
    g_battle_work[2]++;
  }
}
