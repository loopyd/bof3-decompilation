#include "internal.h"

typedef struct Scena16Bank80140000 {
  u8   pad_0000_686b[0x686c];
  vu32 word_686c;
  u8   pad_6870_6873[4];
  vs8  byte_6874;
  u8   pad_6875_832d[0x832e - 0x6875];
  vu8  byte_832e;
} Scena16Bank80140000;

typedef struct Scena16Bank80150000 {
  u8   pad_0000_92d7[0x92d8];
  vu16 half_92d8;
  vu16 half_92da;
  vu16 half_92dc;
  u8   pad_92de_932b[0x932c - 0x92de];
  vu16 half_932c;
} Scena16Bank80150000;

/* @behavior seeds one routed setup path and enters secondary state 3 on success.
 * @source 0x801F6E30
 */
void func_801F6E30(void) {
  u32 arg0;

  ((volatile Scena16Bank80150000*)0x80140000u)->half_92d8 = 0x100u;
  ((volatile Scena16Bank80150000*)0x80140000u)->half_92dc = 0u;
  ((volatile Scena16Bank80150000*)0x80140000u)->half_92da = 0u;
  ((volatile Scena16Bank80150000*)0x80140000u)->half_932c = 0x100u;
  func_8015C100();

  if (func_8015B5D4(((volatile Scena16Bank80140000*)0x80140000u)->word_686c,
                    1) == 0) {
    arg0 = ((volatile Scena16Bank80140000*)0x80140000u)->word_686c;
    ((volatile Scena16Bank80140000*)0x80140000u)->byte_832e = 0u;
    func_8015B580(arg0, 1);
    ((volatile Scena16Bank80140000*)0x80140000u)->byte_6874 = 3;
  }
}
