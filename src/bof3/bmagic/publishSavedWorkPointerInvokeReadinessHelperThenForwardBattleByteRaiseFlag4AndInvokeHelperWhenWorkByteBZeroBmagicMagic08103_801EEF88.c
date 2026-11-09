#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body reads the cell once before publishing and stores
 * the saved value back once after the helper call, and the original
 * re-materialises the cell address with its own lui for each of the two stores,
 * so the cell is reached through the named pointer rather than a folded
 * constant base. The plain declaration is the one the exact siblings
 * publishSavedWorkPointerAdvancePreviousHandlerIndexWhenHelperReadyBmagicMagic
 * 08103_801EEE74 and decrementGlobalWorkByteBAndClearWorkBytes0Through4Bmagic
 * Magic08103_801EF614 use. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. Word 0x74 of
 * that record is the saved work-object pointer: the siblings
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAnd
 * AdvanceHandlerIndexBmagicMagic08103_801EEEDC and
 * reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThen
 * AdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C write the
 * published work object into exactly that word, and this function takes it
 * back out. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Shared readiness helper of the shared EXE image at 0x8014DAEC, outside this
 * overlay's image. The bmagic lifts declare the same zero-extended byte
 * prototype and Splat binds the address through this target's generated
 * undefined_funcs_auto.txt. This function calls the helper for its effect only:
 * the returned byte is overwritten before it is read, so the readiness role
 * stays with the sibling publishSavedWorkPointerAdvancePreviousHandlerIndex
 * WhenHelperReadyBmagicMagic08103_801EEE74 and the result is discarded. */
extern u8 func_8014DAEC(void);

/* Shared helper of the shared EXE image at 0x800A9820, outside this overlay's
 * image and bound through this target's generated undefined_funcs_auto.txt. It
 * consumes the zero-extended shared battle byte as its only argument, as the
 * bmagic forwardBattleByteThenRaiseFlag4AndInvokeHelper family does. */
void func_800A9820(u8 arg0);

/* Shared helper of the shared EXE image at 0x801E5988, outside this overlay's
 * image and bound through this target's generated undefined_funcs_auto.txt. It
 * is invoked with no arguments. */
void func_801E5988(void);

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. Bit 0x4 is raised after the
 * battle byte is forwarded, through an unsigned halfword load, an ori and a
 * halfword store. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF88
 * @behavior Republishes the battle record's saved work-object pointer through
 *           the scratchpad work-object pointer cell at 0x1F800044, invokes the
 *           shared readiness helper and restores the cell, then forwards the
 *           shared battle byte and raises the battle flag only when the work
 *           object's byte 0xB is zero. The publish half is the sibling
 *           publishSavedWorkPointerAdvancePreviousHandlerIndexWhenHelperReady
 *           BmagicMagic08103_801EEE74: read the shared battle record pointer
 *           D_801EC2E0, take that record's word 0x74 - its saved work-object
 *           pointer - and store it at the scratchpad cell while keeping the
 *           cell's previous value in a callee-saved register. Unlike that
 *           sibling it does not test the readiness helper's result; the cell is
 *           restored immediately after the call and the saved pointer is then
 *           tested itself. The third basic block is the exact shape of the
 *           in-target sibling forwardBattleByteThenRaiseFlag4AndInvokeHelper
 *           WhenWorkByteBZeroBmagicMagic10603_801EEE80: when byte 0xB of the
 *           published work object is zero it loads the shared battle byte at
 *           0x80146384 unsigned and passes it to the shared helper
 *           func_800A9820, raises bit 0x4 of the shared battle global halfword
 *           at 0x801462E8 and invokes the shared helper func_801E5988; a
 *           non-zero byte 0xB skips the block. Takes no arguments and returns
 *           nothing; the 0x18-byte frame keeps ra at 0x14 and the saved cell
 *           value at 0x10 across the two calls, and no other state is read or
 *           written. The handler dispatch of this overlay is not touched here,
 *           so the name reports only the publish, the readiness call and the
 *           conditional forward-and-raise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSavedWorkPointerInvokeReadinessHelperThenForwardBattleByteRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic08103_801EEF88(
    void) {
  u8 *saved;

  saved = g_battle_work;
  g_battle_work = *(u8 **)(D_801EC2E0 + 0x74);
  func_8014DAEC();
  g_battle_work = saved;
  if (saved[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
