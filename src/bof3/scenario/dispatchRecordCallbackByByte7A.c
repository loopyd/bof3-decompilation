#include "bof3/scenario/scena00_internal.h"

/* @behavior dispatches one record callback selected by byte 0x7a.
 * @source 0x801FC7D0
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchRecordCallbackByByte7A(void* record) {
  Scena00RecordCallback callback;

  callback = D_801FCA84[((const u8*)record)[0x7a]];
  callback(record, D_8014686C);
}
