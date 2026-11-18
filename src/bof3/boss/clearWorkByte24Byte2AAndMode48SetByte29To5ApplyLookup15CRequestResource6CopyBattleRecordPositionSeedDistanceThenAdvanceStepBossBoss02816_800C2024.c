#include "bof3/bof3.h"

extern u8 *boss028CurrentWorkRecordState; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image; this
 * target's generated undefined_syms_auto.txt already binds the address. This
 * body only reads its words 0x34 and 0x38, the fixed-point position pair the
 * shared distance helper measures, which the exact sibling
 * addWorkWord20ToWord14AccumulatePreviousWord14IntoHalfword3EThenSetGlobal
 * WorkByte1To5ClearGlobalWorkByte2AndInvokeHelperWhenHalfword3EAboveDistance
 * Plus1000BossBoss02816_800C1F2C records for the same two offsets. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable lookup and resource helpers, bound by this target's
 * generated undefined_funcs_auto.txt rows. The exact sibling
 * setupBattleWorkRecordBossBoss02213_800C1ABC declares the same one-argument
 * prototypes for both: 0x8014DD3C applies a main lookup record and 0x8014D6B8
 * requests a resource id. */
extern void func_8014DD3C(s32 entry);   /* @source 0x8014DD3C */
extern void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Shared two-argument distance helper of the main executable; it measures the
 * fixed-point position pair it takes as (x, z) and reports the distance the
 * work halfword 0x3E carries. The bmagic seed
 * seedWorkObjectFromDecrementedRecordPositionPairPublishDistanceRequest
 * Resource0AndAdvanceWorkByte2BmagicMagic05703_801EF900 declares the same s16
 * return and stores it at the same halfword offset. */
extern s16 func_8015477C(s32 x, s32 z); /* @source 0x8015477C */

/* @source 0x800C2024
 * @behavior First step of this overlay's three-word step run based at
 *           0x800C21BC, whose words are 0x800C2024, 0x800C20C8 and 0x800C2120;
 *           the exact sibling dispatchWorkByte2Table21bcThenInvokeSharedService
 *           WhenWorkByte0Bit0SetBossBoss02816_800C1FB8 indexes that base with
 *           the work record byte 2, and no jal in the payload names this span,
 *           so the prologue and the return path place its boundary.
 *           It clears the work record published through the scratchpad pointer
 *           cell 0x1F800044 at bytes 0x24, 0x2A and 0x48, sets the same
 *           record's byte 0x29 to 5, applies main lookup record 0x15C through
 *           func_8014DD3C and requests resource 6 through func_8014D6B8. Each
 *           clear and the byte 0x29 store re-reads the cell, since a store
 *           through the loaded pointer may alias it; the 5 constant is loaded
 *           into $v0 before the first call and that store fills the call's
 *           delay slot, while the 6 is materialised in the second call's delay
 *           slot.
 *           It then copies the shared battle record's position words 0x34 and
 *           0x38 into the work record at the same offsets, measures them with
 *           func_8015477C - the first argument read back from the work record
 *           at 0x34, the second the battle record's own word 0x38 - and stores
 *           the returned halfword at work halfword 0x3E, that copy of word 0x38
 *           filling the call's delay slot. Finally it re-reads the cell, stores
 *           the halfword, and advances work byte 2 by one at byte width, which
 *           hands the next frame of the run to entry 0x800C20C8.
 *           The roles of work bytes 0x24, 0x2A, 0x29 and 0x48 stay
 *           offset-anchored: this lift establishes only that they are cleared
 *           or written here, not what they select. The 0x18-byte frame keeps
 *           $ra at 0x10 across the three calls, the body takes no arguments,
 *           and the value left in $v0 is the distance helper's halfword, which
 *           the indirect step call discards.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte24Byte2AAndMode48SetByte29To5ApplyLookup15CRequestResource6CopyBattleRecordPositionSeedDistanceThenAdvanceStepBossBoss02816_800C2024(
    void) {
  u8 *record;
  u8 *work;
  u8 *step;
  s32 z;
  s16 distance;

  boss028CurrentWorkRecordState[0x24] = 0;
  boss028CurrentWorkRecordState[0x2A] = 0;
  boss028CurrentWorkRecordState[0x48] = 0;
  boss028CurrentWorkRecordState[0x29] = 5;
  func_8014DD3C(0x15C);
  func_8014D6B8(6);
  record = D_801EB4E0;
  work = boss028CurrentWorkRecordState;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
  z = *(s32 *)(record + 0x38);
  *(s32 *)(work + 0x38) = z;
  distance = func_8015477C(*(s32 *)(work + 0x34), z);
  step = boss028CurrentWorkRecordState;
  *(u16 *)(step + 0x3E) = (u16)distance;
  step[2]++;
}
