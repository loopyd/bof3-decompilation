#include "internal.h"

/**
 * @source 0x801F3618
 * @behavior Resets the area state and, when bit zero is clear, arms two flags.
 */
void func_801F3618(void) {
  u8 flags;

  flags = WORLD00_AREA027_FLAGS_90C7;
  WORLD00_AREA027_STATE_90A8 = 0xFFFF;
  if ((flags & 1) == 0) {
    WORLD00_AREA027_FLAG_48EB = 2;
    WORLD00_AREA027_FLAG_48EC = 0;
  }
}
