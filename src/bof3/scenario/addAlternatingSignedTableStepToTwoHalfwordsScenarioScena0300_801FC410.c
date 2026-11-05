#include "bof3/bof3.h"

extern u16 D_80146876;
/* @source 0x80145ECE @kind data */
extern u16 D_80145ECE;
/* @source 0x8014600E @kind data */
extern u16 D_8014600E;
/* Signed steps -1 and 1 at indices 0 and 1.
 * @source 0x801FD008 @kind table
 */
extern s8 alternatingSignedStepTableScenarioScena0300_801FD008[];

/* @source 0x801FC410
 * @behavior Indexes the two signed bytes at 0x801FD008 with the low bit
 * of D_80146876. Adds sixteen times the selected byte to the halfword at
 * 0x80145ECE, then rereads the table entry and adds sixteen times that byte
 * to the halfword at 0x8014600E. Both stores wrap modulo 65536.
 * Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addAlternatingSignedTableStepToTwoHalfwordsScenarioScena0300_801FC410(void) {
    u32 index;
    u16* first;

    index = D_80146876 & 1;
    first = &D_80145ECE;
    *first += alternatingSignedStepTableScenarioScena0300_801FD008[index] * 16;
    D_8014600E += alternatingSignedStepTableScenarioScena0300_801FD008[index] * 16;
}
