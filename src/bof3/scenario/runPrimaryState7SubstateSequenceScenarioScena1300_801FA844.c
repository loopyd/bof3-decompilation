#include "bof3/bof3.h"

extern u16 D_80143C40;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;
extern u8 D_8014832E;
extern s16 D_801492D8;
extern s16 D_801492DA;
extern s16 D_801492DC;
extern s16 D_8014932C;
extern u32 D_80149308;
extern u32 D_8014930C;
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_80143FD4[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
extern u8 D_801FBFEC;
extern u8 g_ScenarioProgress;

void armPaletteFadeOverSharedCountdownThenDrawWindowAndStepChannelsScenarioScena1300_801FAFA0(
    u32 id, u32 frames);

/* Shared front-end record allocator at 0x8014ECAC. */
extern void func_8014ECAC(u16 arg0);
/* Shared front-end record release helper at 0x8015D404. */
extern void func_8015D404(u32 arg0, s32 arg1);
/* Shared front-end cue helper at 0x80161C20. */
extern void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
/* Shared front-end cue helper at 0x8015D4F8. */
extern void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
extern void func_8015B580(u32 flags, s32 bit);
/* Shared overlay release helper at 0x8015C058. */
extern void func_8015C058(void);
/* Shared main-executable helper at 0x80153DE8; role not established. */
extern void func_80153DE8(void);
/* Shared main-executable helper at 0x801BFE34; role not established. */
extern void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
/* Shared main-executable helper at 0x801C7A54; role not established. */
extern void func_801C7A54(s32 arg0);
/* Shared GAME.EMI entry 0 helper at 0x801BE1B0. */
extern void func_801BE1B0(u32 arg0);
/* Shared free work-record selector at 0x8019601C; returns the chosen id or
 * 0xFF when none is free. */
extern s32 func_8019601C(void);

/* @source 0x801FA844
 * @behavior Entry 7 of the nine-entry primary-state handler table at
 * 0x801FBF54 that the exact dispatcher
 * dispatchPrimaryStateHandlerScenarioScena1300_801F8474 (0x801F8474) selects
 * with the shared primary state byte D_80146874 and calls with no arguments, so
 * no in-image jal targets 0x801FA844 and the word at 0x801FBF70 is its only
 * reference: the sub-state switch of the overlay's primary state 7 handler. It
 * reads the shared sub-state byte D_80146875 with lbu, bounds it against 0x1E
 * with sltiu and dispatches through the thirty-word absolute table at
 * 0x801F6DFC in this payload's leading table data, whose entries for sub-states
 * 0x0B..0x13 and 0x1B point at the body exit, so those ten values are the
 * default and the body has cases 0..0x0A, 0x14..0x1A, 0x1C and 0x1D.
 *
 * Sub-state 0, once the shared progress byte g_ScenarioProgress (0x80146864)
 * reads 0x0B, releases the front-end record through func_8014ECAC(0) and
 * func_8015D404(0, 0) and advances the sub-state byte to 1. Sub-state 1, once
 * the shared gate halfword D_80143C40 reads zero, clears the shared front-end
 * phase byte D_8014832E, queues the cue func_80161C20(0x6F, 0x64, 0x10), arms
 * the shared countdown halfword D_80146876 at 0x96 and advances to 2.
 * Sub-states 2, 4, 6 and 9 run the overlay palette ramp at 0x801FAFA0 with
 * (6, 5), (7, 5), (8, 0x0A) and (9, 5); sub-states 5 and 8 do not ramp. Each of
 * 2, 4, 5, 6, 8 and 9 then decrements the shared countdown and returns while it
 * stays nonzero, and on the countdown reaching zero runs its own tail: 2 sets
 * the sub-state byte to 3 and advances the progress byte, 4 re-arms the
 * countdown at 0x3C and advances the sub-state byte to 5, 5 re-arms the
 * countdown at 0x12C and advances to 6, 6 advances to 7, 8 re-arms the
 * countdown at 0x96, advances to 9 and releases a front-end record through
 * func_8014ECAC(6). Sub-state 9's tail stores -0x2AA in the shared halfword
 * D_801492D8 and 0x200 in D_801492DC, advances the progress byte, releases a
 * front-end record through func_8014ECAC(1), sets D_8014832E to 0x1F and
 * advances to 0x0A, and both of its arms then clear D_8014832E again once
 * D_80143C40 reads zero. Sub-state 3, once D_80143C40 reads zero, releases a
 * front-end record through func_8014ECAC(7), sets D_8014832E to 0x1F, seeds the
 * shared halfwords D_801492D8 with -0x386, D_801492DC with 0x3B8 and D_8014932C
 * with -0xDFC, and then, while the free-record selector func_8019601C returns
 * an id other than 0xFF, fills the 0x74-byte shared record that id indexes: its
 * flag byte at +0, the byte at +5 with 0x31, the word at +0x64 with -0x2EC, the
 * word at +0x68 with the D_801492DA halfword, the byte at +9 with 0x3C, the
 * word at +0xC with zero and the word at +0x6C with the D_801492DC halfword.
 * Its selector result is also published to the overlay-local byte D_801FBFEC.
 * Sub-states 0x0A and 0x1D, while the progress byte reads zero, clear the
 * primary state byte D_80146874 and the sub-state byte after setting bit 0x17
 * and bit 0x1A of the shared flag word D_8014686C through func_8015B580,
 * sub-state 0x1D after running the release helper func_8015C058. Sub-state 0x14,
 * once the progress byte reads 9, queues func_8015D4F8(0, 0, 0x64, 0x10), arms
 * the countdown at 0x1E and advances to 0x15. Sub-state 0x15 decrements the
 * countdown and adds 0x50 to D_8014932C before its zero test, then releases a
 * front-end record through func_8015D404(0, 0) and, while the selector
 * func_8019601C returns an id other than 0xFF, sets that record's flag byte at
 * +0 and its +5 byte to 0x87, and re-arms the countdown at 0x1E before advancing
 * to 0x16. Sub-state 0x16, once the countdown reads zero, sets the sub-state
 * byte to 0x17 and advances the progress byte. Sub-state 0x17, once the progress
 * byte reads 0x0B, arms the countdown at 0x1E and advances to 0x18. Sub-state
 * 0x18 decrements the countdown and subtracts 0x50 from D_8014932C before its
 * zero test, then stores 0x32 in the progress byte and advances to 0x19.
 * Sub-state 0x19, once the progress byte reads 0x34, stores 0x240000 in the
 * shared word D_80149308 and 0x1C8000 in D_8014930C, runs func_80153DE8 and
 * func_801BFE34(0x288000, 0x1D8000, 0x25), advances the sub-state byte to 0x1A
 * and advances the progress byte. Sub-state 0x1A, once the progress byte reads
 * 0x37, runs func_801C7A54(0x25) and advances to 0x1B. Sub-state 0x1C advances
 * the progress byte, runs func_801BE1B0(7), publishes the selector
 * func_8019601C to D_801FBFEC and, while that result is not 0xFF, sets that
 * record's flag byte at +0 and its +5 byte to 0x86, then advances the sub-state
 * byte to 0x1D.
 *
 * The body reads no other state, writes only those cells and takes no
 * arguments; its 0x18-byte frame keeps $ra and $s0, the sub-state read is
 * hoisted above that frame's allocation, and the cases that both load and store
 * one shared halfword or the progress byte reach it through a local pointer
 * while the write-only ones materialise the page in $at.
 *
 * Record stores: the original forms the 0x74-byte stride once per re-read of
 * D_801FBFEC and reaches the +0x64, +0x68 and +0x6C words with that same
 * byte offset, so the shared record bases are byte-typed here and the word
 * stores cast. Indexing them as u32 arrays instead measured 1884 -> 1920 bytes
 * because the word stores then scale the stride by four a second time, and
 * writing D_801FBFEC at each of the seven stores measured 1884 -> 2012 with a
 * reload per store; the single `slot` re-read below is what reproduces the
 * original's three reads and three stride expansions at the original length.
 * @status partial
 * @match 99.15
 * @residual Four of 471 instructions differ, all in sub-states 0x15 and 0x18:
 * those two cases emit the shared serial store 0x8014932C before the countdown
 * decrement's store and take $a0 for the serial value, where the original
 * stores the countdown first and takes $v0 for the serial. The store order and
 * the serial register move together, so every shape that fixes one breaks the
 * other: a countdown-first statement order measured 457 of 471 with both
 * registers exchanged, and the form that reaches this order measured 465. Every
 * literal, cell, callee binding and the thirty-word jump table measure equal.
 */
void runPrimaryState7SubstateSequenceScenarioScena1300_801FA844(void) {
  switch (D_80146875) {
  case 0:
    if (g_ScenarioProgress != 0xB) {
      break;
    }
    func_8014ECAC(0);
    func_8015D404(0, 0);
    D_80146875 = 1;
    break;
  case 1:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    func_80161C20(0x6F, 0x64, 0x10);
    D_80146876 = 0x96;
    D_80146875 = 2;
    break;
  case 2: {
    u16 *countdown;
    u8 *progress;

    armPaletteFadeOverSharedCountdownThenDrawWindowAndStepChannelsScenarioScena1300_801FAFA0(
        6, 5);
    countdown = &D_80146876;
    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    progress = &g_ScenarioProgress;
    D_80146875 = 3;
    *progress = *progress + 1;
    break;
  }
  case 3: {
    if (D_80143C40 != 0) {
      break;
    }
    func_8014ECAC(7);
    D_8014832E = 0x1F;
    D_801492D8 = -0x386;
    D_801492DC = 0x3B8;
    D_8014932C = -0xDFC;
    D_801FBFEC = func_8019601C();
    if (D_801FBFEC != 0xFF) {
      u8 slot;

      D_80143FC8[D_801FBFEC * 116] = 1;
      D_80143FCD[D_801FBFEC * 116] = 0x31;
      slot = D_801FBFEC;
      *(s32 *)(D_8014402C + slot * 116) = -0x2EC;
      *(s32 *)(D_80144030 + slot * 116) = D_801492DA;
      *(s32 *)(D_80144034 + slot * 116) = D_801492DC;
      *(s32 *)(D_80143FD4 + slot * 116) = 0;
      D_80143FD1[slot * 116] = 0x3C;
    }
    D_80146876 = 0x96;
    D_80146875 = 4;
    break;
  }
  case 4: {
    u16 *countdown;
    u8 *progress;

    armPaletteFadeOverSharedCountdownThenDrawWindowAndStepChannelsScenarioScena1300_801FAFA0(
        7, 5);
    countdown = &D_80146876;
    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    *countdown = 0x3C;
    progress = &g_ScenarioProgress;
    D_80146875 = 5;
    *progress = *progress + 1;
    break;
  }
  case 5: {
    u16 *countdown = &D_80146876;

    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    *countdown = 0x12C;
    D_80146875 = 6;
    break;
  }
  case 6: {
    u16 *countdown;

    armPaletteFadeOverSharedCountdownThenDrawWindowAndStepChannelsScenarioScena1300_801FAFA0(
        8, 0xA);
    countdown = &D_80146876;
    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    D_80146875 = 7;
    break;
  }
  case 7:
    if (g_ScenarioProgress != 0xE) {
      break;
    }
    D_80146876 = 0x1E;
    D_80146875 = 8;
    break;
  case 8: {
    u16 *countdown = &D_80146876;

    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    *countdown = 0x96;
    D_80146875 = 9;
    func_8014ECAC(6);
    break;
  }
  case 9: {
    u16 *countdown;

    armPaletteFadeOverSharedCountdownThenDrawWindowAndStepChannelsScenarioScena1300_801FAFA0(
        9, 5);
    countdown = &D_80146876;
    if (*countdown != 0) {
      *countdown = *countdown - 1;
    } else {
      u8 *progress = &g_ScenarioProgress;

      D_801492D8 = -0x2AA;
      D_801492DC = 0x200;
      *progress = *progress + 1;
      func_8014ECAC(1);
      D_8014832E = 0x1F;
      D_80146875 = 0xA;
    }
    if (D_80143C40 == 0) {
      D_8014832E = 0;
    }
    break;
  }
  case 10:
    if (g_ScenarioProgress != 0) {
      break;
    }
    func_8015B580(D_8014686C, 0x17);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    break;
  case 20:
    if (g_ScenarioProgress != 9) {
      break;
    }
    func_8015D4F8(0, 0, 0x64, 0x10);
    D_80146876 = 0x1E;
    D_80146875 = 0x15;
    break;
  case 21: {
    u16 *countdown = &D_80146876;
    u8 index;

    D_8014932C = (u16)(D_8014932C + 0x50);
    if (--*countdown != 0) {
      break;
    }
    func_8015D404(0, 0);
    index = func_8019601C();
    if (index != 0xFF) {
      D_80143FC8[index * 116] = 1;
      D_80143FCD[index * 116] = 0x87;
    }
    *countdown = 0x1E;
    D_80146875 = 0x16;
    break;
  }
  case 22: {
    u8 *progress;

    if (D_80146876 != 0) {
      break;
    }
    progress = &g_ScenarioProgress;
    D_80146875 = 0x17;
    *progress = *progress + 1;
    break;
  }
  case 23:
    if (g_ScenarioProgress != 0xB) {
      break;
    }
    D_80146876 = 0x1E;
    D_80146875 = 0x18;
    break;
  case 24: {
    u16 *countdown = &D_80146876;

    D_8014932C = (u16)(D_8014932C - 0x50);
    if (--*countdown != 0) {
      break;
    }
    g_ScenarioProgress = 0x32;
    D_80146875 = 0x19;
    break;
  }
  case 25: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x34) {
      break;
    }
    D_80149308 = 0x240000;
    D_8014930C = 0x1C8000;
    func_80153DE8();
    func_801BFE34(0x288000, 0x1D8000, 0x25);
    D_80146875 = 0x1A;
    *progress = *progress + 1;
    break;
  }
  case 26:
    if (g_ScenarioProgress != 0x37) {
      break;
    }
    func_801C7A54(0x25);
    D_80146875 = 0x1B;
    break;
  case 27:
    break;
  case 28: {
    u8 *progress = &g_ScenarioProgress;

    *progress = *progress + 1;
    func_801BE1B0(7);
    D_801FBFEC = func_8019601C();
    if (D_801FBFEC != 0xFF) {
      D_80143FC8[D_801FBFEC * 116] = 1;
      D_80143FCD[D_801FBFEC * 116] = 0x86;
    }
    D_80146875 = 0x1D;
    break;
  }
  case 29:
    if (g_ScenarioProgress != 0) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 0x1A);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  }
}
