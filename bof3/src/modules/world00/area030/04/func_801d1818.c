#include "internal.h"

/* does: emits one 16x16 sprite marker with UV selected from the 4-entry table
 * at `0x801e1d0c`, using the current global phase bits.
 * @source: 0x801d1818 FUN_801d1818
 */
void func_801d1818(s16 arg0, s16 arg1, u8 arg2) {
  u32 primitive;

  func_801e0c80(0, arg2);

  primitive = (u32)WORLD00_AREA030_PRIMITIVE_PTR;
  SetSprt16((SPRT_16*)primitive);
  *(volatile u8*)(primitive + 4) = 0x80u;
  *(volatile u8*)(primitive + 5) = 0x80u;
  *(volatile u8*)(primitive + 6) = 0x80u;
  *(volatile u16*)(primitive + 0x0eu) = 0x7a40u;
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile u8*)(primitive + 0x0cu) = WORLD00_AREA030_SPRT_TABLE
      [(WORLD00_AREA030_GLOBAL_WORD_3E6C >> 3) & 3u];
  *(volatile u8*)(primitive + 0x0du) = 0x48u;

  func_8014e5a0(arg2, 0x10u);
}
