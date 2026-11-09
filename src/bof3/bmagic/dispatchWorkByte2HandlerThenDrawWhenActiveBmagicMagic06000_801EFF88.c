#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materializes this cell
 * with its own lui/lw pair for the dispatch index and again after the indirect
 * call, so it is read through this named cell instead of the constant-address
 * accessor, whose folded address stays live across the call in a saved
 * register and grows the frame. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Overlay step-handler table for work-object byte 2, based at 0x801F0BBC in
 * this overlay's trailing read-only block; its entries are the overlay state
 * blocks that follow this dispatcher, the first being 0x801F0000. The original
 * adds the scaled byte to this word, so the extern is declared at the first
 * entry, not at the start of the pointer run (0x801F0BB0) that holds the byte 1
 * table. */
extern void (*D_801F0BBC[])(void); /* @source 0x801F0BBC @kind data */

/* Overlay drawing routine this dispatcher invokes once the work object is
 * active: it calls GetGraphType, then SetDrawMode and SetTile through
 * g_PrimCursor plus the graph helper func_8014E5A0. It was still unlifted when
 * this body was seeded and keeps its raw name. */
extern void func_801F012C(void);

/* @source 0x801EFF88
 * @behavior Index 0 of the handler table at 0x801F0BB0 that the work-byte-1
 * dispatcher at 0x801EFF44 indexes with work byte 1, and itself a dispatcher:
 * it reads the scratchpad work-object pointer cell at 0x1F800044 into a work
 * pointer and uses the work object's byte at offset 0x2, scaled by four, to
 * select one entry of the handler table at 0x801F0BBC (first entry 0x801F0000),
 * which it calls with no arguments. The original does not check the index. When
 * the handler returns the pointer cell is read again in a second load; if the
 * work object's byte at offset 0x0 and its byte at offset 0x2 are both non-zero
 * it calls this overlay's drawing routine func_801F012C. The two pointer reads
 * are separate locals because the original materializes the cell twice, before
 * and after the indirect call, and the intervening call is what invalidates the
 * first value. The two guards are one conjunction: both tests branch to the
 * same epilogue label. Reads no other state, stores nothing but the saved
 * return address and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic06000_801EFF88(void) {
  u8 *stepWork;
  u8 *gateWork;

  stepWork = g_battle_work;
  D_801F0BBC[stepWork[2]]();
  gateWork = g_battle_work;
  if (gateWork[0] != 0 && gateWork[2] != 0) {
    func_801F012C();
  }
}
