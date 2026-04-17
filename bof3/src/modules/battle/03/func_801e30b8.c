#include "internal.h"

/* does: returns `0xff` under one global mode byte, otherwise forwards a
 * selection index offset by `3` into the local picker helper.
 * @source: 0x801e30b8 FUN_801e30b8
 */
u8 func_801e30b8(s8 arg0) {
  u8* mode_ptr;
  s32 value;

  mode_ptr = (u8*)0x80140000u;

  if (mode_ptr[0x62f3] == 1u) {
    value = 0xffu;
  } else {
    value = func_801e29b4(arg0 + 3);
  }

  return value;
}
