#include "bof3/bof3.h"

extern u8 D_80144DF2; /* @source 0x80144DF2 @kind data */
extern u8 D_80144DF3; /* @source 0x80144DF3 @kind data */
extern u8 D_80144DF4; /* @source 0x80144DF4 @kind data */
extern u8 D_80144DF5; /* @source 0x80144DF5 @kind data */
extern u8 D_80144DF6; /* @source 0x80144DF6 @kind data */
extern u8 D_80144DF7; /* @source 0x80144DF7 @kind data */
extern u8 D_80148338; /* @source 0x80148338 @kind data */
extern u8 D_8014833D; /* @source 0x8014833D @kind data */
extern u8 D_8014835E; /* @source 0x8014835E @kind data */
extern u8 D_80148511; /* @source 0x80148511 @kind data */
extern u8 D_801D8EE0[]; /* @source 0x801D8EE0 @kind data */
extern u8 D_801D8EE4; /* @source 0x801D8EE4 @kind data */

extern u8 func_80165EC0(u8 first, u8 second);

/* @source 0x801D1FA0
 * @behavior Publishes the overlay mask flag and re-seeds the shared-field
 *           snapshot of the BATE (masters) overlay. It reads the two shared mask
 *           bytes 0x8014833D and 0x80148338 into the call func_80165EC0 and
 *           stores zero into the flag byte 0x80148511 when bit zero of the
 *           answer is set and one when it is clear. It then copies the six
 *           shared field bytes 0x80144DF2 through 0x80144DF7 into the six-byte
 *           snapshot table 0x801D8EE0, whose fifth slot is written twice: the
 *           sixth field byte lands on the same byte the fifth wrote, so the
 *           snapshot keeps the sixth value at that offset. Finally it stores the
 *           0x8014833D mask byte into the snapshot table at the index the cell
 *           0x8014835E holds. Nothing is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBateMaskFlagAndSnapshotSharedFields(void) {
  u8 mask;

  mask = D_8014833D;
  if (func_80165EC0(D_80148338, mask) & 1) {
    D_80148511 = 0;
  } else {
    D_80148511 = 1;
  }
  D_801D8EE0[0] = D_80144DF2;
  D_801D8EE0[1] = D_80144DF3;
  D_801D8EE0[2] = D_80144DF4;
  D_801D8EE0[3] = D_80144DF5;
  D_801D8EE0[4] = D_80144DF6;
  D_801D8EE4 = D_80144DF7;
  D_801D8EE0[D_8014835E] = mask;
}
