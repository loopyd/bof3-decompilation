#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044, bound by this target's own
 * map row g_battle_work. The original reads it once, after PushMatrix and
 * before any record field, so it materializes a single `lui $v1,0x1F80` /
 * `lw $v1,0x44($v1)` pair and every later access is a field of the loaded
 * pointer. Declaration matches the exact same-target siblings that read the
 * same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared screen matrix composed under the freshly built transform. This
 * target's generated undefined_syms_auto.txt row owns the address, so no
 * target-map row is needed. The only access is a matrix base handed to
 * MulMatrix2, so the local declaration keeps the object view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF5C0
 * @behavior Installs the work record's transform for this overlay's draw pass:
 *           the only in-image references to it are the jal words at 0x801EF254
 *           and 0x801EF45C, in the two byte 2 dispatchers, so it takes no
 *           arguments, returns nothing and reads no entry register. It pushes
 *           the GTE matrix stack with PushMatrix inside a 0x50-byte frame that
 *           keeps $ra at 0x4C and $s0 at 0x48, then loads the scratchpad work
 *           record pointer cell at 0x1F800044 once and zeroes the halfword
 *           triplet at 0x18..0x1D. The three halfwords at 0x10..0x15 are built
 *           from that record: word 0x34 and word 0x38 each shifted right by
 *           nine and biased by minus 0x4000, and half the negated signed
 *           halfword 0x3E, whose negation and halving carry the signed
 *           divide-by-two fixup sequence (negu / srl 31 / addu / sra 1) rather
 *           than an arithmetic shift. RotTrans projects that triplet through
 *           the matrices the caller left installed, writing the result vector
 *           into the translation field of the 0x20-byte scratch matrix at
 *           0x20 and a flag word at 0x40; RotMatrix turns the zeroed triplet
 *           into an identity rotation in that same matrix; MulMatrix2 composes
 *           the shared screen matrix at 0x801492E8 under it; and the composed
 *           matrix is published through SetRotMatrix and SetTransMatrix. It
 *           pops no matrix of its own: both callers run PopMatrix after their
 *           draw pass, so the pushed entry belongs to them. The two record
 *           field roles stay offset-anchored.
 *           Shape: bytes 0x801EF5C0..0x801EF66F compare equal to the span at
 *           0x801EF4D0 of bmagic magic092, which was lifted exact as
 *           projectWorkRecordPositionAndInstallTransformWithZeroRotation, and
 *           the clone dry run reported the same 176 bytes, so this body is
 *           that sibling's transcription. The sibling carries the same three
 *           record reads, the same zeroed triplet and the same shared matrix
 *           address, so this file stays self-contained and owns no declaration
 *           with it; the scratchpad cell takes this target's own map name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void installZeroSpinMatrix(
    void) {
  u8 *work;
  s16 position[3];
  s16 zero_rotation[3];
  MATRIX object_work;
  long flag;

  PushMatrix();

  work = g_battle_work;
  zero_rotation[0] = 0;
  zero_rotation[1] = 0;
  zero_rotation[2] = 0;
  position[0] = (s16)(*(s32 *)(work + 0x34) >> 9) - 0x4000;
  position[1] = (s16)(*(s32 *)(work + 0x38) >> 9) - 0x4000;
  position[2] = (s16)(-*(s16 *)(work + 0x3E) / 2);
  RotTrans((SVECTOR *)position, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)zero_rotation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
