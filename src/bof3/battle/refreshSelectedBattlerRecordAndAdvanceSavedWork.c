#include "bof3/battle/battle03_internal.h"

/* Shared main-executable resource request helper, outside this payload; the
 * sibling battle lifts declare the same prototype and pass a resource id.
 */
void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* Shared main-executable palette block row helper, outside this payload; it
 * receives the 0x80035600 palette block base plus the scaled work byte and
 * zero as its second argument. Its own role is not established here.
 */
void func_80166C48(u8* row, u32 flag); /* @source 0x80166C48 */

/* Shared main-executable palette upload helper, outside this payload; the
 * sibling lift func_80196B9C marks the current palette range's source colors
 * as high-bit colors.
 */
void func_80196B9C(void); /* @source 0x80196B9C */

/* @source 0x801E7634
 * @behavior Handler 3 of the five-entry battle-mode table at 0x801D0F58, which
 *           dispatchModeFiveTable indexes with scratchpad work byte +0x01. While
 *           the loader is not ready the body returns with no side effect.
 *           Otherwise it publishes the work record at D_801EB4E0 as the current
 *           scratchpad work, keeps the previous pointer cell value in a saved
 *           local, and names the local work record selected by work byte +0x05
 *           through the pointer cell 0x80146250. It then passes work byte +0x08
 *           plus four to the shared resource request helper func_8014D6B8,
 *           submits the selected D_80145E90 record through func_80196718,
 *           passes the 0x80035600 palette block row at (work byte +0x05 << 6) to
 *           func_80166C48 with mode zero, submits the +0x80 halfword of the
 *           selected record as the positional effect mask, and uploads the
 *           palette through func_80196B9C. It then clears bit 0 of the +0x128
 *           word of the record named by 0x80146250 and rebuilds the local work
 *           bands from the field records, restores the previous scratchpad work
 *           pointer, and advances byte +0x01 of that restored work, the byte the
 *           mode table reads next. The three pointer-cell reads after the calls
 *           are real reloads of the shared cell, so the source reads 0x1F800044
 *           again at each site instead of keeping a local. The body takes no
 *           arguments, returns nothing, and the 0x20-byte frame keeps $ra and
 *           the two callee-saved pointers across the calls. The roles of work
 *           byte +0x08, of the requested resource and of the cleared +0x128 bit
 *           are not established here, so the name describes the observed steps.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshSelectedBattlerRecordAndAdvanceSavedWork(void) {
  Battle03LocalWork* work;
  Battle03LocalWork* saved;

  if (func_80162D00() == 0) {
    return;
  }
  work = D_801EB4E0;
  saved = D_1F800044;
  D_1F800044 = work;
  D_80146250 = &D_80145E90[work->unk_05];
  func_8014D6B8((u8)(work->unk_08 + 4));
  func_80196718(&D_80145E90[D_1F800044->unk_05]);
  func_80166C48((u8*)(0x80035600u + (D_1F800044->unk_05 << 6)), 0);
  submitPositionalEffectBit80(D_80145E90[D_1F800044->unk_05].unk_80);
  func_80196B9C();
  D_80146250->unk_128 &= ~1u;
  rebuildLocalWorkBandsFromFieldRecords();
  D_1F800044 = saved;
  saved->unk_01++;
}
