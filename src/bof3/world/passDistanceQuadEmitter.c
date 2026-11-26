#include "bof3/bof3.h"

/* Quad emitter of this overlay. The lifted span at 0x801F3060 hands it the
 * three-word triple of the work record, and its body loads 0x0($a0), 0x4($a0)
 * and 0x8($a0), jitters the first two words with the shared random helpers
 * func_801782FC and func_801783C8, converts them with func_801AFF04, and emits
 * sixteen semi-transparent textured quads through GetTPage, SetDrawMode,
 * func_80155A08, SetPolyG4 and SetSemiTrans while growing a radius byte four at
 * a time and wrapping it at 0x10. The triple therefore reaches it as a position
 * pair plus the radius those quads spread by.
 * @source 0x801F30A8 @kind function
 */
extern void drawProjectedQuad(s32* position);

/* @source 0x801F3060
 * @behavior Overlay step handler: reads the work record published at the
 * scratchpad pointer cell 0x1F800044 and copies its three words at +0x34, +0x38
 * and +0x3C unchanged into a three-word stack triple, then passes a pointer to
 * that triple to the overlay quad emitter drawProjectedQuad. Takes no arguments and
 * returns nothing; the pointer load is hoisted above the 0x28-byte frame setup,
 * each of the three loads carries a stall nop before its 0x10, 0x14 and 0x18
 * stack store, and the third store fills the call delay slot.
 *
 * Name evidence: this span is entry 1 of the overlay step table at 0x801F3824,
 * the same table the sibling dispatcher dispatchByte1Table3824
 * indexes with the record byte at +1. Entry 0 of that table,
 * stateCursorDistance,
 * hands the record words +0x34 and +0x38 to the shared distance
 * helper func_8015477C and stores that answer shifted left 16 into +0x3C, so the
 * triple the second step forwards is that coordinate pair plus the distance
 * entry 0 computed from it.
 *
 * Levers declined. Data: the span opens with a frame and a saved $ra, ends with
 * jr $ra, and this payload's word 0x801F3828 names it as a handler, so it is a
 * function; analysis data-candidates on this target reports no candidate. Carve:
 * the row is 0x48 bytes with no slack, the body begins at its own prologue and
 * ends at its own jr $ra, and the next subsegment starts at 0x801F30A8, so
 * nothing continues from another segment. Clone: a scan of the 398 extracted
 * under out/binaries for these 72 bytes returns only this span's own offset in
 * its own payload, and the call immediate here names an address inside this
 * payload, so no byte-identical sibling exists.
 *
 * The caller is unnamed: no word of the payload holds this span's address except
 * the table word 0x801F3828, which the dispatcher reaches through an index, so
 * the argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passDistanceQuadEmitter(void) {
  u8* work;
  s32 position[3];

  work = SPAD_PTR_SLOT(u8, 0x44u);
  position[0] = FIELD_REF(s32, work, 0x34u);
  position[1] = FIELD_REF(s32, work, 0x38u);
  position[2] = FIELD_REF(s32, work, 0x3Cu);
  drawProjectedQuad(position);
}
