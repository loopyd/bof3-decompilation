#include "bof3/scenario/scena16_internal.h"

/* @behavior dispatches one record callback selected by byte 0x7a.
 * @source 0x801F8358
 * @status invalid
 * @match 87.50
 * @residual incompatible callback types and unresolved engine return contract; prologue scheduling differs, 64 bytes versus 64 original.
 */
void dispatchRecordCallback(void* record) {
  Scena16RecordCallback callback;

  callback = recordCallbackTable[((const u8*)record)[0x7a]];
  callback(record, D_8014686C);
}
