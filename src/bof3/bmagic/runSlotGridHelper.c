#include "bof3/bof3.h"

/* Local slot-grid draw routine at 0x801F0274. Its own entry takes no arguments
 * and keeps a 0x38-byte frame; this caller discards its result. */
extern void func_801F0274(void); /* @source 0x801F0274 */

/* Scratchpad work-object pointer cell, shared by this overlay's step handlers.
 * The row lives in this target's map. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared helper in the main executable, outside this overlay's image; the
 * target's generated undefined-funcs map binds the address. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F0104
 * @behavior Overlay step handler, entry 11 of this overlay's handler-pointer
 *           run based at 0x801F0CA8 (the word at 0x801F0CD4 is the only
 *           in-image reference to this address), reached through the work
 *           byte 2 dispatcher. Runs the target-local slot-grid draw routine at
 *           0x801F0274, reads the scratchpad work-object pointer cell at
 *           0x1F800044 once, then advances the work object's byte 9 by one at
 *           byte width (lbu 0x9 / addiu 1 / sb 0x9) and tests the truncated
 *           value it just stored (andi 0xFF of the stored register, no second
 *           load of the byte). Only when that stored byte equals 0x70 it
 *           invokes the shared helper at 0x801E5988 with no arguments; it
 *           reloads neither the cell nor any work byte, and it leaves work byte
 *           2 unchanged. Takes no arguments and returns nothing; its 0x18-byte
 *           frame exists only to keep the return address across the calls.
 *           Statement order, operand forms, offset spelling and declaration
 *           style follow the exact sibling at 0x801F009C, which makes the same
 *           leading draw call and byte-9 increment and differs only in the
 *           compared literal (0x60 there, 0x70 here) and in its taken arm (that
 *           sibling reloads the cell and increments work byte 2, this body calls
 *           the shared helper instead). A byte scan of every shipped bmagic
 *           payload found no byte-identical twin of this 0x54-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSlotGridHelper(
    void) {
  func_801F0274();
  if ((g_battle_work[9] += 1) == 0x70) {
    func_801E5988();
  }
}
