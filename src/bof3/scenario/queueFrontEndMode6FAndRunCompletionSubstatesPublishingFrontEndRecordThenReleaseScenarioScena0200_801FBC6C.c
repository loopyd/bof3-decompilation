#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_80143BB0;
extern u16 D_80143C40;
extern u8 D_8014832E;
extern s8 D_8014865C;
extern u8 D_80148698;
extern u8 D_80148650;
extern u8 D_80148699;
extern u16 D_80143B90;
extern u16 D_801490A4;

/* Shared front-end mode request of the main executable */
void func_80150224(u32 arg0);
/* Shared overlay release helper of the main executable */
void func_8015C058(void);
/* Shared front-end mode request of the main executable */
void func_8014ECAC(u16 arg0);
/* Shared field-record reinitializer of the etc/game overlay */
void func_801C16B8(void);
/* Shared front-end cue stopper of the main executable */
void func_80161D20(s32 arg0);
/* Shared front-end cue finisher of the main executable */
void func_80161D74(s32 arg0);
/* Shared frame timer re-arm of the main executable */
void func_801636A0(u32 arg0, u32 arg1);
/* Shared front-end predicate of the main executable */
u32 func_80163EA0(void);
/* Shared front selector context seed of the main executable */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801FBC6C
 * @behavior Slot 30, the last entry (pointer word at 0x801FE300) of the
 *   31-word code-pointer run at 0x801FE288 that
 *   dispatchProgressHandlerScenarioScena0200_801F719C indexes with the signed
 *   chain byte D_80146872, so the overlay reaches this span as that chain
 *   slot. It switches on the shared sub-state byte D_80146875 (0x80146875,
 *   read with lbu before the frame is grown) through the twenty-six-word
 *   absolute case table at 0x801F7134, bounded by `sltiu` against 0x1A, so
 *   sub-states 0 through 0x19 select a case and any larger value returns
 *   through the single epilogue. Sub-state 0 requests front-end mode 0x6F
 *   through func_80150224, arms the shared primary mode byte D_80143BB0
 *   (0x80143BB0) with 2 and advances to 1. Sub-state 1 aborts the ladder while
 *   that mode byte still reads 2 and otherwise clears the progress byte
 *   g_ScenarioProgress (0x80146864) and returns the overlay to primary state 0.
 *   Sub-state 5, once bit 2 of the shared decision halfword D_801490A4
 *   (0x801490A4) is set, treats progress 0x5A as the abort (clearing that byte
 *   and returning to primary state 0) and progress 0x5B as the go-ahead: it
 *   arms the shared byte D_80146866 (0x80146866) with 1, advances the sub-state
 *   byte to 0x14 and requests front-end mode 0x0D through func_8014ECAC.
 *   Sub-states 0x14 through 0x19 run the front-end completion ladder once the
 *   shared field-record halfword D_80143C40 (0x80143C40) reads 0: 0x14 clears
 *   D_8014832E (0x8014832E), reinitializes the field records through
 *   func_801C16B8, advances to 0x15 and stops front-end cue 0x21 through
 *   func_80161D20; 0x15 re-arms the frame timer with func_801636A0(0, 1) and
 *   advances to 0x16; 0x16 waits for the shared predicate func_80163EA0, then
 *   finishes cue 0x21 through func_80161D74 and advances to 0x17; 0x17 arms
 *   D_80146866 with 2 and the sub-state byte with 0x18 before seeding the front
 *   selector context func_8019FA28(0x1B, 0x490000, 0xA0000, 0x99); 0x18, once
 *   the mode byte leaves 2 and the decision bit is set, either publishes the
 *   front-end record (D_8014865C = -2, D_80148698 = 1, D_80143B90 = 7,
 *   D_80148650 = 0, D_80148699 = 0) and advances to 0x19 at progress 0x5C, or
 *   at progress 0x5D clears the progress byte and returns to primary state 0;
 *   0x19 returns the overlay to primary state 0 once the mode byte reads 0.
 *   The reset arm clears D_80146865, D_80146866, D_80146867, runs the shared
 *   release helper func_8015C058 and clears the sub-state byte D_80146875 and
 *   the primary state byte D_80146874 (0x80146874), in that order, and the
 *   three call sites share it. It takes no arguments and returns nothing; its
 *   0x18-byte frame keeps only $ra.
 *
 * Name evidence: no in-payload `jal` targets 0x801FBC6C and the only payload
 * word equal to that address is at 0x801FE300, the last entry of the chain run
 * that dispatchProgressHandlerScenarioScena0200_801F719C indexes, so the role
 * word is the chain-slot sub-state stepper the sibling
 * runFrontEndCompletionSubstatesSeedingSelectorContextsThenReleaseScenarioScena0200_801F936C
 * (0x801F936C, slot 14 of the same run) already established for this overlay:
 * the same sub-state byte, the same twenty-six-slot style of absolute case
 * table, the same field-record reinitializer, cue and timer helpers, the same
 * front selector context seed and the same shared reset arm.
 *
 * Boundary and lever evidence: the payload span 0x801FBC6C-0x801FBF23 is one
 * complete function of 0x2B8 bytes with one prologue, a single `jr $ra`
 * epilogue and one shared reset block, so it is neither data nor a fragment and
 * no data or carve lever applies. The dispatch table is 26 words, 0x801F7134
 * through 0x801F719B, inside the leading table blob of this overlay. Clone is
 * declined on payload evidence: an exact scan of the 0x2B8 bytes over all 398
 * payloads under out/binaries matched this span alone, and a second scan that
 * masks only the absolute immediates (lui, j, jal) matched this span alone as
 * well, so no byte-identical or relocation-different sibling body exists.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontEndMode6FAndRunCompletionSubstatesPublishingFrontEndRecordThenReleaseScenarioScena0200_801FBC6C(void) {
  switch (D_80146875) {
  case 0:
    func_80150224(0x6F);
    D_80143BB0 = 2;
    D_80146875 = 1;
    break;
  case 1:
    if (D_80143BB0 == 2) {
      break;
    }
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 5: {
    u8 *progress;

    if (!(D_801490A4 & 2)) {
      break;
    }
    progress = &g_ScenarioProgress;
    if (*progress == 0x5A) {
      *progress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    } else if (*progress == 0x5B) {
      D_80146866 = 1;
      D_80146875 = 0x14;
      func_8014ECAC(0xD);
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
    func_80161D20(0x21);
    break;
  case 0x15:
    func_801636A0(0, 1);
    D_80146875 = 0x16;
    break;
  case 0x16:
    if ((u8)func_80163EA0() != 0) {
      func_80161D74(0x21);
      D_80146875 = 0x17;
    }
    break;
  case 0x17:
    D_80146866 = 2;
    D_80146875 = 0x18;
    func_8019FA28(0x1B, 0x490000, 0xA0000, 0x99);
    break;
  case 0x18:
    if (!(D_801490A4 & 2)) {
      break;
    }
    if (g_ScenarioProgress == 0x5C) {
      D_8014865C = -2;
      D_80148698 = 1;
      D_80143B90 = 7;
      D_80148650 = 0;
      D_80148699 = 0;
      D_80146875 = 0x19;
    } else if (g_ScenarioProgress == 0x5D) {
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015C058();
      D_80146875 = 0;
      D_80146874 = 0;
    }
    break;
  case 0x19:
    if (D_80143BB0 != 0) {
      break;
    }
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  case 10: {
    u8 *progress;

    if (D_80143BB0 == 2) {
      break;
    }
    progress = &g_ScenarioProgress;
    if (*progress == 0x5C) {
      D_8014865C = -2;
      D_80148698 = 1;
      D_80143B90 = 7;
      D_80148650 = 0;
      D_80148699 = 0;
      D_80146875 = 0x19;
    } else if (*progress == 0x5D) {
      *progress = 0;
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
}
