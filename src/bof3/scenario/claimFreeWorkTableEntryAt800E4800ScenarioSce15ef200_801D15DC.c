#include "bof3/bof3.h"

/* @source 0x801D15DC
 * @behavior Entry 5 of the overlay handler run at 0x801D1B1C, which the overlay
 * entry func_801D0C04 selects with the unsigned work byte at offset +0x1 of the
 * scratchpad work object published at pointer slot 0x1F800044 and calls with no
 * arguments: it walks the 0x10 entries of the 6-byte-stride record table at
 * 0x800E4800 and claims the first entry whose lead byte reads zero by storing 1
 * to that lead byte and zeroing the entry byte at +0x2, then leaves the loop;
 * when every entry is claimed it returns having written nothing. It reads only
 * each entry lead byte, makes no call, keeps no frame, takes no arguments and
 * returns nothing. The only in-payload caller is the step handler func_801D0E30
 * (jal at 0x801D0E50), entry 5 of that same run, which calls it when the work
 * object countdown byte at +0x9 reads zero and then re-arms that byte with rand
 * masked to 0xF plus 0x18. The claim flag is the lead byte, the same table and
 * stride that clearWorkTableAt800E4800ScenarioSce15ef200_801D15B0 clears and
 * that the world and scenario overlays walk as the main-RAM work table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkTableEntryAt800E4800ScenarioSce15ef200_801D15DC(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);

  for (i = 0u; i < 0x10u; i++, work += 0x6u) {
    if (*work == 0) {
      *work = 1;
      work[2] = 0;
      break;
    }
  }
}
