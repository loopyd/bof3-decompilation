#include "bof3/bof3.h"

extern u8 D_80181B1A;
extern u8 D_8014496F[];
extern u8 D_80146254;

void func_801C187C(s32 arg0);

/* @source 0x801FE6DC
 * @behavior Clears bit 0 of the byte at D_8014496F plus 0xA4 times the
 * unsigned index D_80181B1A, clears the shared work-record count D_80146254,
 * then calls the effect-reset helper func_801C187C with id 7. This is entry 4
 * of the callback table at 0x801FE9C0; it ignores the dispatcher's arguments.
 * The indexed byte's wider role is unknown. The helper's return is ignored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearIndexedFlagBit0AndWorkCountThenResetEffect7ScenarioScena0800_801FE6DC(void) {
    s32 offset;

    offset = D_80181B1A * 0xA4;
    D_8014496F[offset] = D_8014496F[offset] & 0xFE;
    D_80146254 = 0;
    func_801C187C(7);
}
