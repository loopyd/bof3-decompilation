#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8 *scenarioScena0900Work;

void clearSelectedRecordBankLeadBytesScenarioScena0900_801F7574(void);
void func_801F76D0(void);

/* @source 0x801F73A0
 * @behavior Initializes the selected record bank by clearing its lead bytes
 * through 0x801F7574 and then calling the record-seeding helper 0x801F76D0.
 * Sets scratchpad work-object byte 9 to 255, reloads the work pointer at
 * 0x1F800044, and increments its unsigned state byte 1 modulo 256.
 * Entry 0 of the work-byte-1 handler table at 0x801FE384. Takes no arguments
 * and returns nothing; both helper results are ignored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeRecordBankArmByte9AndAdvanceStateScenarioScena0900_801F73A0(void) {
    clearSelectedRecordBankLeadBytesScenarioScena0900_801F7574();
    func_801F76D0();
    scenarioScena0900Work[9] = 255;
    scenarioScena0900Work[1]++;
}
