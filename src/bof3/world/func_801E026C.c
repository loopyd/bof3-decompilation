#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E026C
 * @behavior Advances the signed position by arg0 and clamps it to [0, limit].
 */
void func_801E026C(s8 arg0) {
  s32 delta;
  Area030Range* range;
  s16 value;
  s16 result;

  delta = arg0;
  range = D_80146884;
  range->value_8E += delta;
  value = range->value_8E;
  result = range->value_8E;

  if (value < 0) {
    result = 0;
    goto store;
  }
  if (range->limit_8C < value) {
    result = range->limit_8C;
  }
store:
  range->value_8E = result;
}
