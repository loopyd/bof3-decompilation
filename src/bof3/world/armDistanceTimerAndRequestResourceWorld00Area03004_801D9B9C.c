#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 work-byte-3 handler-table entry 0 at 0x801E2210: clears the
 * three overlay flag bytes D_801E3200, D_801E3204 and D_801E3214, stores 5 in
 * byte 0x29 of the scratch work record published at the scratchpad cursor
 * 0x1F800044, publishes the func_8015477C distance of the record x at +0x34 and
 * y at +0x38 into its signed timer at +0x3E, requests the resource 0xA with the
 * second argument 2 through func_8014D6DC while byte 0x79 of the record at
 * D_80146250 is clear and the resource 0xB through func_8014D6B8 otherwise, and
 * finally advances the record dispatch byte 3.
 * @source 0x801D9B9C
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 44/44 instructions, 176 bytes.
 * One cursor variable per scratchpad reload point keeps the four original reads
 * of 0x1F800044 separate. The `level` copy of D_80146250 is issued before the
 * record timer store, which is the original order: the guard pointer load
 * (`lw $a0`) fills the delay slot of the cursor reload and the byte load lands
 * after `sh $v0,0x3E($v1)`. Testing `D_80146250[0x79]` directly instead leaves
 * the pointer load after the store, adding two instructions (a delay-slot nop
 * for the cursor reload and one for the pointer reload).
 */
void armDistanceTimerAndRequestResourceWorld00Area03004_801D9B9C(void)
{
  u8* work;
  u8* args;
  u8* record;
  u8* level;
  u8* phase;
  s16 distance;

  work = D_1F800044;
  D_801E3200 = 0;
  D_801E3204 = 0;
  D_801E3214 = 0;
  work[0x29] = 5;
  args = D_1F800044;
  distance = func_8015477C(*(s32*)(args + 0x34), *(s32*)(args + 0x38));
  record = D_1F800044;
  level = D_80146250;
  *(s16*)(record + 0x3E) = distance;
  if (level[0x79] == 0) {
    func_8014D6DC(0xA, 2);
  } else {
    func_8014D6B8(0xB);
  }
  phase = D_1F800044;
  phase[3] = (u8)(phase[3] + 1u);
}
