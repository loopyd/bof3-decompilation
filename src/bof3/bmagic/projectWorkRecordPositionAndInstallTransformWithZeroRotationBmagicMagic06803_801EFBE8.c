#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work, the name the exact bmagic lifts of this family use for
 * the same shared cell. This body reads the cell once, before any call, so the
 * original materialises one folded lui / lw pair and every later access is a
 * field of the loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built work-record transform.
 * Outside this payload; the target's generated undefined_syms_auto.txt already
 * binds the address as D_801492E8, so no map row is needed and every access is a
 * matrix base handed to MulMatrix2. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EFBE8
 * @behavior Pushes the GTE matrix stack and reads the work record pointer
 *           published at scratchpad cell 0x1F800044 once. It zeroes a halfword
 *           triplet in ascending element order, then fills a position triplet
 *           whose first two elements are work record words 0x34 and 0x38 shifted
 *           right by nine and biased by minus 0x4000 and whose third element is
 *           half the negated signed halfword 0x3E of that same work record.
 *           RotTrans projects the position triplet into the translation field of
 *           a 0x20-byte scratch matrix while storing a flag word in the slot
 *           beside it, RotMatrix turns the zeroed triplet into that matrix
 *           rotation, MulMatrix2 composes the shared matrix at 0x801492E8 under
 *           it, and the result is published through SetRotMatrix and
 *           SetTransMatrix. Takes no arguments, returns nothing, and the
 *           0x50-byte frame keeps $ra at 0x4C and $s0 at 0x48 across the matrix
 *           calls; $s0 holds the scratch matrix base handed to every call after
 *           RotTrans. No jal word in this payload names 0x801EFBE8 and no word of
 *           its data run holds it, so an out-of-payload caller or the overlay
 *           loader reaches it and its roles stay structural. A payload cmp
 *           proved this 0xB0-byte span equal to nine already-lifted bmagic spans,
 *           including projectWorkRecordPositionAndInstallTransformWithZeroRotation
 *           at 0x801F07C8 in bmagic magic078, whose source supplied this body
 *           statement order; record fields and the halfword 0x3E role stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic06803_801EFBE8(void) {
  u8 *work;
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
  rotation[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
