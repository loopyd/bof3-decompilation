#include "bof3/bof3.h"

/* @source 0x801F39AC
 * @behavior Overlay helper: writes zero to the +0xC halfword of each of the
 *           0x80 records of the 0xE-byte-stride record table at 0x800E4800
 *           (the main-RAM work area this overlay shares with the world and
 *           scenario overlays), walking the table with an unsigned byte counter
 *           in a bottom-tested loop; it takes no arguments, returns nothing and
 *           reads no state. The field it clears is that record's countdown: the
 *           overlay's own ticker func_801F39D8 tests the same +0xC halfword of
 *           every 0xE-stride record with `lh`/`beqz` and decrements it, the
 *           free-record scan func_801F3A9C returns the first record whose +0xC
 *           counts zero, and the re-arm helper func_801F3D54 stores 0x20 there
 *           (`sh` at 0x801F3DD0), so this walk empties the whole pool. Its only
 *           in-payload caller is the phase-exit state handler func_801F2E98
 *           (jal at 0x801F2F14), which runs it once its own frame counter
 *           expires. The 44 bytes have the same shape as the already-exact
 *           clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC in
 *           emi/scenario/scena04/00, which walks its table with the same masked
 *           byte counter in the same order and differs only in the stored field
 *           offset, the store width and the stride; this target owns its own
 *           address and boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkRecordCountdownsAt800E4800World01Area06714_801F39AC(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    FIELD_REF(u16, work, 0xCu) = 0;
    work += 0xEu;
    i += 1u;
  } while (i < 0x80u);
}
