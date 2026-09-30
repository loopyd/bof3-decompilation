#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F92DC
 * @behavior Runs the SCENA00 route-stage handler for the route word
 * D_80143F00. When that word reads 1 it runs func_801BE1B0(0) and then, for
 * the shared mode byte D_80146866 reading 0, stores the object byte
 * func_8019601C() in scratchpad 0x1F800000, seeds that object's 0x74-byte row
 * at 0x80143FC8 (+0 = 1, +5 = 0x10, word 2 at +0x18 and +0x1C, word 3 at
 * +0x0C and the route word 1 at +0x10) unless that byte is 0xFF and then
 * subtracts 0x155 from the shared fixed-point channel D_801492DC, while a mode
 * byte reading 3 commits D_80147C38 = 1 and D_80147C3C = 4, arms
 * D_80147C21 = 7 and adds 0x155 to D_801492DC; any other mode byte clears
 * D_80146866 and D_80146875, and the block ends by storing 10 in D_80146874.
 * It then runs func_801F9644() unconditionally; when the route word reads 4 it
 * takes the two-stage wait func_8015B5D4(D_8014686C, 0) and, on a zero return,
 * clears the effect bank D_8014832E, reruns func_8015B580(D_8014686C, 0) and
 * func_8015C088() and requests front-selector mode 2 in D_80146874, and while
 * the front-side byte D_80143F03 reads 2 it arms D_8014832E = 0x1F and runs
 * func_8015C100(). It then runs func_801F9868() unconditionally; when the
 * route word reads 25 it arms D_801468A0/D_801468A4 = 2 and
 * D_80147BA0 = 1/D_80147BA4 = 3, queues frontend cue 0x200, requests
 * front-selector mode 11 in D_80146874, arms the 0x14-frame countdown
 * D_80146876 and clears D_8014832E; when it reads 31 it seeds the shared
 * channel words D_801492D8 = 0x100, D_801492DC = 0, D_801492DA = 0 and
 * D_8014932C = 0x100 and then waits through func_8015B5D4(D_8014686C, 1),
 * clearing D_8014832E, rerunning func_8015B580(D_8014686C, 1) and requesting
 * front-selector mode 3 on a zero return, or running func_801BE1B0(0) and
 * requesting mode 9 otherwise. It finishes by storing 2 in the overlay state
 * byte D_80146872 and returns; the address is entry 2 (the word reading
 * 0x801F92DC) of the overlay's handler table at 0x801FCA10. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the route word is read once into a
 * callee-saved local for the entry test and the two row words that store it,
 * while every later stage test re-reads D_80143F00 through the global (the
 * calls in between force a fresh load), so the source keeps both spellings.
 * The mode-3 arm is an else-if whose failure path is the compiler's shared
 * clear/store tail; the trailing two-comparison stages stay two separate plain
 * ifs, because a switch would reuse one load across them and no longer match.
 * The route-31 wait reads D_8014686C through a non-volatile view: with the
 * volatile read the pre-reload scheduler hoists the trailing `li a1,1` above
 * the four channel stores and the `jal` delay slot degrades to a `nop`
 * (measured 214/219); the non-volatile view keeps the constant adjacent to the
 * closing `jal` (measured 218/218).
 */
void func_801F92DC(void) {
  u8* scratch;
  u8  index;
  u16 route = D_80143F00;

  if (route == 1u) {
    func_801BE1B0(0u);

    if (D_80146866 == 0u) {
      u16* chan;

      index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = index;

      if (index != 0xffu) {
        D_80143FC8[(u32)index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x10u;
        *(s32*)(D_80143FE0 + (u32)*scratch * 0x74u) = 2;
        *(s32*)(D_80143FE4 + (u32)*scratch * 0x74u) = 2;
        *(s32*)(D_80143FD4 + (u32)*scratch * 0x74u) = 3;
        *(s32*)(D_80143FD8 + (u32)*scratch * 0x74u) = (s32)route;
      }

      chan = &D_801492DC;
      *chan = (u16)(*chan - 0x155u);
    } else if (D_80146866 == 3u) {
      u16* chan = &D_801492DC;

      D_80147C38 = (u32)route;
      D_80147C3C = 4u;
      D_80147C21 = 7u;
      *chan = (u16)(*chan + 0x155u);
    }

    if (D_80146866 != 3u) {
      D_80146866 = 0u;
      D_80146875 = 0u;
    }

    D_80146874 = 10u;
  }

  func_801F9644();

  if (D_80143F00 == 4u) {
    if (func_8015B5D4(D_8014686C, 0) == 0) {
      D_8014832E = 0u;
      func_8015B580(D_8014686C, 0);
      func_8015C088();
      D_80146874 = 2u;
    }

    if (D_80143F03 == 2u) {
      D_8014832E = 0x1fu;
      func_8015C100();
    }
  }

  func_801F9868();

  if (D_80143F00 == 25u) {
    D_801468A0 = 2u;
    D_801468A4 = 2u;
    D_80147BA0 = 1u;
    D_80147BA4 = 3u;
    game_queue_frontend_cue(0x200u);
    D_80146874 = 11u;
    D_80146876 = 20u;
    D_8014832E = 0u;
  }

  if (D_80143F00 == 31u) {
    D_801492D8 = 0x100u;
    D_801492DC = 0u;
    D_801492DA = 0u;
    D_8014932C = 0x100u;

    /* Non-volatile view of the shared flag word: the volatile read is a
     * scheduling barrier that displaces the trailing call-argument constant
     * out of the original `jal` delay slot (same idiom as func_801F9644). */
    if (func_8015B5D4(*(u32 *)&D_8014686C, 1) == 0) {
      D_8014832E = 0u;
      func_8015B580(D_8014686C, 1);
      D_80146874 = 3u;
    } else {
      func_801BE1B0(0u);
      D_80146874 = 9u;
    }
  }

  D_80146872 = 2u;
}
