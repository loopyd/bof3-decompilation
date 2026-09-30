#include "bof3/world/area03004_internal.h"

/* @source 0x801DB554
 * @behavior AREA030 selection-confirm step: when the shared selection counter
 * D_80144199 reads 4, clamps the byte D_80144FE4[byte 6 of the active 0x98-byte
 * work record D_80146888[D_801E3208]] up to the record halfword at +0x90 and
 * sets the D_801E3214 flag byte when the byte grew, plays the fixed cue 0x102
 * through func_8015DF18, stores 8 in byte 9 and advances byte 3 of the scratch
 * work record published at 0x1F800044, then runs func_8014D978.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DB554(void) {
  u8* work;
  u8 slot;
  u32 value;

  if (D_80144199_BYTE == 4) {
    slot = D_80146888[D_801E3208].unk_02[4];
    value = *(u16*)&D_80146888[D_801E3208].unk_60[0x30];
    if (D_80144FE4[slot] < value) {
      D_80144FE4[slot] = value;
      D_801E3214 = 1;
    }
    func_8015DF18(0x102);
    work = D_1F800044;
    work[9] = 8;
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
}
