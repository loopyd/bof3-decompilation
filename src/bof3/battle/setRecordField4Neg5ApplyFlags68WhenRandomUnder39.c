#include "bof3/battle/battle15_internal.h"

extern int rand(void);

/* @source 0x8009DF70
 * @behavior Stores -5 into the signed halfword at byte offset 4 of the
 *   selection record pointer D_801463A0, then, when the random value masked to
 *   seven bits is below 0x27, applies the 0x68 flags value to the active
 *   battler index byte D_80146394 through func_800A36F0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordField4Neg5ApplyFlags68WhenRandomUnder39(void) {
  ((s16*)D_801463A0)[2] = -5;
  if ((rand() & 0x7F) < 0x27) {
    func_800A36F0(D_80146394, 0x68);
  }
}
