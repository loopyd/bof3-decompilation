#include "internal.h"

/* @behavior appends one triple `(byte, byte, word)` into the 16-entry UI ring and
 * advances the ring tail.
 * @source 0x801DE8C0
 */
void func_801DE8C0(s8 arg0, s8 arg1, u32 arg2) {
  u8 index;

  D_801EB5B0[D_801EC328].unk_00 = (u8)arg0;
  D_801EB5B0[D_801EC328].unk_01 = (u8)arg1;
  index = D_801EC328;
  D_801EB5B0[index].unk_04 = arg2;
  D_801EC328 = (index + 1u) & 0x0fu;
}
