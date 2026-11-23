#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: the gating arms of
 * sub-states 2, 0x14, 0x15, 0x17, 0x19 and 0x1A compare it against one value
 * each, the arms that leave the walker clear it, and the palette arm publishes
 * 1 there.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared main-RAM work byte of the main executable: four arms return while it
 * still reads 2, the request arms store 2 in it, sub-state 0x17 gates on it,
 * and sub-state 0x1B takes its address once and stores 6 through that pointer.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared mode halfword of the main executable: sub-state 2 sets bit 0x16 of it
 * with lhu/ori/sh and sub-state 0x1A toggles that bit with lhu/xori/sh.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared gate halfword of the main executable: the arms of sub-states 2, 0x16
 * and 0x1A return while it reads non-zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared scenario flag word of the main executable: every flag request passes
 * it to func_8015B580 and func_8015B5A8 as the word to set or clear.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared main-RAM byte of the main executable: the clearing tails zero it; no
 * arm of this walker publishes a record index there.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared main-RAM byte of the main executable: the request arms store the
 * next-window byte 0 or 2 into it and the clearing tails zero it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared main-RAM byte of the main executable: the clearing tails zero it; no
 * arm of this walker publishes a record index there.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared second-level state byte of the main executable: the overlay's handler
 * run at 0x801FD1C4 is indexed with it, so this walker runs while it reads 4,
 * and every exiting arm clears it to return the overlay to its idle entry.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte of the main executable: every arm of this walker reads
 * it once through the jump table and the advancing arms store the next value
 * into it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared work byte of the main executable: the two flag arms mask it with 0x7F
 * and pick one scenario flag per masked value.
 * @source 0x80145024 @kind unknown */
extern u8 D_80145024;

/* Shared signed main-RAM byte of the main executable: sub-state 0x1E requires it
 * to read 1 before it picks a scenario flag.
 * @source 0x8014865F @kind unknown */
extern s8 D_8014865F;

/* Shared effect-bank byte of the main executable: the palette arm arms it with
 * 0x1F, the two releasing tails either arm it with 0x1F or clear it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared palette-stage serial of the main executable: the palette arm publishes
 * 1 there once the whole shadow block is grey.
 * @source 0x80145988 @kind unknown */
extern u8 D_80145988;

/* Shared cue byte of the main executable: the sub-state 0x15 arm stores 0xFF
 * there right after its front-end request.
 * @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;

/* Shared front-end request setter at 0x8019FA28: every request arm passes it a
 * 16-bit selection seed, the two context words and the request kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared front-end action queue at 0x80150224: sub-state 0xA queues action 0xB
 * and sub-state 0x1A queues 0x11.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared window reset helper at 0x801C601C: the sub-state 1 arm passes the
 * window selector 0.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared scenario flag setter at 0x8015B580: the flag arms set one flag of the
 * run 0x1A through 0x23 and sub-state 2 sets the flags 0x16 and 0.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 flag_bank, u32 bit_index);

/* Shared scenario flag clearer at 0x8015B5A8: the sub-state 0x1E arm clears the
 * nine flags of that run the selected value does not name.
 * @source 0x8015B5A8 @kind function */
extern void func_8015B5A8(u32 flag_bank, u32 bit_index);

/* Shared overlay release helper at 0x8015C058: every arm that leaves the walker
 * by releasing the front end runs it before the clearing stores.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared effect-bank stepper at 0x801BE1B0: the sub-state 0x14 arm passes the
 * effect id 1.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 effect_id);

/* Shared front-end selection helper at 0x80161C20: the sub-state 0x18 arm runs
 * it with (0x8F, 0x64, 8).
 * @source 0x80161C20 @kind function */
extern void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);

/* Shared front-end cue helper at 0x8014ECAC: the sub-state 0x19 arm passes the
 * constant 4.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared effect-group arm helper at 0x801C16B8: the sub-state 0x1C arm runs it
 * with no arguments right after its flag request.
 * @source 0x801C16B8 @kind function */
extern void func_801C16B8(void);

/* @source 0x801F8A04
 * @behavior Overlay second-level state handler: it is the word at 0x801FD1D4,
 * entry 4 of the ten-word handler run at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 4;
 * no jal or j of the payload names 0x801F8A04 and that table word is its only
 * image reference. It switches on the shared sub-state byte D_80146875 through
 * the forty-entry absolute jump table jtbl_801F6CC4_main at 0x801F6CC4, an
 * overlay-local table of this payload leading tabledata blob, bounded by sltiu
 * against 0x28, so sub-states 0 through 0x27 are its entries: table entries 3
 * through 9, 0x0C through 0x13 and 0x1F through 0x26 point at the body exit,
 * where those values return without touching any byte.
 *
 * Sub-state 0 stores 1 in the sub-state byte, clears the shared progress byte
 * g_ScenarioProgress (0x80146864) and the byte D_80146866 and requests the
 * front-end context 0x85, 0x50000, 0x5D0000 with kind 0x80 through func_8019FA28
 * (0x8019FA28). Sub-state 1 runs the window reset helper func_801C601C
 * (0x801C601C) with 0 and advances to sub-state 2. Sub-state 2 takes the
 * progress byte address once: while it reads 5 and the gate halfword D_80143C40
 * (0x80143C40) is zero it sets bit 0x16 of the shared mode halfword D_80146258
 * (0x80146258), sets flag 0x16 and then flag 0 of the shared flag word
 * D_8014686C (0x8014686C) through func_8015B580 (0x8015B580), runs the release
 * helper func_8015C058 (0x8015C058), clears the progress byte and falls into
 * the window-clearing tail that zeroes D_80146865, D_80146866 and D_80146867.
 * Sub-state 0xA queues action 0xB through func_80150224 (0x80150224), stores 2
 * in the work byte D_80143BB0 (0x80143BB0) and advances to sub-state 0xB, which
 * waits for that work byte to leave 2 and then runs the release helper and the
 * state-clearing tail alone, without the three window bytes.
 *
 * Sub-state 0x14 waits for the work byte to leave 2, stores 1 in the progress
 * byte, runs the effect-bank stepper func_801BE1B0 (0x801BE1B0) with 1 and
 * advances to sub-state 0x15. Sub-state 0x15 waits for the progress byte to read
 * 6, stores 2 in D_80146866, clears the progress byte, advances to sub-state
 * 0x16, sets flag 0x17, requests the context 0x65, 0x440000, 0x88000 with kind
 * 0x82 and stores 0xFF in the cue byte D_80143F1F (0x80143F1F). Sub-state 0x16
 * returns while the gate halfword is non-zero; otherwise it queues action 0x11,
 * stores 2 in the work byte and advances to sub-state 0x17.
 *
 * Sub-state 0x17 waits for the work byte to leave 2 and then rewrites the whole
 * 0x80037800 shadow palette block in place to grey: 0x20 rows of 0x200 bytes,
 * 0x100 halfwords each, where every halfword has its three 5-bit channels added,
 * the sum divided by three with the 0xAAAAAAAB multiply and that average
 * replicated into all three channel fields while bit 15 of the source word is
 * kept. It then publishes 1 in the palette-stage serial D_80145988, arms the
 * effect-bank byte D_8014832E with 0x1F and publishes 1 in the progress byte,
 * clears D_80146866 and advances to sub-state 0x18. Sub-state 0x18 waits for the
 * progress byte to read 2, clears D_80146866 and that byte, advances to
 * sub-state 0x19, sets flag 0x18, requests the context 0x85, 0x1E0000, 0x5D0000
 * with kind 0x82 and runs the front-end selection func_80161C20 (0x80161C20)
 * with (0x8F, 0x64, 8). Sub-state 0x19 waits for the progress byte to read 4,
 * runs the front-end cue helper func_8014ECAC (0x8014ECAC) with 4 and advances to
 * sub-state 0x1A, which returns while the gate halfword is non-zero; otherwise
 * it toggles bit 0x16 of the mode halfword, clears the progress byte and the
 * effect-bank byte, queues action 0x11, stores 2 in the work byte and advances to
 * sub-state 0x1B.
 *
 * Sub-state 0x1B takes the work byte address once, returns while it reads 2 and
 * otherwise stores 6 through that pointer and advances to sub-state 0x1C. That
 * arm returns while the work byte is non-zero, advances the sub-state to 0x1D,
 * sets flag 0x19 and runs func_801C16B8 (0x801C16B8) before it switches on the
 * masked work byte D_80145024 & 0x7F through the twelve-entry absolute jump table
 * jtbl_801F6D64_main at 0x801F6D64: masked values 7 through 0xA and 0xD through
 * 0x12 set the flag 0x1A through 0x23 that matches their position in that run
 * through func_8015B580, masked values 0xB and 0xC set none, and every path then
 * requests the context 0x85, 0x3D0000, 0x2B8000 with kind 0x83. Sub-state 0x1E
 * gates the same way on the work byte reading 0 and on the signed byte
 * D_8014865F (0x8014865F) reading 1, advances the sub-state to 0x27 and switches
 * on the same masked work byte through the twelve-entry absolute jump table
 * jtbl_801F6D94_main at 0x801F6D94: the arm that mask selects sets its own flag
 * of the run 0x1A through 0x23 with func_8015B580 and clears the other nine with
 * func_8015B5A8 (0x8015B5A8), masked values 0xB and 0xC leave the whole run
 * untouched, and every path then requests the same 0x83 context as sub-state
 * 0x1C. Sub-states 0x1D and 0x27 share the releasing tail: they arm the
 * effect-bank byte with 0x1F, run the release helper and clear the progress byte
 * before falling into the window-clearing tail and the state-clearing tail.
 *
 * It takes no arguments and returns nothing; its 0x18-byte frame keeps $ra and
 * $s0, and $s0 holds the progress byte address sub-state 2 takes once and reuses
 * across the release call. Every cell and helper above is addressed from the main
 * executable except the three jump tables and the 0x80037800 shadow palette
 * block, so the span has no byte-identical copy: a whole-span search of the 398
 * extracted payloads under out/binaries matched no second file, so the clone
 * lever was declined and every literal here was written from this span's own
 * bytes. The palette arm transcribes the exact
 * averagePaletteChannelsScenarioScena1400_801F7960 (emi/scenario/scena14/00),
 * which carries the same two bottom-tested loops, the same 0xAAAAAAAB divide and
 * the same channel pack, and a 0xA4-byte window of this span is byte-identical to
 * the one in emi/scenario/scena10/00. The flag-run arms, the per-arm `break` and
 * `return` forms, the `u8 *` cell pointer and the declaration style come from the
 * exact same-target siblings
 * stepSubstatesArmingRelease
 * (0x801FAE04) and
 * stepSubstatesThroughFrontEndContextsThenReleaseFrontEndAndResetStateScenarioScena1200_801FA7DC
 * (0x801FA7DC), which carry the same masked-work-byte switch over the ids 7
 * through 0x12 and the same post-switch state-clearing tail. The palette arm's
 * closing stores were the only undecided shape: written serial, progress,
 * effect-bank they measured 796 of 799 instructions, progress-first 794,
 * effect-bank-first 796 and window-before-effect 791, while the measured order
 * serial, effect-bank, progress reached 799 of 799, so the two stores the
 * scheduler had already swapped are spelled in the order the sweep showed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void
stepRequestingGreyingArming(
    void) {
  switch (D_80146875) {
  case 0:
    g_ScenarioProgress = 0;
    D_80146866 = 0;
    D_80146875 = 1;
    func_8019FA28(0x85, 0x50000, 0x5D0000, 0x80);
    return;
  case 1:
    func_801C601C(0);
    D_80146875 = 2;
    return;
  case 2: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 5u) {
      return;
    }
    if (D_80143C40 != 0) {
      return;
    }
    D_80146258 |= 0x16;
    func_8015B580(D_8014686C, 0x16);
    func_8015B580(D_8014686C, 0);
    func_8015C058();
    *progress = 0;
    break;
  }
  case 10:
    func_80150224(0xB);
    D_80143BB0 = 2;
    D_80146875 = 0xB;
    return;
  case 11:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8015C058();
    goto resetState;
  case 20:
    if (D_80143BB0 == 2u) {
      return;
    }
    g_ScenarioProgress = 1;
    func_801BE1B0(1);
    D_80146875 = 0x15;
    return;
  case 21: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 6u) {
      return;
    }
    D_80146866 = 2;
    *progress = 0;
    D_80146875 = 0x16;
    func_8015B580(D_8014686C, 0x17);
    func_8019FA28(0x65, 0x440000, 0x88000, 0x82);
    D_80143F1F = 0xFF;
    return;
  }
  case 22:
    if (D_80143C40 != 0) {
      return;
    }
    func_80150224(0x11);
    D_80143BB0 = 2;
    D_80146875 = 0x17;
    return;
  case 23: {
    u32 row;
    u32 col;
    u32 rowBase;
    u16 *cell;
    s32 color;
    u32 rgb;
    u32 red;
    u32 average;
    u32 packed;
    u32 shifted;

    if (D_80143BB0 == 2u) {
      return;
    }
    row = 0u;
    do {
      col = 0u;
      rowBase = 0x80037800u + (row & 0xffu) * 0x200u;
      do {
        cell = PSX_PTR(u16, (col & 0xffffu) * 2u + rowBase);
        color = *cell;
        red = color & 0x1fu;
        rgb = color & 0xffffu;
        average = (red + ((rgb >> 5) & 0x1fu) + ((rgb >> 10) & 0x1fu)) / 3u;
        packed = color & 0x8000u;
        shifted = average << 10;
        packed = packed | shifted;
        shifted = average << 5;
        packed = packed | shifted;
        *cell = (u16)(average | packed);
        col++;
      } while ((col & 0xffffu) < 0x100u);
      row++;
    } while ((row & 0xffu) < 0x20u);

    D_80145988 = 1;
    D_8014832E = 0x1F;
    g_ScenarioProgress = 1;
    D_80146866 = 0;
    D_80146875 = 0x18;
    return;
  }
  case 24: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 2u) {
      return;
    }
    D_80146866 = 0;
    *progress = 0;
    D_80146875 = 0x19;
    func_8015B580(D_8014686C, 0x18);
    func_8019FA28(0x85, 0x1E0000, 0x5D0000, 0x82);
    func_80161C20(0x8F, 0x64, 8);
    return;
  }
  case 25:
    if (g_ScenarioProgress != 4u) {
      return;
    }
    func_8014ECAC(4);
    D_80146875 = 0x1A;
    return;
  case 26:
    if (D_80143C40 != 0) {
      return;
    }
    D_80146258 ^= 0x16;
    g_ScenarioProgress = 0;
    D_8014832E = 0;
    func_80150224(0x11);
    D_80143BB0 = 2;
    D_80146875 = 0x1B;
    return;
  case 27: {
    u8 *workByte = &D_80143BB0;
    if (*workByte == 2u) {
      return;
    }
    *workByte = 6;
    D_80146875 = 0x1C;
    return;
  }
  case 28:
    if (D_80143BB0 != 0) {
      return;
    }
    D_80146875 = 0x1D;
    func_8015B580(D_8014686C, 0x19);
    func_801C16B8();
    switch (D_80145024 & 0x7F) {
    case 7:
      func_8015B580(D_8014686C, 0x1A);
      break;
    case 8:
      func_8015B580(D_8014686C, 0x1B);
      break;
    case 9:
      func_8015B580(D_8014686C, 0x1C);
      break;
    case 10:
      func_8015B580(D_8014686C, 0x1D);
      break;
    case 13:
      func_8015B580(D_8014686C, 0x1E);
      break;
    case 14:
      func_8015B580(D_8014686C, 0x1F);
      break;
    case 15:
      func_8015B580(D_8014686C, 0x20);
      break;
    case 16:
      func_8015B580(D_8014686C, 0x21);
      break;
    case 17:
      func_8015B580(D_8014686C, 0x22);
      break;
    case 18:
      func_8015B580(D_8014686C, 0x23);
      break;
    }
    func_8019FA28(0x85, 0x3D0000, 0x2B8000, 0x83);
    return;
  case 30:
    if (D_80143BB0 != 0) {
      return;
    }
    if (D_8014865F != 1) {
      return;
    }
    D_80146875 = 0x27;
    switch (D_80145024 & 0x7F) {
    case 7:
      func_8015B580(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 8:
      func_8015B580(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 9:
      func_8015B580(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 10:
      func_8015B580(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 13:
      func_8015B580(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 14:
      func_8015B580(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 15:
      func_8015B580(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 16:
      func_8015B580(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 17:
      func_8015B580(D_8014686C, 0x22);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x23);
      break;
    case 18:
      func_8015B580(D_8014686C, 0x23);
      func_8015B5A8(D_8014686C, 0x1A);
      func_8015B5A8(D_8014686C, 0x1B);
      func_8015B5A8(D_8014686C, 0x1C);
      func_8015B5A8(D_8014686C, 0x1D);
      func_8015B5A8(D_8014686C, 0x1E);
      func_8015B5A8(D_8014686C, 0x1F);
      func_8015B5A8(D_8014686C, 0x20);
      func_8015B5A8(D_8014686C, 0x21);
      func_8015B5A8(D_8014686C, 0x22);
      break;
    }
    func_8019FA28(0x85, 0x3D0000, 0x2B8000, 0x83);
    return;
  case 29:
  case 39:
    D_8014832E = 0x1F;
    func_8015C058();
    g_ScenarioProgress = 0;
    break;
  default:
    return;
  }
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
resetState:
  D_80146875 = 0;
  D_80146874 = 0;
}
