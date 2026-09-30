#include "bof3/battle/battle03_internal.h"

/* @source 0x801DF914
 * @behavior Arms the scratch two-stage timer pair. When the current local work
 * record word +0x128 carries bit 0x2 the pair bytes +0x09 and +0x0A take the
 * value of the byte table D_801EB198 selected by the global byte
 * BATTLE_GLOBAL_BYTE_63C9; otherwise they take the value of the byte table
 * D_801EB18C selected by that record's kind byte +0x79. Both paths then submit
 * effect id scratch byte +0x08 plus 0x0C through func_8014D8D4, run
 * localReadyOrHelper1 and advance the scratch state byte +0x01. The handler is
 * installed for the local substeps 2..12 of the D_801EB154 table dispatched by
 * func_801DF3F8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DF914(void) {
  Battle03LocalWork* work;
  u8*                index;

  work = D_80146250;
  if ((work->unk_128 & 0x2u) != 0u) {
    index = &BATTLE_GLOBAL_BYTE_63C9;
    D_1F800044->pad_09[0] = D_801EB198[*index];
    D_1F800044->pad_09[1] = D_801EB198[*index];
  } else {
    D_1F800044->pad_09[0] = D_801EB18C[work->unk_79];
    D_1F800044->pad_09[1] = D_801EB18C[D_80146250->unk_79];
  }
  func_8014D8D4(D_1F800044->unk_08 + 0x0Cu);
  localReadyOrHelper1();
  D_1F800044->unk_01++;
}
