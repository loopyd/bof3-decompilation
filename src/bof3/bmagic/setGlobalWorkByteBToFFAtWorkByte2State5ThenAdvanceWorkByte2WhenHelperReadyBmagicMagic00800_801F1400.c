#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact sibling
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C uses: the original
 * re-reads the cell per access and the named symbol lets the assembler fold the
 * constant address into the load (lui $v0,0x1F80 / lw $v0,68($v0)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling bmagic lifts (decrementMotionCountdownAndSubtractMotionPairThen
 * AdvanceWorkByte2BmagicMagic00800_801F04DC and
 * subtractWorkColorTripletBy3ThenDecrementWorkByte9SetGlobalWorkByteBToFF
 * AndInvokeHelperWhenZeroBmagicMagic00800_801F08C4) write the same record's
 * byte 0xB; splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the sibling entry 0x801F117C also reads as a non-zero ready flag.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801F1400
 * @behavior Entry 2 and entry 5 of the seven-entry handler run at 0x801EECAC
 * (0x801F133C, 0x801F1384, 0x801F1400, 0x801F1474, 0x801F1384, 0x801F1400,
 * 0x801F16E8), which 0x801F1294 copies into its own 0x38-byte frame and
 * dispatches on scratchpad work-object byte 2 before running the shared
 * main-executable service func_8014D290 when work byte 0 is non-zero. Per frame
 * it calls the shared readiness helper func_8014DAEC and tests its returned
 * byte; while the helper reports not-ready the handler does nothing. Once it
 * reports ready it re-reads the work-object pointer cell at 0x1F800044 and,
 * when the work object state byte 2 still equals 5, stores 0xFF into byte 0xB of
 * the shared battle work record at 0x801EB4E0, the 0xFF constant filling that
 * test's branch delay slot. It then re-reads the pointer cell and increments
 * state byte 2 with a byte-width add, so entry 2 hands off to entry 3
 * (0x801F1474) and entry 5 to entry 6 (0x801F16E8). The 0x18-byte frame only
 * keeps $ra at 0x10 across the helper call; nothing is returned and no other
 * state is read or written. The body matches the sibling entry at 0x801F117C
 * except for the guarded record store and the branch displacement it adds, and
 * one scan of the extracted payloads found no byte-identical copy of this span
 * in any target.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setGlobalWorkByteBToFFAtWorkByte2State5ThenAdvanceWorkByte2WhenHelperReadyBmagicMagic00800_801F1400(void) {
  if (func_8014DAEC() != 0u) {
    if (g_battle_work[2] == 5) {
      D_801EB4E0[0xB] = 0xFF;
    }
    g_battle_work[2]++;
  }
}
