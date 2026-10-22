#include "bof3/battle/battle15_internal.h"

/* @behavior handles battle camera adjustment during the selection phase. the
 * mode byte selects one of four numeric cases (1, 2, 3, 4); the bytes prove only
 * the four-way dispatch, not what each case means in play. adjusts
 * camera-position variables and resets per-character visual offsets.
 * @source 0x800AAA74
 * @status partial
 * @match 95.49
 * @residual scheduling: 127/133 instructions with an exact 532-byte image. The
 *   mode byte, each per-arm camera base and the 0x140-scaled offsets assemble
 *   like the original (one named base plus displacements, folded
 *   `lui at,%hi; addu at,at,offset; op %lo(at)` loads), and per-case local
 *   scopes keep the scaled slot offset in $v0. The 6 remaining differences are
 *   mode-3 only: the panel argument's `addu` operand order, the position of its
 *   `addiu`, and the tracked/fixed load and store order, which the volatile
 *   accesses pin after the call argument is loaded.
 */
void func_800AAA74(void) {
  switch (D_80144F58) {
  case 1u: {
    u8* base;

    base = D_80145F44;
    func_801654F4((volatile u16*)base, (u32)(*(volatile u16*)base) >> 1u);
    func_801654F4((volatile u16*)(base + 2),
                  -(s32)((u32)(*(volatile u16*)(base + 2)) >> 2u));
    func_80165694(base + 21, *(volatile u8*)(base + 21));
    return;
  }
  case 2u: {
    u8  slot;
    u32 offset;

    slot = 0u;
    while (slot < D_801462F0) {
      if (func_801DB524(slot) == 0u) {
        volatile u16* panel;

        offset = (u32)slot * 0x140u;
        panel = (volatile u16*)(D_80145F04 + offset);
        func_801654F4(panel + 0x22,
                      -(s32)((u32)(*(volatile u16*)(D_80145F04 + offset + 0x44u)) >> 2u));
      }
      slot += 1u;
    }
    return;
  }
  case 3u: {
    u8  slot;
    u32 offset;

    slot = 0u;
    while (slot < D_801462F0) {
      if (func_801DB524(slot) == 0u) {
        volatile u16* panel;

        offset = (u32)slot * 0x140u;
        *(volatile u16*)(D_80145F04 + offset + 0x44u) =
            *(volatile u16*)(D_80145F04 + 0x44u);
        panel = (volatile u16*)(D_80145F04 + offset);
        func_801654F4(panel + 0x21,
                      -(s32)((u32)(*(volatile u16*)(D_80145F04 + offset + 0x42u)) >> 1u));
      }
      slot += 1u;
    }
    break;
  }
  case 4u: {
    u8* base;

    base = D_801461CA;
    func_801654F4((volatile u16*)base, (u32)(*(volatile u16*)base) >> 1u);
    func_801654F4((volatile u16*)(base - 0x280),
                  -(s32)((u32)(*(volatile u16*)(base - 0x280)) >> 2u));
    func_801654F4((volatile u16*)(base - 0x140),
                  -(s32)((u32)(*(volatile u16*)(base - 0x140)) >> 2u));
    break;
  }
  }
}
