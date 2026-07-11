#include "internal.h"

typedef struct Scena16EffectBank80140000 {
  u8  pad_0000_4f58[0x4f59];
  vu8 byte_4f59;
  vu8 byte_4f5a;
  vu8 byte_4f5b;
  vu8 byte_4f5c;
  vu8 byte_4f5d;
  vu8 byte_4f5e;
  vu8 byte_4f5f;
  u8  pad_4f60_6253[0x6254 - 0x4f60];
  vu8 byte_6254;
} Scena16EffectBank80140000;

/* @behavior resets one local effect bank and marks the frontend flag byte.
 * @source 0x801f84ac FUN_801f84ac
 */
void func_801f84ac(void) {
  u8 flags;

  func_80166e88(10, 0xff, 0xff, 0);
  flags = ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f59;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f5a = 0xffu;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f5b = 0xffu;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f5c = 0xffu;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f5d = 0xffu;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f5e = 0xffu;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f5f = 0xffu;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_6254 = 0u;
  ((volatile Scena16EffectBank80140000*)0x80140000u)->byte_4f59 =
      (u8)(flags | 7u);
  func_801c187c(10);
}
