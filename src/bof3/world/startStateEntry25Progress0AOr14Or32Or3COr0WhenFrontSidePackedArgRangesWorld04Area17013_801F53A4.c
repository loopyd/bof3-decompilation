#include "bof3/bof3.h"

/* Front-side byte of the shared 0x80143F00 game-mode tuple; the sibling
 * overlay request helpers gate on it and this body reads it with `lbu`, twice.
 * @source 0x80143F03 @kind data */
extern u8 D_80143F03;

/* Shared state-entry selector: the world overlays publish their next state
 * entry here and the front end dispatches its handler table with it. Stored
 * with one `sb` per arm; the 0x14 arm reaches the shared flag bank 0x80144F28
 * as 0x63D bytes past this byte, as the sibling overlay step does from its mode
 * byte.
 * @source 0x801448EB @kind data */
extern u8 D_801448EB;

/* Progress byte of the shared state-entry tuple 0x801448EB..0x801448ED, the
 * step index this overlay's own step handler switches on.
 * @source 0x801448EC @kind data */
extern u8 D_801448EC;

/* Leading mode byte of the main-RAM game-mode record table 0x80145E98[], read
 * here with `lbu` and compared against 7 and 3.
 * @source 0x80145E98 @kind data */
extern u8 D_80145E98;

/* Shared main-RAM flag bank of the main executable, used only as the base
 * argument of the shared flag tester and setter. Splat binds the address
 * through this target's generated undefined-syms map, so no target-map row is
 * needed.
 * @source 0x80144F28 @kind data */
extern u8 D_80144F28;

/* Shared world-flags halfword, bound at 0x80146258 by the shared map. The two
 * bits cleared through the 0xEFEF mask here are the ones the sibling routines
 * of this overlay raise and clear, and their roles stay address-anchored.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

void func_8015C088(void);
void func_8015B580(void* flag_bank, u8 bit_index);
s32 func_8015B5D4(u32 bank, s32 flag);

/* @source 0x801F53A4
 * @behavior Overlay request that starts the shared state entry 0x25 from one
 *   of two front sides. While the front-side byte D_80143F03 reads 4 it tests
 *   the two packed arguments against the window x 2.5..5.5 by y 112.5..115.5:
 *   a match is the y edge pair 0x708000/0x738000 with the high half of the
 *   first argument in 3..5, or the x edge 0x28000 or 0x58000 with the high
 *   half of the second argument in 0x71..0x73. While the byte reads 5 it tests
 *   two windows that share the y edge pair 0x8E8000/0x918000 and the y range
 *   0x8F..0x91: first the one bounded by the x edges 0x88000/0xB80000 with the
 *   first argument's high half in 9..11, then the one bounded by 0xF8000 and
 *   0x128000 with that half in 0x10..0x12. Every match runs the shared
 *   front-end startup helper func_8015C088, stores 0x25 into D_801448EB and
 *   stores the arm's progress step into D_801448EC: 0 for the side-4 window,
 *   0xA and 0x14 for the two side-5 windows. The 0x14 arm also sets flag 0x7E
 *   of the main-RAM flag bank through func_8015B580, which it reaches as 0x63D
 *   bytes past the state byte, and clears bits 0x10 and 0x1000 of D_80146258;
 *   those two arms return 1. When neither window
 *   matched, the request returns 0 as soon as the shared flag tester
 *   func_8015B5D4 reports that same flag 0x7E as set. Otherwise it takes the
 *   two mode-gated corner points: the pair 0x208000/0x900000 with the game-mode
 *   byte reading 7 starts progress 0x32 and returns 1, while the pair
 *   0x218000/0x900000 with that byte reading 3 stores progress 0x3C without
 *   running the startup helper and falls out returning 0. Both packed
 *   arguments are compared whole against the edge constants and shifted
 *   arithmetically for the range tests, so they are s32. The 0x20-byte frame
 *   keeps $ra, $s0 and $s1, and each byte store materialises its own 0x8014
 *   page in $at except the 0x25 store of the 0x14 arm, which goes through the
 *   state-byte pointer the body then offsets to the flag bank.
 *
 * Name evidence: D_801448EB is the shared state selector and D_801448EC the
 * progress byte of the same tuple, D_80143F03 the front-side byte the sibling
 * overlay requests return on, and func_8015C088 the shared front-end startup
 * helper of that family, so the body is one of the family's
 * startStateEntry<entry>Progress<step> requests. It stores five progress
 * steps, 0, 0xA, 0x14, 0x32 and 0x3C, and 0x25 is the entry every arm
 * publishes.
 * The two arguments carry no evidenced role beyond the packed edge constants
 * and high-half ranges, so the name keeps them structural. The payload holds
 * no reference to this span at all: no `jal`, no table word and no `lui`/
 * `addiu` pair materialises 0x801F53A4, and it sits after the step handler at
 * 0x801F4B58 whose jump table at 0x801F2C04 indexes 0x3D progress steps ending
 * at 0x3C, the last step this body stores.
 *
 * Shape evidence: seeded from this target's original bytes. The D_80143F03
 * gate, the `bne` edge tests and the `(u32)((arg >> 16) - base) < 3u` range
 * rejects follow the sibling overlay request
 * startStateEntry23Progress0WhenPackedCommandAndModeByteWorld04Area17213_801F4CC4
 * (0x801F4CC4) of the neighbouring area, and the multi-arm progress selection
 * follows startStateEntry0BProgress0AOr0WhenMapRangeAndScenarioGateWorld01Area04613_801F2CE4
 * (0x801F2CE4). No byte-identical twin exists: the 0x2F4-byte span and its
 * discriminating immediates 0x708000 and 0x738000 occur in no other world04
 * payload, so the clone lever is ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startStateEntry25Progress0AOr14Or32Or3COr0WhenFrontSidePackedArgRangesWorld04Area17013_801F53A4(
    s32 arg0, s32 arg1) {
  u8* state;

  if (D_80143F03 == 4) {
    if (((arg1 == 0x708000 || arg1 == 0x738000) &&
         (u32)((arg0 >> 16) - 3) < 3u) ||
        (arg0 == 0x28000 && (u32)((arg1 >> 16) - 0x71) < 3u) ||
        (arg0 == 0x58000 && (u32)((arg1 >> 16) - 0x71) < 3u)) {
      func_8015C088();
      D_801448EB = 0x25;
      D_801448EC = 0;
      return 1;
    }
  }
  if (D_80143F03 == 5) {
    if (((arg1 == 0x8E8000 || arg1 == 0x918000) &&
         (u32)((arg0 >> 16) - 9) < 3u) ||
        (arg0 == 0x88000 && (u32)((arg1 >> 16) - 0x8F) < 3u) ||
        (arg0 == 0xB8000 && (u32)((arg1 >> 16) - 0x8F) < 3u)) {
      func_8015C088();
      D_801448EB = 0x25;
      D_801448EC = 0x0A;
      return 1;
    }
    if (((arg1 == 0x8E8000 || arg1 == 0x918000) &&
         (u32)((arg0 >> 16) - 0x10) < 3u) ||
        (arg0 == 0xF8000 && (u32)((arg1 >> 16) - 0x8F) < 3u) ||
        (arg0 == 0x128000 && (u32)((arg1 >> 16) - 0x8F) < 3u)) {
      func_8015C088();
      state = &D_801448EB;
      *state = 0x25;
      D_801448EC = 0x14;
      func_8015B580(state + 0x63D, 0x7E);
      D_80146258 &= 0xEFEF;
      return 1;
    }
    if (func_8015B5D4((u32)&D_80144F28, 0x7E) != 0) {
      return 0;
    }
    if (arg0 == 0x208000 && arg1 == 0x900000 && D_80145E98 == 7) {
      func_8015C088();
      D_801448EB = 0x25;
      D_801448EC = 0x32;
      return 1;
    }
    if (arg0 == 0x218000 && arg1 == 0x900000 && D_80145E98 == 3) {
      D_801448EB = 0x25;
      D_801448EC = 0x3C;
    }
  }
  return 0;
}
