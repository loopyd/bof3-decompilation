#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body re-reads the cell before each of the five byte
 * clears: a store through the loaded pointer may alias the cell, so the original
 * re-materialises the address with its own lui / lw pair before every clear. The
 * plain cell is kept rather than the volatile one several bmagic siblings use,
 * because every read after the first is separated from the previous read by a
 * store through the loaded pointer; the exact sibling
 * clearWorkBytes0Through4BmagicMagic14603_801EF964 stores the same five bytes
 * from the same plain declaration and re-materialises the cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The generated undefined_syms_auto.txt already binds the address, so no
 * target-map row is needed. The record's byte 0xB is the countdown the exact
 * siblings decrementWorkByte9ByTwoThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenZeroBmagicMagic17203_801EF4BC and
 * addWorkMotionPairIncrementByte9DecrementByteABy2ThenClearWorkBytes0To4And
 * DecrementGlobalWorkByteBBmagicMagic04303_801EF64C decrement too, each
 * unloading the cell once with lui / lw and reading the byte through it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable helper at 0x800AF764. This overlay calls it with no
 * arguments and no result as a step ends; the generated undefined_funcs_auto.txt
 * row for that address binds it. No lifted body for it existed in this tree when
 * this seed was written, so it keeps its raw name. */
void func_800AF764(void);

/* This overlay's own draw step at 0x801EF6A0, still an asm row in this target:
 * it selects a draw mode from GetGraphType, installs it on the prim cursor and
 * queues the record's primitives. Kept at its raw name because the span has no
 * authored body. */
void func_801EF6A0(void);

/* @source 0x801EF614
 * @behavior Ends one step of this overlay's work handler. It first calls the
 *           shared main-executable helper at 0x800AF764 and then this overlay's
 *           draw step at 0x801EF6A0, both with no arguments and no result, then
 *           reads the shared battle work record pointer from 0x801EB4E0 and
 *           decrements that record's byte 0xB by one with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb). Finally it clears work
 *           bytes 0x00 through 0x04 of the record published through the
 *           scratchpad pointer cell at 0x1F800044, storing $zero with one sb per
 *           byte and re-materialising the cell address with its own lui / lw
 *           pair before each store, because a store through the loaded pointer
 *           may alias the cell. The fifth cell load's delay slot holds the $ra
 *           restore, its sb fills the load-to-store gap, and the return uses the
 *           jr / addiu delay-slot pair. The 0x18-byte frame keeps $ra at 0x10
 *           across the two calls; nothing is returned and no other state is read
 *           or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndClearWorkBytes0Through4BmagicMagic08103_801EF614(void) {
  func_800AF764();
  func_801EF6A0();
  D_801EB4E0[0xB]--;
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
