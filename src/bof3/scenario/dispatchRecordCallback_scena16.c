#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches one record callback selected by byte 0x7a.
 * @source 0x801F8358
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchRecordCallback(void* record) {
  Scena16RecordCallback callback;

  callback = recordCallbackTable[((const u8*)record)[0x7a]];
  callback(record, D_8014686C);
}
