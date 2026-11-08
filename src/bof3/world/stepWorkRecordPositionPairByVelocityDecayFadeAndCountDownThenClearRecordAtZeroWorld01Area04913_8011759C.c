#include "bof3/bof3.h"

/* Work-record pointer cell of this overlay. func_801172D0 seeds a record in it,
 * func_801176B8 and func_80117768 publish the record they claim in it, and the
 * record pass func_80117900 walks the sixteen records of the 0x800E4800 table
 * by repointing it. The word is zero in the image and lies inside the Splat
 * textbin row D_8011818C, so this declaration binds through the generated
 * undefined_syms_auto.txt row and owes no target-map row. A store through the
 * record pointer can alias the cell, so the original re-reads it after its
 * stores instead of reusing the pointer of the first read. */
extern u8* D_801181DC; /* @source 0x801181DC @kind data */

/* @source 0x8011759C
 * @behavior Step handler 3 of this overlay's per-record handler run, the
 * thirteen in-image pointers the Splat textbin row D_8011818C places, reachable
 * through dispatchWorkByte1Table818cWorld01Area04913_80117004 when the
 * published work object's byte at offset 1 reads 3. It moves the work record
 * published in D_801181DC by adding the record's word at 0x14 to its word at
 * 0x04 and its word at 0x18 to its word at 0x08: the position pair
 * func_801172D0 seeds from the shared coordinate triple, moved per frame by the
 * random angle-step velocity pair it stores in those two words. Between those
 * two stores it decrements the record fade byte at 0x03, the single value the
 * renderer func_801170D8 passes as the color component of every channel the
 * bits of byte 0x26 select, and it re-reads the pointer cell for the countdown
 * byte at 0x02 that claimWorkRecordSeedWorkPositionWorld01Area04913_80117864
 * seeds to 0x20. When that countdown reaches zero it reads the cell a third time
 * and clears the claim byte at 0x00, so
 * findFreeWorkRecordAt800E4800World01Area04913_801178C4 finds the record free
 * again and the renderer stops drawing it. Takes no arguments, makes no call,
 * and its 0x74 bytes are a leaf whose only return path is the trailing jr $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkRecordPositionPairByVelocityDecayFadeAndCountDownThenClearRecordAtZeroWorld01Area04913_8011759C(
    void) {
  u8* work;
  u8* cur;

  work = D_801181DC;
  *(s32*)(work + 0x04) += *(s32*)(work + 0x14);
  work[3] -= 1;
  cur = D_801181DC;
  *(s32*)(work + 0x08) += *(s32*)(work + 0x18);
  if (--cur[2] == 0) {
    D_801181DC[0] = 0;
  }
}
