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
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The flag tail returns through two separate blocks: the original keeps the
 * `return 1` block reachable from both the flag-true fallthrough and the scan's
 * list-terminated edge, so the spelling branches on `flag == 0` into the shared
 * epilogue and labels the `return 1` block the scan jumps back to. Writing the
 * tail as `if (flag != 0) return 1; return 0;` lets the pinned compiler
 * if-convert it into `sltu` and gives case 0xC its own copy (59/72). The scan
 * keeps the current list byte in a u8 local so one `lbu` feeds both the
 * 0xFF terminator test and the func_801E4D4C argument. */
s32 func_801E4C2C(u8 arg0) {
  u8* p;
  u8 c;
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
  if (flag == 0) {
    return 0;
  }
flagged:
  return 1;
scan:
  p = D_801E5FC0[arg0];
  c = *p;
  while (c != 0xFF) {
    if ((u8)func_801E4D4C(c) == 0) {
      return 0;
    }
    p++;
    c = *p;
  }
  goto flagged;
}
