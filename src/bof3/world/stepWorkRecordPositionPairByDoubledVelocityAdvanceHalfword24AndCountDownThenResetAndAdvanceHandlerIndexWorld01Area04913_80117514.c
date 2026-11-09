#include "bof3/bof3.h"

/* Work-record pointer cell of this overlay. func_801172D0 seeds a record in it,
 * func_801176B8 and func_80117768 publish the record they claim in it, and the
 * record pass func_80117900 walks the sixteen records of the 0x800E4800 table
 * by repointing it. The word is zero in the image and lies inside the Splat
 * textbin row D_8011818C, so this declaration binds through the generated
 * undefined_syms_auto.txt row and owes no target-map row. A store through any
 * of the record offsets above can alias the cell, so the original re-loads it
 * instead of reusing the pointer of the first read. */
extern u8* D_801181DC; /* @source 0x801181DC @kind data */

/* @source 0x80117514
 * @behavior Step handler 2 of this overlay's per-record handler run: word 2 of
 * the thirteen in-image pointers the Splat textbin row D_8011818C holds at
 * 0x80118194, reached through dispatchWorkByte1Table818cWorld01Area04913_80117004
 * when the work record's byte at offset 1 reads 2, and the counterweight of the
 * adjacent step handler 3 at 0x8011759C. It reads the work record published in
 * D_801181DC once and adds twice its word at 0x14 to its word at 0x04 and twice
 * its word at 0x18 to its word at 0x08: the position pair
 * claimWorkRecordSeedWorkPositionWorld01Area04913_80117864 seeds from the shared
 * coordinate triple, moved per frame by the random angle-step velocity pair those
 * two words hold. It also adds 0x80 to the record halfword at 0x24, the value
 * the renderer func_801170D8 reads twice into the halfword pair it passes on, and
 * the original writes that halfword unconditionally before its test, in the
 * branch delay slot. The bytes at 0x02 and 0x24 are localised only by offset in
 * this payload, so they keep offset roles: byte 0x02 counts down by one and takes
 * 0x40 back when it reaches zero, and on that same frame the original re-loads
 * the cell to store the 0x40 and re-loads it once more to increment the record
 * handler index byte at offset 1, the byte the dispatcher scales by four into the
 * handler table. Takes no arguments, makes no call and returns nothing; its 0x88
 * bytes are a leaf whose only return path is the trailing jr $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordPositionPairByDoubledVelocityAdvanceHalfword24AndCountDownThenResetAndAdvanceHandlerIndexWorld01Area04913_80117514(
    void) {
  u8* work;
  u8 timer;

  work = D_801181DC;
  timer = work[2] - 1;
  *(s32*)(work + 0x04) += *(s32*)(work + 0x14) * 2;
  *(s32*)(work + 0x08) += *(s32*)(work + 0x18) * 2;
  work[2] = timer;
  *(u16*)(work + 0x24) += 0x80;
  if (timer == 0) {
    D_801181DC[2] = 0x40;
    D_801181DC[1]++;
  }
}
