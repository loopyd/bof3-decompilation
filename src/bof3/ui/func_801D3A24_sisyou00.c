#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3A24
 * @behavior when the phase byte D_80143BB0 is not 2, reads the record byte
 *           selected by the D_801D428A step through D_80145FCC and compares the
 *           owner byte of that 164-byte record in D_80144983 with masterIndex:
 *           when they differ nothing is started. Otherwise the difference
 *           between the record's byte +6 and its byte +0x84 selects the master
 *           confirmation path keyed by the owner byte: 0xB, 0xD and 0xE set bit
 *           3, 4 or 5 of D_80144F59 and 0xC additionally tests the flag 0x6C of
 *           the flag bank at masterIndex + 0x63B through func_8015B5D4, runs the
 *           func_801650B4 gate and sets that flag through func_8015B580; each
 *           of those paths only copies the record's five-byte prefix into the
 *           shared work buffer once the difference reaches three. Every other
 *           owner byte scans the twelve-byte per-master descriptor at
 *           D_801D4088, whose six (limit, gate bit) pairs are tested in order:
 *           the first pair whose limit the difference does not fall short of,
 *           whose bit is clear in the masterIndex + 0x693 gate table and which
 *           func_801651DC confirms copies the same five-byte prefix, fills the
 *           D_801490F8 name buffer from the D_801CA70C ability record selected
 *           by the gate byte and sets that gate bit. Whenever one of those paths
 *           ran, the selected master's +0xE action is started through
 *           func_80150224 and the phase byte is written as 2; the handler index
 *           D_801D4286 is advanced either way.
 * Residual: the gate-scan loop keeps the three table bases (D_801D4088,
 *           D_801D4089 and D_80145FCC) in callee-saved registers where the
 *           original re-materializes %hi/%lo per use, and two address adds
 *           commute in the opposite order; the head, the four confirmation
 *           paths and the tail match instruction for instruction.
 * @status partial
 * @match 83.03
 * @residual address materialization and commutative add order in the gate-scan loop
 */
void func_801D3A24(void) {
  u8* master;
  u8* state;
  u32* gates;
  u8 flag;
  u8 index;
  u8 owner;
  u8 diff;
  s32 record_offset;
  s32 offset;
  s32 cell;
  s32 i;

  flag = 0;
  if (D_80143BB0 == 2) {
    return;
  }
  /* masterIndex is the leading byte of the shared progress block: its
   * per-master flag bank sits 0x63B bytes above it and its gate bit table
   * 0x693. */
  master = &masterIndex;
  offset = D_801D428A * 0x140;
  index = D_80145FCC[offset];
  record_offset = index * 164;
  owner = D_80144983[record_offset];
  if (owner != *master) {
    goto done;
  }
  diff = D_80144968[record_offset + 6] - D_80144968[record_offset + 0x84];

  switch (owner) {
    case 0xB:
      if (diff < 3) {
        goto done;
      }
      if (D_80144F59 & 8) {
        goto done;
      }
      D_80144F59 = D_80144F59 | 8;
      copySelectedRecordPrefixToWorkBuffer(D_80145FCC[offset]);
      flag = 1;
      break;
    case 0xC:
      if (diff < 3) {
        goto done;
      }
      if (func_8015B5D4((u32)(master + 0x63B), 0x6C) != 0) {
        goto done;
      }
      if (func_801650B4(3, 0x16, 1, 0) == 0) {
        goto done;
      }
      func_8015B580(master + 0x63B, 0x6C);
      copySelectedRecordPrefixToWorkBuffer(D_80145FCC[D_801D428A * 0x140]);
      flag = 1;
      break;
    case 0xD:
      if (diff < 3) {
        goto done;
      }
      if (D_80144F59 & 0x10) {
        goto done;
      }
      D_80144F59 = D_80144F59 | 0x10;
      copySelectedRecordPrefixToWorkBuffer(D_80145FCC[offset]);
      flag = 1;
      break;
    case 0xE:
      if (diff < 3) {
        goto done;
      }
      if (D_80144F59 & 0x20) {
        goto done;
      }
      D_80144F59 = D_80144F59 | 0x20;
      copySelectedRecordPrefixToWorkBuffer(D_80145FCC[offset]);
      flag = 1;
      break;
    default:
      state = &masterIndex;
      gates = (u32*)(state + 0x693);
      for (cell = 0; cell < 0xC; cell += 2) {
        if (diff < D_801D4088[cell + *state * 0xC]) {
          continue;
        }
        if ((gates[D_801D4089[cell + *state * 0xC] >> 5] &
             (1 << (D_801D4089[cell + *state * 0xC] & 0x1F))) != 0) {
          continue;
        }
        offset = D_801D428A * 0x140;
        if (func_801651DC(D_801D4089[cell + *state * 0xC],
                          D_80145FCC[offset], 0, 0) == 0) {
          continue;
        }
        offset = D_801D428A * 0x140;
        copySelectedRecordPrefixToWorkBuffer(D_80145FCC[offset]);
        for (i = 0; i < 0xC; i++) {
          D_801490F8[i] = D_801CA70C[D_801D4089[cell + *state * 0xC] * 20 + i];
        }
        D_801490F8[0xC] = 0;
        gates[D_801D4089[cell + *state * 0xC] >> 5] =
            gates[D_801D4089[cell + *state * 0xC] >> 5] |
            (1 << (D_801D4089[cell + *state * 0xC] & 0x1F));
        flag = 1;
        break;
      }
      break;
  }

done:
  if (flag != 0) {
    func_80150224((s16)(masterActionBaseTable[masterIndex] + 0xE));
    D_80143BB0 = 2;
  }
  D_801D4286 = D_801D4286 + 1;
}
