#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9644
 * @behavior Stages the SCENA00 route-path-2 resource/object setup sequence
 * gated on the route word D_80143F00 being 2. When that gate holds it first
 * runs the two-stage wait func_8015B5D4(D_8014686C, 2): on a zero return it
 * clears the scenario progress word g_ScenarioProgress, reruns
 * func_80154FD8(0x300) and func_801C601C(0), raises D_801492D8 by 0xAA, stores
 * the object byte func_8019601C() in scratchpad 0x1F800000 and, unless that
 * byte is 0xFF, seeds that object's 0x74-byte row at 0x80143FC8 (+0 = 1,
 * +5 = 0x13, word D_801492D8 - 0xAA at +0x64, the signed halfwords
 * D_801492DA/D_801492DC at +0x68/+0x6C and +9 = 0x40); it then runs
 * func_8015C100(), commits the front selector word pair D_80145EC4 =
 * 0x330000/D_80145EC8 = 0x400000 and calls func_8015B580(D_8014686C, 2). When
 * the first wait instead returns nonzero it retries through
 * func_8015B5D4(D_8014686C, 10) and, on a zero return, calls
 * func_8015B580(D_8014686C, 10), func_801BE1B0(0) and rebases the fixed-point
 * channel pair D_80149308 = 0x260000/D_8014930C = 0x1B8000. Takes no arguments
 * and returns nothing; the address is the single jal target of the SCENA00
 * sub-controller at 0x801F94AC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F9644(void) {
  u8 *scratch;
  u16 *ramp;
  u8 index;
  u32 flag_word;

  if (D_80143F00 == 2u) {
    if (func_8015B5D4(D_8014686C, 2) == 0) {
      g_ScenarioProgress = 0u;
      func_80154FD8(0x300u);
      func_801C601C(0u);
      ramp = &D_801492D8;
      *ramp = (u16)(*ramp + 0xaau);
      index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = index;

      if (index != 0xffu) {
        D_80143FC8[(u32)index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) =
            (s32)(s16)*ramp - 0xaau;
        *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) =
            (s32)(s16)D_801492DA;
        *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) =
            (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x40u;
      }

      func_8015C100();
      /* Non-volatile view of the shared scenario flag word: the volatile read
       * is a scheduling barrier that displaces the trailing call-argument
       * constant out of the original `jal` delay slot. */
      flag_word = *(u32 *)&D_8014686C;
      D_80145EC4 = 0x330000u;
      D_80145EC8 = 0x400000u;
      func_8015B580(flag_word, 2);
    } else if (func_8015B5D4(D_8014686C, 10) == 0) {
      func_8015B580(D_8014686C, 10);
      func_801BE1B0(0u);
      D_80149308 = 0x260000u;
      D_8014930C = 0x1b8000;
    }
  }
}
