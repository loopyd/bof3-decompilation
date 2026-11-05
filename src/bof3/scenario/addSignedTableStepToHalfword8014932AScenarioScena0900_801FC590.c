#include "bof3/bof3.h"

extern u16 D_80146876;
extern u8 D_80149333;
/* @source 0x8014932A @kind data */
extern u16 D_8014932A;
/* Four bytes: 0, 1, 0, -1 when interpreted as signed steps.
 * @source 0x801FE430 @kind table
 */
extern u8 signedStepTableScenarioScena0900_801FE430[];

/* @source 0x801FC590
 * @behavior Selects a byte from the four-entry table at 0x801FE430 using
 * D_80146876 & 3, publishes mode 2 in D_80149333, then adds four times
 * the signed byte to halfword D_8014932A, wrapping modulo 65536.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addSignedTableStepToHalfword8014932AScenarioScena0900_801FC590(void) {
    u8 step;

    step = signedStepTableScenarioScena0900_801FE430[D_80146876 & 3];
    D_80149333 = 2;
    D_8014932A = (u16)(D_8014932A + (s32)(s8)step * 4);
}
