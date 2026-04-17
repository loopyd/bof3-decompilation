#include "internal.h"

/* does: copies one 16-word palette block and bumps the stage serial.
 * @source: 0x801f845c FUN_801f845c
 */
void func_801f845c(void) {
  u16*       dst;
  const u16* src;
  s32        i;
  u8         serial;
  vu8*       serial_base;

  i = 0;
  dst = (u16*)BOF3_SCENA16_PALETTE_DST;
  src = (const u16*)BOF3_SCENA16_PALETTE_SRC;

  do {
    dst[i] = src[i];
    i++;
  } while (i < 0x10);

  serial_base = (vu8*)0x80140000u;
  serial = serial_base[0x5988];
  serial = serial + 1;
  serial_base[0x5988] = serial;
}
