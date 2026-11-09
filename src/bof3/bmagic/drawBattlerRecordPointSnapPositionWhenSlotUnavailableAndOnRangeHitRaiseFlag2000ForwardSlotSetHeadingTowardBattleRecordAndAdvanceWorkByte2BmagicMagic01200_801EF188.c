#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address (config/targets/shared/symbols.txt) and the exact
 * same-target sibling advanceWorkByteAOnFrameParityInvokePointHelperCountdown
 * WorkByte9DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4 reads the
 * same word, its low bit as a per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Battle target byte in main RAM, outside this overlay's image. Its 0x40 bit
 * separates the enemy group from the party group; the exact same-target
 * sibling seedWorkObjectFromBattlerRecordWhenBattlerReadyAndStateByteNotSix
 * ElseFinishWorkByte2BmagicMagic01200_801EF4B8 branches on the same bit to
 * select the enemy or party action record array. Bound through this target's
 * generated undefined_syms_auto.txt. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword in main RAM, outside this overlay's image. The
 * sibling battle lifts and the bmagic lift forwardBattleByteThenRaiseFlag4And
 * InvokeHelperWhenWorkByteBZeroBmagicMagic10603_801EEE80 read and write the same
 * halfword. Bound through this target's generated undefined_syms_auto.txt. */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Shared battle record pointer in main RAM, outside this overlay's image. The
 * exact same-target siblings decrementGlobalWorkByteBAndInvokeHelperBmagic
 * Magic01200_801EF670 and seedWorkObjectFromBattlerRecordWhenBattlerReadyAnd
 * StateByteNotSixElseFinishWorkByte2BmagicMagic01200_801EF4B8 read the same
 * cell. Bound through this target's generated undefined_syms_auto.txt. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Party and enemy action record arrays in main RAM, outside this overlay's
 * image: the 0x140-byte-stride party records at 0x80145E90 and the
 * 0x118-byte-stride enemy records at 0x801EB630 that the battle targets own.
 * This lift reads the projected position words 0x34, 0x38 and 0x3E of one row,
 * so both stay byte arrays with the stride written at the index, the spelling
 * the exact same-target sibling seedWorkObjectFromBattlerRecordWhenBattlerReady
 * AndStateByteNotSixElseFinishWorkByte2BmagicMagic01200_801EF4B8 uses for the
 * same two arrays. Bound through this target's generated
 * undefined_syms_auto.txt. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind table */

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui 0x1F80 plus lw 0x44 pair at every read, because a store
 * through the loaded pointer may alias the cell, so the cell is named rather
 * than written as a bare scratchpad constant. The exact same-target siblings
 * advanceWorkByteAStepWorkPositionPairByAngleWord0CAddWord20ToWord14Accumulate
 * IntoHalfword3ECountdownWorkByte9AdvanceWorkByte2OnZeroAndDrawSpriteMode1
 * BmagicMagic01200_801EF3C4 and advanceWorkByteAOnFrameParityInvokePointHelper
 * CountdownWorkByte9DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4
 * declare the same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Helpers of the battle image and of the main executable, all outside this
 * overlay's image and bound through this target's generated
 * undefined_funcs_auto.txt except the overlay's own sprite emitter at
 * 0x801EF79C, which this target's map binds. The status helper reports whether
 * one battler slot is unavailable; the point helper draws the four-word point
 * with a size word; the range helper is the battle lift
 * testWorkRangeAgainstRecord; the halfword helper is the one the sibling
 * forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic
 * 10603_801EEE80 forwards the battle target byte to; the atan2 helper is the
 * rotator the exact same-target sibling advanceWorkByteAStepWorkPositionPairBy
 * AngleWord0CAddWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9
 * AdvanceWorkByte2OnZeroAndDrawSpriteMode1BmagicMagic01200_801EF3C4 calls. */
u8 func_801DB524(u8 arg0);
void func_800AF3A0(s32 x, s32 y, s32 z, s32 depth, s32 size);
u32 func_800AF66C(u8 *record, u32 size);
void func_800A9820(u8 arg0);
void func_8015D71C(s32 arg0);
s32 func_8017A138(s32 x, s32 y);
void func_800AF764(void);
void drawRotatedTexturedQuadAtWorkScreenPairFromWorkByteBRowTintedByWorkByte9AndPhasedByWorkByteABmagicMagic01200_801EF79C(u8 index, s32 mode);

/* @source 0x801EF188
 * @behavior Work-byte-2 handler, the opening phase of this overlay's
 *           three-phase groups. The payload words at 0x801EFBB8, 0x801EFBC4 and
 *           0x801EFBD0 of the thirteen-entry table at 0x801EFBB0 store this
 *           address, and the dispatcher at 0x801EF03C indexes that table from
 *           the work object's byte 0x2 plus one entry, so the byte holds 1, 4 or
 *           7 while this body runs and the step phase following it at
 *           0x801EF3C4 takes 2, 5 or 8.
 *           The shared frame counter at 0x80143E6C supplies the phase: when its
 *           low bit is set the handler increments the work object's byte 0xA, so
 *           that byte counts frames in pairs.
 *           It then selects the action record row of work byte 0x4, from the
 *           0x118-byte-stride enemy records at 0x801EB630 when bit 0x40 of the
 *           battle target byte 0x80146384 is set and from the 0x140-byte-stride
 *           party records at 0x80145E90 otherwise, and projects that row's
 *           position into a local four-word point: the words at 0x34 and 0x38
 *           are shifted down nine bits and biased by -0x4000, and the signed
 *           halfword at 0x3E takes 0xC0 added and a single-bit shift down. The
 *           fourth component is never assigned, so its argument load reads the
 *           point slot past the three written words and passes dead stack
 *           content, exactly as the sibling handler at 0x801EF6A4 does.
 *           When the shared status helper at 0x801DB524 reports the battler slot
 *           of work byte 0x3 unavailable, the handler snaps the work object's
 *           projected words 0xC, 0x10 and 0x14 onto those same three record
 *           values through one pointer-cell load, and stores 0xB into work byte
 *           0x2, the selector of the handler at 0x801EF6A4 that draws and steps
 *           the countdown back down.
 *           It then passes the point with size word 0x40 to the shared helper at
 *           0x800AF3A0 and range-tests the same selected row against the shared
 *           work object through 0x800AF66C with the bound 0x8000. On a hit it
 *           raises bit 0x2000 of the shared battle global halfword at 0x801462E8
 *           (lui/addiu of that address into one base register, lhu 0(base), ori,
 *           sh 0(base), so the halfword is reached through a local pointer and
 *           never through the $at re-materialisation a plain global access
 *           emits), forwards the battler slot to the shared helper at 0x800A9820, calls
 *           the main-executable helper at 0x8015D71C with the selector 0x203,
 *           and stores the angle that the shared atan2 helper at 0x8017A138
 *           returns for the battle record's position at 0x801EB4E0 minus the
 *           work object's position into word 0xC, the direction word the step
 *           phase steers by. It takes 0x40 and -8 into words 0x14 and 0x20 and
 *           advances work byte 0x2 through one further pointer-cell read, which
 *           selects the step phase.
 *           Finally it calls the shared helper at 0x800AF764 and passes work
 *           byte 0xB with mode word 0 to the overlay's sprite emitter at
 *           0x801EF79C. Takes no arguments, returns nothing and reads or writes
 *           no other state; the 0x38-byte frame holds the point, its outgoing
 *           argument word, $ra and the two saved registers, one of which carries
 *           the 0x40 that both the draw size and word 0x14 use. No twin exists
 *           in any extracted payload under out/binaries once every direct call
 *           and jump target word of the span is masked, so the body was written
 *           from the original bytes rather than cloned; the selection chain,
 *           the point projection and the frame layout mirror the exact
 *           same-target siblings of this group.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBattlerRecordPointSnapPositionWhenSlotUnavailableAndOnRangeHitRaiseFlag2000ForwardSlotSetHeadingTowardBattleRecordAndAdvanceWorkByte2BmagicMagic01200_801EF188(void) {
  s32 point[4];
  u8 *record;
  u8 *work;
  u8 *battle;
  u8 *target;
  s32 angle;
  u16 *flags;

  if (D_80143E6C & 1u) {
    g_battle_work[0xA]++;
  }
  if (D_80146384 & 0x40) {
    record = &D_801EB630[g_battle_work[4] * 0x118];
  } else {
    record = &D_80145E90[g_battle_work[4] * 0x140];
  }
  point[0] = (*(s32 *)(record + 0x34) >> 9) - 0x4000;
  point[1] = (*(s32 *)(record + 0x38) >> 9) - 0x4000;
  point[2] = (*(s16 *)(record + 0x3E) + 0xC0) >> 1;

  if (func_801DB524(g_battle_work[3])) {
    work = g_battle_work;
    *(s32 *)(work + 0xC) = (*(s32 *)(record + 0x34) >> 9) - 0x4000;
    *(s32 *)(work + 0x10) = (*(s32 *)(record + 0x38) >> 9) - 0x4000;
    *(s32 *)(work + 0x14) = (*(s16 *)(record + 0x3E) + 0xC0) >> 1;
    work[2] = 0xB;
  }

  func_800AF3A0(point[0], point[1], point[2], point[3], 0x40);

  if (func_800AF66C(record, 0x8000)) {
    flags = &D_801462E8;
    *flags |= 0x2000;
    func_800A9820(g_battle_work[3]);
    func_8015D71C(0x203);
    battle = D_801EB4E0;
    target = g_battle_work;
    angle = func_8017A138(*(s32 *)(battle + 0x34) - *(s32 *)(target + 0x34),
                          *(s32 *)(battle + 0x38) - *(s32 *)(target + 0x38));
    work = g_battle_work;
    *(s32 *)(work + 0xC) = angle;
    *(s32 *)(work + 0x14) = 0x40;
    *(s32 *)(work + 0x20) = -8;
    work[2]++;
  }

  func_800AF764();
  drawRotatedTexturedQuadAtWorkScreenPairFromWorkByteBRowTintedByWorkByte9AndPhasedByWorkByteABmagicMagic01200_801EF79C(g_battle_work[0xB], 0);
}
