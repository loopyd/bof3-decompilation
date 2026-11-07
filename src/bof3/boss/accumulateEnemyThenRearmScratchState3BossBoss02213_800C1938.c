#include "bof3/bof3.h"

/* Work-object pointer cell in scratchpad. The original takes the pointer into a
 * register before the record flag store and re-reads the cell after each of the
 * byte stores that follow, so the source names a local for the first access and
 * evaluates the cell again at every later one. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared main-program readiness helper; its result is unused here. */
void func_8014DAEC(void); /* @source 0x8014DAEC */

/* Shared main-program enemy accumulation pass: it raises scratch work flag 0x40,
 * folds the current enemy work halfwords into the global accumulators, submits
 * the scratch work and leaves 2/0/0 in scratch work bytes +1..+3. */
void accumulateEnemyHalfwordsAndResetScratch(void); /* @source 0x801E54EC */

/* @source 0x800C1938
 * @behavior Battle-step handler of this overlay, referenced only from the
 * handler run that starts at 0x800C1B2C of this payload's 0x800C1B18 data row
 * and reached by no jal in the payload, so the prologue and return path alone
 * place its boundary. It runs the shared readiness helper at 0x8014DAEC, then
 * the shared enemy accumulation pass at 0x801E54EC, and overrides the scratch
 * work re-arm that pass writes: it raises bit 0x1000 of word +0x100 of the
 * current enemy work record published at 0x801EB4E8, clears bit 0x40 -- the
 * accumulate flag the pass raises -- of byte 0 of the work object held by the
 * scratchpad pointer cell 0x1F800044, and writes 3, 0, 0 into that object's
 * bytes 1, 2 and 3 where the pass leaves 2, 0, 0. Roles of the 0x1000 bit and of
 * the remaining scratch work bytes are not established here. The original takes
 * the 0x1F800044 pointer into a local before the flag store and re-reads the
 * cell for each of the three byte stores after it, so the source does the same.
 * It takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void accumulateEnemyThenRearmScratchState3BossBoss02213_800C1938(void) {
  u8 *work;
  u8 *spad;

  func_8014DAEC();
  accumulateEnemyHalfwordsAndResetScratch();
  work = D_801EB4E8;
  spad = D_1F800044;
  FIELD_REF(u32, work, 0x100u) |= 0x1000u;
  spad[0] &= 0xBFu;
  D_1F800044[1] = 3u;
  D_1F800044[2] = 0u;
  D_1F800044[3] = 0u;
}
