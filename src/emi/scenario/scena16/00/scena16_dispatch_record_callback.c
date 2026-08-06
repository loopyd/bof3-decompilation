#include "internal.h"

/* @behavior dispatches one record callback selected by byte 0x7a.
 * @source 0x801F8358
 */
void scena16_dispatch_record_callback(void* record) {
  Scena16RecordCallback callback;

  /*
   * MATCHING_AID:
   * Original emits the prologue (addiu $sp / sw $ra) before the record and
   * D_8014686C loads; current schedules both loads above the prologue. This
   * barrier keeps the loads after the frame setup. Remove when the original
   * scheduling constraint is understood.
   */
  barrier();
  callback = scena16_record_callbackTable[((const u8*)record)[0x7a]];
  callback(record, SCENA16_D_8014686C);
}
