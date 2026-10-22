#include "bof3/battle/battle03_internal.h"

/* @behavior selects a target/result byte from the current enemy state and chosen
 * enemy kind, delegating special cases to several smaller picker helpers.
 * @source 0x801E2A88
 * @status partial
 * @match 43.70
 * @residual non-exact live audit: 59/135 instructions at 516 versus 540 bytes;
 * first difference at +0x0004, where this build copies the argument into a3 and
 * the original copies it into a2. The two-test 0x80/0x40 form, the per-arm record
 * zero-extension and the three `lui $at,%hi` / `addu $at,$at,scale` /
 * `lhu %lo(record)($at)` folds match. The remaining difference is one register
 * web: the original holds the kind byte in v1, the flags halfword in a0, the
 * widened index in a1 and the raw argument in a2, so each indexed call needs
 * `move a0,a1` (three times), while this build holds the index in a0 and the
 * flags in a2 and sign-extends the two picker arguments instead (four sll/sra).
 * pickTargetOrFf and pickTargetByMode declare s8 parameters and sit outside this
 * lane's write set; retail passes the zero-extended byte unconverted, so `u8`
 * parameters there would drop those four instructions. Two `andi` masks and one
 * branch also stay unmerged relative to the original.
 * Matching note: the switch operand takes the target's non-volatile byte view,
 * `*(u8*)&BATTLE_GLOBAL_BYTE_6375`, as the exact checkEnemyQueuedPredicate and
 * checkLocalQueuedBranch rows do; a volatile read keeps a redundant
 * `andi ...,0xff` and costs three instructions (56/135 at 520). The widened index
 * is promoted per case arm (`index = arg0` inside case 1 and case 4, the raw
 * parameter in default) rather than once before the switch, which reproduces the
 * original's per-arm `andi` copies (49/135 at 504 for one promotion, 36/135 for
 * none). The 0x80/0x40 pair must stay nested: folding it into `(kind & 0xC0) ==
 * 0x80` costs an instruction at each site (47/135 at 480 with both folded). The
 * record fields are not volatile in this build because no path reads one twice.
 */
u8 func_801E2A88(u8 arg0) {
  u16 flags;
  u8  kind_flags;
  u8  value;
  u32 index;

  switch (*(u8*)&BATTLE_GLOBAL_BYTE_6375) {
  case 1u: {
    index = arg0;
    flags = D_801EB630[index].unk_80;
    if ((flags & 1u) != 0u) {
      value = pickTargetOrFf((s8)index);
      if (value == 0xFFu) {
        return func_801E2E30();
      }
      return value;
    }
    if ((flags & 0x10u) != 0u) {
      return func_801E2D90();
    }
    return func_801E2E30();
  }
  case 4u: {
    index = arg0;
    kind_flags = D_801CA718[(u32)D_801EB630[index].unk_f6 * 0x14u];
    flags = D_801EB630[index].unk_80;

    if ((flags & 1u) != 0u) {
      if ((kind_flags & 0x10u) != 0u) {
        if ((kind_flags & 0x80u) != 0u) {
          if ((kind_flags & 0x40u) == 0u) {
            return 0xC0;
          }
        }
        if ((kind_flags & 0x20u) != 0u) {
          return 0x40;
        }
        return 0x80;
      }
      if ((kind_flags & 0x40u) == 0u) {
        return (u8)(arg0 + 3u);
      }
      if ((kind_flags & 0x20u) != 0u) {
        return pickTargetByMode((s8)index);
      }
      if ((flags & 0x10u) != 0u) {
        return func_801E2D90();
      }
      return func_801E2E30();
    }

    if ((kind_flags & 0x10u) != 0u) {
      if ((kind_flags & 0x80u) != 0u) {
        if ((kind_flags & 0x40u) == 0u) {
          return 0xC0;
        }
      }
      if ((kind_flags & 0x20u) != 0u) {
        return 0x80;
      }
      return 0x40;
    }
    if ((kind_flags & 0x40u) == 0u) {
      return (u8)(arg0 + 3u);
    }
    if ((kind_flags & 0x20u) == 0u) {
      return func_801E2CA4();
    }
    if ((flags & 0x10u) != 0u) {
      return func_801E2D90();
    }
    return func_801E2E30();
  }
  default:
    return D_801EB630[arg0].unk_05;
  }
}
