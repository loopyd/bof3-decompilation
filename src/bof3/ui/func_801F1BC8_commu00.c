#include "bof3/ui/commu00_internal.h"
#include <stdlib.h>

/* @source 0x801F1BC8
 * @behavior Fairy-slot battle outcome draw: state 3 of the fairy slot stride-8
 * active-record state byte starts action 0x52 through func_80150224; state 2
 * draws rand() & 0x7F capped at 100, subtracts the first three bytes of the
 * four-byte weight record selected by the local record-kind slot byte while
 * stepping through the record's remaining weights to pick the band the drawn
 * value falls in, resolves the local reward-id byte pair selected by that band
 * times sixteen plus a random nibble through func_801650B4 and, when that pair
 * reports as consumed, copies the twelve-byte name func_80165D48 resolves for
 * it into the shared name buffer, terminates that buffer and starts action
 * 0x4F, otherwise starts action 0x50; every continuing path publishes the
 * elapsed battle count as the fairy slot record progress anchor, clears that
 * record state byte and labels the current commu00 task 0x51, and all paths
 * advance the fairy progress byte and latch the shared frontend mode byte at
 * 0x80143BB0 to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1BC8(void) {
  /* Each four-byte record of the container-local table at 0x801EEC98 holds the
   * weights this draw subtracts in order; its fourth byte is not read here. */
  u8 weights[3][4];
  u8 *name;
  u8 *p;
  /* The slot byte is read through volatile views: the record accesses re-read
   * it, the state-3 arm keeps its addressed view live across the action call,
   * and the reward arm re-derives that view after the name copy. */
  volatile u8 *slot;
  volatile u8 *fresh;
  u8 value;
  u8 hi;
  u8 lo;
  s32 record;
  s32 state;
  s32 pair_index;
  s32 i;

  __builtin_memcpy(weights, D_801EEC98, sizeof(weights));

  slot = &fairySlotIndex;
  state = D_801455CA[*slot * 8];
  if (state == 3) {
    func_80150224(0x52);
    activeRecordBytes[*slot].progress_anchor = D_8014502C;
    activeRecordBytes[*slot].record_state = 0;
    currentCommu00Task->label_id = 0x51;
  } else if (state == 2) {
    value = rand() & 0x7F;
    if (value > 100) {
      value = 100;
    }
    /* The record-kind byte selects the weight record; the band it yields is
     * re-derived from the slot byte for the second weight walk. */
    record = D_801455C9[*slot * 8] - 1;
    if (value < weights[D_801457A9[record * 8]][0]) {
      value = 0;
    } else {
      value -= weights[D_801457A9[record * 8]][0];
    }
    i = 1;
    record = D_801455C9[fairySlotIndex * 8] - 1;
    p = &weights[D_801457A9[record * 8]][1];
    for (; i < 3; i++) {
      if (*p >= value) {
        break;
      }
      value -= *p;
      p++;
    }
    i = (i - 1) * 16;
    pair_index = i + (rand() & 0xF);
    hi = D_801F2619[pair_index * 2];
    lo = D_801F2618[pair_index * 2];
    if (func_801650B4(hi, lo, 1, 0) == 0) {
      func_80150224(0x50);
    } else {
      name = func_80165D48(hi, lo);
      for (i = 0; i < 12; i++) {
        D_801490D8[i] = name[i];
      }
      D_801490E4[0] = 0;
      func_80150224(0x4F);
      fresh = &fairySlotIndex;
      activeRecordBytes[*fresh].progress_anchor = D_8014502C;
      activeRecordBytes[*fresh].record_state = 0;
      currentCommu00Task->label_id = 0x51;
    }
  }

  fairyProgress[0] += 1;
  D_80143BB0 = 2;
}
