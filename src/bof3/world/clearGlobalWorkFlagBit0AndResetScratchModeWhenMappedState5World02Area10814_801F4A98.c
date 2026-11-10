#include "bof3/bof3.h"

extern u8* D_80146884; /* @source 0x80146884 @kind data */
extern u8 D_80145F09; /* @source 0x80145F09 @kind data */
extern u8 D_80181B10[]; /* @source 0x80181B10 @kind table */
extern u8 D_80144F28[]; /* @source 0x80144F28 @kind data */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

extern void func_8015B580(void* bitmap, s32 bit_index);
extern void func_8015507C(s32 arg0, s32 arg1);

/* @source 0x801F4A98
 * @behavior Clears bit 0 of byte +0x74 in the record published at 0x80146884.
 *           If table 0x80181B10 indexed by byte 0x80145F09 yields 5, calls
 *           0x8015B580 with the shared flag bank 0x80144F28 and bit index 0x30,
 *           calls 0x8015507C with (1, 0x0B), then clears byte 0 of the record
 *           published at scratchpad pointer cell 0x1F800044. Takes no arguments
 *           and returns nothing; helper 0x8015507C retains its unknown role.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearGlobalWorkFlagBit0AndResetScratchModeWhenMappedState5World02Area10814_801F4A98(void) {
  u8* record;

  record = D_80146884;
  record[0x74] &= ~1u;
  if (D_80181B10[D_80145F09] == 5) {
    func_8015B580(D_80144F28, 0x30);
    func_8015507C(1, 0x0B);
    workRecord44World02Area10814[0] = 0;
  }
}
