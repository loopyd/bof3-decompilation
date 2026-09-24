#include "bof3/bof3.h"

typedef void (*Scena18RecordCallback)(u8* record, u32 arg1);

extern Scena18RecordCallback D_801F6D7C[];
extern u32 D_8014686C;

/* @source 0x801F6CAC
 * @behavior Dispatches the per-record callback indexed by the record's unsigned
 * @status exact
 * @match 100.00
 * @residual none
 * byte field at offset 0x7A through this overlay's callback-pointer table at
 * 0x801F6D7C, passing the record pointer and the word held in the shared cell
 * at 0x8014686C. It is the scena18 twin of scena00's
 * dispatchRecordCallbackByByte7A and saves the return address because the
 * target callback is an ordinary call, not a sibling tail call.
 */
void dispatchRecordCallback_scena18(u8* record) {
  D_801F6D7C[record[0x7A]](record, D_8014686C);
}
