#include "bof3/battle/battle03_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */
void func_800AAEBC(s16 arg0, u8 arg1); /* @source 0x800AAEBC */

/* @source 0x801E1494
 * @behavior Local work state-3 entry-0 step: counts the published local work
 * record 0x80146250 halfword +0x88 down by the record signed halfword +0x11C,
 * arms halfword +0x80 with 0x4000 and submits work effect variant 1 when +0x11C
 * has reached +0x88, otherwise submits variant 0 or raises front-end cue 0x206
 * for a negative +0x11C and lowers the rewritten +0x88 to the +0x90 ceiling,
 * then plays the pending effect halves +0x11C and +0x11E gated by bits 1 and 2
 * of record byte +0x120, runs localReadyOrHelper1 and advances scratch work
 * byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkCountdownAndSubmitPendingEffects(void) {
  Battle03LocalWork* record;
  Battle03LocalWork* counted;
  u16 remaining;

  record = D_80146250;
  if (record->unk_88 <= (s16)record->unk_11c) {
    record->unk_80 = 0x4000;
    record->unk_88 = 0;
    submitCurrentWorkEffectVariant(1u);
  } else {
    if ((s16)record->unk_11c >= 0) {
      submitCurrentWorkEffectVariant(0u);
    } else {
      func_8015D71C(0x206u);
    }
    counted = D_80146250;
    remaining = (u16)(counted->unk_88 - counted->unk_11c);
    counted->unk_88 = remaining;
    if ((u32)remaining > counted->unk_90) {
      counted->unk_88 = counted->unk_90;
    }
  }

  record = D_80146250;
  if ((record->unk_120 & 1u) != 0u) {
    func_800AAC88((s16)record->unk_11c, D_1F800044->unk_05);
    record = D_80146250;
  }
  if ((record->unk_120 & 2u) != 0u) {
    func_800AAEBC((s16)record->unk_11e, D_1F800044->unk_05);
  }

  localReadyOrHelper1();
  D_1F800044->unk_03++;
}
