#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell 0x1F800044, bound by this target's own
 * map row g_battle_work, the name the nearest exact sibling
 * projectWorkRecordPositionAndInstallTransformWithZeroRotation at 0x801EF4D0 in
 * bmagic magic092 keeps for the same word. The original reads it once before
 * the heading call and once after it, and both reads keep one folded
 * `lui $v1,0x1F80` / `lw $v1,0x44($v1)` pair because the call may rewrite the
 * cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared heading helper in the main executable: it turns a signed difference
 * pair into a 12-bit angle. It lives outside this overlay, so the generated
 * undefined-symbol table binds it and the target-local prototype only supplies
 * the argument shape this body uses. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* Shared screen matrix composed under the freshly built work record transform.
 * This target's generated undefined_syms_auto.txt owns the address through its
 * own D_<address> row, so no target-map row is needed. The only access is a
 * matrix base handed to MulMatrix2, so the local declaration keeps the object
 * view. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* @source 0x801EF5C8
 * @behavior Pushes the GTE matrix stack and reads the scratchpad work-record
 *           pointer cell at 0x1F800044 once through this target's map name
 *           g_battle_work. It zeroes the first two elements of a translation
 *           halfword triplet, then asks the shared main-executable helper
 *           func_8017A138 for the 12-bit angle of the work record word 0x10
 *           minus its word 0x38 and its word 0xC minus its word 0x34. The cell
 *           is re-read into a second pointer after that call, because the call
 *           may rewrite it, and the masked angle becomes the triplet third
 *           element. A rotation
 *           triplet is then built from the record: words 0x34 and 0x38
 *           shifted right by nine and biased by minus 0x4000, and half the
 *           negated signed halfword 0x3E. RotTrans projects the rotation
 *           triplet into the translation field of a 0x20-byte scratch matrix
 *           while storing a flag word in the slot
 *           beside it, RotMatrix turns the translation triplet into that same
 *           matrix, MulMatrix2 composes the shared matrix at 0x801492E8 under
 *           it, and the result is published through SetRotMatrix and
 *           SetTransMatrix. Takes no arguments and returns nothing; the
 *           0x50-byte frame keeps $ra at 0x4C and $s0 at 0x48 across the matrix
 *           calls, and the same-payload caller at 0x801EEEB4 invokes it between
 *           its own matrix push and PopMatrix. No byte-identical twin exists:
 *           a scan of every payload under out/binaries found the 0xD8-byte span
 *           only at its own offset, so clone was declined and this source was
 *           seeded from the nearest exact sibling,
 *           projectWorkRecordPositionAndInstallTransformWithZeroRotation at
 *           0x801EF4D0 in bmagic magic092, which supplies the statement order,
 *           operand forms and declaration style of the shared frame, the
 *           zeroed translation prefix and the RotTrans/RotMatrix/MulMatrix2
 *           tail; this span's own bytes supply its heading call, the cell
 *           re-read and the third translation element. Record fields stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */

/* The first measured spelling used one pointer local for both reads of the
 * cell: it matched 39 of 54 instructions with the base in $a1 where the
 * original keeps it in $v0. Giving the re-read its own local matched all 54
 * instructions with no byte difference. */
void projectWorkRecordPositionAndInstallTransformWithHeadingFromWorkWord0CPairBmagicMagic04803_801EF5C8(
    void) {
  u8 *work;
  u8 *position;
  s16 rotation[3];
  s16 translation[3];
  MATRIX object_work;
  long flag;
  s32 heading;

  PushMatrix();

  work = g_battle_work;
  translation[0] = 0;
  translation[1] = 0;
  heading = func_8017A138(*(s32 *)(work + 0x10) - *(s32 *)(work + 0x38),
                          *(s32 *)(work + 0xC) - *(s32 *)(work + 0x34));
  position = g_battle_work;
  translation[2] = heading & 0xFFF;
  rotation[0] = (s16)(*(s32 *)(position + 0x34) >> 9) - 0x4000;
  rotation[1] = (s16)(*(s32 *)(position + 0x38) >> 9) - 0x4000;
  rotation[2] = (s16)(-*(s16 *)(position + 0x3E) / 2);
  RotTrans((SVECTOR *)rotation, (VECTOR *)object_work.t, &flag);
  RotMatrix((SVECTOR *)translation, (MATRIX *)&object_work);
  MulMatrix2(&D_801492E8, (MATRIX *)&object_work);
  SetRotMatrix((MATRIX *)&object_work);
  SetTransMatrix((MATRIX *)&object_work);
}
