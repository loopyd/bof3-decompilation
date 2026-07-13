#include "internal.h"

/* @behavior stages one routed resource/object setup sequence for area path 2.
 * @source 0x801f6f30 FUN_801f6f30
 */
void func_801f6f30(void) {
  if (func_8015b5d4(SCENA16_DAT_8014686c, 2) == 0) {
    vu8* object;
    u8   object_index;

    SCENA16_DAT_80146864 = 0u;
    func_80154fd8(0x300u);
    func_801c601c(0u);
    SCENA16_DAT_801492d8 = (u16)(SCENA16_DAT_801492d8 + 0xaau);
    object_index = func_8019601c();
    *(vu8*)0x1f800000u = object_index;

    if (object_index != 0xffu) {
      object = (vu8*)(0x80143fc8u + ((u32)object_index * 0x74u));
      object[0] = 1u;
      object[5] = 0x13u;
      *(vs32*)(object + 0x64) = (s32)((s16)SCENA16_DAT_801492d8 - (s16)0xaau);
      *(vs32*)(object + 0x68) = (s32)(s16)SCENA16_DAT_801492da;
      *(vs32*)(object + 0x6c) = (s32)(s16)SCENA16_DAT_801492dc;
      object[9] = 0x40u;
    }

    func_8015c100();
    *(vu32*)0x80145ec4u = 0x330000u;
    *(vu32*)0x80145ec8u = 0x400000u;
    func_8015b580(SCENA16_DAT_8014686c, 2);
  } else if (func_8015b5d4(SCENA16_DAT_8014686c, 10) == 0) {
    func_8015b580(SCENA16_DAT_8014686c, 10);
    func_801be1b0(0u);
    *(vu32*)0x80149308u = 0x260000u;
    *(vu32*)0x8014930cu = 0x1b8000u;
  }
}
