#include "bof3/battle/battle03_internal.h"

/* @behavior selects a target/result byte from the current enemy state and chosen
 * enemy kind, delegating special cases to several smaller picker helpers.
 * @source 0x801E2A88
 * @status partial
 * @match 44.44
 * @residual 60/135 instructions; 540 original bytes versus 516 current.
 * From +0x0004, argument and record-index register assignments differ. The
 * unsigned picker arguments preserve the exact helper bodies but do not
 * restore the original caller's register web or merged mask branches.
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
      value = pickTargetOrFf((u8)index);
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
        return pickTargetByMode((u8)index);
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
