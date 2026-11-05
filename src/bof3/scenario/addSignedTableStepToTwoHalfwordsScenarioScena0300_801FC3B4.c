#include "bof3/bof3.h"

extern u16 D_80146876;
/* @source 0x80145ECE @kind data */
extern u16 D_80145ECE;
/* @source 0x8014600E @kind data */
extern u16 D_8014600E;
/* Signed steps -1, 0, 1, 0 at indices 0 through 3.
 * @source 0x801FD004 @kind table
 */
extern s8 signedStepTableScenarioScena0300_801FD004[];

/* @source 0x801FC3B4
 * @behavior Indexes the four signed bytes at 0x801FD004 with the low two
 * bits of D_80146876. Adds sixteen times the selected byte to the halfword
 * at 0x80145ECE, then rereads the table entry and adds sixteen times that
 * byte to the halfword at 0x8014600E. Both stores wrap modulo 65536.
 * Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addSignedTableStepToTwoHalfwordsScenarioScena0300_801FC3B4(void) {
    u32 index;
    u16* first;

    index = D_80146876 & 3;
    first = &D_80145ECE;
    *first += signedStepTableScenarioScena0300_801FD004[index] * 16;
    D_8014600E += signedStepTableScenarioScena0300_801FD004[index] * 16;
}
