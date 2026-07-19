#include "internal.h"

/* @behavior copies one 16-word palette block and bumps the stage serial.
 * @source 0x801F845C
 */
void func_801F845C(void) {
  u16*         dst;
  const u16*   src;
  s32          i;
  u8           serial;
  volatile u8* serial_base;

  i = 0;
  dst = (u16*)SCENA16_PALETTE_DST;
  src = (const u16*)SCENA16_PALETTE_SRC;

  do {
    dst[i] = src[i];
    i++;
  } while (i < 0x10);

  serial_base = PSX_PTR(volatile u8, 0x80140000u);
  serial = serial_base[0x5988];
  serial = serial + 1;
  serial_base[0x5988] = serial;
}
