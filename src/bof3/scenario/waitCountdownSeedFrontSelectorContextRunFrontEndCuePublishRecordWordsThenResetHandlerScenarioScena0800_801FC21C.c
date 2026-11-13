#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80145029;
extern u8 *D_80148208;
extern u8 D_8014832E;
extern u16 D_80143C40;
extern u32 D_8014686C;
extern s8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

void func_8014ECAC(u16 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_80161D20(s32 arg0);
void func_80161D74(s32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C16B8(void);

/* @source 0x801FC21C
 * @behavior Entry 7 of the overlay progress-handler table at D_801FE970 that
 * dispatchProgressHandlerScenarioScena0800_801FAB5C indexes with the signed
 * byte D_80146874, so no in-image jal targets this address and that table word
 * is its only reference in this payload. It switches the shared sub-state byte
 * D_80146875 through the sixteen-entry absolute jump table jtbl_801F6CE4_main
 * at 0x801F6CE4, an overlay-local table in this payload leading table data.
 * Sub-state 0 decrements the shared halfword D_80146876 and, when the value it
 * replaced was zero, runs func_801BE1B0(0) and advances to 1. Sub-state 1, at
 * the shared scenario progress byte g_ScenarioProgress (0x80146864, read with
 * lbu) reading 0, sets scenario flag bit 9 through func_8015B580 on the shared
 * flag word D_8014686C, releases the shared selector through func_8015C058 and
 * returns the overlay to handler 0 by clearing D_80146874 and the sub-state
 * byte. Sub-state 0xA requests front selector context seed 0 with the two
 * 16.16 context words 0x5C0000 and 0x498000 and kind 0x86 through
 * func_8019FA28 and advances to 0xB. Sub-state 0xB, at progress byte 0xC, runs
 * the shared front-end mode setter func_8014ECAC(0) and advances to 0xC.
 * Sub-state 0xC, while the shared gate halfword D_80143C40 reads 0, clears the
 * effect bank byte D_8014832E, runs the shared field-record reinitializer
 * func_801C16B8, stops the shared front-end cue through func_80161D20 with the
 * shared selection index byte D_80145029, queues effect group 0 with flag 1
 * through func_801636A0 and advances to 0xD. Sub-state 0xD, unless the
 * frontend-transition predicate func_80163EA0 reports a nonzero low byte,
 * finishes that cue through func_80161D74 with D_80145029, arms the effect bank
 * byte D_8014832E to 0x1F, runs func_8014ECAC(1) and advances to 0xE.
 * Sub-state 0xE, while D_80143C40 reads 0, sets scenario flag bit 0xB through
 * func_8015B580 on D_8014686C, advances to 0xF and raises g_ScenarioProgress.
 * Sub-state 0xF, at progress byte 0, publishes the record reached through the
 * pointer cell D_80148208 (0x80148208, read with lw twice because a record
 * store may alias the cell): byte 6 = 5, word 0x18 = 1 and word 0x1C = 3, and
 * then runs the same release-and-reset tail as sub-state 1. Sub-states 2
 * through 9 and any value outside the table return without touching the tail.
 * It takes no arguments, returns nothing, and its 0x18-byte frame keeps only
 * $ra.
 * Shape notes from the original bytes: the adjacent entry-5 sibling at
 * 0x801FBA70 supplied the statement order, the one-extern-per-cell bindings and
 * the release-and-reset tail, and this span's own bytes decided the two
 * read-modify-write sites, which take the cell address through a local pointer:
 * count = &D_80146876 in sub-state 0 and progress = &g_ScenarioProgress in
 * sub-state 0xE. That spelling is what keeps the address live in $v0 and lets
 * the scheduler move the sub-state 0 halfword store into the branch delay slot,
 * where the direct global spellings materialise a second %lo base in $at and
 * keep a separate slot. Probes below the gate, all clean C: the first seed with
 * both cells written directly measured 584 bytes against this 576-byte span,
 * the post-decrement and value-local spellings of sub-state 0 measured 592 and
 * 580, the pointer form of sub-state 0xE alone measured 580, and the final
 * store-then-test pointer form measured 576 bytes with 142 of 144 instructions
 * equal. The two words still different at that point are the dispatch base, a
 * pair psyq gcc resolves against the .rodata it emits for the sixteen absolute
 * case words; only the reviewed matching.section_placements row for .rodata at
 * 0x801F6CE4 in this target manifest places it, since the original keeps that
 * table in the payload leading table data.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitCountdownSeedFrontSelectorContextRunFrontEndCuePublishRecordWordsThenResetHandlerScenarioScena0800_801FC21C(
    void) {
  switch (D_80146875) {
  case 0: {
    u16 *count;
    u16 value;

    count = &D_80146876;
    value = *count;
    *count = value - 1;
    if (value != 0u) {
      break;
    }
    func_801BE1B0(0);
    D_80146875 = 1u;
    break;
  }
  case 1:
    if (g_ScenarioProgress != 0u) {
      break;
    }
    func_8015B580(D_8014686C, 9);
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0u;
    break;
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
    break;
  case 0xA:
    func_8019FA28(0, 0x5C0000, 0x498000, 0x86);
    D_80146875 = 0xBu;
    break;
  case 0xB:
    if (g_ScenarioProgress != 0xCu) {
      break;
    }
    func_8014ECAC(0);
    D_80146875 = 0xCu;
    break;
  case 0xC:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    func_801C16B8();
    func_80161D20(D_80145029);
    func_801636A0(0, 1);
    D_80146875 = 0xDu;
    break;
  case 0xD:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    func_80161D74(D_80145029);
    D_8014832E = 0x1Fu;
    func_8014ECAC(1);
    D_80146875 = 0xEu;
    break;
  case 0xE: {
    u8 *progress;

    if (D_80143C40 != 0u) {
      break;
    }
    func_8015B580(D_8014686C, 0xB);
    progress = &g_ScenarioProgress;
    D_80146875 = 0xFu;
    *progress = *progress + 1;
    break;
  }
  case 0xF:
    if (g_ScenarioProgress != 0u) {
      break;
    }
    *(u8 *)(D_80148208 + 6) = 5;
    *(u32 *)(D_80148208 + 0x18) = 1;
    *(u32 *)(D_80148208 + 0x1C) = 3;
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0u;
    break;
  }
}
