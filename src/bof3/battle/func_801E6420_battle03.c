#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6420
 * @behavior Queues the 0x1E battle event with the message string selected by the
 * 0x80146374 progress byte (row 7 below 3, otherwise row 8), then raises the
 * 0x801483C3 gate, stores 2 into queued slot byte +0x09, 0x100 into +0x0C and 0
 * into +0x10, and advances the slot state byte +0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E6420(void) {
  Battle03QueuedSlot* work;

  if (BATTLE_GLOBAL_BYTE_6374 < 3u) {
    func_801DE560(1u, 0u, 0u, 0x1Eu, (u32)D_801EB05B);
  } else {
    func_801DE560(1u, 0u, 0u, 0x1Eu, (u32)D_801EB068);
  }
  work = D_801EC2E0;
  D_801483C3 = 1;
  work->unk_09 = 2;
  work->unk_0c = 0x100;
  work->unk_10 = 0;
  D_801EC2E0->unk_01++;
}
