#include "bof3/bof3.h"

/* Shared frame counter. @source 0x80143E6C @kind unknown */
extern s32 D_80143E6C;
/* @source 0x80149318 @kind unknown */
extern u32 D_80149318;
/* Area status byte. @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Four bytes: 0, 1, 0, -1 when interpreted as signed frame steps.
 * @source 0x801FA220 @kind table
 */
extern s8 signedStepTableScenarioScena0400_801FA220[];

/* @source 0x801F9CFC
 * @behavior Overlay handler: takes bits 1 and 2 of the shared frame counter
 *           D_80143E6C as an index into the four-entry signed step table at
 *           0x801FA220 (0, 1, 0, -1), so the index steps once every two
 *           frames; publishes mode 2 in the area status byte D_80149333 and
 *           then adds four times the selected step to the word D_80149318, so
 *           the eight-frame cycle adds four on frames 2 and 3, subtracts four
 *           on frames 6 and 7 and changes nothing on the other four. It
 *           re-reads D_80149318 after the status store and writes the sum back
 *           through its own address pair. Its 0x4C bytes are the exact shape of
 *           addSignedTableStepToWord80149318World02Area10313_801F57B4, which
 *           indexes the same table contents with the counter's low two bits
 *           and scales by eight. Takes no arguments, returns nothing and uses
 *           no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addSignedTableStepToWord80149318ScenarioScena0400_801F9CFC(void) {
  s32 step;

  step = signedStepTableScenarioScena0400_801FA220[((u32)D_80143E6C >> 1) & 3];
  D_80149333 = 2;
  D_80149318 = D_80149318 + step * 4;
}
