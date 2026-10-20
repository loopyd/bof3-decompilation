#include "bof3/ui/sisyou00_internal.h"

/* Main-exe primitive initialiser (also called by drawFlatLinePrim). */
void func_8017AA80(u32 arg0);

/* EMI-local row-summary helper defined by func_801D1C44.c. */
s32 func_801D1C44(u8 row, u8 count);

/* @source 0x801D1B28
 * @behavior emits the numeric value row of one master-list record as a flat
 *           0x10-byte semi-transparent line primitive at the caller-supplied
 *           coordinates (arg0, arg1): it sums the first arg3 and the first
 *           arg3+1 values of the arg2-selected row of the shared 0x801CB8DC
 *           value table through func_801D1C44 and, when the two sums differ,
 *           stretches the line's second corner along x by
 *           (arg4 - first) * 57 / (second - first); when they are equal both
 *           corners keep arg0. The colour bytes are 0x80/0/0 and the
 *           primitive is queued through func_8014E5A0(1, 0x10).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D1B28(u16 arg0, u16 arg1, u8 arg2, u8 arg3, u32 arg4) {
  u8* primitive;
  s32 first;
  s32 second;

  primitive = g_PrimCursor;
  func_8017AA80((u32)primitive);
  SetSemiTrans(primitive, 0);
  *(s16*)(primitive + 8) = arg0;
  *(s16*)(primitive + 10) = arg1;
  first = func_801D1C44(arg2, arg3);
  second = func_801D1C44(arg2, (u8)(arg3 + 1));
  if (second != first) {
    *(s16*)(primitive + 12) = (s16)(arg0 + ((arg4 - first) * 57) / (second - first));
  } else {
    *(s16*)(primitive + 12) = arg0;
  }
  *(s16*)(primitive + 14) = arg1;
  *(u8*)(primitive + 4) = 0x80;
  *(u8*)(primitive + 5) = 0;
  *(u8*)(primitive + 6) = 0;
  func_8014E5A0(1, 0x10);
}
