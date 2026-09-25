#include "bof3/battle/battle15_internal.h"

/* @source 0x8009CCBC
 * @behavior Applies one percentage-scaled halfword update selected by index to
 *   the battle actor whose fields are at 0x88/0x94/0x96/0xC0..0xCA. Indexes 0
 *   and 1 scale the values at 0x94/0x96 by (value * 100) / 1000 and keep the
 *   smaller of that result and the paired limit at 0xC0/0xC2; indexes 2..5
 *   scale 0xC4/0xC6/0xC8/0xCA and clamp the result to 999; index 6 scales 0x88
 *   by value * (value * 100) / 1000 and clamps it to 999; indexes 7 and above
 *   change nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009CCBC(u8 *obj, u8 index, u8 value) {
  s32 amount;
  u32 scaled;
  u16 limit;

  /* `amount` keeps the scaled percent a single operand: the compiler otherwise
   * reassociates `field * (value * 100)` onto the loaded field, and splitting
   * the assignment into its own statement reallocates the actor pointer. */
  switch (index) {
  case 0:
    scaled = *(u16 *)(obj + 0x94) * (amount = value * 100) / 1000;
    limit = *(u16 *)(obj + 0xC0);
    if (scaled <= limit) {
      *(u16 *)(obj + 0x94) = scaled;
    } else {
      *(u16 *)(obj + 0x94) = limit;
    }
    break;
  case 1:
    scaled = *(u16 *)(obj + 0x96) * (amount = value * 100) / 1000;
    limit = *(u16 *)(obj + 0xC2);
    if (scaled <= limit) {
      *(u16 *)(obj + 0x96) = scaled;
    } else {
      *(u16 *)(obj + 0x96) = limit;
    }
    break;
  case 2:
    scaled = *(u16 *)(obj + 0xC4) * (amount = value * 100) / 1000;
    if (scaled < 1000) {
      *(u16 *)(obj + 0xC4) = scaled;
    } else {
      *(u16 *)(obj + 0xC4) = 999;
    }
    break;
  case 3:
    scaled = *(u16 *)(obj + 0xC6) * (amount = value * 100) / 1000;
    if (scaled < 1000) {
      *(u16 *)(obj + 0xC6) = scaled;
    } else {
      *(u16 *)(obj + 0xC6) = 999;
    }
    break;
  case 4:
    scaled = *(u16 *)(obj + 0xC8) * (amount = value * 100) / 1000;
    if (scaled < 1000) {
      *(u16 *)(obj + 0xC8) = scaled;
    } else {
      *(u16 *)(obj + 0xC8) = 999;
    }
    break;
  case 5:
    scaled = *(u16 *)(obj + 0xCA) * (amount = value * 100) / 1000;
    if (scaled < 1000) {
      *(u16 *)(obj + 0xCA) = scaled;
    } else {
      *(u16 *)(obj + 0xCA) = 999;
    }
    break;
  case 6:
    scaled = *(u16 *)(obj + 0x88) * value * (amount = value * 100) / 1000;
    if (scaled < 1000) {
      *(u16 *)(obj + 0x88) = scaled;
    } else {
      *(u16 *)(obj + 0x88) = 999;
    }
    break;
  default:
    break;
  }
}
