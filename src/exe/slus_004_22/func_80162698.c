#include "internal.h"

extern u32 D_80146458;
extern u32 D_8014646C;
extern s8  D_80146489;
extern u8  D_801464A0[];
extern u8  D_8018B4A0;
extern u8  D_8018B4A4;
extern u8  D_8018B4A8;
extern u8  D_8018B4AC;

/* @behavior derives a packed EMI dispatch word, records it for the active ring
 * slot, advances the packed-word cursor, and advances the loader step.
 * @source 0x80162698
 */
void func_80162698(void) {
  u32* loader_step;
  u32* dispatch_base;
  u32* slot;
  u32  packed;
  u32  dispatch;
  u32  dispatch_hi;
  u32  state;
  u8   b4a4;
  u8   next_index;

  loader_step = &D_8014646C;
  if (*loader_step == 0) {
    D_8018B4AC = 0;
    packed = D_80146458;
    D_8018B4A0 = (packed >> 24) & 0x3f;
    D_8018B4A4 = (packed >> 16) & 0x1f;
    D_8018B4A8 = (packed >> 8) & 0x3f;
  }

  dispatch_base = loader_step + 0x13;
  state = 3;
  D_801464A0[D_80146489] = state;
  slot = &dispatch_base[D_80146489];
  next_index = D_8018B4AC + 1;
  /*
   * MATCHING_AID:
   * Splitting the shift result through a temporary keeps the original
   * register web in the join block (dispatch stays in $v0, the shift chain
   * issues after the slot-address addu). Permuter-found; a plain assignment
   * lets GCC hoist the D_8018B4A4 load early and sink the slot address.
   * Remove if a cleaner shape reproduces the same allocation.
   */
  dispatch_hi = (D_8018B4AC + D_8018B4A0) << 24;
  dispatch = dispatch_hi;
  D_8018B4AC = next_index;
  b4a4 = D_8018B4A4;
  dispatch += b4a4 << 16;
  *slot = dispatch;

  if (next_index >= D_8018B4A8) {
    D_8018B4AC = 0;
    D_8018B4A4 = b4a4 + 1;
  }

  *loader_step = *loader_step + 1;
}
