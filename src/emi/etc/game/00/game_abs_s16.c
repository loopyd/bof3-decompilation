#include "internal.h"

/* @behavior returns the absolute value of one signed 16-bit argument.
 * @source 0x801C7188
 */
s16 game_abs_s16(s16 arg0) {
  if ((s32)arg0 < 0)
    return -arg0;
  return arg0;
}
