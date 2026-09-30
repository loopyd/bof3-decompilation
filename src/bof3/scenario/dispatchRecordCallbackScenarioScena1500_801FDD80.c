#include "bof3/bof3.h"

typedef void (*Scena1500RecordCallback)(u8* record, u32 arg1);

extern Scena1500RecordCallback D_801FE6EC[];
extern u32 D_8014686C;

/* @source 0x801FDD80
 * @behavior Dispatches this overlay's per-record callback selected by the record's unsigned
 * byte field at offset 0x7A through the overlay-local callback-pointer table at 0x801FE6EC
 * (twelve entries: 0x801FDDC0 five times, then 0x801FDE2C, 0x801FDE64, 0x801FDEB4, 0x801FDF04,
 * 0x801FDF4C, 0x801FDF9C and 0x801FDFEC), passing the record pointer unchanged and the word
 * held in the shared cell D_8014686C as the second argument. It is the SCENA15 twin of the
 * exact scena18 dispatcher at 0x801F6CAC (dispatchRecordCallback_scena18) and of the scena00
 * dispatcher dispatchRecordCallbackByByte7A at 0x801FC7D0. Takes the record as its only
 * argument; the 0x18-byte frame saves $ra because the target callback is an ordinary indirect
 * call rather than a sibling tail call, and the shared cell is read before the index is scaled
 * by four.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena1500_801FDD80(u8* record) {
  D_801FE6EC[record[0x7A]](record, D_8014686C);
}
