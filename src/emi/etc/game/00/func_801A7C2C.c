#include "internal.h"

/* @behavior resets frontend state bytes and copies a 16-byte state table. */
/* @source 0x801A7C2C */
void func_801A7C2C(void) {
  u32 i;

  PSX_REF(u8, 0x801448E8u) = 0;
  D_801448EA = 1;
  D_801448EB = 0;
  PSX_REF(u8, 0x801448ECu) = 0;
  PSX_REF(u16, 0x801448EEu) = 0;
  PSX_REF(u8, 0x801448EDu) = 0;
  PSX_REF(u8, 0x801448E9u) = 0;
  PSX_REF(u16, 0x801448F0u) = 0;

  for (i = 0; (u8)i < 16; i++) {
    D_80144F28[(u8)i] = D_801C84AC[(u8)i];
  }
}
