#include "bof3/bof3.h"

/* 0x801C1ABC — party/character helper; its exact role is unresolved, so it
 * stays address-anchored like the sibling scena09 wrapper that calls it. */
void func_801C1ABC(s32 arg0);

/* @source 0x80145E90 @kind unknown — base of the shared 0x140-byte-stride local
 * work records. */
extern u8 D_80145E90[];
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte-stride
 * local work records based at 0x80145E90. */
extern u8 D_80145F09[];
/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records. */
extern u8 D_80146254;

/* @source 0x801FC328
 * @behavior Overlay per-record callback: it rebuilds the shared live
 *           work-record count and then re-invokes the party helper for the
 *           first three snapshot slots. It walks the first three
 *           0x140-byte-stride local work records based at D_80145E90, counts
 *           the ones whose leading byte has bit 0 set, snapshots each of those
 *           three records' party byte (+0x79, at D_80145F09) into an eight-byte
 *           stack array, and publishes the count in the shared live-record
 *           count byte D_80146254. It then walks the first three snapshot bytes
 *           and calls func_801C1ABC with every byte that is not 7. It ignores
 *           the record pointer and the second argument its dispatcher passes,
 *           returns nothing, and its 0x28-byte frame holds the eight-byte
 *           snapshot at $sp+0x10 plus the saved $ra and $s0-$s2. No in-image
 *           jal reaches the address; its only image word is 0x801FC4BC, entry
 *           21 of the twenty-two-word overlay callback run at 0x801FC468
 *           (0x801FC468 through 0x801FC4BC) that
 *           dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C indexes
 *           with the unsigned record byte at offset 0x7A, so the overlay
 *           selects this entry for the record kind 0x15. The three-slot
 *           snapshot with the != 7 helper tail is the shape the exact scena09
 *           sibling copyRecordPartyBytesThenInvokeHelperScenarioScena0900_801FE234
 *           runs; this member differs by rebuilding the count from the first
 *           three records' bit 0 instead of walking the published count, by not
 *           storing the D_80144908 party mirror, and by having no func_801C1630
 *           tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void recountLiveWorkRecordsThenCopyPartyBytesAndInvokeHelperScenarioScena1400_801FC328(void) {
  u8 slots[8];
  u8 i;
  u8 count;
  u32 offset;

  i = 0;
  count = 0;
  do {
    offset = i * 0x140;
    if (D_80145E90[offset] & 1) {
      count++;
    }
    /* The offset is dead after this load; reuse it for the snapshot value. */
    offset = D_80145F09[offset];
    slots[i] = offset;
    i++;
  } while (i < 3);
  D_80146254 = count;
  for (i = 0; i < 3; i++) {
    if (slots[i] != 7) {
      func_801C1ABC(slots[i]);
    }
  }
}
