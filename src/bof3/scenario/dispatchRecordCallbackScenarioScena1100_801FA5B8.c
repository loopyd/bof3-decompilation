#include "bof3/bof3.h"

typedef void (*Scena1100RecordCallback)(u8* record, u32 arg1);

extern Scena1100RecordCallback D_801FAF24[];
extern u32 D_8014686C;

/* @source 0x801FA5B8
 * @behavior Dispatches this overlay's per-record callback selected by the record's unsigned byte
 * field at offset 0x7A through the overlay-local callback-pointer run at 0x801FAF24 (its 29
 * entries begin with noopHandlerScenarioScena1100_801FA5F8, func_801FA600, func_801FA674 and
 * func_801FA6C0 and end with invokeHelperArgThenHelperScenarioScena1100_801FAE98), passing the
 * record pointer unchanged as the first argument and the word held in the shared cell D_8014686C
 * as the second. Takes the record as its only argument and returns nothing; the 0x18-byte frame
 * saves $ra because the target callback is an ordinary indirect call rather than a sibling tail
 * call, and the shared cell is read before the index is scaled by four. The 64 bytes are
 * instruction-for-instruction identical to the exact record-callback dispatcher of the sibling
 * EMI scenario overlay SCENA15 (dispatchRecordCallbackScenarioScena1500_801FDD80 0x801FDD80),
 * which differs only in the table address; the address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena1100_801FA5B8(u8* record) {
  D_801FAF24[record[0x7A]](record, D_8014686C);
}
