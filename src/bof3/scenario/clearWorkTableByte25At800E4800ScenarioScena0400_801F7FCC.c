#include "bof3/bof3.h"

/* @source 0x801F7FCC
 * @behavior Overlay work-table clear: it zeroes byte 0x25 of each of the 0x80
 * entries of the 0x28-byte-stride record table at 0x800E4800 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned byte counter in a bottom-tested loop. It takes no
 * arguments, returns nothing and reads no state; the counter is left in
 * $v0/$v1 only because the loop test (masked to 0xFF, compared below 0x80)
 * needs it. Byte 0x25 is the field the record-claiming handlers of this overlay
 * seed with 0x80 (`sb $s3, 0x25($s0)` at 0x801F79D0 in func_801F78F4 and
 * `sb $a0, 0x25($a1)` at 0x801F7B6C in func_801F7B00), so this empties that
 * field for the whole table. Its 44 bytes have the same shape as
 * clearWorkTableAt800E4800ScenarioScena0800_801F7DE8 (0x801F7DE8 in
 * emi/scenario/scena08/00), which walks the same table with the same stride but
 * clears the lead byte of only 0x40 entries; both the count and the cleared
 * offset differ, so this target owns its own address and boundary. Its only
 * in-payload callers are the init handler func_801F77EC (jal at 0x801F78AC,
 * after it has filled the 0x800E5C00-based coordinate table) and the state-0
 * handler at 0x801F7AC8 (jal at 0x801F7AD0).
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    work[0x25] = 0;
    work += 0x28u;
    i += 1u;
  } while (i < 0x80u);
}
