#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_80146254;
extern u8 D_80143BB0;
extern u16 D_80143C40;
extern u8 D_8014832E;
extern s8 D_8014865C;
extern u8 D_80148698;
extern u8 D_80148650;
extern u8 D_80148699;
extern u16 D_80143B90;

void func_8014ECAC(u16 arg0);
void func_8015C058(void);
void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801C16B8(void);

/* @source 0x801F936C
 * @behavior Slot 14 of the 31-word code-pointer run at 0x801FE288 (pointer word
 *   at 0x801FE2C0) that dispatchProgressHandlerScenarioScena0200_801F719C
 *   indexes with the signed chain byte D_80146872, so the overlay reaches this
 *   span as that chain slot; no jal of the payload names 0x801F936C and that
 *   table word is its only reference. It switches on the shared sub-state byte
 *   D_80146875 (0x80146875, read with lbu before the frame is grown) through
 *   the twenty-five-word absolute case table at 0x801F6DA4, bounded by `sltiu`
 *   against 0x19, so sub-states 0 through 0x18 select a case and any larger
 *   value returns through the single epilogue. Sub-state 0 ends the front-end
 *   phase: while the shared progress byte g_ScenarioProgress (0x80146864) reads
 *   0x64 it clears that byte, requests front-end mode 0x0D through
 *   func_8014ECAC and advances to 0x14, and while that byte reads 0x65 it
 *   clears it, clears D_80146865 and D_80146866, releases the overlay through
 *   func_8015C058 and returns the sub-state and primary-state bytes
 *   D_80146875 and D_80146874 (0x80146874) to zero. Sub-states 1, 2, 3, 7 and 8
 *   share that closing arm: once the progress byte reads 1, sub-states 1 and
 *   0x18 clear D_80146865, D_80146866 and D_80146867 before releasing; while
 *   the area-mode byte D_80143BB0 (0x80143BB0) reads zero, sub-states 3 and 8
 *   clear the progress byte and then that same byte triple; and sub-states 2
 *   and 7, unless the area-mode byte already reads 2, continue on progress
 *   0xC8 - seeding the front-end cells D_8014865C with -2, D_80148698 with 1,
 *   the shared halfword D_80143B90 (0x80143B90) with 7 and D_80148650 and
 *   D_80148699 with zero before advancing to sub-state 3 - and on progress
 *   0xC9 clear the progress byte, D_80146865 and D_80146866 and release.
 *   Sub-state 0x14, while the shared halfword D_80143C40 (0x80143C40) reads
 *   zero, clears D_8014832E, runs func_801C16B8, advances to 0x15 and starts
 *   the scripted motion func_8015D554(0, 0, 4, 0x1E). Sub-state 0x15 queues
 *   effect group 1 through func_801636A0(0, 1) and advances to 0x16. Sub-state
 *   0x16 advances to 0x17 once func_80163EA0 reports a nonzero low byte.
 *   Sub-state 0x17 stores 5 in D_80146866, advances to 0x18 and seeds the front
 *   selector context through func_8019FA28 with selection seed 5, taking
 *   0x510000, 0x80000 and kind 0x85 while the mode byte D_80146254
 *   (0x80146254) reads 3 and 0x500000, 0x50000 and kind 0x86 otherwise, then
 *   queues the front-end cue func_80161C20(7, 0x64, 8). It takes no arguments
 *   and returns nothing; its 0x18-byte frame keeps $ra and $s0, the latter
 *   holding the address of the progress byte that sub-state 0 reads twice and
 *   writes once across the mode request.
 *
 * Name evidence: the span is chain slot 14 of the same 31-entry run that holds
 * the exact sibling
 * runSubstateProgressLadderThenToggleModeFlags8AndReleaseScenarioScena0200_801F9678
 * (0x801F9678 at slot 15), which is the adjacent sub-state controller of the
 * same overlay, and its own behaviour gates the front-end completion progress
 * values 0x64 and 0x65 that the overlay's setScenarioScena0200 handlers publish
 * before walking the selector-context sub-states 0x14 through 0x18 and
 * releasing, so the name is target-qualified and clause-based. The mode byte
 * D_80146254, the area-mode byte D_80143BB0, the count constants and the
 * cleared cells are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801F936C-0x801F9677 is one
 * complete function of 0x30C bytes with a single `jr $ra` epilogue, so it is
 * neither data nor a fragment and no data or carve lever applies. The dispatch
 * table is 25 words, 0x801F6DA4 through 0x801F6E07, and word 25 would sit at
 * 0x801F6E08, which is neither a case address nor inside this span.
 *
 * Representation notes, measured: the first seed wrote the literal 5 as the
 * seed argument of func_8019FA28 and set the progress pointer of sub-states 2
 * and 7 before their area-mode gates; that measured 162 of 195 instructions at
 * 776 bytes, one instruction short because the seed constant then sat after the
 * mode-byte branch instead of in its delay slot, and because both pointer
 * setups preceded the gates. Moving the seed into a case-local u16 assigned
 * before the mode-byte test, and moving each pointer setup after its gate,
 * measured 193 of 195 instructions at the original 780 bytes with the remaining
 * two on the dispatch-table address alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runFrontEndCompletionSubstatesSeedingSelectorContextsThenReleaseScenarioScena0200_801F936C(void) {
  switch (D_80146875) {
  case 0: {
    u8 *progress;

    progress = &g_ScenarioProgress;
    if (*progress == 0x64) {
      *progress = 0;
      func_8014ECAC(0xD);
      D_80146875 = 0x14;
    }
    if (*progress == 0x65) {
      *progress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  }
  case 0x14:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    func_801C16B8();
    D_80146875 = 0x15;
    func_8015D554(0, 0, 4, 0x1E);
    break;
  case 0x15:
    func_801636A0(0, 1);
    D_80146875 = 0x16;
    break;
  case 0x16:
    if ((u8)func_80163EA0() != 0) {
      D_80146875 = 0x17;
    }
    break;
  case 0x17: {
    u16 selection_seed;
    u32 context_a;
    u32 context_b;
    u8 context_kind;

    selection_seed = 5;
    if (D_80146254 == 3) {
      context_a = 0x510000;
      context_b = 0x80000;
      context_kind = 0x85;
    } else {
      context_a = 0x500000;
      context_b = 0x50000;
      context_kind = 0x86;
    }
    D_80146866 = 5;
    D_80146875 = 0x18;
    func_8019FA28(selection_seed, context_a, context_b, context_kind);
    func_80161C20(7, 0x64, 8);
    break;
  }
  case 0x18:
    if (g_ScenarioProgress == 1) {
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  case 1:
    if (g_ScenarioProgress == 1) {
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  case 2: {
    u8 *progress;

    if (D_80143BB0 == 2) {
      break;
    }
    progress = &g_ScenarioProgress;
    if (*progress == 0xC8) {
      D_8014865C = -2;
      D_80148698 = 1;
      D_80143B90 = 7;
      D_80148650 = 0;
      D_80148699 = 0;
      D_80146875 = 3;
    } else if (*progress == 0xC9) {
      *progress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  }
  case 7: {
    u8 *progress;

    if (D_80143BB0 == 2) {
      break;
    }
    progress = &g_ScenarioProgress;
    if (*progress == 0xC8) {
      D_8014865C = -2;
      D_80148698 = 1;
      D_80143B90 = 7;
      D_80148650 = 0;
      D_80148699 = 0;
      D_80146875 = 3;
    } else if (*progress == 0xC9) {
      *progress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  }
  case 3:
  case 8:
    if (D_80143BB0 == 0) {
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  }
}
