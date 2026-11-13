#include "bof3/core/slus_internal.h"

/* PsyQ 4.7 libsnd internal key-off entry; the shipped libsnd.h does not declare
 * it. The callee ignores this argument and reads D_8018E7F2 instead. */
extern void _SsVmKeyOffNow(s32 arg);

/* Slot record view anchored at the compared owner id halfword. The record stride
 * is 0x34 bytes; only the offset this scan reads is typed. */
typedef struct KeySlotRecord {
  s16 unk_00;
  u8  unk_02[0x32];
} KeySlotRecord;

/* @source 0x8018DBFE @kind bss */
extern KeySlotRecord D_8018DBFE[];
/* @source 0x8018E264 @kind bss */
extern u8 D_8018E264;
/* @source 0x8018E7F2 @kind bss */
extern u16 D_8018E7F2;

/* @behavior walks the 0x34-byte slot records anchored at 0x8018DBFE, up to the
 * byte count at 0x8018E264, and for every record whose signed owner id halfword
 * equals the requested owner publishes the voice index to 0x8018E7F2 and calls
 * the libsnd key-off helper. The helper reads that published cell, clears the
 * slot's fields and updates the driver key masks.
 * @source 0x8017301C
 * @status exact
 * @match 100.00
 * @residual none
 */
void releaseOwnerVoices(s16 owner) {
  u8 i;

  for (i = 0; i < D_8018E264; i++) {
    if (D_8018DBFE[i].unk_00 == owner) {
      D_8018E7F2 = i;
      _SsVmKeyOffNow(0);
    }
  }
}
