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
 * @match 96.24
 * @residual initial setup and product input registers differ from the original
 */
void initializeModePanelWorkAndAdvanceStep(void) {
  s8* mode;
  u8* seedWork;
  u8* levelWork;
  u8* indexWork;
  u8* budgetWork;
  u8* entryWork;
  u8* setupWork;
  u8* flagsWork;
  u8* clearWork;
  u8* stepWork;
  s32 level;
  s32 remainingBudget;
  u32 value;
  u32 budget;
  u8 index;

  seedWork = AREA030_SCRATCH;
  value = D_80145EC4;
  do {
    budget = D_80145EC8;
    *(volatile u32*)(seedWork + 0x34u) = value;
    *(u32*)(seedWork + 0x38u) = budget - 0x8000u;
    level = func_8015477c((s32)(*(volatile u32*)(seedWork + 0x34u)), (s32)(budget - 0x8000u));
  } while (0);

  levelWork = AREA030_SCRATCH;
  mode = D_801E320C;
  *(volatile u16*)(levelWork + 0x3Eu) = (u16)(level + 0x100);

  do {
    level = (s32)(D_801440BC * (u32)(u8)mode[2]) / 255 + 8;
    levelWork[6] = (u8)(level / 15);
  } while (0);
  indexWork = AREA030_SCRATCH;
  *(volatile u32*)(levelWork + 0x10) = (u32)level;
  if (indexWork[6] >= 4u) {
    indexWork[6] = 3;
  }

  budgetWork = AREA030_SCRATCH;
  remainingBudget = (s32)(*(volatile u32*)(budgetWork + 0x38) - (*(volatile u32*)(budgetWork + 0x10) << 15));
  *(u32*)(budgetWork + 0x10) = (u32)remainingBudget;
  if (remainingBudget > 0x3BFFFF) {
    D_8014930C = 0x3C0000u;
  } else if (remainingBudget <= 0x14FFFF) {
    D_8014930C = 0x150000u;
  } else {
    D_8014930C = (u32)remainingBudget;
  }

  entryWork = AREA030_SCRATCH;
  D_8014932E = 0u;
  index = entryWork[6];
  *(volatile u32*)(entryWork + 0x0Cu) = 0u;
  value = D_801E1D38[index].first;
  index = entryWork[6];
  *(volatile u32*)(entryWork + 0x14u) = value;
  *(volatile u32*)(entryWork + 0x20u) = D_801E1D38[index].second;
  entryWork[0x24u] = 0u;

  setupWork = AREA030_SCRATCH;
  setupWork[0x29u] = 5u;
  AREA030_SCRATCH[0x48u] = 1u;
  flagsWork = AREA030_SCRATCH;
  flagsWork[0] = (u8)(flagsWork[0] & 0xDFu);
  AREA030_SCRATCH[0x0Bu] = 0u;
  AREA030_SCRATCH[0x5Du] = 0u;
  AREA030_SCRATCH[0x5Eu] = 0u;
  AREA030_SCRATCH[0x5Fu] = 0u;
  clearWork = AREA030_SCRATCH;
  clearWork[0x2Au] = 0u;
  stepWork = AREA030_SCRATCH;
  *(volatile u32*)(clearWork + 0x60u) = 0u;
  *(volatile u32*)(clearWork + 0x64u) = 0u;
  *(volatile u32*)(clearWork + 0x68u) = 0u;
  *(volatile u32*)(clearWork + 0x6Cu) = 0u;
  stepWork[0x48u] = 1u;

  func_8014dd3c((s32)(*(volatile u16*)(D_801E3210 + 0x0Cu)));
  func_8014d6b8(1);
  index = (u8)func_8019601c();
  if (index != 0xFFu) {
    D_80143FC8[index].flags_00 = 1u;
    D_80143FC8[index].byte_05 = 10u;
  }

  AREA030_SCRATCH[7] = D_801E3210[0x0Fu];
  stepWork = AREA030_SCRATCH;
  stepWork[1] = (u8)(stepWork[1] + 1u);
}
