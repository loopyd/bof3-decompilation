#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: the waiting arms of
 * sub-states 1, 0x15, 0x1E, 0x1F and 0x20 compare it against one value each and
 * clear it on the way out.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared main-RAM byte of the main executable: the gating arms return while it
 * still reads 2, and the arms that take the front end store the armed value 2.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared main-RAM byte of the main executable: sub-state 0 switches on it to
 * pick the action id its request arms pass to func_80150224.
 * @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;

/* Shared main-RAM byte of the main executable: the two sub-switches of
 * sub-state 0 read it as the second selector of the action request.
 * @source 0x80145F09 @kind unknown */
extern u8 D_80145F09;

/* Shared gate halfword of the main executable: sub-states 3 and 5 return while
 * it reads non-zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared effect-bank byte of the main executable: sub-states 4 and 0x15 arm it
 * with 0x1F and the gating arms clear it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared signed main-RAM byte of the main executable: sub-state 0x14 requires it
 * to read 1 before it selects a scenario flag.
 * @source 0x8014865F @kind unknown */
extern s8 D_8014865F;

/* Shared work byte of the main executable: sub-state 0x14 masks it with 0x7F and
 * selects one scenario flag per masked value.
 * @source 0x80145024 @kind unknown */
extern u8 D_80145024;

/* Shared scenario flag word of the main executable: every flag request passes it
 * to func_8015B580 and func_8015B5A8 as the word to arm or clear.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared main-RAM byte of the main executable: the clearing arms zero it.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared main-RAM byte of the main executable: the clearing arms zero it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared main-RAM byte of the main executable: the clearing arms zero it.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared state byte of the main executable: the overlay's second-level handler
 * run at 0x801FD1C4 is indexed with it, and every exiting arm clears it to
 * return the overlay to its idle entry.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte of the main executable: every arm of this walker reads
 * it once through the jump table and the advancing arms store the next value
 * into it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared action trigger at 0x80150224: the sub-state 0 arms pass it the action
 * ids 0x18, 0x19 and 0x1A and sub-state 0x1E passes 0x20.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared front-end mode setter at 0x8014ECAC: sub-state 1 sets mode 0xD and
 * sub-state 4 sets mode 0xE through it.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared effect-group arm helper at 0x801C16B8: sub-state 1 runs it with no
 * arguments before the four-argument call that follows it.
 * @source 0x801C16B8 @kind function */
extern void func_801C16B8(void);

/* Shared four-argument helper at 0x8015D554: sub-state 1 runs it with
 * (0, 1, 4, 0x1E).
 * @source 0x8015D554 @kind function */
extern void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared two-argument helper at 0x801636A0: sub-state 2 runs it with (0, 1).
 * @source 0x801636A0 @kind function */
extern void func_801636A0(u32 arg0, u32 arg1);

/* Shared byte answer helper at 0x80163EA0: sub-state 3 advances only when the
 * masked answer is non-zero.
 * @source 0x80163EA0 @kind function */
extern u32 func_80163EA0(void);

/* Shared front-end selection at 0x80161C20: sub-state 5 runs it with
 * (0x8B, 0x64, 8).
 * @source 0x80161C20 @kind function */
extern void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);

/* Shared front-end release helper at 0x8015C058: the releasing arms run it
 * before they clear the progress byte or the whole state.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end context setter at 0x8019FA28: sub-state 0x14 requests the
 * kind 0x95 with the seed 0xBC and the two context words.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared scenario flag setter at 0x8015B580: sub-state 0x14 arms one flag of
 * the run 0x32 through 0x3B.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared scenario flag clearer at 0x8015B5A8: sub-state 0x14 clears the nine
 * flags of that run the selected value does not name.
 * @source 0x8015B5A8 @kind function */
extern void func_8015B5A8(u32 arg0, s32 arg1);

/* Probe record for the case 1 arm: keeping the scenario progress byte in a
 * local read before the arm tests it, and taking the cell address from a
 * pointer assigned after the front-end mode gate, measured 664 of 664
 * instructions on the native gate. Initialising that pointer in its
 * declaration instead measured 662 of 664, with the pointer setup hoisted above
 * the mode test. */

/* @source 0x801FAE04
 * @behavior Overlay second-level state handler: it is the word stored at
 * 0x801FD1E4, entry 8 of the ten-word handler run at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 8;
 * that table word is the only image reference to this address and every exiting
 * arm clears D_80146874 itself. It switches on the shared sub-state byte
 * D_80146875 through the thirty-three-entry absolute jump table at 0x801F6FBC,
 * an overlay-local table of the leading tabledata blob, so sub-states 0 through
 * 0x20 are its table entries: sub-states 6 through 9, 0x0B through 0x13 and
 * 0x16 through 0x1D are holes that point at the return path, and an explicit
 * default returns untouched too. Sub-state 0 returns while the shared
 * front-end mode byte D_80143BB0 still reads 2, switches on the shared byte
 * D_80143F03 and then on D_80145F09 through the two seven-entry absolute jump
 * tables at 0x801F7044 and 0x801F7064 to pass the shared action trigger
 * func_80150224 (0x80150224) the action ids 0x18, 0x19 and 0x1A before it arms
 * D_80143BB0 with 2 and advances to sub-state 1 or 0xA. Sub-state 1 takes one of
 * two exits on the shared progress byte g_ScenarioProgress (0x80146864): at
 * 0x0A it clears that byte, sets front-end mode 0xD through func_8014ECAC
 * (0x8014ECAC), arms the effect group through func_801C16B8 (0x801C16B8) and
 * runs func_8015D554 (0x8015D554) with (0, 1, 4, 0x1E); the zero exit waits for
 * progress 5 and releases the front-end through func_8015C058 (0x8015C058). Sub-
 * states 2, 3 and 4 step a short setup chain: func_801636A0 (0x801636A0) with
 * (0, 1), then a wait on the shared gate halfword D_80143C40 with a clear of
 * D_8014832E and a step gated on the masked func_80163EA0 (0x80163EA0) answer,
 * then front-end mode 0xE with D_8014832E armed to 0x1F; sub-state 5 waits on
 * the same gate halfword, runs func_80161C20 (0x80161C20) with (0x8B, 0x64, 8)
 * and releases the front-end. Sub-state 0x14 gates on D_80143BB0 reading 0 and
 * the signed shared byte D_8014865F reading 1, clears D_8014832E, advances to
 * 0x15 and selects one scenario flag per masked value of the shared work byte
 * D_80145024 through the twelve-entry absolute jump table at 0x801F7084: masked
 * values 7 through 0x12 arm the flag 0x32 through 0x3B with func_8015B580
 * (0x8015B580) and clear the other nine flags of that run with func_8015B5A8
 * (0x8015B5A8), masked values 0x0B and 0x0C and every value outside 7 through
 * 0x12 leave the run untouched, and every path then requests the front-end
 * context kind 0x95 with the seed 0xBC and the context words 0xC8000 and 0xA8000
 * through func_8019FA28 (0x8019FA28). Sub-state 0x15 arms D_8014832E with 0x1F,
 * releases the front-end, clears the progress byte and the window bytes
 * D_80146865, D_80146866 and D_80146867. Sub-states 0x1E, 0x1F and 0x20 drive
 * one front-end excursion through the armed byte D_80143BB0 and the progress
 * byte, run the action trigger func_80150224 with 0x20 and clear the progress
 * byte and those three window bytes on the way out, and every exiting arm
 * clears D_80146875 and D_80146874 to return the overlay to its idle entry. Its
 * 0xA60 bytes have no copy in the tree: a sixteen-byte window scan over every
 * payload under out/binaries matched no second file, so the body was written
 * from this span's own bytes with the statement order, operand forms and
 * declaration style of the same-target sibling
 * stepSubstatesThroughFrontEndContextsThenReleaseFrontEndAndResetStateScenarioScena1200_801FA7DC
 * (0x801FA7DC), which carries the same switch on D_80146875 and the same
 * request, gate and clearing arm shapes. The 0x18-byte frame keeps $ra and $s0,
 * the latter for the one cell pointer that each of the three arms holding a
 * pointer across a call materialises.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSubstatesArmingOneScenarioFlagFromWorkByteThenReleaseFrontEndAndResetStateScenarioScena1200_801FAE04(void) {
  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2) {
      return;
    }
    switch (D_80143F03) {
    case 2:
      switch (D_80145F09) {
      case 2:
      case 4:
      case 6:
      case 7:
        func_80150224(0x19);
        D_80143BB0 = 2;
        D_80146875 = 1;
        return;
      case 5:
      case 8:
        func_80150224(0x1A);
        D_80143BB0 = 2;
        D_80146875 = 0xA;
        return;
      }
      return;
    case 3:
      switch (D_80145F09) {
      case 2:
      case 4:
      case 6:
      case 7:
        func_80150224(0x18);
        D_80143BB0 = 2;
        D_80146875 = 0xA;
        return;
      case 5:
      case 8:
        func_80150224(0x19);
        D_80143BB0 = 2;
        D_80146875 = 1;
        return;
      }
      return;
    }
    return;
  case 1: {
    u8 *progress;
    u8 value;
    if (D_80143BB0 == 2) {
      return;
    }
    progress = &g_ScenarioProgress;
    value = *progress;
    if (value == 0xA) {
      *progress = 0;
      D_80146875 = 2;
      func_8014ECAC(0xD);
      func_801C16B8();
      func_8015D554(0, 1, 4, 0x1E);
      return;
    }
    if (value != 5) {
      return;
    }
    func_8015C058();
    *progress = 0;
    break;
  }
  case 2:
    func_801636A0(0, 1);
    D_80146875 = 3;
    return;
  case 3:
    if (D_80143C40 != 0) {
      return;
    }
    D_8014832E = 0;
    if ((func_80163EA0() & 0xFF) == 0) {
      return;
    }
    D_80146875 = 4;
    return;
  case 4:
    func_8014ECAC(0xE);
    D_8014832E = 0x1F;
    D_80146875 = 5;
    return;
  case 5:
    if (D_80143C40 != 0) {
      return;
    }
    func_80161C20(0x8B, 0x64, 8);
    func_8015C058();
    break;
  case 10:
    if (D_80143BB0 == 2) {
      return;
    }
    func_8015C058();
    g_ScenarioProgress = 0;
    break;
  case 20:
    if (D_80143BB0 != 0) {
      return;
    }
    D_8014832E = 0;
    if (D_8014865F != 1) {
      return;
    }
    D_80146875 = 0x15;
    switch (D_80145024 & 0x7F) {
    case 7:
      func_8015B580(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 8:
      func_8015B580(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 9:
      func_8015B580(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 10:
      func_8015B580(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 13:
      func_8015B580(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 14:
      func_8015B580(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 15:
      func_8015B580(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 16:
      func_8015B580(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 17:
      func_8015B580(D_8014686C, 0x3A);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3B);
      break;
    case 18:
      func_8015B580(D_8014686C, 0x3B);
      func_8015B5A8(D_8014686C, 0x32);
      func_8015B5A8(D_8014686C, 0x33);
      func_8015B5A8(D_8014686C, 0x34);
      func_8015B5A8(D_8014686C, 0x35);
      func_8015B5A8(D_8014686C, 0x36);
      func_8015B5A8(D_8014686C, 0x37);
      func_8015B5A8(D_8014686C, 0x38);
      func_8015B5A8(D_8014686C, 0x39);
      func_8015B5A8(D_8014686C, 0x3A);
      break;
    }
    func_8019FA28(0xBC, 0xC8000, 0xA8000, 0x95);
    return;
  case 21:
    D_8014832E = 0x1F;
    func_8015C058();
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    break;
  case 30: {
    u8 *armed = &D_80143BB0;
    if (*armed != 0) {
      return;
    }
    g_ScenarioProgress = 1;
    func_80150224(0x20);
    *armed = 2;
    D_80146875 = 0x1F;
    return;
  }
  case 31:
    if (D_80143BB0 != 0) {
      return;
    }
    g_ScenarioProgress = 2;
    D_80146875 = 0x20;
    return;
  case 32: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 3) {
      return;
    }
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
