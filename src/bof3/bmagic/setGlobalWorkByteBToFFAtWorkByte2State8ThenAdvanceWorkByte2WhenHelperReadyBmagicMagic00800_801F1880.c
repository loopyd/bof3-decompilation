#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the sibling lift at
 * 0x801F1400 uses: the original re-reads the cell per access and the named
 * symbol lets the assembler fold the constant address into the load
 * (lui $v0,0x1F80 / lw $v0,68($v0)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling lift at 0x801F1400 writes the same record's byte 0xB; splat binds
 * the address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the sibling lifts at 0x801F117C and 0x801F1400 read as a non-zero
 * ready flag. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801F1880
 * @behavior Entry 2, entry 5 and entry 8 of the nine-entry handler table at
 * 0x801EECC8 (0x801F17BC, 0x801F1804, 0x801F1880, 0x801F18F4, 0x801F1804,
 * 0x801F1880, 0x801F18F4, 0x801F1804, 0x801F1880), which 0x801F1708 copies
 * into its own 0x40-byte frame and dispatches on scratchpad work-object byte 2
 * before running the shared main-executable service func_8014D290 when work
 * byte 0 is non-zero. Per frame it calls the shared readiness helper
 * func_8014DAEC and tests its returned byte; while the helper reports not-ready
 * the handler does nothing. Once it reports ready it re-reads the work-object
 * pointer cell at 0x1F800044 and, when the work object state byte 2 still
 * equals 8, stores 0xFF into byte 0xB of the shared battle work record at
 * 0x801EB4E0, the 0xFF constant filling that test's branch delay slot. It then
 * re-reads the pointer cell and increments state byte 2 with a byte-width add,
 * so entry 2 hands off to entry 3 (0x801F18F4), entry 5 to entry 6 (0x801F18F4)
 * and entry 8 past the end of the run. The 0x18-byte frame only keeps $ra at
 * 0x10 across the helper call; nothing is returned and no other state is read
 * or written. One payload comparison of the two extracted 0x74-byte spans
 * showed the sibling lift at 0x801F1400 differed from this one in a single
 * word, the state literal fed to its equality test (0x24020005 there,
 * 0x24020008 here), so this body took that sibling's statement order, operand
 * forms and declarations and wrote this span's own literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setGlobalWorkByteBToFFAtWorkByte2State8ThenAdvanceWorkByte2WhenHelperReadyBmagicMagic00800_801F1880(void) {
  if (func_8014DAEC() != 0u) {
    if (g_battle_work[2] == 8) {
      D_801EB4E0[0xB] = 0xFF;
    }
    g_battle_work[2]++;
  }
}
