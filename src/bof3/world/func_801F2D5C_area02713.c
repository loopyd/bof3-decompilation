#include "bof3/world/area02713_internal.h"

/* @source 0x801F2D5C
 * @behavior Steps each of the three local 0x98-byte world work records at
 *           0x800E4800 through four projections: every step subtracts 0x400
 *           from the record word at 0x08, subtracts 0x200 from the record word
 *           at 0x10, adds 0x40 to the record halfword at 0x14 and re-projects
 *           the record trail through the local projector 0x801F2E3C, after
 *           which the record's whole trail is emitted through the local
 *           emitTrailStrip. Once all three records are done the scratch-work
 *           byte 9 is counted down and, when it reaches zero, the shared
 *           0x80196070 work-area reset helper runs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2D5C(void) {
  World00Area027Work* work;
  u8 record;
  u8 step;

  work = WORLD00_AREA027_WORK_BASE;
  record = 0u;
  do {
    for (step = 0u; step < 4u; step++) {
      work->unk_08 -= 0x400u;
      work->unk_10 -= 0x200u;
      work->unk_14 += 0x40u;
      func_801F2E3C(work);
    }
    emitTrailStrip(work);
    work = (World00Area027Work*)((u8*)work + 0x98u);
    record += 1u;
  } while (record < 3u);

  if (--WORLD00_AREA027_SCRATCH_PTR[9] == 0) {
    func_80196070();
  }
}
