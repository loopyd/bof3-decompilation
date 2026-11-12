#include "bof3/bof3.h"

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The exact sibling copyRecordPositionPairOnFrameParityElseAddStepPairThen
 * InvokeHelperDecrementWorkByte9AndOnZeroReseedWorkByte9FromWorkByteBPlus4And
 * AdvanceWorkByte2BmagicMagic00800_801F0FA4 reads the same record through this
 * pointer. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts subtractWorkColorTripletBy3ThenDecrementWorkByte9SetGlobalWorkByteBToFF
 * AndInvokeHelperWhenZeroBmagicMagic00800_801F08C4 and
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C declare. The named
 * symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell after
 * every store made through the pointer it supplied, because such a store may
 * alias the cell object. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Two parallel words of the 8-byte step records that follow the overlay handler
 * table in this target's rodata: record [n] of 0x801F1C04 holds the step this
 * overlay adds to work word 0x34 and record [n] of 0x801F1C08 the step it adds
 * to work word 0x38, the two steps the exact sibling copyRecordPositionPairOn
 * FrameParityElseAddStepPairThenInvokeHelperDecrementWorkByte9AndOnZeroReseed
 * WorkByte9FromWorkByteBPlus4AndAdvanceWorkByte2BmagicMagic00800_801F0FA4
 * accumulates. The body indexes both with work byte 0xB, scaled by 8 because
 * the records are eight bytes wide, so both tables are declared s32 and the
 * index is doubled. */
extern s32 D_801F1C04[]; /* @source 0x801F1C04 @kind data */
extern s32 D_801F1C08[]; /* @source 0x801F1C08 @kind data */

/* Shared transform helper of the battle overlays, outside this overlay's image:
 * the sibling lifts of the battle overlays pass the scratchpad work object this
 * body already holds. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */

/* Shared battle helper at 0x800AFF98, outside this overlay's image. It consumes
 * no arguments and returns a byte, which the exact sibling seedWorkByte9From
 * HelperAFF98AndAdvanceWorkByte2BmagicMagic00800_801F133C also stores in work
 * byte 9. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added. */
u8 func_800AFF98(void);

/* @source 0x801F0E54
 * @behavior Work byte 2 state 0 of this overlay: entry 0 of the six-entry code
 * pointer table at 0x801EEC94 (0x801F0E54, 0x801F0FA4, 0x801F1090, 0x801F117C,
 * 0x801F11C4, 0x801F1274), which 0x801F0DA8 copies into its own frame and
 * selects with scratchpad work-object byte 2, so this body runs while work byte
 * 2 is 0. It raises bit 0x20 of work byte 0, seeds mode byte 0x5C and byte 0x2B
 * with 1, seeds the signed tint triplet 0x5D/0x5E/0x5F with -0x50, copies the
 * shared battle work record's byte 0x27 and byte 8 into the work object at the
 * same offsets, copies the record's position word triplet 0x34/0x38/0x3C into
 * the object at the same offsets, then takes the object's byte 0xB as the index
 * of the step-word records at 0x801F1C04 and 0x801F1C08 and stores record [byte
 * 0xB] of each into object words 0xC and 0x10. It then invokes the shared
 * transform helper 0x801DD350 on the work object, stores the byte 0x800AFF98
 * returns in work byte 9, and advances work byte 2 with a byte-width add, moving
 * dispatch to entry 1 at 0x801F0FA4. The 0x18-byte frame keeps only $ra at 0x10
 * across the two calls; the function takes no arguments, returns nothing and
 * reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkFlagsTintAndRecordFieldsThenStepPairFromByteBInvokeTransformAndSeedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic00800_801F0E54(
    void) {
  u8 *flags;
  u8 *mode;
  u8 *kind;
  u8 *tintR;
  u8 *tintG;
  u8 *tintB;
  u8 *record;
  u8 *work;

  flags = g_battle_work;
  flags[0] |= 0x20;
  mode = g_battle_work;
  mode[0x5C] = 1;
  kind = g_battle_work;
  kind[0x2B] = 1;
  tintR = g_battle_work;
  *(s8 *)(tintR + 0x5D) = -0x50;
  tintG = g_battle_work;
  *(s8 *)(tintG + 0x5E) = -0x50;
  tintB = g_battle_work;
  *(s8 *)(tintB + 0x5F) = -0x50;
  g_battle_work[0x27] = D_801EB4E0[0x27];
  g_battle_work[8] = D_801EB4E0[8];
  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34);
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38);
  *(s32 *)(work + 0x3C) = *(s32 *)(record + 0x3C);
  *(s32 *)(work + 0xC) = D_801F1C04[work[0xB] * 2];
  *(s32 *)(work + 0x10) = D_801F1C08[work[0xB] * 2];
  func_801DD350(work);
  g_battle_work[9] = func_800AFF98();
  g_battle_work[2]++;
}
