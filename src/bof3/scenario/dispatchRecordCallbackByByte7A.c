#include "bof3/scenario/scena00_internal.h"

/* @behavior dispatches one record callback selected by byte 0x7a.
 * @source 0x801FC7D0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchRecordCallbackByByte7A(void* record) {
  Scena00RecordCallback callback;

  /*
   * MATCHING_AID:
   * Original emits the prologue (addiu $sp / sw $ra) before the record and
   * D_8014686C loads; current schedules both loads above the prologue. This
   * barrier keeps the loads after the frame setup. Remove when the original
   * scheduling constraint is understood.
   */
  barrier();
  callback = D_801FCA84[((const u8*)record)[0x7a]];
  callback(record, D_8014686C);
}
