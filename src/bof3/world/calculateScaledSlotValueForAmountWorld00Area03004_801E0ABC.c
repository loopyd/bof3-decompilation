#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0ABC
 * @behavior Read the slot selected by the low byte of slotIndex and the low halfword of
 * slotAmount. Return the slot value unchanged when the amount reaches its
 * limit. Otherwise quantize amount * 10 / limit before multiplying by the slot
 * value and dividing by 10, then return the low halfword.
 * @status exact
 * @match 100.00
 * @residual none
 * The inverted early return (`amount >= limit`) is what places the scaled path
 * in the branch fall-through; the signed division keeps the original
 * break 7/break 6 trap sequence through the recorded per-object
 * `-Wa,--expand-div` profile in config/compiler/object-flags.cmake.
 */
s32 calculateScaledSlotValueForAmountWorld00Area03004(s32 slotIndex, s32 slotAmount) {
  u8 recordIndex = slotIndex & 0xff;
  u16 amount = slotAmount & 0xffff;

  if (amount >= D_801E2AB8[recordIndex].limit) {
    return D_801E2AB8[recordIndex].value;
  }
  return (u16)(D_801E2AB8[recordIndex].value *
               (amount * 10 / D_801E2AB8[recordIndex].limit) / 10);
}
