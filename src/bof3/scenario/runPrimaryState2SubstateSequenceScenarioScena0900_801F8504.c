#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80146258;
extern u16 D_8014625A;
extern u8 D_80146866;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_80149332;

/* Shared overlay reset helper at 0x801C601C: each arm that steps from one
 * window to the next runs it with the window selector the original encodes. */
void func_801C601C(u32 arg0);
/* Shared front-end helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared scenario flag-word bit-clear helper at 0x8015B5A8. */
void func_8015B5A8(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end scripted-motion starter at 0x8015D554. */
void func_8015D554(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
/* Shared two-argument controller-step helper at 0x801636A0. */
void func_801636A0(u32 arg0, u32 arg1);
/* Shared record-state query at 0x80163EA0, returning the live record byte
 * zero-extended into a word. */
u32 func_80163EA0(void);
/* Shared front-end request setter at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F8504
 * @behavior Handler for primary state 2 of this overlay: word 2 (the word
 * stored at 0x801FE3F4) of the seventeen-word in-image primary state table at
 * 0x801FE3EC that dispatchPrimaryStateHandlerScenarioScena0900_801F83D8
 * (0x801F83D8) indexes with the signed shared primary state byte D_80146874 and
 * invokes with no arguments, so that table word is this address's only image
 * word and no in-image jal targets it. It switches on the shared sub-state byte
 * D_80146875 through the twelve-entry absolute jump table at 0x801F6C64 in this
 * payload's leading tabledata blob, reading that byte before it creates its
 * 0x18-byte frame, which keeps only $ra. Sub-state 0, while the low three bits
 * of the shared mode halfword D_8014625A (0x8014625A, read with lhu) are clear,
 * sets bit 3 of the shared flag halfword D_80146258 (lhu, ori, sh), runs the
 * shared overlay reset helper func_801C601C with selector 1 and the shared
 * front-end helper func_801BE1B0 with 5, then advances the sub-state byte to 2.
 * Sub-state 2, on scenario progress byte 0x12, sets flag bit 2 of the shared
 * flag word D_8014686C (0x8014686C, read with lw) through func_8015B580, clears
 * the progress byte and the shared byte D_80146866, advances the sub-state byte
 * to 3 and requests front selector context seed 0x31 with the two 16.16 context
 * words 0x540000 and 0x640000 and kind 0x86 through func_8019FA28. Sub-state 3,
 * while the shared byte D_80149332 (0x80149332) reads 0, stores 1 in the
 * progress byte and advances the sub-state byte to 5. Sub-state 5, on progress
 * byte 0x11, runs the shared front-end scripted-motion starter
 * func_8015D554(0, 0, 4, 0x1E) and the shared controller-step helper
 * func_801636A0(3, 1), then advances to 6. Sub-state 6, unless the record-state
 * query func_80163EA0 returns a zero low byte, stores 0x12 in the progress byte,
 * runs the shared front-end cue helper func_80161C20(0x10, 0x64, 8) and
 * advances to 7. Sub-state 7, on progress byte 0x12, toggles bit 3 of
 * D_80146258 (lhu, xori, sh), sets flag bit 3 of D_8014686C through
 * func_8015B580 and clears bit 0x25 of that word through the shared bit-clear
 * helper func_8015B5A8, then runs the shared overlay release helper
 * func_8015C058 and clears the sub-state byte and the primary state byte
 * D_80146874, which ends the state. Sub-state 10 advances the sub-state byte to
 * 0xB, sets flag bit 4 of D_8014686C through func_8015B580 and requests front
 * selector context seed 0x31 with the context words 0x3F8000 and 0x140000 and
 * kind 0x87 through func_8019FA28. Sub-state 11, on progress byte 0xB, sets flag
 * bit 4 of D_8014686C through func_8015B580 and then takes the same release-and-
 * clear tail. Sub-states 1, 4, 8 and 9 and every value above 11 return without
 * touching any state. It takes no arguments, reads no other state and writes no
 * other byte. Its twelve-word switch table is the compiler's own jump table for
 * this switch, placed at 0x801F6C64 by this target's manifest. Sub-state 2
 * reaches the progress byte through one local pointer because the original
 * keeps that address in $a2 across both the guard and the clear; every other
 * store re-forms its own base with a fresh lui, so those cells stay one extern
 * each. Three spellings of sub-state 2 were measured. The direct-symbol spelling
 * compiled one word long with a nop in the guard's delay slot instead of the
 * sub-state constant. The pointer spelling that stores the sub-state byte before
 * the two clears compiled to 153 of 155 words with the state store ahead of the
 * progress clear, first difference at 0x801F85AC. Clearing the progress byte and
 * D_80146866 first and storing the sub-state byte last puts the constant 3 in the
 * guard delay slot and reproduces all 155 words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState2SubstateSequenceScenarioScena0900_801F8504(void) {
  switch (D_80146875) {
  case 0:
    if ((D_8014625A & 7) != 0) {
      break;
    }
    D_80146258 |= 8;
    func_801C601C(1);
    func_801BE1B0(5);
    D_80146875 = 2u;
    break;
  case 1:
    break;
  case 2: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 0x12u) {
      break;
    }
    *progress = 0u;
    D_80146866 = 0u;
    D_80146875 = 3u;
    func_8015B580(D_8014686C, 2);
    func_8019FA28(0x31, 0x540000, 0x640000, 0x86);
    break;
  }
  case 3:
    if (D_80149332 != 0u) {
      break;
    }
    g_ScenarioProgress = 1u;
    D_80146875 = 5u;
    break;
  case 4:
    break;
  case 5:
    if (g_ScenarioProgress != 0x11u) {
      break;
    }
    func_8015D554(0, 0, 4, 0x1E);
    func_801636A0(3, 1);
    D_80146875 = 6u;
    break;
  case 6:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    g_ScenarioProgress = 0x12u;
    func_80161C20(0x10, 0x64, 8);
    D_80146875 = 7u;
    break;
  case 7:
    if (g_ScenarioProgress != 0x12u) {
      break;
    }
    D_80146258 ^= 8;
    func_8015B580(D_8014686C, 3);
    func_8015B5A8(D_8014686C, 0x25);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 8:
    break;
  case 9:
    break;
  case 10:
    D_80146875 = 0xBu;
    func_8015B580(D_8014686C, 4);
    func_8019FA28(0x31, 0x3F8000, 0x140000, 0x87);
    break;
  case 11:
    if (g_ScenarioProgress != 0xBu) {
      break;
    }
    func_8015B580(D_8014686C, 4);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
}
