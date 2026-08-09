#include "bof3/scenario/scena16_internal.h"

typedef struct Scena16Bank80140000 {
  u8           pad_0000_686b[0x686c];
  volatile u32 word_686c;
  u8           pad_6870_6873[4];
  volatile s8  byte_6874;
  u8           pad_6875_832d[0x832e - 0x6875];
  volatile u8  byte_832e;
} Scena16Bank80140000;

typedef struct Scena16Bank80150000 {
  u8           pad_0000_92d7[0x92d8];
  volatile u16 half_92d8;
  volatile u16 half_92da;
  volatile u16 half_92dc;
  u8           pad_92de_932b[0x932c - 0x92de];
  volatile u16 half_932c;
} Scena16Bank80150000;

#define SCENA16_BANK_80140000(type) PSX_PTR(volatile type, 0x80140000u)

/* @behavior seeds one routed setup path and enters secondary state 3 on success.
 * @source 0x801F6E30
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void seedRouteEnterState3(void) {
  u32 arg0;

  ((volatile Scena16Bank80150000*)SCENA16_BANK_80140000(Scena16Bank80150000))
      ->half_92d8 = 0x100u;
  ((volatile Scena16Bank80150000*)SCENA16_BANK_80140000(Scena16Bank80150000))
      ->half_92dc = 0u;
  ((volatile Scena16Bank80150000*)SCENA16_BANK_80140000(Scena16Bank80150000))
      ->half_92da = 0u;
  ((volatile Scena16Bank80150000*)SCENA16_BANK_80140000(Scena16Bank80150000))
      ->half_932c = 0x100u;
  func_8015C100();

  if (func_8015B5D4(((volatile Scena16Bank80140000*)SCENA16_BANK_80140000(
                         Scena16Bank80140000))
                        ->word_686c,
                    1) == 0) {
    arg0 = ((volatile Scena16Bank80140000*)SCENA16_BANK_80140000(
                Scena16Bank80140000))
               ->word_686c;
    ((volatile Scena16Bank80140000*)SCENA16_BANK_80140000(Scena16Bank80140000))
        ->byte_832e = 0u;
    /*
     * MATCHING_AID: memory-access ordering. asm-diff showed the original
     * keeps `sb zero,-31954(at)` immediately before `jal func_8015B580`
     * with `li a1,1` in the delay slot; without the barrier GCC schedules
     * `li a1,1` before the store and emits a nop delay slot (+4 bytes).
     * Live bin/byte-match after this aid is exact. Remove if GCC's
     * scheduler placement for this call is otherwise reproduced.
     */
    barrier();
    func_8015B580(arg0, 1);
    ((volatile Scena16Bank80140000*)SCENA16_BANK_80140000(Scena16Bank80140000))
        ->byte_6874 = 3;
  }
}
