#include "bof3/battle/battle03_internal.h"

/* @source 0x801DEFE4
 * @behavior Forwards one presentation offset of the current battle work. While
 * the front-end gate byte 0x80146325 carries bit 0x20 and mode byte 0x801462E0
 * reads 5 with state byte 0x80146328 carrying bit 2 -- or, with that gate clear,
 * under the same mode/state pair -- it stages either the work route byte at
 * scratch +0x4B or the effect base scratch byte +0x08 raised by 0x1C or 0x20
 * through func_8014D5F0 for the 0x800F0800 source of 0x1800 bytes and returns.
 * Otherwise it forwards the effect id scratch byte +0x08 raised by 0x1C while
 * work halfword +0x80 carries bit 0x4000, by 0x30 while it carries bit 0x800,
 * and while battle halfword 0x801462E8 carries bit 0x10 by 4, 0x18 or 8
 * according to bits 0x4 and 0x20C0 of the same halfword; failing those, by 0x14
 * while work word +0x124 carries bit 2, and otherwise by an offset selected
 * from work byte +0x119: case 0 by 0x18 or 4 on bits 0x20C0 of halfword +0x80,
 * case 1 by 8, case 2 by 0x14, case 4 by 0x38 when work word +0x128 carries bit
 * 4, then by 8 when the 0x801CA71C kind mask of work halfword +0x11A carries
 * bit 0x800 and by 0x28 otherwise, and case 5 by 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardSelectedPresentationOffset(void) {
  Battle03LocalWork* work;

  if ((D_80146325 & 0x20u) != 0u) {
    if (D_801462E0 == 5 && (D_80146328 & 2u) != 0u) {
      if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x4000u) != 0u) {
        func_8014D5F0(D_1F800044->unk_08 + 0x1cu, 0x800f0800u, 0x1800);
      } else {
        func_8014D5F0(BATTLE_LOCAL_BYTE_4B(D_1F800044), 0x800f0800u, 0x1800);
      }
      return;
    }
  } else {
    if (D_801462E0 == 5 && (D_80146328 & 2u) != 0u) {
      if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x4000u) != 0u) {
        func_8014D5F0(D_1F800044->unk_08 + 0x1cu, 0x800f0800u, 0x1800);
      } else {
        func_8014D5F0(D_1F800044->unk_08 + 0x20u, 0x800f0800u, 0x1800);
      }
      return;
    }
  }

  work = D_80146250;
  if ((work->unk_80 & 0x4000u) != 0u) {
    func_8014D8D4(D_1F800044->unk_08 + 0x1cu);
  } else if ((work->unk_80 & 0x0800u) != 0u) {
    func_8014D8D4(D_1F800044->unk_08 + 0x30u);
  } else if ((BATTLE_GLOBAL_HALF_62E8 & 0x0010u) != 0u) {
    if ((work->unk_80 & 0x0004u) != 0u) {
      func_8014D8D4(D_1F800044->unk_08 + 4u);
    } else if ((work->unk_80 & 0x20c0u) != 0u) {
      func_8014D8D4(D_1F800044->unk_08 + 0x18u);
    } else {
      func_8014D8D4(D_1F800044->unk_08 + 8u);
    }
  } else if ((work->unk_124 & 2u) != 0u) {
    func_8014D8D4(D_1F800044->unk_08 + 0x14u);
  } else {
    switch (work->unk_119) {
    case 0:
      if ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x20c0u) != 0u) {
        func_8014D8D4(D_1F800044->unk_08 + 0x18u);
      } else {
        func_8014D8D4(D_1F800044->unk_08 + 4u);
      }
      break;
    case 1:
      func_8014D8D4(D_1F800044->unk_08 + 8u);
      break;
    case 2:
      func_8014D8D4(D_1F800044->unk_08 + 0x14u);
      break;
    case 4: {
      Battle03LocalWork* selected;

      selected = D_80146250;
      if ((selected->unk_128 & 4u) != 0u) {
        func_8014D8D4(D_1F800044->unk_08 + 0x38u);
      } else if ((D_801CA71C[selected->unk_11a].mask_00 & 0x0800u) != 0u) {
        func_8014D8D4(D_1F800044->unk_08 + 8u);
      } else {
        func_8014D8D4(D_1F800044->unk_08 + 0x28u);
      }
      break;
    }
    case 5:
      func_8014D8D4(D_1F800044->unk_08 + 4u);
      break;
    }
  }
}
