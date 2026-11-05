#include "bof3/bof3.h"

/* Shared frame counter. @source 0x80143E6C @kind unknown */
extern s32 D_80143E6C;
/* @source 0x80149318 @kind unknown */
extern u32 D_80149318;
/* Area status byte. @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Four bytes: 0, 1, 0, -1 when interpreted as signed frame steps.
 * @source 0x801F5D68 @kind table
 */
extern s8 signedStepTableWorld02Area10313_801F5D68[];

/* @source 0x801F57B4
 * @behavior Overlay handler: selects a signed byte from the four-entry table
 *           at 0x801F5D68 with the low two bits of the shared frame counter
 *           D_80143E6C, publishes mode 2 in the area status byte D_80149333,
 *           then adds eight times the selected step to the word D_80149318, so
 *           the counter cycle contributes 0, 8, 0 and -8 over four frames. It
 *           re-reads D_80149318 after the status store and writes the sum back
 *           through its own address pair. Takes no arguments, returns nothing
 *           and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addSignedTableStepToWord80149318World02Area10313_801F57B4(void) {
  s32 step;

  step = signedStepTableWorld02Area10313_801F5D68[D_80143E6C & 3];
  D_80149333 = 2;
  D_80149318 = D_80149318 + step * 8;
}
