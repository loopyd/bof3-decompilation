#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80143BB0;
extern u16 D_80143C40;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_8014832E;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end action starter at 0x80150224. */
void func_80150224(u32 arg0);
/* Shared front selection/context request at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared scenario state bit setter at 0x801A78F8. */
void func_801A78F8(void);

/* @source 0x801FA3A4
 * @behavior Word 9 (the word stored at 0x801FAF20) of the ten-entry
 * overlay-local second-level handler run at 0x801FAEFC that the exact
 * dispatcher dispatchSecondaryStateScenarioScena1100_801F7748 (0x801F7748)
 * indexes with the shared primary state byte D_80146874, so a primary state
 * reading 9 selects it, and the address appears in no jal or j of the payload.
 * It switches on the shared secondary sub-state byte D_80146875 through the
 * ten-word absolute jump table jtbl_801F708C_main at 0x801F708C in this
 * payload's leading table data, and the table words of sub-states 2, 3 and 4
 * are the only ones that point at the body exit. Sub-state 0, while the shared
 * front-end mode byte D_80143BB0 (0x80143BB0, read with lbu) is not 2, stores 1
 * in the shared scenario progress byte g_ScenarioProgress (0x80146864), runs the
 * shared GAME.EMI helper func_801BE1B0 (0x801BE1B0) with argument 9 and
 * advances the sub-state byte to 1. Sub-state 1, once the progress byte reads 7,
 * sets flag bit 0x13 of the shared flag word D_8014686C (0x8014686C, read with
 * lw) through the shared flag helper func_8015B580 (0x8015B580), runs the shared
 * overlay release helper func_8015C058 (0x8015C058), clears the progress byte,
 * the shared bytes 0x80146865, 0x80146866 and 0x80146867, the sub-state byte
 * D_80146875 and finally the primary state byte D_80146874 (0x80146874), which
 * ends the chain. Sub-state 5 repeats the sub-state 0 arm with the helper
 * argument 0xA and advances the sub-state byte to 6 instead. Sub-state 6
 * advances to 7 once the progress byte reads 6, sub-state 7 runs the shared
 * front-end mode setter func_8014ECAC (0x8014ECAC) with mode 4 once the progress
 * byte reads 0xF and advances to 8, and sub-state 8, while the shared gate
 * halfword D_80143C40 (0x80143C40, read with lhu) is clear, clears the shared
 * effect bank byte D_8014832E (0x8014832E), queues front-end mode 0x3F through
 * the shared front-end action starter func_80150224 (0x80150224), arms the
 * front-end mode byte D_80143BB0 to 2 and advances to 9. Sub-state 9, once that
 * mode byte leaves 2, clears the sub-state byte and the primary state byte,
 * runs the shared scenario state bit setter func_801A78F8 (0x801A78F8), stores
 * 2 in the shared byte 0x80146866, toggles bits 0x16 of the shared mode halfword
 * D_80146258 (0x80146258, read with lhu) and requests the front selection
 * context (0x79, 0x290000, 0x330000, 7) through the shared front
 * selection/context request func_8019FA28 (0x8019FA28). It takes no arguments
 * and returns nothing. Its 0x18-byte frame keeps $ra and $s0: sub-state 0 stores
 * the constant 1 before the func_801BE1B0 call and again after it, so the
 * materialisation lives in $s0 across the call, and sub-state 1 takes the
 * address of the progress byte into $s0 once and reaches its `lbu` and its
 * `sb` through it, while every other cell materialises the 0x8014 page in $at
 * on its own. Each sub-state ends in its own `j` to the body exit label and the
 * switch bound is the `sltiu` against 10 that sends 10 and above straight
 * there. Its 0x214 bytes have no copy in the tree: an exact-span scan of the
 * 396 extracted EMI payloads under out/binaries finds this 0x214-byte body only
 * at its own offset, so the clone lever was declined and every literal here was
 * written from this span's own bytes. The nearest shape donor is the exact
 * neighbour at 0x801F7BFC, which supplies the address-taken progress-byte
 * pointer, the shared release helper followed by the same clearing tail and the
 * same guarded sub-state 0 arm, and differs in its sub-state set, its case tests
 * and its constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState9SubstateSequenceScenarioScena1100_801FA3A4(void) {
  switch (D_80146875) {
  case 0:
    if (D_80143BB0 == 2u) {
      break;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(9u);
    D_80146875 = 1u;
    break;
  case 1: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 7u) {
      break;
    }
    func_8015B580(D_8014686C, 0x13);
    func_8015C058();
    *progress = 0u;
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
  case 5:
    if (D_80143BB0 == 2u) {
      break;
    }
    g_ScenarioProgress = 1u;
    func_801BE1B0(0xAu);
    D_80146875 = 6u;
    break;
  case 6:
    if (g_ScenarioProgress != 6u) {
      break;
    }
    D_80146875 = 7u;
    break;
  case 7:
    if (g_ScenarioProgress != 0xFu) {
      break;
    }
    func_8014ECAC(4);
    D_80146875 = 8u;
    break;
  case 8:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    func_80150224(0x3Fu);
    D_80143BB0 = 2u;
    D_80146875 = 9u;
    break;
  case 9:
    if (D_80143BB0 == 2u) {
      break;
    }
    D_80146875 = 0u;
    D_80146874 = 0u;
    func_801A78F8();
    D_80146866 = 2u;
    D_80146258 = D_80146258 ^ 0x16u;
    func_8019FA28(0x79u, 0x290000u, 0x330000u, 7u);
    break;
  }
}
