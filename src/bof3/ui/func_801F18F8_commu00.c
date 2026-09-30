#include "bof3/ui/commu00_internal.h"

/* @source 0x801F18F8
 * @behavior Scans the container-local twenty-entry band table at 0x801EEC48 for
 * the entry whose halfword bound first exceeds the elapsed battle count of the
 * fairy slot's active record (the shared count 0x8014502C minus that record's
 * progress anchor), stepping back one entry afterwards. When no entry matches
 * it starts action 0x98 through func_80150224. Otherwise it resolves the
 * selected entry's +3/+2 bytes into the shared twelve-byte item-name buffer
 * through func_80165D48, terminates that buffer at 0x801490E4, starts action
 * 0x96, and then either advances the fairy progress byte when func_801650B4
 * reports the entry as consumed or publishes the shared count as the record's
 * new progress anchor. Every path advances the fairy progress byte once more
 * and latches the shared phase byte at 0x80143BB0 to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F18F8(void) {
  Commu00BandRecord bands[20];
  u8 *name;
  s32 i;
  s32 j;
  u32 elapsed;

  __builtin_memcpy(bands, commu00BandTable, sizeof(bands));

  i = 0;
  elapsed = D_8014502C - activeRecordBytes[fairySlotIndex].progress_anchor;
  for (; i < 20; i++) {
    if (elapsed < bands[i].threshold) {
      break;
    }
  }

  i -= 1;
  if (i < 0) {
    func_80150224(0x98);
  } else {
    name = func_80165D48(bands[i].unk_03, bands[i].unk_02);
    for (j = 0; j < 12; j++) {
      D_801490D8[j] = name[j];
    }
    D_801490E4[0] = 0;
    func_80150224(0x96);
    if (func_801650B4(bands[i].unk_03, bands[i].unk_02, 1, 0) == 0) {
      fairyProgress[0] += 1;
    } else {
      activeRecordBytes[fairySlotIndex].progress_anchor = D_8014502C;
    }
  }

  fairyProgress[0] += 1;
  D_80143BB0 = 2;
}
