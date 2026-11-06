#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * by the battle lifts. This function reads it once (lui $a1,0x1F80 / lw
 * $a1,68($a1)) and keeps the loaded pointer for its two record reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword at the base of the block the dispatchers and the scene
 * lifts use. This function reads it once with lh after both horizontal
 * products, so the cell is declared signed here at the s16 width that load
 * shows; siblings read other widths of the same address at their own use
 * sites. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind unknown */

/* @source 0x801EF70C
 * @behavior Spatial dominance predicate between the position record passed in
 *           $a0 and the work object published through the scratchpad pointer
 *           cell at 0x1F800044. Both records' words at 0x34 and 0x38 are
 *           converted from the 0x200 fixed-point unit by an arithmetic shift
 *           of nine, the two differences are squared and summed into the
 *           horizontal span, the signed scratchpad halfword at 0x1F800000 is
 *           squared into the vertical span, and the function returns 1 when
 *           the horizontal span is at most the vertical span, 0 otherwise.
 *           0x34 is the x channel and 0x38 the z channel, the pairing the
 *           overlay's exact sibling incrementWorkByte9AndAddVelocityPairThen
 *           AdvanceWorkByte2At18BmagicMagic04303_801EF5E0 and the battle lifts
 *           use. Leaf with no frame, no calls and no stores. The entry has no
 *           in-image jal or pointer word in this overlay, so the boundary comes
 *           from the entry and the jr $ra at 0x801EF768; the identical
 *           0x64-byte body is MAGIC058.EMI#3 0x801EFDD4, whose two call sites in
 *           that overlay pass a record pointer and branch on the result as a
 *           0/1 flag.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801EF70C(u8 *record) {
  u8 *work;
  s32 dx;
  s32 work_z;
  s32 dz;
  s32 horizontal;
  s32 vertical;

  work = g_battle_work;
  dx = (*(s32 *)(record + 0x34) >> 9) - (*(s32 *)(work + 0x34) >> 9);
  work_z = *(s32 *)(work + 0x38) >> 9;
  dz = (*(s32 *)(record + 0x38) >> 9) - work_z;
  horizontal = dx * dx + dz * dz;
  vertical = D_1F800000;
  /* The shifted work value is dead after dz; reuse it for the vertical
   * product while keeping the halfword load in its own scalar. */
  work_z = vertical * vertical;
  return horizontal <= work_z;
}
