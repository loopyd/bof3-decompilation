#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9868
 * @behavior Runs the SCENA00 route-path-3 (0x18) step gated on the route word
 * D_80143F00 reading 0x18 and dispatched by the front-side byte D_80143F03;
 * the route-stage handler func_801F92DC runs it as its unconditional third
 * step. Case 0 clears the scenario progress word and waits through
 * func_8015B5D4(D_8014686C, 6): on a zero return it requests primary state 8
 * by storing D_80146874 = 8 and D_80146875 = 0. Case 1 takes
 * func_8015B5D4(D_8014686C, 3) and, on a nonzero return, copies the 16-halfword
 * tails of the fixed buffers at 0x80033E00 and 0x80037E00 (elements 0x50..0x5F
 * onto elements 0x40..0x4F), publishes 1 in D_80145988 and then, when
 * func_8015B5D4(D_8014686C, 5) reports zero, arms the effect bank
 * D_8014832E = 0x1F, clears the progress word and runs func_801BE1B0(1),
 * otherwise just runs func_8015C058; on a zero return from that first wait it
 * instead runs func_80161BBC(3), polls func_80162D00 in a loop that reruns
 * func_8014B87C(1), then arms D_8014832E = 0x1F, clears the progress word,
 * runs func_8015B580(D_8014686C, 3) and func_80154FD8(0x480), seeds
 * D_8014932C = 0x480 and D_801492DC = 0x350, runs func_801BE1B0(0) and
 * func_801C601C(0), stores the object byte func_8019601C() in scratchpad
 * 0x1F800000 and, unless that byte is 0xFF, seeds that object's 0x74-byte row
 * at 0x80143FC8 (+0 = 1, +5 = 0x13, the signed channel words
 * D_801492D8/D_801492DA at +0x64/+0x68, word 0x200 at +0x6C and +9 = 0x90).
 * Cases 2 and 4 wait through func_8015B5D4(D_8014686C, 8) and
 * func_8015B5D4(D_8014686C, 9) and, on a zero return, clear the progress word
 * and request primary state 6 by storing D_80146874 = 6 and D_80146875 = 0.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F9868(void) {
  u16 *rows;
  u16 *mirror;
  u8 index;
  u8 *scratch;
  u8 i;
  /* Non-volatile view of the shared scenario flag word. The volatile read
   * blocks the scheduler, which displaces the trailing call-argument constant
   * out of the original `jal` delay slot; the plain load keeps every wait's
   * constant in place (same idiom as the sibling func_801F9644). */
  u32 flag_word;

  if (D_80143F00 != 0x18u) {
    return;
  }

  switch (D_80143F03) {
  case 1:
    if (func_8015B5D4(D_8014686C, 3) == 0) {
      func_80161BBC(3u);

      while (func_80162D00() == 0) {
        func_8014B87C(1u);
      }

      flag_word = *(u32 *)&D_8014686C;
      D_8014832E = 0x1fu;
      g_ScenarioProgress = 0u;
      func_8015B580(flag_word, 3);
      func_80154FD8(0x480u);
      D_8014932C = 0x480u;
      D_801492DC = 0x350u;
      func_801BE1B0(0u);
      func_801C601C(0u);
      index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = index;

      if (index != 0xffu) {
        D_80143FC8[(u32)index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) =
            (s32)(s16)D_801492D8;
        *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) =
            (s32)(s16)D_801492DA;
        *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x90u;
      }
    } else {
      /* The two fixed halfword buffers hold the 16-word tail twice: the block
       * at 0x80033E00 feeds 0x80037E00's owner and each one's elements
       * 0x50..0x5F are rolled down onto 0x40..0x4F. */
      i = 0u;
      rows = PSX_PTR(u16, 0x80033E00u);
      mirror = PSX_PTR(u16, 0x80037E00u);

      for (; i < 16u; i++) {
        rows[0x40u + i] = rows[0x50u + i];
        mirror[0x40u + i] = mirror[0x50u + i];
      }

      D_80145988 = 1u;
      /* Same non-volatile view as the first arm: the plain load also keeps
       * this wait's constant in its `jal` delay slot. */
      flag_word = *(u32 *)&D_8014686C;

      if (func_8015B5D4(flag_word, 5) == 0) {
        D_8014832E = 0x1fu;
        g_ScenarioProgress = 0u;
        func_801BE1B0(1u);
      } else {
        func_8015C058();
      }
    }
    break;

  case 0:
    g_ScenarioProgress = 0u;

    if (func_8015B5D4(D_8014686C, 6) == 0) {
      D_80146874 = 8;
      D_80146875 = 0;
    }
    break;

  case 2:
    if (func_8015B5D4(D_8014686C, 8) == 0) {
      g_ScenarioProgress = 0u;
      D_80146874 = 6;
      D_80146875 = 0;
    }
    break;

  case 4:
    flag_word = *(u32 *)&D_8014686C;

    if (func_8015B5D4(flag_word, 9) == 0) {
      g_ScenarioProgress = 0u;
      D_80146874 = 6;
      D_80146875 = 0;
    }
    break;

  default:
    break;
  }
}
