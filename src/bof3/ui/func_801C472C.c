#include "bof3/ui/game00_internal.h"

/* @source 0x801C472C @behavior dispatches a four-argument tile query by mode */
u8 func_801C472C(s32 arg0, s32 arg1, s32 mode, u8 value, u8 option)
{
  if (mode == 0) {
    return func_801C476C(arg0, arg1, value, option);
  }
  return func_801C48FC(arg0, arg1, value, option);
}
