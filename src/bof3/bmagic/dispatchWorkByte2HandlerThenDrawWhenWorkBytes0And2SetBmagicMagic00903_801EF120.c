#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original
 * re-materialises the cell with its own lui/lw pair for the dispatch and again
 * with a second lui/lw pair after the indirect call, because that call may
 * republish the pointer, so both reads stay on this named cell rather than on a
 * pointer local, as in the exact bmagic siblings
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes2And0SetBmagicMagic17203_801EF518
 * and dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenActiveBmagicMagic00603_801F0064,
 * which read the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Third word of the fourteen-word code-pointer run this overlay stores at
 * 0x801EF89C, which Splat places as the single `textbin` row D_801EF89C. The
 * run's first five words are in-image code pointers - 0x801EF120, 0x801EF6B0,
 * 0x801EF198, 0x801EF2D4 and 0x801EF340 - and the byte-2 handlers this
 * dispatcher reaches are the run's tail from this word on: 0x801EF198 for index
 * 0, 0x801EF2D4 for index 1 and 0x801EF340 for index 2. The work record's byte
 * 2 scales by four and is added to 0x801EF8A4, so the extern is declared at the
 * indexed word and the run stays the single D_801EF89C row in this target's
 * map; the generated undefined_syms_auto.txt row for 0x801EF8A4 already names
 * this word. */
extern void (*D_801EF8A4[])(void); /* @source 0x801EF8A4 @kind data */

/* This overlay's translucent textured quad draw step at 0x801EF3B8, lifted
 * exact from this span's own bytes in
 * src/bof3/bmagic/drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWork
 * Byte9TintedByWorkByteATimes10And9BmagicMagic00903_801EF3B8.c. It selects a
 * draw mode from GetGraphType, arms SetDrawMode on the primitive cursor, links a
 * 0xC-byte packet, then emits a textured quad through SetPolyFT4 and
 * SetSemiTrans with its vertices taken from the work record's screen pair at
 * halfwords 0x2E and 0x30 and the tile and clut from GetTPage and GetClut. */
void drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes10And9BmagicMagic00903_801EF3B8(
    void);

/* @source 0x801EF120
 * @behavior Dispatches one step of the overlay work handler and then draws the
 *   active record. The work record is read from the scratchpad pointer cell at
 *   0x1F800044, its unsigned byte at offset 0x2 is scaled by four and selects
 *   one entry of the handler table word at 0x801EF8A4, which is called with no
 *   arguments; the original does not bound the index. The cell is then re-read
 *   and the record's bytes at offsets 0x0 and 0x2 are tested, so only when both
 *   are non-zero does the arm run: it calls this overlay's draw step at
 *   0x801EF3B8 with no arguments. The cell is re-read rather than kept live
 *   because the dispatched handler may republish it, which is why the original
 *   materialises it twice. The 0x18-byte frame only keeps $ra at 0x10 across
 *   the indirect call and the load of the cell precedes frame creation. The
 *   120-byte span takes nothing, returns nothing and reads or writes no other
 *   state, and the branch slots after the index byte, the table load and both
 *   condition bytes stay empty.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic00903_801EF120(
    void) {
  D_801EF8A4[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimes10And9BmagicMagic00903_801EF3B8();
  }
}
