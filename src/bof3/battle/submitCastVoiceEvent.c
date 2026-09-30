#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1888
 * @behavior Waits on the scratch battle-work byte +0x09: when it is zero it
 * submits effect variant 5, then also submits variant 3 unless the object
 * published at 0x80146250 lacks flag 0x2 or the global byte at 0x801463C9 is 4,
 * 7 or 8, and then increments scratch battle-work byte +0x02; otherwise it
 * decrements byte +0x09. Both paths finish with the readiness helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitCastVoiceEvent(void) {
  if (D_1F800044->pad_09[0] == 0) {
    submitCurrentWorkEffectVariant(5);
    if ((D_80146250->unk_128 & 0x2u) != 0u && BATTLE_GLOBAL_BYTE_63C9 != 4u &&
        (BATTLE_GLOBAL_BYTE_63C9 < 7u || BATTLE_GLOBAL_BYTE_63C9 > 8u)) {
      submitCurrentWorkEffectVariant(3);
    }
    D_1F800044->unk_02++;
  } else {
    D_1F800044->pad_09[0]--;
  }
  localReadyOrHelper2();
}
