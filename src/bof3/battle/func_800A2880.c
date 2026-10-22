#include "bof3/battle/battle15_internal.h"

/* @source 0x800A2880
 * @behavior Scales the attack power of the selected battle action for the
 *   battler's defence rate, element affinity, randomised damage band and guard
 *   flags, and halves that result while the 0x200 flag of the battler's word
 *   at +0x04 is set. A battler whose word at +0x00 carries bit 0x10000 takes no
 *   damage. The first argument is the 0x80146374 selection byte, which the body
 *   never reads; the battler record index is the second argument. The rate and
 *   percent scalings both divide unsigned, so a non-positive intermediate
 *   wraps the way the original's own unsigned divide does.
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 func_800A2880(u8 selection_index, u8 battler_index, u16 base_value,
                  u8 element_flag)
{
  s32 damage;
  s32 percent;
  s32 rate;
  u16 kind_mask;
  s16 modifier;

  /* The original promotes the defence rate to a percent-times-hundred fixed
   * point before applying it, so the *0x64 against the 0x64u divisor are the
   * original's own operations. */
  rate = (D_801EC312 + 0x64) * 0x64;
  damage = base_value * rate / 0x64u;

  /* Split statements keep the division unsigned: the dividend is the s32 local
   * and the divisor is 5u, so GCC emits `multu`/`mfhi`/`srl`; the subtraction
   * and the clamp then act on the integer local and the original's `subu`, `slti`
   * and single clamp store follow. */
  percent = D_801EC2F2;
  percent = 0x64 - percent / 5u;
  if (percent < 0x32) {
    percent = 0x32;
  }
  /* Unsigned divisor: the original scales by the clamped percent with
   * `multu`/`mfhi`/`srl`, while a signed divide emits `mult`/`sra`/`subu`. */
  damage = damage * percent / 0x64u;

  kind_mask = D_801CA71C[D_801463C0].mask & 0x1FFu;
  if (kind_mask != 0u) {
    if (element_flag != 0u) {
      modifier = (s16)func_800A2EF0(battler_index, kind_mask);
    } else {
      modifier = (s16)func_800A2AE0(battler_index, kind_mask);
    }
    damage = damage * modifier / 0x64;
  }

  damage = damage * D_800B492C[rand() & 7] / 10000;

  if (D_80146394 < 3u && PSX_REF(u8, 0x80144F58u) == 2u && damage > 0) {
    damage -= damage >> 2;
  }

  if (battler_index < 3u) {
    if (D_80145FB4[battler_index].unk_04 & 0x200u) {
      damage /= 2;
    }
    if (D_80145FB4[battler_index].unk_00 & 0x10000u) {
      damage = 0;
    }
  } else {
    if (D_801EB730[battler_index - 3].unk_04 & 0x200u) {
      damage /= 2;
    }
    if (D_801EB730[battler_index - 3].unk_00 & 0x10000u) {
      damage = 0;
    }
  }
  return (s16)damage;
}
