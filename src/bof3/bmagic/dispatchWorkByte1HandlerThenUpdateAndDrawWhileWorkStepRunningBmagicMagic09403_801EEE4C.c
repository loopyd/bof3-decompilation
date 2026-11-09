#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original materialises the cell with its own lui/lw pair before the
 * frame is created and never reads it again, so the plain named cell suffices. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Four-entry code-pointer run at 0x801EF988, the work-step handler table this
 * dispatcher indexes: entry 0 = 0x801EEF1C, entry 1 = 0x801EEF7C, entry 2 =
 * 0x801EF078 and entry 3 = 0x801EF0FC, the work steps of this overlay, of which
 * the first two and the last are lifted. The run closes the payload as its own
 * 16-byte Splat row, whose label the map keeps because the row is not this
 * function's span; only the word the scaled index adds to is declared here and
 * the index is not bounded. */
extern void (*func_801EF988[])(void); /* @source 0x801EF988 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The remaining asm span func_801EF710 still references the address, so Splat's
 * generated undefined_syms_auto.txt binds it and no target-map row is needed.
 * Byte 0xB of that record is the work step the exact sibling
 * incrementWorkByte9ThenSetGlobalWorkByteBToFFAndInvokeHelperWhenWorkByte9Is20BmagicMagic09403_801EF0FC
 * rewrites to 0xFF when the step finishes, which is the terminal value tested
 * here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* This overlay's transform step: pushes the current matrix, installs the work
 * record's rotation and translation through SetRotMatrix/SetTransMatrix/
 * RotMatrix/MulMatrix2 and projects the record origin with RotTrans. Lifted
 * exact at 0x801EF654, and it takes no arguments. */
void projectWorkObjectPositionAndInstallTransformWithWorkByteAAngleBmagicMagic09403_801EF654(
    void);

/* This overlay's large draw step: samples the shared phase helpers
 * func_801782FC/func_801783C8/func_8017A97C, picks its draw-mode tpage from
 * GetGraphType and emits three-vertex primitives through SetDrawMode,
 * SetSemiTrans and RotTransPers3. Unlifted, so it keeps its raw name. */
void func_801EF14C(void);

/* This overlay's second draw step over the same shared helpers, emitting prims
 * through SetDrawMode and SetSemiTrans. Unlifted, so it keeps its raw name. */
void func_801EF710(void);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EEE4C
 * @behavior Per-frame work-step entry of this overlay. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 before it creates its
 *           0x20-byte frame, scales the work object's unsigned byte 1 by four
 *           and calls that entry of the four-entry code-pointer run at
 *           0x801EF988 with no arguments; the original does not bound the
 *           index. It then reads the shared battle work-record pointer at
 *           0x801EB4E0 and, while that record's byte 0xB differs from 0xFF (the
 *           step-finished value the exact sibling at 0x801EF0FC stores), calls
 *           this overlay's transform step at 0x801EF654, its large draw step at
 *           0x801EF14C, its second draw step at 0x801EF710 and PopMatrix, in
 *           that order and with no arguments. Finally it selects the draw-mode
 *           tpage from GetGraphType - 0x25 for graph types 1 and 2, 0x15
 *           otherwise - seeds SetDrawMode on the shared primitive cursor at
 *           0x8014598C with dfe 0, dtd 1 and a null tw (the zero store at
 *           0x10($sp) fills that 0x20-byte frame's argument slot) and appends a
 *           0xC-byte primitive for output list 3 through the shared
 *           primitive-append helper at 0x8014E5A0. The $ra reload precedes the
 *           final append. It takes no arguments, returns nothing and reads or
 *           writes no other state.
 *
 *           Shape and naming evidence: the work-byte dispatch through an
 *           overlay code-pointer run follows the lifted bmagic sibling
 *           dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24
 *           (same cell-then-scaled-byte indexing, same unbounded index, same
 *           PopMatrix arm) and
 *           dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenActiveBmagicMagic00603_801F0064
 *           (dispatch, then the overlay's update and draw calls, then the
 *           draw-mode seed); the tpage selection over GetGraphType and the
 *           SetDrawMode call with dfe 0 and dtd 1 follow
 *           drawFourRotatedSpokesFromPublishedCenterTintedByWorkByte5BmagicMagic16203_801EF80C,
 *           and the primitive append form follows
 *           drawSixTileTexturedRowAtWorkHalfwords2EAnd30FromTileTableBmagicMagic08000_801EED94
 *           of this same bmagic family. This target's own siblings supply the
 *           pointer-cell and record-byte declarations. A byte scan of all 122
 *           emi/bmagic payloads finds no exact twin of this 0xD0-byte span (not
 *           even on its first 0x40 bytes), so the clone lever had no source and
 *           the body is seeded from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerThenUpdateAndDrawWhileWorkStepRunningBmagicMagic09403_801EEE4C(
    void) {
  u8 *record;
  s32 tpage;

  func_801EF988[g_battle_work[1]]();
  record = D_801EB4E0;
  if (record[0xB] != 0xFF) {
    projectWorkObjectPositionAndInstallTransformWithWorkByteAAngleBmagicMagic09403_801EF654();
    func_801EF14C();
    func_801EF710();
    PopMatrix();
  }
  if (GetGraphType() == 1) {
    tpage = 0x25;
  } else if (GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(3, 0xC);
}
