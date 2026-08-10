#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DDE94
 * @behavior Stores the requested state and advances selected shared modes.
 */
void func_801DDE94(s32 state)
{
  u8* modePtr;
  s32 savedState;
  u8 mode;

  savedState = state;
  mode = modeByte;
  if ((mode == 4) || (mode == 6) || (mode == 0)) {
    modePtr = &modeByte;
    D_8014412B = 1;
    *modePtr = *modePtr + 1;
  }
  D_8014412A = savedState;
}
