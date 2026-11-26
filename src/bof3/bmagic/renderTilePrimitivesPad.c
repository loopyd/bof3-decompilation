#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name the exact bmagic lifts of this family use for the
 * same cell. The original materialises the cell once, after the pad test, with
 * its own lui / lw pair, so the store stays on the named cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared pressed-pad-state halfword in main RAM at 0x80145AA8, outside this
 * payload and the same cell the exact bmagic lift
 * drawPartyOrEnemyTargetResultPanelThenSetWorkByteBTo83AndAdvanceWorkByte1WhenPadStateSetBmagicMagic06000_801EEE84
 * tests as `D_80145AA8 != 0` after a frame. Splat already binds the address
 * through this target's generated undefined_syms_auto.txt. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind unknown */

/* Drawing helper of this overlay at 0x801EF55C, still an asm row of this
 * target: it reads GetGraphType, sets a draw mode, walks the work record's flag
 * bytes and emits SetTile primitives at g_PrimCursor. Its role stays
 * address-anchored here because its own body is not lifted. */
extern void renderTilePrimitives(void); /* @source 0x801EF55C @kind function */

/* @source 0x801EF4A8
 * @behavior Frame step of this overlay's leading handler run: the word at
 *           payload offset 0x10, entry 3 of the five code pointers 0x801EEC88,
 *           0x801EED50, 0x801EF44C, 0x801EF4A8, 0x801EF4EC that the dispatcher
 *           at 0x801EEC18 copies and indexes with the work object's byte 1, so
 *           no jal word in this payload names this address. It first calls the
 *           drawing helper at 0x801EF55C with no arguments, then reads the
 *           shared pad-state halfword at 0x80145AA8 at halfword width (lhu) and
 *           tests it against zero (beqz). Only when it is nonzero it
 *           re-materialises the scratchpad work pointer cell 0x1F800044 and
 *           stores the literal 4 into the work object's byte 1 (sb), whose
 *           constant the original schedules into the beqz delay slot; that
 *           store selects entry 4, 0x801EF4EC, of the same leading run on the
 *           next dispatch. The cell is read once and only on that path, so it
 *           stays a single access after the test. Frame 0x18 keeps only $ra at
 *           0x10; it takes no arguments, returns nothing and touches no other
 *           state. Field and cell roles stay offset and address anchored: byte
 *           1 is the state index the 0x801EEC18 dispatcher reads, the pad-state
 *           halfword only selects this store, and the drawing helper's own role
 *           is not established by this body. A byte scan of every extracted
 *           payload under out/binaries finds this 68-byte span only in this
 *           target's binary, and the same scan with every j/jal word masked
 *           finds no shape variant either, so no sibling seeds these
 *           statements; the statement order and declaration style follow the
 *           exact bmagic sibling named above for the pad-state and work-byte
 *           idiom.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderTilePrimitivesPad(void) {
  renderTilePrimitives();
  if (D_80145AA8 != 0) {
    g_battle_work[1] = 4;
  }
}
