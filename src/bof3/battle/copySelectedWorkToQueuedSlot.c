#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1EC0
 * @behavior Allocates the queued slot-store entry for mode byte 0xF through
 * func_801E590C, copies the 0x74-byte head of the D_80145E90 work record
 * selected by battle byte 0x80146374 into that entry, re-initializes the entry
 * bytes +0x01..+0x06 (with 0xF at +0x05), stores the allocated index at +0x0B
 * and publishes the scratch work pointer at +0x74, then raises bit 0x40 of the
 * scratch work flags, sets the scratch bytes +0x09/+0x0A to 0x10 and increments
 * the scratch byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySelectedWorkToQueuedSlot(void) {
  Battle03LocalWork* scratch;
  Battle03SlotStore* record;
  Battle03SlotBody*  src;
  u8                 index;

  index = (u8)func_801E590C(0u, 0xFu);
  record = &D_801EC330[index];
  src = (Battle03SlotBody*)&D_80145E90[BATTLE_GLOBAL_BYTE_6374];
  *(Battle03SlotBody*)record = *src;
  D_801EC330[index].unk_06 = 0u;
  D_801EC330[index].unk_05 = 0xFu;
  D_801EC330[index].unk_01 = 0u;
  D_801EC330[index].unk_02 = 0u;
  D_801EC330[index].unk_03 = 0u;
  D_801EC330[index].unk_04 = 0u;
  D_801EC330[index].unk_0b = index;
  scratch = D_1F800044;
  D_801EC330[index].ptr_74 = (u32)scratch;
  scratch->flags_00 |= 0x40u;
  D_1F800044->pad_09[0] = 0x10u;
  D_1F800044->pad_09[1] = 0x10u;
  D_1F800044->unk_02 += 1u;
}
