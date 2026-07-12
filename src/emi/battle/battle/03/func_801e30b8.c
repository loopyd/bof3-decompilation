#include "internal.h"

/* @behavior returns `0xff` under one global mode byte, otherwise forwards a
 * selection index offset by `3` into the local picker helper.
 * @source 0x801e30b8 FUN_801e30b8
 */
u8 func_801e30b8(s8 arg0) {
  if (BATTLE_GLOBAL_BYTE_62F3 == 1u) {
    return 0xffu;
  }
  return func_801e29b4((u8)(arg0 + 3u));
}
