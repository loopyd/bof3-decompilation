#include "bof3/battle/battle03_internal.h"

/* @behavior computes average/max thresholds for eligible enemy and local battlers,
 * then marks the battlers that pass each threshold pair with the shared
 * `0x8000` state bit.
 * @source 0x801DB058
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 markBattlersMeetingOpposingThresholds(void) {
  u8  result;
  u16 total;
  u16 maximum;
  u8  count;
  u8  index;

  result = 0u;
  total = 0u;
  maximum = 0u;
  count = 0u;
  index = 3u;
  while (index < 0x0bu) {
    if (func_801DB2F8(index) != 0u) {
      u16 value;

      value = D_801EB6D8[index - 3u].half_00;
      total += value;
      if (maximum < value) {
        maximum = value;
      }
      count += 1u;
    }
    index += 1u;
  }

  if (total != 0u) {
    total = total / count;
  }

  count = 0u;
  index = 0u;
  while (index < 3u) {
    if ((func_801DB9E4(index) != 0u) &&
        (func_801DB3A0(index, total, maximum) != 0u)) {
      D_80145FB8[index].flags_00 |= 0x8000u;
      count += 1u;
    }
    index += 1u;
  }

  if (count != 0u) {
    result += 1u;
  }

  total = 0u;
  maximum = 0u;
  count = 0u;
  index = 0u;
  while (index < 3u) {
    if (func_801DB2F8(index) != 0u) {
      u16 value;

      value = D_80145F28[index].half_00;
      total += value;
      if (maximum < value) {
        maximum = value;
      }
      count += 1u;
    }
    index += 1u;
  }

  if (total != 0u) {
    total = total / count;
  }

  index = 3u;
  while (index < 0x0bu) {
    if ((func_801DB9E4(index) != 0u) &&
        (enemyBattlerMeetsThresholds(index, total, maximum) != 0u)) {
      D_801EB734[index - 3u].word_00 |= 0x8000u;
    }
    index += 1u;
  }

  return result;
}
