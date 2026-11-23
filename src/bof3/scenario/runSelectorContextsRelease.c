#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u8 D_80143F1D;

/* Shared GAME.EMI mode helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front selector context seed at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F8070
 * @behavior Slot 7 of the 31-word code-pointer run at 0x801FE288, the chain
 *   table dispatchProgressHandlerScenarioScena0200_801F719C indexes with the
 *   signed chain byte D_80146872; its own pointer word sits at 0x801FE2A4 and
 *   no jal or j in the payload targets 0x801F8070, so the overlay reaches this
 *   span as that chain slot. It reads the shared sub-state byte D_80146875
 *   (0x80146875, read with lbu before the frame is grown) and switches it
 *   through the nine-word absolute case table at 0x801F6CCC, bounded by sltiu
 *   against 9, so sub-states 0 through 8 select a case and any larger value
 *   returns through the single epilogue. Sub-states 2, 3 and 4 have no body.
 *   Sub-state 0 stores 1 in the sub-state byte and seeds the front selector
 *   context func_8019FA28(0xF, 0x2C0000, 0x430000, 0x80). Sub-state 1, once
 *   the shared progress byte g_ScenarioProgress (0x80146864) reads 8, raises
 *   mode flag bits 0x6 of the shared halfword D_80146258 (0x80146258, read
 *   with lhu and written with sh), releases the overlay through the shared
 *   release helper func_8015C058, clears D_80146865 (0x80146865), D_80146866,
 *   the sub-state byte and D_80146874 (0x80146874), and sets flag bit 4 of the
 *   shared flag word D_8014686C (0x8014686C, read with lw) through the shared
 *   flag helper func_8015B580. Sub-state 5 runs the shared mode helper
 *   func_801BE1B0 with mode 1 and advances to sub-state 6; sub-state 7 runs the
 *   same helper with mode 2 and advances to 8. Sub-states 6 and 8, once the
 *   progress byte reads 0x22, release the overlay, clear the same four bytes
 *   and then share one tail that sets flag bits 5 and 6 of D_8014686C, toggles
 *   mode flag 0x80 of D_80146258, seeds the front selector context
 *   func_8019FA28(0x1A, 0x310000, 0x4B0000, 0x85) and stores 1 in the front
 *   selector request-kind byte D_80143F1D (0x80143F1D). It takes no arguments
 *   and returns nothing; its 0x18-byte frame keeps only $ra, and every absolute
 *   access materialises the 0x8014 page in $at on its own because the body
 *   keeps no base register across the switch.
 *
 * Name evidence: the span is chain slot 7 of the same 31-entry run that holds
 * the exact siblings
 * runCompletionSubstatesSeeding
 * (0x801F936C at slot 14) and
 * queueFrontEndMode3EThenWaitModeLeave2ReleaseAndClearStateScenarioScena0200_801F8264
 * (0x801F8264 at slot 8), whose own bodies are the adjacent sub-state steppers
 * of this overlay and seed or wait on the same sub-state byte; this body owns
 * the sub-state ladder 0, 1, 5, 6, 7, 8 that seeds two front selector contexts,
 * runs the shared mode helper with modes 1 and 2, raises the mode flag bits and
 * releases the overlay, so the name is target-qualified and clause-based. The
 * progress values 8 and 0x22, the selector seeds, the two cleared-cell orders
 * and the flag bits are this span's own.
 *
 * Boundary and lever evidence: the payload span 0x801F8070-0x801F8263 is one
 * complete function of 0x1F4 bytes that opens on its own frame setup and closes
 * on a single `jr $ra` epilogue at 0x801F825C inside the declared boundary, so
 * it is neither data nor a fragment and the data and carve levers are
 * ineligible. The dispatch table is 9 words, 0x801F6CCC through 0x801F6CEB, and
 * its only reference is this body's own `lw` through %hi/%lo. A scan of every
 * payload under out/binaries found this span at its own offset alone, both
 * byte-exact and with every jal and j word masked in little-endian byte order,
 * and a further pass at opcode/register granularity found no family member, so
 * no byte-identical sibling exists and the clone lever is declined; the
 * statement order, operand forms and cell names were instead taken from the two
 * exact same-target siblings above.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectorContextsRelease(void) {
  switch (D_80146875) {
  case 0:
    D_80146875 = 1;
    func_8019FA28(0xF, 0x2C0000, 0x430000, 0x80);
    break;
  case 1:
    if (g_ScenarioProgress != 8) {
      break;
    }
    D_80146258 |= 6;
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    func_8015B580(D_8014686C, 4);
    break;
  case 5:
    func_801BE1B0(1);
    D_80146875 = 6;
    break;
  case 6:
    if (g_ScenarioProgress != 0x22) {
      break;
    }
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
    func_8015B580(D_8014686C, 5);
    func_8015B580(D_8014686C, 6);
    D_80146258 ^= 0x80;
    func_8019FA28(0x1A, 0x310000, 0x4B0000, 0x85);
    D_80143F1D = 1;
    break;
  case 7:
    func_801BE1B0(2);
    D_80146875 = 8;
    break;
  case 8:
    if (g_ScenarioProgress != 0x22) {
      break;
    }
    func_8015C058();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146874 = 0;
    D_80146875 = 0;
    func_8015B580(D_8014686C, 5);
    func_8015B580(D_8014686C, 6);
    D_80146258 ^= 0x80;
    func_8019FA28(0x1A, 0x310000, 0x4B0000, 0x85);
    D_80143F1D = 1;
    break;
  case 2:
  case 3:
  case 4:
    break;
  }
}
