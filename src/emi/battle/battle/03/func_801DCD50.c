#include "internal.h"

extern int rand(void);
/* @behavior applies the current scratchpad damage modifiers, variance table, and
 * optional battler-specific scale table to one signed damage value.
 * @source 0x801DCD50
 */
u32 func_801DCD50(u32 arg0, u8 arg1, s32 arg2) {
  volatile u16 *scratch;
  s32 scaled;
  s32 scale;
  s32 value;
  /*
   * MATCHING_AID:
   * REGISTER_PIN pair reproduces the original's first scratchpad-flags test
   * `lhu a1,0(s0); andi v0,a1,0x1f; beqz v0`; clean C coalesces both into
   * v0 (`lhu v0,0(s0); andi v0,v0,0x1f`). Exhausted rungs: type/declaration
   * and statement-order variants, flag-search profiles, and one 60s
   * permuter run (best score 30, same residual). The live bin/byte-match
   * immediately after adding this aid was exact (106/106, 424 bytes).
   * Remove if a clean-C shape reproduces the a1 load / v0 mask split.
   */
  REGISTER_PIN(u16, flags, "a1");
  REGISTER_PIN(s32, mode, "v0");

  value = arg2 << 8;
  scaled = value;
  scale = (0x100 - ((scaled / 0x3e800) * 0x20000)) >> 8;
  if (scale < 0xcd) {
    scale = 0xcd;
  }

  value = (((scaled * scale) >> 8) * D_801EAFA0[rand() & 7]) >> 8;

  scratch = &D_1F800000;
  flags = scratch[0];
  mode = flags & 0x1f;
  if (mode != 0) {
    value = (value * (s16)func_800A2AE0(arg1)) / 100;
  }
  if ((scratch[0] & 0x20) != 0) {
    if (arg1 < 3) {
      value = (value * D_801EAFC0[D_80145E90[arg1].unk_a4]) / 100;
    } else {
      value = (value * D_801EAFC0[D_801EB630[arg1 - 3].unk_b4]) / 100;
    }
  }

  if ((value & 0xff) >= 0x80) {
    value += 0x100;
  }
  return (u32)((value << 8) >> 16);
}
