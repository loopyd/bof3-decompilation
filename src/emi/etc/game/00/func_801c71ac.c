#include "internal.h"

s32 func_801c71ac(s32 arg0) {
  s32 v0;
  s32 v1;

  v0 = arg0;
  v1 = v0 << 0x10;
  if (v1 > 0) {
    return 1;
  }
  return v1 >> 0x1F;
}
