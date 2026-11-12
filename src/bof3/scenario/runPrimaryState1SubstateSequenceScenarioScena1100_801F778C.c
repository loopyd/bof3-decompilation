#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80143BB0;
extern u16 D_80143C40;
extern u8 D_80143F1E;
extern u8 D_80143F1F;
extern u8 D_80145E98;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;
extern u16 D_8014932A;
extern u8 D_80149332;
extern u8 D_80149333;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared front-end mode setter at 0x8014ECAC. */
void func_8014ECAC(u16 local_mode);
/* Shared front-end action starter at 0x80150224. */
void func_80150224(u32 arg0);
/* Shared cue dispatcher at 0x80154FD8. */
void func_80154FD8(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-selector context seeder at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared front-end record helper at 0x801C601C. */
void func_801C601C(s32 arg0);

/* @source 0x801F778C
 * @behavior Word 1 (the word stored at 0x801FAF00) of the ten-entry
 *           overlay-local second-level handler run at 0x801FAEFC that the exact
 *           dispatcher dispatchSecondaryStateScenarioScena1100_801F7748
 *           (0x801F7748) indexes with the shared primary state byte D_80146874,
 *           so a primary state reading 1 selects it; no jal or j of the payload
 *           names 0x801F778C, and that table word is its only reference. It
 *           switches on the shared secondary sub-state byte D_80146875 (read
 *           with lbu) through the sixty-one-word absolute jump table
 *           jtbl_801F6C64_main at 0x801F6C64 in this payload leading table
 *           data, bounded by `sltiu` against 0x3D. Sub-states 16 through 49,
 *           52 through 59 and every value from 0x3D up are the table words that
 *           point at the body exit, so those values return touching no byte.
 *           Sub-state 0 stores 1 in the sub-state byte, sets bit 3 of the
 *           shared mode halfword D_80146258 (0x80146258, read with lhu) and
 *           then branches on the shared byte D_80145E98 (0x80145E98, read with
 *           lbu): 6 runs the GAME.EMI helper func_801BE1B0 with 3, 7 runs it
 *           with 1, and 0 runs it with 4, while every other value returns.
 *           Sub-state 1, once g_ScenarioProgress (0x80146864) reads 1, runs the
 *           front-end mode setter func_8014ECAC with 0xD and advances the
 *           sub-state byte to 2. Sub-state 2, while the shared gate halfword
 *           D_80143C40 (0x80143C40) is clear, clears the effect bank byte
 *           D_8014832E and advances to 3. Sub-state 3, once the progress byte
 *           reads 2 through its address taken into a local pointer, clears the
 *           progress byte, stores 2 in the shared byte 0x80146866, advances to
 *           4, seeds front selection context (0x83, 0x50000, 0x5D0000, 0x83)
 *           through func_8019FA28 and stores 0xFF in 0x80143F1E. Sub-state 4,
 *           once the progress byte reads 1 through that same address-taken
 *           pointer, clears it, stores 2 in D_80149333, clears 0x80146866 and
 *           the halfword D_8014932A, advances to 5, seeds context (0x9C,
 *           0x190000, 0x190000, 0x80) and stores 0x92 in 0x80143F1F. Sub-states
 *           5, 6 and 7 only advance the sub-state byte to 6, 7 and 8.
 *           Sub-state 8, once the progress byte reads 1, clears it, advances to
 *           9, applies the flag-word helper func_8015B580 to D_8014686C with id
 *           1 and queues context (0x83, 0x50000, 0x5D0000, 0x84). Sub-state 9,
 *           once the progress byte reads 2, clears it, stores 1 in 0x80146866,
 *           advances to 0xA and queues context (0x83, 0x200000, 0x2F0000,
 *           0x85). Sub-state 10 runs func_801C601C with 0, dispatches cue 0x498
 *           through func_80154FD8, stores 2 in D_80149333 and advances to 0xB.
 *           Sub-state 11, while the shared byte D_80149332 (0x80149332) is
 *           zero, stores 1 in the progress byte and 0xC in the sub-state byte,
 *           and either way falls into the sub-state 12 body, which once the
 *           progress byte reads 2 runs func_8014ECAC with 0xD and advances to
 *           0xD. Sub-state 13, while D_80143C40 is clear, clears D_8014832E and
 *           advances to 0xE. Sub-state 14, once the progress byte reads 3,
 *           stores 3 in 0x80146866, clears the progress byte, advances to 0xF,
 *           applies func_8015B580 to D_8014686C with id 2 and queues context
 *           (0x83, 0x70000, 0x5C0000, 0x86). Sub-state 15, once the progress
 *           byte reads 0x14 through an address-taken pointer that stays in $s0
 *           across three calls, applies func_8015B580 to D_8014686C with ids 0
 *           and 4, runs the overlay release helper func_8015C058, toggles bit 3
 *           of D_80146258 and ends in the clearing tail that clears the progress
 *           byte, 0x80146865, 0x80146866, 0x80146867, D_80146875 and D_80146874
 *           in that order. Sub-state 0x32 queues front-end action 6 through
 *           func_80150224, stores 2 in the shared front-end mode byte
 *           D_80143BB0 and advances the sub-state byte to 0x33; sub-state 0x3C
 *           does the same with action 7, so the two arms share one call site.
 *           Sub-state 0x33, while D_80143BB0 is not 2, runs func_8015C058 and
 *           clears the sub-state byte and the primary state byte. Takes no
 *           arguments and returns nothing; its 0x18-byte frame keeps $ra and
 *           $s0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState1SubstateSequenceScenarioScena1100_801F778C(void) {
  switch (D_80146875) {
  case 0:
    D_80146258 |= 8u;
    D_80146875 = 1u;
    switch (D_80145E98) {
    case 6:
      func_801BE1B0(3u);
      break;
    case 7:
      func_801BE1B0(1u);
      break;
    case 0:
      func_801BE1B0(4u);
      break;
    }
    break;
  case 1:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_8014ECAC(0xDu);
    D_80146875 = 2u;
    break;
  case 2:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    D_80146875 = 3u;
    break;
  case 3: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 2u) {
      break;
    }
    *progress = 0u;
    D_80146866 = 2u;
    D_80146875 = 4u;
    func_8019FA28(0x83u, 0x50000u, 0x5D0000u, 0x83u);
    D_80143F1E = 0xFFu;
    break;
  }
  case 4: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 1u) {
      break;
    }
    *progress = 0u;
    D_80149333 = 2u;
    D_80146866 = 0u;
    D_8014932A = 0u;
    D_80146875 = 5u;
    func_8019FA28(0x9Cu, 0x190000u, 0x190000u, 0x80u);
    D_80143F1F = 0x92u;
    break;
  }
  case 5:
    D_80146875 = 6u;
    break;
  case 6:
    D_80146875 = 7u;
    break;
  case 7:
    D_80146875 = 8u;
    break;
  case 8: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 1u) {
      break;
    }
    *progress = 0u;
    D_80146875 = 9u;
    func_8015B580(D_8014686C, 1);
    func_8019FA28(0x83u, 0x50000u, 0x5D0000u, 0x84u);
    break;
  }
  case 9: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 2u) {
      break;
    }
    *progress = 0u;
    D_80146866 = 1u;
    D_80146875 = 0xAu;
    func_8019FA28(0x83u, 0x200000u, 0x2F0000u, 0x85u);
    break;
  }
  case 10:
    func_801C601C(0);
    func_80154FD8(0x498u);
    D_80149333 = 2u;
    D_80146875 = 0xBu;
    break;
  case 11:
    if (D_80149332 == 0u) {
      g_ScenarioProgress = 1u;
      D_80146875 = 0xCu;
    }
    /* Falls into the sub-state 12 body. */
  case 12:
    if (g_ScenarioProgress != 2u) {
      break;
    }
    func_8014ECAC(0xDu);
    D_80146875 = 0xDu;
    break;
  case 13:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    D_80146875 = 0xEu;
    break;
  case 14: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 3u) {
      break;
    }
    D_80146866 = 3u;
    *progress = 0u;
    D_80146875 = 0xFu;
    func_8015B580(D_8014686C, 2);
    func_8019FA28(0x83u, 0x70000u, 0x5C0000u, 0x86u);
    break;
  }
  case 15: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x14u) {
      break;
    }
    func_8015B580(D_8014686C, 0);
    func_8015B580(D_8014686C, 4);
    func_8015C058();
    D_80146258 ^= 8u;
    *progress = 0u;
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
  case 0x32:
    func_80150224(6u);
    D_80143BB0 = 2u;
    D_80146875 = 0x33u;
    break;
  case 0x33:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 0x3C:
    func_80150224(7u);
    D_80143BB0 = 2u;
    D_80146875 = 0x33u;
    break;
  }
}
