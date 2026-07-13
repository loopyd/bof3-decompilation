#include "internal.h"

/* @behavior sums the active COMMU00 template weights for kinds 10 and 11 into the
 * two cached byte totals at `0x801455c5` and `0x801455c6`.
 * @source 0x801f0320 FUN_801f0320
 */
void func_801f0320(void) {
  u8* total;
  s32 slot_weight;
  s32 slot_weight_end;
  s32 offset;
  s32 kind;

  total = (u8*)0x801455c5u;
  slot_weight = 0x801f2706u;
  offset = 0;
  slot_weight_end = slot_weight + 0x21c;
  total[0] = 0u;
  total[1] = 0u;
  do {
    if (((const volatile u8*)0x801455c8u)[offset] != 0u) {
      kind = ((const volatile u8*)0x801455c9u)[offset];
      if (kind == 10) {
        total[0] = (u8)(total[0] + *(const u8*)slot_weight);
      } else if (kind == 11) {
        total[1] = (u8)(total[1] + *(const u8*)slot_weight);
      }
    }

    slot_weight += 9;
    offset += 8;
  } while (slot_weight < slot_weight_end);
}
