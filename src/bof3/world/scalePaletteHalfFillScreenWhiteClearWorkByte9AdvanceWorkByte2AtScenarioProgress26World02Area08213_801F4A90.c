#include "bof3/bof3.h"

extern u8 D_8014933A;
extern u8 g_ScenarioProgress;

/* Scratch work-record pointer cell published by the overlay. The original reads
 * it once per access, re-materializing its address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) for each of the two accesses, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Palette-scale helper, the overlay's own routine at 0x801F5388: it multiplies
 * every 5-bit colour channel of all 0x100 entries of the palette window by the
 * signed caller scale, shifts each product right by 7 and preserves bit 15 of
 * the source word, so 0x80 is the unity scale. Splat binds the address through
 * this target's map row. */
void scalePaletteChannelsWorld02Area08213_801F5388(s32 scale);

/* Screen-gradient fill helper, the overlay's own routine at 0x801F5428. Its
 * body splits the packed word it is handed into two five-bit-per-channel
 * colours (top edge from bits 0-14, bottom edge from bits 16-30), draws a
 * 0x140 x 0xF0 SetPolyG4 quad through the primitive cursor cell at 0x8014598C
 * with one colour on the top edge and the other on the bottom edge, advances
 * that cursor by 0x24 and returns nothing. Splat binds the address through
 * this target's map row. */
void func_801F5428(s32 color);

/* @source 0x801F4A90
 * @behavior Overlay step handler, entry index 1 of the sixteen-pointer step
 *           table at 0x801F57D0, gated on the shared scenario progress byte at
 *           0x80146864. Only when that byte is 0x26 does it scale the
 *           0x100-entry palette window by 0x40 through the palette-scale helper
 *           at 0x801F5388, which halves every channel because that helper
 *           divides each 5-bit channel by 2^7; it then clears the shared
 *           palette-stage byte at 0x8014933A and hands the colour word -1 to
 *           the screen-gradient fill helper at 0x801F5428, whose extraction
 *           leaves 0xF8 for every channel of both edges, so the whole
 *           0x140 x 0xF0 screen is filled with solid white. It finishes by
 *           clearing byte +0x09 of the scratch work record published through
 *           pointer cell 0x1F800044 and advancing that record's dispatch byte
 *           at offset 0x02 by one, which passes control to the next handler;
 *           the work cell is re-read for each of those two accesses, each with
 *           its own re-materialized cell address. Every other progress value
 *           leaves the record, the palette and the screen untouched. The
 *           function has a 0x18-byte frame holding the saved return address at
 *           0x10, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void scalePaletteHalfFillScreenWhiteClearWorkByte9AdvanceWorkByte2AtScenarioProgress26World02Area08213_801F4A90(
    void) {
  if (g_ScenarioProgress == 0x26) {
    scalePaletteChannelsWorld02Area08213_801F5388(0x40);
    D_8014933A = 0;
    func_801F5428(-1);
    workRecordCursor1F800044[9] = 0;
    workRecordCursor1F800044[2]++;
  }
}
