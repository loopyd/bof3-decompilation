#include "bof3/bof3.h"

/* @source 0x801D25C0
 * @behavior Overlay init helper: clears the lead byte of each of the 16
 * entries of the 6-byte-stride record table at 0x800E60A0 in main RAM,
 * walking the table with an unsigned byte counter in a bottom-tested loop. It
 * takes no arguments, returns nothing and touches no other state.
 * The only in-payload caller is the work-state handler at 0x801D15CC (entry 16
 * of the handler table at 0x801D27CC), which stores a 0x12C word at +0x2E of
 * the scratchpad work object, clears its +0x9 byte, empties this table and then
 * advances the work byte at +0x1; the table's lead byte is the active flag read
 * by the sweep at 0x801D2634, which walks the same 16 6-byte entries and calls
 * func_801D2424 for each active one.
 * The 44-byte instruction shape (bottom-tested stride clear with an unsigned
 * byte counter) is already exact in this tree as
 * clearWorkTableAt800E4800ScenarioScena0100_801F7AB8 (0x801F7AB8, base
 * 0x800E4800, stride 0x14, 0x80 entries) and its scena15/00 twin at
 * 0x801F75AC; only the base, stride and count immediates differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E60A0ScenarioSce15ef300_801D25C0(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E60A0u);
  i = 0u;

  do {
    *work = 0;
    work += 6u;
    i += 1u;
  } while (i < 16u);
}
