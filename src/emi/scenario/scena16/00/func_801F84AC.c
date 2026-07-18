#include "internal.h"

typedef struct Scena16EffectBank80140000 {
  u8  pad_0000_4f58[0x4f59];
  volatile u8 byte_4f59;
  volatile u8 byte_4f5a;
  volatile u8 byte_4f5b;
  volatile u8 byte_4f5c;
  volatile u8 byte_4f5d;
  volatile u8 byte_4f5e;
  volatile u8 byte_4f5f;
  u8  pad_4f60_6253[0x6254 - 0x4f60];
  volatile u8 byte_6254;
} Scena16EffectBank80140000;

/* @behavior resets one local effect bank and marks the frontend flag byte.
 * @source 0x801F84AC
 */
void func_801F84AC(void) {
  u8 flags;

  func_80166E88(10, 0xff, 0xff, 0);
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
  func_801C187C(10);
}
