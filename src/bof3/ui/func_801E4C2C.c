#include "bof3/ui/shop00_internal.h"

/* @source 0x801E4C2C
 * @behavior shop entry requirement predicate, called by the shop list builder
 *           func_801E45C0 once per non-empty slot with the stored slot byte
 *           minus one: for the ids 0xB, 0xD and 0xE it reports whether bit 3, 4
 *           or 5 of the main-RAM flag byte D_80144F59 is set and for the id
 *           0xC whether flag 0x6C of the main-RAM flag bank D_80144F28 is set,
 *           which it reads through the shared flag tester func_8015B5D4. For
 *           every other id it selects the 0xFF-terminated requirement list of
 *           the EMI-local pointer table D_801E5FC0 with that id and returns 1
 *           when the list is empty or every listed id is recorded in the skill
 *           lists scanned by func_801E4D4C, otherwise 0.
 * @status partial
 * @match 81.94
 * @residual tail normalization plus delay-slot allocation. The
 *           `default: goto scan;` spelling fixes the block order the earlier
 *           candidate missed: the flag arm now sits before the scan, the
 *           dispatch jumps to the original's 0x801E4CDC, and the first
 *           difference moves from +0x0030 to +0x004C (59/72, was 55/72). What
 *           still differs is the pinned gcc-2.7.2-psx pipeline's own
 *           normalization of this tail: it if-converts
 *           `if (flag != 0) return 1; return 0;` into one `j epilogue` plus
 *           `sltu v0,zero,v0` and gives case 0xC its own copy, so the return-1
 *           block keeps a single predecessor instead of the original's two
 *           (flag-true fallthrough plus the scan's list-terminated edge), and
 *           two delay slots then allocate differently (the default index mask
 *           and `li s1,255`). Peeled-list/do-while spellings that add the
 *           second predecessor measured 54/72 in the same sweep. Smallest
 *           missing evidence: a clean-C spelling that keeps this boolean
 *           return branchy.
 */
s32 func_801E4C2C(u8 arg0) {
  u8* p;
  s32 flag;

  switch (arg0) {
  case 0xB:
    flag = D_80144F59 & 8;
    break;
  case 0xC:
    flag = func_8015B5D4((u32)&D_80144F28, 0x6C);
    break;
  case 0xD:
    flag = D_80144F59 & 0x10;
    break;
  case 0xE:
    flag = D_80144F59 & 0x20;
    break;
  default:
    goto scan;
  }
  if (flag != 0) {
    return 1;
  }
  return 0;
scan:
  p = D_801E5FC0[arg0];
  while (*p != 0xFF) {
    if ((u8)func_801E4D4C(*p) == 0) {
      return 0;
    }
    p++;
  }
  return 1;
}
