#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound as g_battle_work by this target's
 * map. The original re-materialises the cell with its own lui/lw pair before
 * the dispatch and again after the indirect call, because the dispatched
 * handler may republish the pointer, so both reads stay on this named cell
 * rather than on a pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work byte 2 step-handler table this overlay dispatches through, indexed at
 * its own base 0x801EFC30: entry 0 = 0x801EF0B8, entry 1 = 0x801EF0F8,
 * entry 2 = 0x801EF148, entry 3 = 0x801EF18C, entry 4 = 0x801EF404. The base
 * is the second word of the nine-word code-pointer run this target stores at
 * 0x801EFC2C and Splat places as the single `textbin` row T_801EFC2C, so the
 * extern is declared at the indexed word and the run stays one map row; the
 * generated undefined_syms_auto.txt row for 0x801EFC30 already names this
 * word. */
extern void (*D_801EFC30[])(void); /* @source 0x801EFC30 @kind data */

/* This overlay's fullscreen quad draw step at 0x801EF1E8, lifted exact from
 * this span's own bytes in
 * src/bof3/bmagic/drawFullscreenTranslucentGouraudQuadTintedByWorkByte9TimesEightAtVerticesZeroAndThreeMirroredByGlobalWorkByte8BitZeroThenRearmGraphTypeDrawModeBmagicMagic22303_801EF1E8.c.
 * It selects the graph-type draw mode, arms SetDrawMode on the primitive
 * cursor, appends a semi-transparent POLY_G4 covering the 320x240 screen
 * rectangle tinted from work byte 9, then re-arms the draw mode. */
void drawFullscreenTranslucentGouraudQuadTintedByWorkByte9TimesEightAtVerticesZeroAndThreeMirroredByGlobalWorkByte8BitZeroThenRearmGraphTypeDrawModeBmagicMagic22303_801EF1E8(
    void);

/* @source 0x801EF050
 * @behavior Dispatches one frame of the overlay work object through its work
 *   byte 2 step table and then draws the fullscreen quad. The work-object
 *   pointer cell at 0x1F800044 is read before the frame adjustment and its
 *   unsigned byte at offset 0x2, scaled by four, selects the code pointer at
 *   0x801EFC30 plus that offset, which is invoked with no arguments; the
 *   table is indexed where it lives and the selector is not bounds-checked.
 *   After the handler returns the pointer cell is read a second time and, when
 *   the work object's byte 0 is non-zero, the overlay's fullscreen quad
 *   drawing routine at 0x801EF1E8 runs. The cell is re-read rather than kept
 *   live because the dispatched handler may republish it, which is why the
 *   original materialises it twice. The 0x18-byte frame keeps only $ra at
 *   0x10 across the indirect call, and the cell load precedes frame creation.
 *   The 104-byte span takes nothing, returns nothing and reads or writes no
 *   other state. A payload cmp proved bytes 0x801EF050..0x801EF0B7 equal to
 *   the 104-byte spans at 0x801EFC58 of emi/bmagic/magic137/03, 0x801EFF24 of
 *   magic067/03 and both 0x800C1E70 and 0x800C256C of emi/boss/boss033/16
 *   apart from the words that encode each span's own table address and callee
 *   address, so statement order, operand forms, offset-local spelling and
 *   declaration style follow those sources with this function's own table
 *   base and callee.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawFullscreenQuadWhenWorkByte0SetBmagicMagic22303_801EF050(
    void) {
  u8 *work = g_battle_work;

  D_801EFC30[work[2]]();
  work = g_battle_work;
  if (work[0] != 0) {
    drawFullscreenTranslucentGouraudQuadTintedByWorkByte9TimesEightAtVerticesZeroAndThreeMirroredByGlobalWorkByte8BitZeroThenRearmGraphTypeDrawModeBmagicMagic22303_801EF1E8();
  }
}
