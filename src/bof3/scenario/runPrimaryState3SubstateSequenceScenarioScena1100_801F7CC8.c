#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80143BB0;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

/* Shared command-code mapper at 0x8016628C: table, code, mode. */
extern s32 func_8016628C(s32 table, s32 code, s32 mode);
/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end action starter at 0x80150224. */
void func_80150224(u32 arg0);

/* @source 0x801F7CC8
 * @behavior Word 3 (the word stored at 0x801FAF08) of the ten-entry
 * overlay-local second-level handler run at 0x801FAEFC that the exact
 * dispatcher dispatchSecondaryStateScenarioScena1100_801F7748 (0x801F7748)
 * indexes with the shared primary state byte D_80146874, so a primary state
 * reading 3 selects it; no jal or j of the payload names 0x801F7CC8, and that
 * table word is its only reference. It switches on the shared secondary
 * sub-state byte D_80146875 (read with lbu) through the seventeen-word absolute
 * jump table jtbl_801F6D5C_main at 0x801F6D5C in this payload leading table
 * data, bounded by `sltiu` against 0x11. Sub-states 2 through 9, 13 and 14 are
 * the table words that point at the body exit, so those values return without
 * touching any byte. Sub-state 0, while the shared front-end mode byte
 * D_80143BB0 (0x80143BB0, read with lbu) is not 2, stores 1 in the shared
 * scenario progress byte g_ScenarioProgress (0x80146864), runs the shared
 * GAME.EMI helper func_801BE1B0 (0x801BE1B0) with argument 4 and advances the
 * sub-state byte to 1. Sub-state 1, once the progress byte reads 6, sets flag
 * bit 6 of the shared flag word D_8014686C (0x8014686C, read with lw) through
 * the shared flag helper func_8015B580 (0x8015B580) with id 6, runs the shared
 * overlay release helper func_8015C058 (0x8015C058) and ends in the shared
 * clearing tail. Sub-state 10, while that mode byte is not 2, asks the shared
 * command-code mapper func_8016628C (0x8016628C) in table 0 and mode 0 which of
 * the four input codes 0x23, 0x24, 0x56 and 0x4D map to a non-zero low halfword:
 * the first answer that is zero stores 0xB in the sub-state byte, and four
 * non-zero answers store 0xF. Sub-state 11 queues front-end action 0x35 through
 * func_80150224 (0x80150224), stores 2 in the mode byte D_80143BB0 and advances
 * the sub-state byte to 0xC, which is the only arm that reads no mode byte.
 * Sub-state 12, while the mode byte is not 2 and the progress byte reads 6
 * through the address taken into a local pointer, runs the release helper,
 * clears the progress byte through that same pointer and ends in the clearing
 * tail. Sub-state 15 stores 1 in the progress byte, runs func_801BE1B0 with
 * argument 5 and advances the sub-state byte to 0x10. Sub-state 16, while the
 * mode byte is not 2 and the progress byte reads 5, runs the release helper,
 * sets flag bit 9 of D_8014686C through func_8015B580 and ends in the clearing
 * tail. That tail clears the shared bytes 0x80146865, 0x80146866, 0x80146867,
 * D_80146875 and D_80146874 in that order; sub-state 16 falls into it while
 * sub-states 1 and 12 reach it with their own j. It takes no arguments and
 * returns nothing; its 0x18-byte frame keeps $ra and $s0 because the constant 1
 * of sub-state 0 stays live in $s0 across the func_801BE1B0 call and feeds both
 * its progress store and its sub-state store, while sub-state 12 computes the
 * address of the progress byte into $s0 once for its compare and its clear. Its
 * 0x240 bytes have no copy in the tree: a span scan of the 396 extracted EMI
 * payloads under out/binaries finds this body only at its own offset, so the
 * clone lever was declined and every literal here was written from this span
 * own bytes. The nearest shape donors are the exact neighbours at 0x801FA3A4
 * and 0x801F7BFC of the same handler run, which supply the sub-state switch
 * over the absolute jump table, the mode-byte guarded sub-state 0 arm, the
 * address-taken progress pointer and the same release helper followed by the
 * same clearing tail, while this body differs in its sub-state set, its case
 * tests, its constants and its four-code mapper probe.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState3SubstateSequenceScenarioScena1100_801F7CC8(void) {
  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2u) {
      break;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(4u);
    D_80146875 = 1u;
    break;
  case 1:
    if (g_ScenarioProgress != 6u) {
      break;
    }
    func_8015B580(D_8014686C, 6);
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 10:
    if (D_80143BB0 == 2u) {
      break;
    }
    if ((u16)func_8016628C(0, 0x23, 0) == 0 ||
        (u16)func_8016628C(0, 0x24, 0) == 0 ||
        (u16)func_8016628C(0, 0x56, 0) == 0 ||
        (u16)func_8016628C(0, 0x4D, 0) == 0) {
      D_80146875 = 0xBu;
    } else {
      D_80146875 = 0xFu;
    }
    break;
  case 11:
    func_80150224(0x35u);
    D_80143BB0 = 2u;
    D_80146875 = 0xCu;
    break;
  case 12: {
    u8 *progress;

    if (D_80143BB0 == 2u) {
      break;
    }
    progress = &g_ScenarioProgress;
    if (*progress != 6u) {
      break;
    }
    func_8015C058();
    *progress = 0u;
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
  case 15:
    g_ScenarioProgress = 1u;
    func_801BE1B0(5u);
    D_80146875 = 0x10u;
    break;
  case 16:
    if (D_80143BB0 == 2u) {
      break;
    }
    if (g_ScenarioProgress != 5u) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 9);
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
}
