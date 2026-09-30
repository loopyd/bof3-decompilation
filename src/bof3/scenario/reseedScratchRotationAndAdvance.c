#include "bof3/scenario/sce10eff_internal.h"

extern int rand(void);

/* @source 0x801D12A8
 * @behavior Reseeds the scratchpad work object, storing `rand() & 0xF` in its
 * byte 11 and clearing bytes 9 and 10, then derives the rotation counter from
 * its byte 4 as `byte4 << 5`, publishes the counter's low seven bits shifted
 * left 5 as the angle offset in the volatile scratchpad cell at 0x1F800004
 * together with the fixed 0x20 block size in the cell at 0x1F800000, scales the
 * local rotation results func_801783C8/func_801782FC by that block size and
 * adds them to the 0x34/0x38 coordinate pair cached in the object at
 * 0x801D2744, copies that object's 0x3E halfword into the work object and
 * advances byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D12A8(void) {
  ScenarioSce10effScratch* work;
  volatile s32* slot;
  s32 counter;
  s32 phase;
  s32 delta;
  u8* source;
  u16 length;

  ((u8*)D_1F800044)[0x0b] = (u8)(rand() & 0x0f);
  ((u8*)D_1F800044)[0x09] = 0;
  ((u8*)D_1F800044)[0x0a] = 0;

  counter = ((u8*)D_1F800044)[0x04] << 5;
  phase = (counter & 0x7f) << 5;
  D_1F800044->unk_0c = counter;
  slot = &D_1F800004;
  *slot = phase;
  D_1F800000 = 0x20;

  delta = func_801783C8(phase) * D_1F800000;
  D_1F800044->unk_34 = *(s32*)(D_801D2744 + 0x34) + delta;
  delta = func_801782FC(*slot) * D_1F800000;
  source = D_801D2744;
  work = D_1F800044;
  work->unk_38 = *(s32*)(source + 0x38) + delta;
  length = *(u16*)(source + 0x3e);
  ((u8*)work)[0x03]++;
  work->unk_3e = length;
}
