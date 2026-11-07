#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This function reads it once (lui $a1,0x1F80 / lw $a1,68($a1)) and
 * keeps the loaded pointer for its two record reads. The map row of this
 * target binds the same cell as g_battle_work; the overlay's own
 * clearWorkBytesBmagicMagic05803_801EFE9C lift uses that spelling. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword at the base of the block the dispatchers of this overlay
 * read. This function loads it once with lh after both horizontal products, so
 * the cell is declared signed here at the s16 width that load shows; other
 * lifts read other widths of the same address at their own use sites, and
 * nothing in this overlay stores it. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind unknown */

/* @source 0x801EFDD4
 * @behavior Radial proximity predicate between the position record passed in
 *           $a0 and the work object published through the scratchpad pointer
 *           cell at 0x1F800044. Both objects' words at 0x34 and 0x38 are
 *           converted from the 0x200 fixed-point unit by an arithmetic shift
 *           of nine, the two differences are squared and summed into the
 *           horizontal distance, the signed scratchpad halfword at 0x1F800000
 *           is squared into the radius, and the function returns 1 when the
 *           distance is at most the radius, 0 otherwise. 0x34 is the x channel
 *           and 0x38 the z channel, the pairing the overlay's exact sibling
 *           incrementWorkByte9AndAddVelocityPairThenAdvanceWorkByte2At18
 *           BmagicMagic04303_801EF5E0 and the battle lifts use; the height
 *           channel is not read, so the test is horizontal. Leaf with no
 *           frame, no calls and no stores. Its boundary is the entry and the
 *           jr $ra at 0x801EFE30; the two in-image callers are the jal at
 *           0x801EF100 in func_801EF080, which passes D_801EB630 + 117 * index,
 *           and the jal at 0x801EF218 in the same function, which passes
 *           D_80145E90 + 320 * index, and both branch on the result as a 0/1
 *           flag. The 0x64-byte body is byte-identical to the already-exact
 *           MAGIC218.EMI#3 0x801EF70C lift, which supplied the shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 recordWithinWorkPositionRadiusBmagicMagic05803_801EFDD4(u8 *record) {
  u8 *work;
  s32 dx;
  s32 work_z;
  s32 dz;
  s32 horizontal;
  s32 radius;

  work = g_battle_work;
  dx = (*(s32 *)(record + 0x34) >> 9) - (*(s32 *)(work + 0x34) >> 9);
  work_z = *(s32 *)(work + 0x38) >> 9;
  dz = (*(s32 *)(record + 0x38) >> 9) - work_z;
  horizontal = dx * dx + dz * dz;
  radius = D_1F800000;
  /* The shifted work value is dead after dz; reuse it for the radius product
   * while keeping the halfword load in its own scalar, the allocation the
   * original shows (mflo into $a2, then $a1, then the halfword). */
  work_z = radius * radius;
  return horizontal <= work_z;
}
