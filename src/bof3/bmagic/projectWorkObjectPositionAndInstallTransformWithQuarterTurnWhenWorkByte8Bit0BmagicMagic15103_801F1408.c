#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map and read once before any call, so the original materializes
 * `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly once while every later access
 * is a field of the loaded pointer. Declaration style follows the exact
 * same-target siblings projectWorkObjectPositionAndInstallTransformWithZero
 * TranslationBmagicMagic15103_801F01BC at 0x801F01BC and the same function in
 * bmagic magic006/03 at 0x801F06E0, which bind the identical cell the same
 * way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built transform. This
 * target's generated undefined_syms_auto.txt row D_801492E8 = 0x801492E8 owns
 * the address, so no target-map row is needed. The only access is a matrix base
 * handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801F1408
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
 *           calls, and the matching PopMatrix sits in the caller, which is the
 *           work byte 2 step wrapper at 0x801F10E8 of this same overlay; it was
 *           reached through a local prototype so the call is ordinary. The
 *           200-byte span was byte-identical to the then exact sibling
 *           projectWorkObjectPositionAndInstallTransformWithQuarterTurnWhen
 *           WorkByte8Bit0BmagicMagic10703_801EFA18 at 0x801EFA18 in bmagic
 *           magic107 - a 200-byte cmp of both spans at their own
 *           (runtime address - load address) offsets, which the clone adapter
 *           repeated as one byte-identical representative - so this body is that
 *           sibling body, and the same-target sibling at 0x801F01BC is this one
 *           minus the guarded quarter turn store. Every literal, passed
 *           constant and symbol binding comes from this span's own bytes; the
 *           body carries the zeroed-then-conditional rotation triplet order,
 *           which is the only place the two siblings differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkObjectPositionAndInstallTransformWithQuarterTurnWhenWorkByte8Bit0BmagicMagic15103_801F1408(
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
