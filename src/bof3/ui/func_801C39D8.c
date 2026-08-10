#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801C39D8
 * @behavior Dispatches to one of two helpers, forwarding the low bytes of the
 * fourth and fifth arguments.
 */
u8 func_801C39D8(s32 arg0, s32 arg1, s32 use_alt, u8 arg3, u8 arg4)
{
  u8 result;

  if (use_alt != 0) {
    result = func_801C3B78(arg0, arg1, arg3, arg4);
  } else {
    result = func_801C3A18(arg0, arg1, arg3, arg4);
  }
  return result;
}
