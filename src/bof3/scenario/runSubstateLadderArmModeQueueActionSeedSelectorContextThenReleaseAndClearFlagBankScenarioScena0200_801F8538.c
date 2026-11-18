#include "bof3/bof3.h"

extern u16 D_80143C40;
extern u8 D_80143BB0;
extern u8 D_80143F1F;
extern u16 D_80146258;
extern u8 g_ScenarioProgress;
extern u8 D_80146866;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;

/* Shared action dispatcher at 0x80150224. */
void func_80150224(s32 action);
/* Shared front-end flag setter at 0x8015B580. */
void func_8015B580(u32 flags, s32 bit);
/* Shared front-end flag clearer at 0x8015B5A8. */
void func_8015B5A8(u32 flags, s32 bit);
/* Shared front-end flag tester at 0x8015B5D4. */
s32 func_8015B5D4(u32 flags, s32 bit);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front selector context seed at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared GAME.EMI mode helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);

/* @source 0x801F8538
 * @behavior Slot 10 of the 31-word code-pointer run at 0x801FE288, the chain
 *   table dispatchProgressHandlerScenarioScena0200_801F719C indexes with the
 *   signed chain byte D_80146872; its own pointer word sits at 0x801FE2B0 and
 *   no jal or j in the payload targets 0x801F8538, so the overlay reaches this
 *   span as that chain slot. It reads the shared sub-state byte D_80146875
 *   (0x80146875, read with lbu before the frame is grown) and switches it
 *   through the five-word absolute case table at 0x801F6D0C, bounded by sltiu
 *   against 5, so sub-states 0 through 4 each select a case and any larger
 *   value returns through the single epilogue; every case has a body.
 *   Sub-state 0 stores 5 in the shared selector byte D_80146866 (0x80146866),
 *   runs the shared mode helper func_801BE1B0 with mode 6 and advances to
 *   sub-state 1. Sub-state 1 waits for the shared progress byte
 *   g_ScenarioProgress (0x80146864, read with lbu) to read 0x21, then asks the
 *   shared front-end mode setter func_8014ECAC for mode 0xD and advances to
 *   sub-state 2. Sub-state 2 waits for the shared gate halfword D_80143C40
 *   (0x80143C40, read with lhu) to read zero, clears the shared phase byte
 *   D_8014832E (0x8014832E), dispatches action 0x16 through the shared action
 *   dispatcher func_80150224, stores 2 in the shared front-end local mode byte
 *   D_80143BB0 (0x80143BB0) and advances to sub-state 3. Sub-state 3 waits for
 *   that mode byte to stop reading 2, then clears D_80146866 and the progress
 *   byte, advances to sub-state 4, seeds the front selector context
 *   func_8019FA28(0xF, 0x260000, 0x190000, 0x86), stores 0x11 in the front
 *   selector request-kind byte D_80143F1F (0x80143F1F) and raises mode flag
 *   0x80 of the shared halfword D_80146258 (0x80146258, read with lhu and
 *   written with sh). Sub-state 4 releases the overlay through the shared
 *   release helper func_8015C058, clears the sub-state byte and the shared byte
 *   D_80146874 (0x80146874), clears flag 0x24 of the shared flag word
 *   D_8014686C (0x8014686C, read with lw) through the shared flag clearer
 *   func_8015B5A8, and then, only when the shared flag tester func_8015B5D4
 *   reports flag 7 of that word set, clears flags 0x2B, 0x2C, 0x3E, 0x35, 9,
 *   0xA and 0xB of the same word through func_8015B5A8 and sets flag 0x34
 *   through the shared flag setter func_8015B580; its last call falls straight
 *   into the epilogue. It takes no arguments and returns nothing; its
 *   0x18-byte frame keeps only $ra, and every absolute access materialises its
 *   own page in $at because the body keeps no base register across the switch.
 *
 * Name evidence: the span is chain slot 10 of the same 31-entry run that holds
 * the exact siblings
 * runSubstateSequenceSeedSelectorContextsArmModeFlagsAndReleaseScenarioScena0200_801F8070
 * (0x801F8070 at slot 7) and
 * queueFrontEndMode3EThenWaitModeLeave2ReleaseAndClearStateScenarioScena0200_801F8264
 * (0x801F8264 at slot 8), whose bodies step adjacent values of the same
 * sub-state byte and share the same mode helper, action dispatcher, front
 * selector seed and overlay release helper; this body owns the sub-state ladder
 * 0 through 4 that arms shared mode 6 through the mode helper, waits the
 * progress byte and mode byte, dispatches action 0x16, seeds one front selector
 * context and finally releases the overlay and clears a flag bank, so the name
 * is target-qualified and clause-based. The progress value 0x21, the action
 * 0x16, the selector seeds, the mode and flag bits and the cleared byte order
 * are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801F8538-0x801F873F is one
 * complete function of 0x208 bytes that opens on its own frame setup and closes
 * on a single jr $ra epilogue at 0x801F8738 inside the declared boundary, so it
 * is neither data nor a fragment and the data and carve levers are ineligible.
 * The dispatch table is 5 words, 0x801F6D0C through 0x801F6D1F, and its only
 * reference is this body's own lw through %hi/%lo, declared as a rodata
 * placement for this function. A scan of every payload under out/binaries found
 * this span at its own offset alone, both byte-exact and with every jal and j
 * word masked in little-endian byte order, so no byte-identical sibling exists
 * and the clone lever is declined; the statement order, operand forms and cell
 * names were instead taken from the exact same-target siblings above, whose
 * cell spellings this target's shared map already binds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSubstateLadderArmModeQueueActionSeedSelectorContextThenReleaseAndClearFlagBankScenarioScena0200_801F8538(void) {
  switch (D_80146875) {
  case 0:
    D_80146866 = 5;
    func_801BE1B0(6);
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress != 0x21) {
      break;
    }
    func_8014ECAC(0xD);
    D_80146875 = 2;
    break;
  case 2:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    func_80150224(0x16);
    D_80143BB0 = 2;
    D_80146875 = 3;
    break;
  case 3:
    if (D_80143BB0 == 2) {
      break;
    }
    D_80146866 = 0;
    g_ScenarioProgress = 0;
    D_80146875 = 4;
    func_8019FA28(0xF, 0x260000, 0x190000, 0x86);
    D_80143F1F = 0x11;
    D_80146258 |= 0x80;
    break;
  case 4:
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    func_8015B5A8(D_8014686C, 0x24);
    if (func_8015B5D4(D_8014686C, 7) == 0) {
      break;
    }
    func_8015B5A8(D_8014686C, 0x2B);
    func_8015B5A8(D_8014686C, 0x2C);
    func_8015B5A8(D_8014686C, 0x3E);
    func_8015B580(D_8014686C, 0x34);
    func_8015B5A8(D_8014686C, 0x35);
    func_8015B5A8(D_8014686C, 9);
    func_8015B5A8(D_8014686C, 0xA);
    func_8015B5A8(D_8014686C, 0xB);
    break;
  }
}
