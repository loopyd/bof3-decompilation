#include "bof3/world/area02713_internal.h"

/* @source 0x801F37E4
 * @behavior Drives the shared front-end mode byte at 0x801448EC through this
 *           area's five-step startup. Mode 0 waits until the shared phase byte
 *           at 0x80143BB0 differs from 2, then runs the shared func_8015C088
 *           helper, releases the shared 0x80144F28 block under bit 1 through
 *           func_8015B580 and advances the mode to 1. Mode 1 runs the shared
 *           func_801BE1B0(1) helper and advances to 2. Mode 2 waits for the
 *           shared counter byte at 0x80146867 to read 0x20, seeds the shared
 *           halfword at 0x801448EE with 0x10 and advances to 3. Mode 3 counts
 *           that halfword down and, once it reaches zero, requests the shared
 *           front selector context (0x1B, 0x4B0000, 0x380000, 0x8F) through
 *           func_8019FA28, advances to 4 and increments the counter byte. Mode
 *           4, when the counter byte reaches 0x31, clears it, releases the area
 *           through the shared func_8015C058 helper, clears bit 1 of the shared
 *           0x80144F28 block through func_8015B5A8 and resets the shared flag
 *           0x801448EB together with the mode byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F37E4(void) {
  switch (D_801448EC) {
  case 0:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C088();
    func_8015B580(&D_80144F28, 1);
    D_801448EC = 1;
    break;
  case 1:
    func_801BE1B0(1);
    D_801448EC = 2;
    break;
  case 2:
    if (D_80146867 != 0x20) {
      break;
    }
    D_801448EE = 0x10;
    D_801448EC = 3;
    break;
  case 3:
    {
      u16* countdown = &D_801448EE;

      *countdown = *countdown - 1;
      if (*countdown != 0) {
        break;
      }
    }
    func_8019FA28(0x1B, 0x4B0000, 0x380000, 0x8F);
    D_801448EC = 4;
    {
      u8* counter = &D_80146867;

      *counter = *counter + 1;
    }
    break;
  case 4:
    {
      u8* counter = &D_80146867;

      if (*counter == 0x31) {
        *counter = 0;
        func_8015C058();
        func_8015B5A8(&D_80144F28, 1);
        D_801448EB = 0;
        D_801448EC = 0;
      }
    }
    break;
  }
}
