#include "bof3/bof3.h"

/* @source 0x801F9DA8
 * @behavior Clears the lead byte of each of the 0x40 entries of the
 * 0x20-byte-stride table at 0x800E515C, walking the table with an unsigned byte
 * counter in a bottom-tested loop. The sibling scan at 0x801F9DD4 reads the
 * same 0x40-entry, 0x20-byte-stride table (it returns the address of the first
 * entry whose lead byte is still clear, or zero when the pool is full), so this
 * empties the whole pool. It takes no arguments, returns nothing, and touches
 * no other state; the incremented counter is left in $v0/$v1 only because the
 * loop test needs it.
 * The 44-byte instruction shape (bottom-tested stride clear with an unsigned
 * byte counter) is already exact in this tree as
 * clearWorkTableAt800E4800ScenarioScena0100_801F7AB8 (0x801F7AB8: base
 * 0x800E4800, stride 0x14, 0x80 entries), its scena08/00 twins, and the
 * scena15/00 twin at 0x801F75AC; only the base, stride and count immediates
 * differ. The only in-payload reference in the tracked disassembly is the jal
 * at 0x801F7434 inside func_801F7408.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E515CScenarioScena0700_801F9DA8(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E515Cu);
  i = 0u;

  do {
    *work = 0;
    work += 0x20u;
    i += 1u;
  } while (i < 0x40u);
}
