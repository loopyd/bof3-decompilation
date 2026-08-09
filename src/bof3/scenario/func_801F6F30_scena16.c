#include "bof3/scenario/scena16_internal.h"

/* @behavior stages one routed resource/object setup sequence for area path 2.
 * @source 0x801F6F30
 * @status partial
 * @match 39.85
 * @residual non-exact live audit: 53/133 instructions; 532 original bytes versus 428 current.
 */
void func_801F6F30(void) {
  if (func_8015B5D4(D_8014686C, 2) == 0) {
    volatile u8* object;
    u8           object_index;

    D_80146864 = 0u;
    func_80154FD8(0x300u);
    func_801C601C(0u);
    D_801492D8 = (u16)(D_801492D8 + 0xaau);
    object_index = func_8019601C();
    SPAD_REF(volatile u8, 0x0u) = object_index;

    if (object_index != 0xffu) {
      object = PSX_PTR(volatile u8, 0x80143fc8u) + ((u32)object_index * 0x74u);
      object[0] = 1u;
      object[5] = 0x13u;
      *(volatile s32*)(object + 0x64) =
          (s32)((s16)D_801492D8 - (s16)0xaau);
      *(volatile s32*)(object + 0x68) = (s32)(s16)D_801492DA;
      *(volatile s32*)(object + 0x6c) = (s32)(s16)D_801492DC;
      object[9] = 0x40u;
    }

    func_8015C100();
    D_80145EC4 = 0x330000u;
    D_80145EC8 = 0x400000u;
    func_8015B580(D_8014686C, 2);
  } else if (func_8015B5D4(D_8014686C, 10) == 0) {
    func_8015B580(D_8014686C, 10);
    func_801BE1B0(0u);
    D_80149308 = 0x260000u;
    D_8014930C = (s32)0x1b8000u;
  }
}
