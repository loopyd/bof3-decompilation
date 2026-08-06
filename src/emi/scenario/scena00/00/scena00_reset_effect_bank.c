#include "internal.h"

typedef struct Scena00EffectBank80140000 {
  u8          pad_0000_4f58[0x4f59];
  volatile u8 byte_4f59;
  volatile u8 byte_4f5a;
  volatile u8 byte_4f5b;
  volatile u8 byte_4f5c;
  volatile u8 byte_4f5d;
  volatile u8 byte_4f5e;
  volatile u8 byte_4f5f;
  u8          pad_4f60_6253[0x6254 - 0x4f60];
  volatile u8 byte_6254;
} Scena00EffectBank80140000;

#define SCENA00_EFFECT_BANK                                                    \
  PSX_PTR(volatile Scena00EffectBank80140000, 0x80140000u)

/* @behavior resets one local effect bank and marks the frontend flag byte.
 * @source 0x801FC8FC
 */
void scena00_reset_effect_bank(void) {
  u8 flags;

  func_80166E88(10, 0xff, 0xff, 0);
  flags = SCENA00_EFFECT_BANK->byte_4f59;
  SCENA00_EFFECT_BANK->byte_4f5a = 0xffu;
  SCENA00_EFFECT_BANK->byte_4f5b = 0xffu;
  SCENA00_EFFECT_BANK->byte_4f5c = 0xffu;
  SCENA00_EFFECT_BANK->byte_4f5d = 0xffu;
  SCENA00_EFFECT_BANK->byte_4f5e = 0xffu;
  SCENA00_EFFECT_BANK->byte_4f5f = 0xffu;
  SCENA00_EFFECT_BANK->byte_6254 = 0u;
  SCENA00_EFFECT_BANK->byte_4f59 = (u8)(flags | 7u);
  /* MATCHING_AID: CLOBBER_CALLER_REG(a0) forces a0=10 into the jal delay slot
   * instead of being hoisted after the first call. */
  CLOBBER_CALLER_REG(a0);
  func_801C187C(10);
}
