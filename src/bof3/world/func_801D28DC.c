#include "bof3/world/area03004_internal.h"

/* @source 0x801D28DC
 * @behavior AREA030 mode-5 handler of the local work-record mode table 0x801E1D20
 * (dispatched through scratch work-record byte 1): requests the shared selection
 * effect 3 through func_8014D8D4, or effect 2 when the selected record value
 * D_8014421C is negative, then seeds the scratch work record published at the
 * scratchpad cursor 0x1F800044 from the active 0x98-byte work record
 * D_80146888[D_801E3208]: the x word at +0x34, the y word at +0x38 and the
 * halfword counter at +0x3E; it finally releases the shared hold through
 * func_8014D978 and runs the per-record scroll updater func_801E084C.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The effect request is written as the two per-arm calls the original carries:
 * `bgez` keeps the positive arm's `addiu $a0,$zero,3` in its delay slot and the
 * fall-through sets 2, which one merged `jal` then serves through
 * cross-jumping. A single `effect` local instead hoists `li $a0,3` above the
 * test and fills the delay slot with `sw $ra` (36/39).
 *
 * The record stays addressed by the shared byte index rather than by a record
 * pointer local: one `&D_80146888[slot]` local makes GCC materialise
 * `D_80146888` with `lui`/`addiu` and index from it (`lw $v1,0x34($v0)`, 37/39),
 * while per-access subscripts keep the original's folded
 * `lui $at,%hi(D_801468C0)` / `addu $at,$at,$v0` / `lw $v1,%lo(...)($at)` pair
 * for each of the three copies. The +0x3E counter is typed as the signed
 * countdown for its other readers (`lh` in func_801DE4AC/func_801E0770) but this
 * copy site loads it with the original's unsigned `lhu`, so it reads through an
 * unsigned halfword view of the same cell.
 */
void func_801D28DC(void) {
  u8* work;
  u8 slot;

  if (D_8014421C < 0) {
    func_8014D8D4(2);
  } else {
    func_8014D8D4(3);
  }
  slot = D_801E3208;
  work = D_1F800044;
  *(s32*)(work + 0x34) = D_80146888[slot].x_34;
  *(s32*)(work + 0x38) = D_80146888[slot].y_38;
  *(u16*)(work + 0x3E) = *(u16*)((u8*)&D_80146888[slot] + 0x3E);
  func_8014D978();
  func_801E084C();
}
