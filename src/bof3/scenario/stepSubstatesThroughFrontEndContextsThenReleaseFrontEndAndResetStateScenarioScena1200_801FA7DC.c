#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: the waiting arms of
 * this walker compare it against one value each and clear it on the way to the
 * next sub-state.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared mode halfword of the main executable: one arm flips bit 0x80, another
 * sets bit 0x08 and the closing arm toggles 0x28, all with lhu/xori/ori/sh.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared effect-bank byte of the main executable: the request arms arm it with
 * 0xFF, 0x8F or 0x8B and no arm reads it.
 * @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;

/* Shared effect-bank byte of the main executable: sub-state 0x1D arms it with
 * 0x1F and the gate arm at sub-state 0x11 clears it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared gate halfword of the main executable: the arm at sub-state 0x11 waits
 * for it to read 0.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared main-RAM byte of the main executable: the closing arm clears it.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared main-RAM byte of the main executable: the request arms store the
 * next-window byte into it and the closing arm clears it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared main-RAM byte of the main executable: only the closing arm touches it,
 * clearing the record index an earlier handler published.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario flag word of the main executable: every request that arms a
 * scenario bit passes it to func_8015B580 as the flag word.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared state byte of the main executable: the overlay's second-level handler
 * run at 0x801FD1C4 is indexed with it, and the closing arm clears it to return
 * the overlay to its idle entry.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte of the main executable: every arm of this walker reads
 * it once through the jump table and most of them store the next value into it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared main-RAM byte of the main executable: it is the armed-state byte the
 * overlay's second-level substate arms poll, and sub-states 5, 6, 0x12, 0x13
 * and 0x14 drive it.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared main-RAM byte of the main executable: sub-state 0x14 masks it with
 * 0x7F and selects one flag request per masked value.
 * @source 0x80145024 @kind unknown */
extern u8 D_80145024;

/* Shared front-end request setter at 0x8019FA28: every request arm passes it a
 * 16-bit selection seed, the two 16.16 context words and the request kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared scenario flag helper at 0x8015B580: the arms that leave the walker
 * pass it the shared flag word and the bit to set.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared overlay reset helper at 0x801C601C of the main executable: the arm at
 * sub-state 0x0E runs it with the window selector the original encodes as 4.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared front-end release helper at 0x8015C058: sub-states 1, 6 and 0x1E run
 * it before they clear the window byte or the whole state.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end cue queue at 0x8015DF18: sub-state 0x0C queues cue 0x20E
 * through it.
 * @source 0x8015DF18 @kind function */
extern void func_8015DF18(u16 cue);

/* Shared front-end mode setter at 0x8014ECAC: the arm at sub-state 0x10 sets
 * mode 0xD through it.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared action trigger at 0x80150224: sub-states 5 and 0x12 run it with the
 * action ids 0xB and 0x17.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared front-end teardown helper at 0x8015C088: sub-state 0x0A runs it with
 * no arguments before it sets mode bit 0x08.
 * @source 0x8015C088 @kind function */
extern void func_8015C088(void);

/* Shared mode-and-effect selector at 0x801BE1B0: sub-state 0x0A passes it 0x10.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared two-argument cell setter at 0x8015D404: sub-state 0x14 runs it twice,
 * with (0, 0) and (0, 1).
 * @source 0x8015D404 @kind function */
extern void func_8015D404(u32 arg0, s32 arg1);

/* Shared effect-group arm helper at 0x801C16B8: sub-state 0x14 runs it with no
 * arguments before it selects one flag request from D_80145024.
 * @source 0x801C16B8 @kind function */
extern void func_801C16B8(void);

/* Shared sprite-cell publisher at 0x801A4BC0: sub-state 0x1E publishes ten cell
 * entries through it, each with a column, a row and a size byte.
 * @source 0x801A4BC0 @kind function */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801FA7DC
 * @behavior Overlay second-level state handler: it is the word stored at
 * 0x801FD1E0, entry 7 of the ten-word handler run at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 7;
 * that table word is the only image reference to this address, no jal or j in
 * the payload targets it, and the walker clears D_80146874 itself on its last
 * sub-state. It switches on the shared sub-state byte D_80146875 through the
 * thirty-one-entry absolute jump table at 0x801F6F0C, an overlay-local table of
 * the leading tabledata blob, so sub-states 0 through 0x1E are its table
 * entries: sub-states 2, 3, 4, 7, 8, 9 and 0x15 through 0x1C are holes that
 * point at the return path, and an explicit default returns untouched too.
 * Most arms wait for the shared progress byte g_ScenarioProgress (0x80146864)
 * to reach one value and clear it, then arm the shared effect-bank byte
 * D_80143F1F, store the next-window byte D_80146866, flip or set one bit of the
 * shared mode halfword D_80146258 and request a front-end context through the
 * shared setter func_8019FA28 (0x8019FA28) with the 16-bit seed 0xBC and the
 * request kinds 0x8F, 0x91, 0x92, 0x93 and 0x94. Sub-states 5 and 0x12 run the
 * shared action trigger func_80150224 (0x80150224) with the action ids 0xB and
 * 0x17 and arm the shared byte D_80143BB0 with 2; sub-states 6 and 0x13 poll
 * that byte and either return or release the front-end through func_8015C058
 * (0x8015C058). Sub-state 0x0A runs func_8015C088, sets mode bit 0x08 and runs
 * func_801BE1B0 with 0x10, sub-state 0x0C queues cue 0x20E through
 * func_8015DF18 and runs func_80161C20(0x8F, 0x64, 8), sub-state 0x0E runs the
 * shared overlay reset helper func_801C601C with selector 4, sub-state 0x10
 * waits for progress 5 and sets front-end mode 0xD through func_8014ECAC, and
 * sub-state 0x11 waits for the shared gate halfword D_80143C40 to read 0 before
 * it clears D_8014832E. Sub-state 0x14 clears D_80143BB0, flips bit 0x80 of
 * D_80146258, runs func_8015D404(0, 0) and func_8015D404(0, 1),
 * func_8015B580(D_8014686C, 0x2C) and func_801C16B8, and then selects one more
 * flag request per masked value of D_80145024 through the second, twelve-entry
 * absolute jump table at 0x801F6F8C: masked values 7, 8, 9, 0x0A and 0x0D
 * through 0x12 pass func_8015B580 the bits 0x32 through 0x3B in that order,
 * while masked values 0x0B and 0x0C, and every value outside 7 through 0x12,
 * fall straight to the shared tail that stores 9 in D_80146866, requests context
 * kind 0x94 and arms D_80143F1F with 0x8B. Sub-state 0x1D arms D_8014832E with
 * 0x1F and steps to 0x1E, and sub-state 0x1E waits for progress 1, publishes ten
 * func_801A4BC0 (0x801A4BC0) cell entries, toggles 0x28 of D_80146258, releases
 * the front-end through func_8015C058 and clears the progress, the two window
 * bytes and the record index before it falls out of the switch; that exit, and
 * the ones of sub-states 1 and 6, run the trailing stores that clear D_80146875
 * and D_80146874 and so return the overlay to the idle entry of its handler run.
 * Its 0x628 bytes have no copy in the tree: a sixty-four-byte span probe over
 * every payload under out/binaries matched no second file, so the body was
 * written from this span's own bytes with the statement order, operand forms and
 * declaration style of the same-target sibling
 * stepSubstatesThroughFrontEndContextsAndFieldRecordSlotsThenReleaseStateScenarioScena1200_801F9F0C
 * (0x801F9F0C), which measured exact when this seed was written and carries the
 * same switch on D_80146875 and the same request and progress-arm shapes. The
 * original shares one armed tail between sub-states 0x0E and 0x10 and one bit
 * request between the ten inner cases; the source spells those tails once per
 * arm and the compiler cross-jumps them back together. The frame keeps $ra and
 * $s0, the latter for the progress-cell pointer the ten cell publishers of
 * sub-state 0x1E must survive.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSubstatesThroughFrontEndContextsThenReleaseFrontEndAndResetStateScenarioScena1200_801FA7DC(void) {
  switch (D_80146875) {
  case 0:
    D_80146875 = 1;
    func_8019FA28(0xBC, 0x158000, 0xB0000, 0x8F);
    D_80143F1F = 0xFF;
    return;
  case 1:
    if (g_ScenarioProgress != 2) {
      return;
    }
    D_80146258 |= 0x80;
    func_8015B580(D_8014686C, 0x2A);
    D_80146866 = 7;
    func_8015C058();
    break;
  case 5:
    func_80150224(0xB);
    D_80143BB0 = 2;
    D_80146875 = 6;
    return;
  case 6:
    if (D_80143BB0 == 2) {
      return;
    }
    func_8015C058();
    D_80146866 = 7;
    break;
  case 10:
    func_8015C088();
    D_80146258 |= 8;
    func_801BE1B0(0x10);
    D_80146875 = 0xB;
    return;
  case 11: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      return;
    }
    *progress = 0;
    D_80146866 = 8;
    D_80146875 = 0xC;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0xA0000, 0x570000, 0x91);
    return;
  }
  case 12: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 4) {
      return;
    }
    *progress = 5;
    D_80146875 = 0xD;
    func_8015DF18(0x20E);
    func_80161C20(0x8F, 0x64, 8);
    return;
  }
  case 13: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 0x14) {
      return;
    }
    *progress = 0;
    D_80146866 = 0;
    D_80146875 = 0xE;
    func_8015B580(D_8014686C, 0x2B);
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0x120000, 0x70000, 0x92);
    D_80143F1F = 0x8F;
    return;
  }
  case 14:
    func_801C601C(4);
    D_80146875 = 0xF;
    return;
  case 15: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      return;
    }
    *progress = 0;
    D_80146875 = 0x10;
    D_80146866 = 0;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0xC0000, 0x590000, 0x93);
    D_80143F1F = 0x8F;
    return;
  }
  case 16:
    if (g_ScenarioProgress != 5) {
      return;
    }
    func_8014ECAC(0xD);
    D_80146875 = 0x11;
    return;
  case 17:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    D_80146875 = 0x12;
    return;
  case 18:
    func_80150224(0x17);
    D_80143BB0 = 2;
    D_80146875 = 0x13;
    return;
  case 19: {
    u8 *armed = &D_80143BB0;
    if (*armed == 2) {
      return;
    }
    *armed = 6;
    D_80146875 = 0x14;
    return;
  }
  case 20:
    if (D_80143BB0 != 0) {
      return;
    }
    D_80146875 = 0x1D;
    D_80146258 ^= 0x80;
    func_8015D404(0, 0);
    func_8015D404(0, 1);
    func_8015B580(D_8014686C, 0x2C);
    func_801C16B8();
    switch (D_80145024 & 0x7F) {
    case 7:
      func_8015B580(D_8014686C, 0x32);
      break;
    case 8:
      func_8015B580(D_8014686C, 0x33);
      break;
    case 9:
      func_8015B580(D_8014686C, 0x34);
      break;
    case 10:
      func_8015B580(D_8014686C, 0x35);
      break;
    case 13:
      func_8015B580(D_8014686C, 0x36);
      break;
    case 14:
      func_8015B580(D_8014686C, 0x37);
      break;
    case 15:
      func_8015B580(D_8014686C, 0x38);
      break;
    case 16:
      func_8015B580(D_8014686C, 0x39);
      break;
    case 17:
      func_8015B580(D_8014686C, 0x3A);
      break;
    case 18:
      func_8015B580(D_8014686C, 0x3B);
      break;
    }
    D_80146866 = 9;
    func_8019FA28(0xBC, 0x370000, 0x3F8000, 0x94);
    D_80143F1F = 0x8B;
    return;
  case 29:
    D_8014832E = 0x1F;
    D_80146875 = 0x1E;
    return;
  case 30: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      return;
    }
    func_801A4BC0(0x38, 0x47, 0xC0);
    func_801A4BC0(0x38, 0x48, 0xC0);
    func_801A4BC0(0x39, 0x47, 0xA1);
    func_801A4BC0(0x39, 0x48, 0xA1);
    func_801A4BC0(0x37, 0x3F, 0xC0);
    func_801A4BC0(0x37, 0x40, 0xC0);
    func_801A4BC0(0x1F, 0x15, 0xA1);
    func_801A4BC0(0x1F, 0x16, 0xA1);
    func_801A4BC0(0x1C, 3, 0xC0);
    func_801A4BC0(0x1D, 3, 0xC0);
    D_80146258 ^= 0x28;
    func_8015C058();
    *progress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    break;
  }
  default:
    return;
  }
  D_80146875 = 0;
  D_80146874 = 0;
}
