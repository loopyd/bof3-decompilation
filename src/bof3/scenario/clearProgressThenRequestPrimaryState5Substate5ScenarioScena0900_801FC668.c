#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC668
 * @behavior Entry 2 of the 16-word in-image callback table at 0x801FE434 (word
 * at 0x801FE43C) that the overlay dispatcher
 * dispatchRecordCallbackScenarioScena0900_801FC5E4 indexes with byte 0x7A of a
 * record, so the overlay reaches this handler only indirectly. It runs the
 * shared front-end startup helper func_8015C088 and then clears the shared
 * scenario progress byte g_ScenarioProgress (0x80146864) to zero and requests
 * primary scene state 5 with secondary sub-state 5: it stores 5 in the shared
 * sub-state byte D_80146875 and then 5 in the shared state byte D_80146874.
 * Takes no arguments and returns nothing; the 0x18-byte frame only keeps $ra
 * across the call, each of the three byte stores materialises the 0x8014 page
 * in $at on its own, and the stored constant 5 is materialised in $v0
 * (addiu $v0,$zero,5) ahead of the zero store, which is how the psyq compiler
 * schedules that constant while it keeps the store order. Its 0x3C bytes keep
 * the progress-clear-then-request shape of the exact sibling
 * clearProgressThenRequestPrimaryState5Substate0ScenarioScena0900_801FC62C
 * (0x801FC62C, the table's entry 1), which clears the same progress byte and
 * requests state 5 with sub-state 0 after the same helper call; the two
 * handlers differ only in the sub-state constant, so the two stores share the
 * register holding 5.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressThenRequestPrimaryState5Substate5ScenarioScena0900_801FC668(
    void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146875 = 5;
  D_80146874 = 5;
}
