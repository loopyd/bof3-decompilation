#include "bof3/scenario/scena00_internal.h"

/* @source 0x801FC7D0
 * @behavior Dispatches one record callback selected by the callback index byte
 * 0x7A of the record, passing that record and the shared flag word D_8014686C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackByByte7A(Scena00RecordDispatch* record) {
  D_801FCA84[record->callback_index_7A](record, D_8014686C);
}
