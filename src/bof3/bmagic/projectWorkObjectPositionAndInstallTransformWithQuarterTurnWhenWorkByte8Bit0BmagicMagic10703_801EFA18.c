#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, read once before the
 * transform is built; every later field access goes through the loaded
 * pointer, so the cell is materialized exactly once (`lui $v1,0x1F80` /
 * `lw $v1,0x44($v1)`). Declaration style follows the exact sibling
 * projectWorkObjectPositionAndInstallTransformWithZeroTranslation (0x801F06E0,
 * bmagic magic006/03), whose last 44 instructions matched this span word for
 * word when this seed was written. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built transform. This
 * target's generated undefined_syms_auto.txt row D_801492E8 = 0x801492E8 owns
 * the address, so no target-map row is needed. The only access is a matrix base
 * handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EFA18
 * @behavior Pushes the GTE matrix stack and reads the scratchpad work-object
 *           pointer cell at 0x1F800044 exactly once. It zeroes a three halfword
 *           rotation triplet and, only when unsigned work byte 0x08 has bit 0
 *           set, replaces the triplet third element with 0x400, a quarter turn
 *           of the 0x1000 angle circle. It then fills a three halfword position
 *           triplet from the work object: words 0x34 and 0x38 shifted right by
 *           nine and biased by minus 0x4000, and half the negated signed
 *           halfword 0x3E taken as a truncating signed division. RotTrans
 *           projects that position triplet into the translation field of a
 *           0x20-byte scratch matrix while storing a flag word beside it,
 *           RotMatrix builds the same matrix from the zeroed or quarter turned
 *           rotation triplet, MulMatrix2 composes the shared matrix at
 *           0x801492E8 under it, and the result is published through
 *           SetRotMatrix and SetTransMatrix. Takes no arguments and returns
 *           nothing; the 0x50-byte frame keeps $ra and $s0 across the matrix
 *           calls, and the matching PopMatrix sits in the caller, which no jal
 *           and no in-image word of this payload names. The 200-byte span was
 *           byte-identical to func_801F1408 in bmagic magic151/03, whose own
 *           row was still asm when this seed was written and so held no source
 *           to clone; the 0x801F06E0 body of bmagic magic006/03 was this span
 *           minus the guarded quarter turn store, so that exact sibling
 *           supplied the statement order, operand forms and declaration style
 *           while every literal here comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkObjectPositionAndInstallTransformWithQuarterTurnWhenWorkByte8Bit0BmagicMagic10703_801EFA18(
    void) {
  u8 *work;
  s16 position[3];
  s16 rotation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  rotation[0] = 0;
  rotation[1] = 0;
  rotation[2] = 0;
  if (work[0x08] & 1) {
    rotation[2] = 0x400;
  }
  position[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  position[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  position[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)position, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)rotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
