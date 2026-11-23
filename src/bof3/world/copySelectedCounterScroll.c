#include "bof3/world/area03004_internal.h"

/* @source 0x801D28DC
 * @behavior Call func_8014D8D4 with 2 when D_8014421C is negative and with 3 otherwise.
 * Copy the selected record's x, y and unsigned halfword counter to the current
 * work record, then step the pair sequence and update the work scroll words.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The effect request is written as the two per-arm calls the original carries:
 * `bgez` keeps the positive arm's `addiu $a0,$zero,3` in its delay recordIndex and the
 * fall-through sets 2, which one merged `jal` then serves through
 * cross-jumping. A single `effect` local instead hoists `li $a0,3` above the
 * test and fills the delay recordIndex with `sw $ra` (36/39).
 *
 * The record stays addressed by the shared byte index rather than by a record
 * pointer local: one `&D_80146888[recordIndex]` local makes GCC materialise
 * `D_80146888` with `lui`/`addiu` and index from it (`lw $v1,0x34($v0)`, 37/39),
 * while per-access subscripts keep the original's folded
 * `lui $at,%hi(D_801468C0)` / `addu $at,$at,$v0` / `lw $v1,%lo(...)($at)` pair
 * for each of the three copies. The +0x3E counter is typed as the signed
 * countdown for its other readers (`lh` in callActiveUpdateLevels/checkReferenceUnderGates) but this
 * copy site loads it with the original's unsigned `lhu`, so it reads through an
 * unsigned halfword view of the same cell.
 */
void copySelectedCounterScroll(void) {
  u8* work;
  u8 recordIndex;

  if (D_8014421C < 0) {
    func_8014D8D4(2);
  } else {
    func_8014D8D4(3);
  }
  recordIndex = D_801E3208;
  work = D_1F800044;
  *(s32*)(work + 0x34) = D_80146888[recordIndex].x_34;
  *(s32*)(work + 0x38) = D_80146888[recordIndex].y_38;
  *(u16*)(work + 0x3E) = *(u16*)((u8*)&D_80146888[recordIndex] + 0x3E);
  func_8014D978();
  stateWorkScrollWords();
}
