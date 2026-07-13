#include "internal.h"

extern u32 DAT_80146458;
extern u32 DAT_8014646c;
extern s8  DAT_80146489;
extern u8  DAT_801464a0[];
extern u32 DAT_801464b8[];
extern u8  DAT_8018b4a0;
extern u8  DAT_8018b4a4;
extern u8  DAT_8018b4a8;
extern u8  DAT_8018b4ac;

/* @behavior derives a packed EMI dispatch word, records it for the active ring
 * slot, advances the packed-word cursor, and advances the loader step.
 * @source 0x80162698 FUN_80162698
 */
void func_80162698(void) {
  u32* loader_step;
  u32  packed;
  u32  dispatch;
  u32  state;
  u8   next_index;

  loader_step = &DAT_8014646c;
  if (*loader_step == 0) {
    DAT_8018b4ac = 0;
    packed = DAT_80146458;
    DAT_8018b4a0 = (packed >> 24) & 0x3f;
    DAT_8018b4a4 = (packed >> 16) & 0x1f;
    DAT_8018b4a8 = (packed >> 8) & 0x3f;
  }

  state = 3;
  DAT_801464a0[DAT_80146489] = state;
  next_index = DAT_8018b4ac + 1;
  dispatch = (DAT_8018b4ac + DAT_8018b4a0) << 24;
  DAT_8018b4ac = next_index;
  dispatch += DAT_8018b4a4 << 16;
  DAT_801464b8[DAT_80146489] = dispatch;

  if (next_index >= DAT_8018b4a8) {
    DAT_8018b4ac = 0;
    DAT_8018b4a4++;
  }

  *loader_step = *loader_step + 1;
}
