#include "bof3/bof3.h"

/* Shared front-end mode byte of the shared EXE image: every claim arm of this
 * payload treats 2 as busy and refuses to act, and the arms that start a
 * front-end sequence leave 2 behind for the next frame to wait on. */
extern u8 D_80143BB0;

/* Shared gate halfword of the shared EXE image; the two arms that wait for it
 * act only while it reads zero. */
extern u16 D_80143C40;

/* Shared flag/prompt byte of the shared EXE image. */
extern u8 D_80143F1F;

/* Shared flag word of the shared EXE image, handed to the flag helper
 * func_8015B580 with the bit ids this body stores. */
extern u32 D_8014686C;

/* Shared scenario progress byte and the shared state bytes that follow it:
 * the progress byte is the completion counter this body waits on, the sub-state
 * byte below is its own switch word and the primary state byte selects the
 * handler run that reaches it. */
extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

/* Shared effect-bank byte of the shared EXE image, armed with 0x1F by the arms
 * that start a front-end sequence and cleared by the arms that end one. */
extern u8 D_8014832E;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared front-end mode setter at 0x8014ECAC. */
void func_8014ECAC(u16 local_mode);
/* Shared front-end action starter at 0x80150224. */
void func_80150224(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end startup helper at 0x8015C088. */
void func_8015C088(void);
/* Shared front-end timing starter at 0x8015D300. */
void func_8015D300(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
/* Shared front-end scripted-motion starter at 0x8015D554. */
void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Shared front-end cue helper at 0x80161C20. */
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
/* Shared front-end cue stopper at 0x80161D20. */
void func_80161D20(s32 arg0);
/* Shared front-end cue finisher at 0x80161D74. */
void func_80161D74(s32 arg0);
/* Shared front-end sequence helper at 0x801636A0. */
void func_801636A0(u32 arg0, u32 arg1);
/* Shared front-end readiness poll of the shared EXE image. */
u32 func_80163EA0(void);
/* Shared scenario flag setter at 0x8016813C. */
void func_8016813C(s32 arg0, u8 arg1);
/* Shared front-selector context seeder at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F8B84
 * @behavior Word 6 (the word stored at 0x801FAF14) of the ten-entry
 *           overlay-local second-level handler run at 0x801FAEFC that the exact
 *           dispatcher dispatchSecondaryStateScenarioScena1100_801F7748
 *           (0x801F7748) indexes with the shared primary state byte D_80146874,
 *           so a primary state reading 6 selects it; no jal or j of the payload
 *           names 0x801F8B84, and that table word is its only reference. It
 *           switches on the shared secondary sub-state byte D_80146875 (read
 *           with lbu) through the forty-nine-word absolute jump table
 *           jtbl_801F6F24_main at 0x801F6F24 in this payload leading table
 *           data, bounded by `sltiu` against 0x31. Sub-states 2, 3, 4, 8, 9, 15
 *           through 19, 21 through 29 and 36 through 39 are the table words that
 *           point at the body exit, so those values return touching no byte.
 *
 *           Sub-state 0, while the shared mode byte D_80143BB0 (0x80143BB0) is
 *           not 2, stores 1 in the progress byte g_ScenarioProgress
 *           (0x80146864), runs the GAME.EMI helper func_801BE1B0 with 1 and
 *           advances the sub-state byte to 1. Sub-state 1, once the progress
 *           byte reads 7, leaves unless the mode byte reads 2, and otherwise
 *           stores 8 in it and clears the effect-bank byte D_8014832E
 *           (0x8014832E). Sub-state 5, while that mode byte is zero, stores 6
 *           in the sub-state byte, clears the progress byte, sets flag bit 0xA
 *           of the shared flag word D_8014686C (0x8014686C) through
 *           func_8015B580 (0x8015B580) and seeds the front selection context
 *           (0x82, 0x290000, 0x540000, 0x82) through func_8019FA28 (0x8019FA28).
 *           Sub-state 6 arms the effect-bank byte with 0x1F and advances to 7.
 *
 *           Sub-states 7 and 14 are the same guard over two different waits:
 *           once the progress byte reads 3 they run the shared overlay release
 *           helper func_8015C058 (0x8015C058) and end in the clearing tail.
 *           Sub-state 10 runs the shared front-end startup helper
 *           func_8015C088 (0x8015C088), leaves if the mode byte reads 2, runs
 *           func_8014ECAC (0x8014ECAC) with 0xD, starts the scripted motion
 *           func_8015D554 (0x8015D554) with (0, 0, 4, 0x1E) and advances to 0xB.
 *           Sub-state 11, while the shared gate halfword D_80143C40
 *           (0x80143C40) is clear, stores 8 in the mode byte, clears the
 *           effect-bank byte and runs func_8015D300 (0x8015D300) with
 *           (0, 1, 0x64, 0x64). Sub-state 12, while the mode byte is zero,
 *           advances to 0xD, clears the progress byte and seeds the same front
 *           selection context. Sub-state 13 arms the effect-bank byte with 0x1F
 *           and advances to 0xE.
 *
 *           Sub-state 20, while the mode byte is zero, clears the progress byte,
 *           sets flag bit 0xA through func_8015B580 and then splits on the
 *           shared byte D_80146867 (0x80146867): when it reads 1 the arm stores
 *           0x1E in the sub-state byte, seeds the context (0x82, 0x290000,
 *           0x540000, 0x83) and stores 0xFF in the shared prompt byte D_80143F1F
 *           (0x80143F1F), while any other value stores 0x28 in the sub-state byte
 *           and seeds the context (0x82, 0x290000, 0x540000, 0x84). Sub-state 30
 *           arms the effect-bank byte with 0x1F and advances to 0x1F, one
 *           constant short of the sub-states 31 through 0x23 that follow.
 *
 *           Sub-state 31, once the progress byte reads 1, runs
 *           func_801636A0 (0x801636A0) with (7, 1) and advances to 0x20, and
 *           sub-state 32 advances to 0x21 as soon as the readiness poll
 *           func_80163EA0 (0x80163EA0) answers non-zero through its low byte.
 *           Sub-state 33, once the progress byte reads 4, runs func_801636A0
 *           with (6, 1) and advances to 0x22, and sub-state 34 stores 5 in the
 *           progress byte before advancing to 0x23 on the same non-zero poll.
 *           Sub-state 35, once the progress byte reads 6, runs the shared flag
 *           setter func_8016813C (0x8016813C) with (0xFFFF, 7) and ends in the
 *           clearing tail.
 *
 *           Sub-state 40 arms the effect-bank byte with 0x1F and advances to
 *           0x29. Sub-state 41, once the progress byte reads 5, runs
 *           func_8015D300 with (0, 1, 0x64, 0x64) and advances to 0x2A.
 *           Sub-state 42 takes the address of the progress byte once into $s0,
 *           and while it reads 0xA runs func_8015D554 with (0, 1, 4, 0x1E),
 *           clears it through that held pointer and advances to 0x2B, then runs
 *           func_8014ECAC with 0xD. Sub-state 43, while the gate halfword is
 *           clear, clears the effect-bank byte and advances to 0x2C. Sub-state
 *           44 queues front-end action 0x40 through func_80150224, stores 2 in
 *           the mode byte and advances to 0x2D. Sub-state 45, while the mode
 *           byte does not read 2, advances to 0x2E, stores 1 in the shared byte
 *           D_80146866 (0x80146866), sets flag bit 0xB through func_8015B580,
 *           seeds the context (0x82, 0x290000, 0x530000, 0x85) and runs
 *           func_80161C20 (0x80161C20) with (0x83, 0x64, 8). Sub-state 46, once
 *           the progress byte reads 4, stops cue 0x83 through func_80161D20
 *           (0x80161D20), runs func_801636A0 with (2, 1) and advances to 0x2F;
 *           sub-state 47 finishes that cue through func_80161D74 (0x80161D74)
 *           and advances to 0x30 on the non-zero readiness poll. Sub-state 48,
 *           once the progress byte reads 0xA, sets flag bit 0xC through
 *           func_8015B580 and ends in the same clearing tail.
 *
 *           That tail runs func_8015C058 and then clears the shared bytes
 *           0x80146865, 0x80146866, 0x80146867, the sub-state byte D_80146875 and
 *           the primary state byte D_80146874 in that order, leaving the
 *           progress byte alone. It takes no arguments and returns nothing; its
 *           0x18-byte frame keeps $ra and $s0. $s0 holds the constant 1 across
 *           the helper call of sub-state 0, which is the value both stores of
 *           that arm write, and the address of the progress byte in sub-state 42.
 *
 *           The boundary is proven by the 0x18-byte frame at 0x801F8B84 and the
 *           `jr $ra` at 0x801F9174, and the span ends exactly at the next
 *           declared start, 0x801F917C. The clone lever is declined: thirteen
 *           scenario overlays under out/binaries share this body's first sixteen
 *           bytes (the same switch word and the same 0x31 bound), but a
 *           full-span comparison of each against these 0x5F8 bytes differs in 341
 *           to 368 words, so none is byte-identical and each is a different
 *           scenario's state machine rather than a copy of this one. The arms
 *           were written from this span's own bytes, with the family shape -
 *           a sub-state switch over an absolute jump table, the mode-byte and
 *           gate-halfword guards, the front-selection context seed and the
 *           clearing tail - taken from the exact neighbours at 0x801F7CC8,
 *           0x801F7F08 and 0x801FA3A4 of the same handler run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState6SubstateSequenceScenarioScena1100_801F8B84(void) {
  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2u) {
      break;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(1u);
    D_80146875 = 1u;
    break;
  case 1: {
    u8 *mode;

    if (g_ScenarioProgress != 7u) {
      break;
    }
    mode = &D_80143BB0;
    if (*mode == 2u) {
      break;
    }
    *mode = 8u;
    D_8014832E = 0u;
    break;
  }
  case 5:
    if (D_80143BB0 != 0u) {
      break;
    }
    D_80146875 = 6u;
    g_ScenarioProgress = 0u;
    func_8015B580(D_8014686C, 0xA);
    func_8019FA28(0x82u, 0x290000u, 0x540000u, 0x82u);
    break;
  case 6:
    D_8014832E = 0x1Fu;
    D_80146875 = 7u;
    break;
  case 7:
    if (g_ScenarioProgress != 3u) {
      break;
    }
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 10:
    func_8015C088();
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8014ECAC(0xDu);
    func_8015D554(0, 0, 4, 0x1E);
    D_80146875 = 0xBu;
    break;
  case 11:
    if (D_80143C40 != 0u) {
      break;
    }
    D_80143BB0 = 8u;
    D_8014832E = 0u;
    func_8015D300(0, 1, 0x64, 0x64);
    break;
  case 12:
    if (D_80143BB0 != 0u) {
      break;
    }
    D_80146875 = 0xDu;
    g_ScenarioProgress = 0u;
    func_8019FA28(0x82u, 0x290000u, 0x540000u, 0x82u);
    break;
  case 13:
    D_8014832E = 0x1Fu;
    D_80146875 = 0xEu;
    break;
  case 14:
    if (g_ScenarioProgress != 3u) {
      break;
    }
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 20:
    if (D_80143BB0 != 0u) {
      break;
    }
    g_ScenarioProgress = 0u;
    func_8015B580(D_8014686C, 0xA);
    if (D_80146867 == 1u) {
      D_80146875 = 0x1Eu;
      func_8019FA28(0x82u, 0x290000u, 0x540000u, 0x83u);
      D_80143F1F = 0xFFu;
    } else {
      D_80146875 = 0x28u;
      func_8019FA28(0x82u, 0x290000u, 0x540000u, 0x84u);
    }
    break;
  case 30:
    D_8014832E = 0x1Fu;
    D_80146875 = 0x1Fu;
    break;
  case 31:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_801636A0(7u, 1u);
    D_80146875 = 0x20u;
    break;
  case 32:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    D_80146875 = 0x21u;
    break;
  case 33:
    if (g_ScenarioProgress != 4u) {
      break;
    }
    func_801636A0(6u, 1u);
    D_80146875 = 0x22u;
    break;
  case 34:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    g_ScenarioProgress = 5u;
    D_80146875 = 0x23u;
    break;
  case 35:
    if (g_ScenarioProgress != 6u) {
      break;
    }
    func_8016813C(0xFFFF, 7u);
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 40:
    D_8014832E = 0x1Fu;
    D_80146875 = 0x29u;
    break;
  case 41:
    if (g_ScenarioProgress != 5u) {
      break;
    }
    func_8015D300(0, 1, 0x64, 0x64);
    D_80146875 = 0x2Au;
    break;
  case 42: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0xAu) {
      break;
    }
    func_8015D554(0, 1, 4, 0x1E);
    *progress = 0u;
    D_80146875 = 0x2Bu;
    func_8014ECAC(0xDu);
    break;
  }
  case 43:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    D_80146875 = 0x2Cu;
    break;
  case 44:
    func_80150224(0x40u);
    D_80143BB0 = 2u;
    D_80146875 = 0x2Du;
    break;
  case 45:
    if (D_80143BB0 == 2u) {
      break;
    }
    D_80146875 = 0x2Eu;
    D_80146866 = 1u;
    func_8015B580(D_8014686C, 0xBu);
    func_8019FA28(0x82u, 0x290000u, 0x530000u, 0x85u);
    func_80161C20(0x83u, 0x64u, 8u);
    break;
  case 46:
    if (g_ScenarioProgress != 4u) {
      break;
    }
    func_80161D20(0x83u);
    func_801636A0(2u, 1u);
    D_80146875 = 0x2Fu;
    break;
  case 47:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    func_80161D74(0x83u);
    D_80146875 = 0x30u;
    break;
  case 48:
    if (g_ScenarioProgress != 0xAu) {
      break;
    }
    func_8015B580(D_8014686C, 0xCu);
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
}
