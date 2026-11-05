#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. Declared as the same volatile u8 pointer the exact sibling
 * subtractWorkColorTripletBy3ThenAdvanceWorkByte2WhenChannelIs80
 * BmagicMagic00800_801F11C4 uses: the original re-reads the cell per access and
 * the named symbol lets the assembler fold the constant address into the load
 * (lui $v1,0x1F80 / lw $v1,68($v1)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the exact main-executable lifts (clearWorkFlagsWhenReady,
 * stepWorkSequenceAndClearByte12BOnNonzero) read as a non-zero ready flag.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801F117C
 * @behavior Entry 3 (index 3) of the six-entry pointer table at 0x801EEC94
 * (0x801F0E54, 0x801F0FA4, 0x801F1090, 0x801F117C, 0x801F11C4, 0x801F1274),
 * which 0x801F0DA8 copies into its frame and dispatches on scratchpad
 * work-object byte 2. Per frame it calls the shared readiness helper
 * func_8014DAEC and tests its returned byte; while the helper reports
 * not-ready the handler does nothing, and once it reports ready the handler
 * re-reads the work-object pointer cell at 0x1F800044 and increments the work
 * object's state byte 2 with a byte-width add (lbu / addiu 1 / sb), advancing
 * dispatch to table entry 4 (0x801F11C4). The function has a 0x18-byte frame
 * holding the saved return address at 0x10, reads or writes no other state, and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C(void) {
  if (func_8014DAEC() != 0u) {
    g_battle_work[2]++;
  }
}
