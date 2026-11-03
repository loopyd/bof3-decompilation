#include "bof3/world/area03004_internal.h"

/* Per-evaluation reload of the scratch cursor cell, matching the original's
 * repeated lui/lw pairs before each store group. */
#define AREA030_SCRATCH ((u8*)D_1F800044)

/* @behavior seeds the AREA030 scratch record for the selected mode: publishes
 * the two shared panel words, derives the level from the mode byte and the
 * shared scale word, clamps the remaining budget into 0x150000..0x3C0000,
 * copies the per-mode word pair, clears the record flags, starts the panel
 * services and marks the selected entry record.
 * @source 0x801D210C
 * @status partial
 * @match 63.02
 * @residual 121 of 192 current instructions match 186 original; 768 current against
 * 744 original bytes. Each scratch cursor group now reads the cell through its own
 * variable, which recovered 24 matches and 4 bytes over the single-variable form.
 * The first difference is at +0x10, where the second group's D_80145EC4 load is
 * allocated to a1 instead of the original v0, and the remaining groups still differ
 * in register choice and schedule.
 */
void func_801D210C(void) {
  u8* s1;
  u8* s2;
  u8* s3;
  u8* s4;
  u8* s5;
  u8* s6;
  u8* s7;
  u8* s8;
  u8* s9;
  u8* s0;
  s32 level;
  u32 value;
  u32 budget;
  u8 index;

  s1 = AREA030_SCRATCH;
  value = D_80145EC4;
  budget = D_80145EC8;
  *(volatile u32*)(s1 + 0x34u) = value;
  *(volatile u32*)(s1 + 0x38u) = budget - 0x8000u;
  level = func_8015477c((s32)(*(volatile u32*)(s1 + 0x34u)), (s32)(budget - 0x8000u));

  s2 = AREA030_SCRATCH;
  *(volatile u16*)(s2 + 0x3Eu) = (u16)(level + 0x100);

  level = (s32)((u32)(u8)D_801E320C[2] * D_801440BC) / 255 + 8;
  s3 = AREA030_SCRATCH;
  s3[6] = (u8)(level / 15);
  *(volatile u32*)(s3 + 0x10) = (u32)level;
  if (AREA030_SCRATCH[6] >= 4u) {
    AREA030_SCRATCH[6] = 3;
  }

  s4 = AREA030_SCRATCH;
  level = (s32)(*(volatile u32*)(s4 + 0x38) - (*(volatile u32*)(s4 + 0x10) << 15));
  *(volatile u32*)(s4 + 0x10) = (u32)level;
  if (level > 0x3BFFFF) {
    D_8014930C = 0x3C0000u;
  } else if (level > 0x14FFFF) {
    D_8014930C = (u32)level;
  } else {
    D_8014930C = 0x150000u;
  }

  s5 = AREA030_SCRATCH;
  D_8014932E = 0u;
  *(volatile u32*)(s5 + 0x0Cu) = 0u;
  index = s5[6];
  *(volatile u32*)(s5 + 0x14u) = D_801E1D38[index].first;
  *(volatile u32*)(s5 + 0x20u) = D_801E1D38[index].second;
  s5[0x24u] = 0u;

  s6 = AREA030_SCRATCH;
  s6[0x29u] = 5u;
  AREA030_SCRATCH[0x48u] = 1u;
  s7 = AREA030_SCRATCH;
  s7[0] = (u8)(s7[0] & 0xDFu);
  AREA030_SCRATCH[0x0Bu] = 0u;
  AREA030_SCRATCH[0x5Du] = 0u;
  AREA030_SCRATCH[0x5Eu] = 0u;
  AREA030_SCRATCH[0x5Fu] = 0u;
  AREA030_SCRATCH[0x2Au] = 0u;
  s8 = AREA030_SCRATCH;
  *(volatile u32*)(s8 + 0x60u) = 0u;
  *(volatile u32*)(s8 + 0x64u) = 0u;
  *(volatile u32*)(s8 + 0x68u) = 0u;
  *(volatile u32*)(s8 + 0x6Cu) = 0u;
  AREA030_SCRATCH[0x48u] = 1u;

  func_8014dd3c((s32)(*(volatile u16*)(D_801E3210 + 0x0Cu)));
  func_8014d6b8(1);
  index = (u8)func_8019601c();
  if (index != 0xFFu) {
    D_80143FC8[index].flags_00 = 1u;
    D_80143FC8[index].byte_05 = 10u;
  }

  AREA030_SCRATCH[7] = D_801E3210[0x0Fu];
  s9 = AREA030_SCRATCH;
  s9[1] = (u8)(s9[1] + 1u);
}
