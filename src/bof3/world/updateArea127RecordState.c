#include "bof3/bof3.h"

extern void* D_80146884;
extern u8 D_80145F09;
extern u8 D_80144F28;
void func_8015B580(void* arg0, s32 arg1);
void func_8015507C(s32 arg0, s32 arg1);

/* @source 0x801F2C04
 * @behavior Clears bit 0 of the byte at +0x74 of the record published at 0x80146884, then,
 * when the shared state byte 0x80145F09 holds 5, dispatches sound cue 0x32 through
 * func_8015B580 with the shared parameter block 0x80144F28, cue 0x33 through func_8015507C,
 * and clears the byte addressed by the scratchpad pointer at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateArea127RecordState(void) {
  u8* record;
  s8* slot;

  record = D_80146884;
  record[0x74] &= ~1u;
  if (D_80145F09 == 5) {
    func_8015B580(&D_80144F28, 0x32);
    func_8015507C(0x4E, 0x33);
    slot = *(s8**)0x1F800044;
    *slot = 0;
  }
}
