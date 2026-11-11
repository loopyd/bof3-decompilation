#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044, bound by this target's own map
 * row g_battle_work, the name this target's exact lifts
 * reserveFirstFreeRecordBmagicMagic21903_801EF934 and
 * decrementWorkByteAAndCountDownByte9ThenDecrementRecordByteBAndClearAtZeroBmagicMagic21903_801EF5DC
 * already use for it. This body reads the cell once, before any call, so the
 * original materializes one folded `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair and
 * every later access is a field of the loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay's image. This
 * target's generated undefined_syms_auto.txt already binds the address under its
 * own D_<address> form, so the local declaration needs no map row. The halfword
 * 0x3E it supplies is the record field the exact bmagic siblings copy into the
 * work record before projecting it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared screen matrix composed under the freshly built work record transform.
 * The target's generated undefined_syms_auto.txt owns the address through the
 * same D_<address> row, so no map row is needed. Every access is a matrix base
 * handed to MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF664
 * @behavior Pushes the GTE matrix stack and reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 once. It zeroes a translation
 *           halfword triplet in ascending element order, then builds a rotation
 *           triplet whose first two elements are work record words 0x34 and 0x38
 *           shifted right by nine and biased by minus 0x4000 and whose third
 *           element is half the negated signed halfword 0x3E of the shared
 *           battle record read through the main-RAM pointer cell 0x801EB4E0.
 *           RotTrans projects that triplet into the translation field of a
 *           0x20-byte scratch matrix while storing a flag word in the slot beside
 *           it, RotMatrix turns the zeroed triplet into an identity rotation in
 *           that same matrix, MulMatrix2 composes the shared matrix at
 *           0x801492E8 under it, and the result is published through SetRotMatrix
 *           and SetTransMatrix. Takes no arguments, returns nothing, and the
 *           0x50-byte frame keeps $ra at 0x4C and $s0 at 0x48 across the matrix
 *           calls. No jal word in the payload names 0x801EF664 and no payload
 *           table word holds it, so an out-of-payload caller or the overlay
 *           loader reaches it and its roles stay structural. The 0xB8-byte span
 *           is byte-identical to the rows still on asm at 0x801EF728 in bmagic
 *           magic053, 0x801EF938 in bmagic magic220 and 0x801EF75C in bmagic
 *           magic226, which carry no source to clone from, and it differs from
 *           the exact sibling
 *           projectWorkRecordPositionAndInstallTransformWithSharedRecordHalfword3E
 *           at 0x801EFB90 in bmagic magic046 in exactly the two words of that
 *           triplets store order, so the sibling supplied this body's statement
 *           order while this span's own bytes supplied the ascending order.
 *           Record fields and halfword 0x3E stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithSharedRecordHalfword3EBmagicMagic21903_801EF664(
    void) {
  u8 *work;
  u8 *record;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0;
  rotation[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  record = D_801EB4E0;
  rotation[2] = (s16)(-*(s16 *)(record + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
