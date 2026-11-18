#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original materialises the cell with its own lui/lw pair for the
 * dispatch and again with a second lui/lw pair after the indirect call, because
 * that call may republish the pointer, so both reads stay on this named cell
 * rather than on a pointer local, as in the exact bmagic sibling
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic00903_801EF120. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The six-word code-pointer run at 0x801EFC9C, which Splat places as the
 * single `textbin` row T_801EFC9C. Its words are this overlay's step handlers
 * 0x801EEE0C, 0x801EEF48, 0x801EF020, 0x801EF090, 0x801EF0E0 and 0x801EF130,
 * the ones the exact handlers of this target advance with work byte 0x01. The
 * work object's byte 0x01 scales by four and is added to this base, so the
 * extern is declared at the run head and the run keeps its one row in this
 * target's Splat. */
extern void (*T_801EFC9C[])(void); /* @source 0x801EFC9C @kind data */

/* This overlay's two transform passes, both still `asm` rows in this target's
 * Splat. 0x801EF5C0 pushes the matrix stack and installs the projection
 * transform for the active record, 0x801EF6C0 draws that record through
 * SetDrawMode/SetPolyG4/RotTransPers4, 0x801EF180 pushes the matrix stack and
 * installs the record transform through RotMatrix/MulMatrix2/SetRotMatrix, and
 * 0x801EF2D4 draws it through RotTransPers. */
void func_801EF5C0(void);
void func_801EF6C0(void);
void func_801EF180(void);
void func_801EF2D4(void);

/* @source 0x801EED7C
 * @behavior Per-frame update of the work object published at the scratchpad
 *           pointer cell 0x1F800044. It reads the object's unsigned byte 0x01,
 *           scales it by four and calls that word of the six-entry step-handler
 *           run at 0x801EFC9C with no arguments; the original does not bound
 *           the index. The cell is then re-read, because the handler may
 *           republish it, and the object's byte 0x00 is tested, so only when it
 *           is non-zero do the two draw passes run: 0x801EF5C0 pushes the
 *           matrix stack and installs the projection transform, 0x801EF6C0
 *           draws the active record, PopMatrix at 0x80178C18 pops the stack,
 *           0x801EF180 pushes the matrix stack and installs the record
 *           transform, 0x801EF2D4 draws it and PopMatrix pops again. The
 *           0x18-byte frame only keeps $ra at 0x10 across the calls, the cell
 *           load precedes frame creation, and every branch slot stays empty.
 *           The body takes no arguments and returns nothing. No direct branch
 *           or aligned pointer word in the payload names 0x801EED7C; its caller
 *           and runtime entry route remain unproven. Statement order, operand forms and
 *           declaration style follow the exact siblings
 *           dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic00903_801EF120
 *           (same cell, same indirect dispatch of a work-byte index over an
 *           in-image handler run, same re-read and non-zero byte 0x00 gate)
 *           and setDrawModeThenDispatchWorkByte2HandlerBmagicMagic12003_801EFD20
 *           (same jalr over a run of no-argument code pointers). Bytes
 *           0x801EED7C..0x801EEE0C are one body: the words Splat placed in the
 *           `textbin` row 0x801EEDBC continue the load of the work pointer made
 *           at 0x801EEDB4, and the trailing row 0x801EEDFC holds this body's own
 *           epilogue, the 0x801EEDC4 branch's target and its fall-through, with
 *           no other in-image reference. No byte-identical copy of this 0x90
 *           byte span exists in any overlay payload of this repository's
 *           binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerThenDrawTwoMatrixPassesWhenWorkByte0SetBmagicMagic10803_801EED7C(
    void) {
  T_801EFC9C[g_battle_work[1]]();
  if (g_battle_work[0] != 0) {
    func_801EF5C0();
    func_801EF6C0();
    PopMatrix();
    func_801EF180();
    func_801EF2D4();
    PopMatrix();
  }
}
