#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui 0x1F80 plus lw 0x44 pair at each of this function's six
 * reads: the byte 0x0A count sits alone between a store through the loaded
 * pointer, the two rotation samples sit either side of a call, the position,
 * countdown and accumulation group shares one read, and the branch on the
 * countdown store and the sprite call each force a further reload. The cell is
 * therefore named rather than written as a bare scratchpad constant, and it is
 * not volatile: every read pair is separated by a store through the previously
 * loaded pointer or by a call. The exact same-target sibling
 * advanceWorkByteAOnFrameParityInvokePointHelperCountdownWorkByte9DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4
 * declares the same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

void func_800AF764(void);
void func_801EF79C(u8 index, s32 mode);
s32 func_801782FC(s32 angle);
s32 func_801783C8(s32 angle);

/* @source 0x801EF3C4
 * @behavior Middle handler of this overlay's three-phase groups, entry 3 of the
 *           thirteen-entry dispatch table at 0x801EFBB0 (payload words
 *           0x801EFBBC, 0x801EFBC8 and 0x801EFBD4, the three in-image
 *           references to this address), so work-object byte 0x2 holds 3, 6 or
 *           9 while this body runs. The scratchpad work-object pointer cell at
 *           0x1F800044 names the work object at every access. The body
 *           increments the object's frame byte 0x0A (lbu 0xA / addiu 1 / sb
 *           0xA), then advances the tracked position pair: it passes the
 *           object's direction word 0xC to the shared rotator at 0x801782FC and
 *           adds the 4.12 result scaled by 0x2000 and shifted down 12
 *           (sll 13 / sra 12) onto word 0x34, then calls the second rotator at
 *           0x801783C8 with the same word 0xC and adds its identically scaled
 *           result onto word 0x38 with the store filling the call's delay slot.
 *           The direction word 0xC is the angle the preceding phase stores from
 *           the shared atan2 helper at 0x8017A138 over the position difference
 *           to the battle record reached through 0x801EB4E0, so the two words
 *           steer the effect toward that record. One further pointer-cell read
 *           then carries the position accumulation and the countdown: word 0x14
 *           takes word 0x14 plus word 0x20, halfword 0x3E takes itself plus the
 *           new word 0x14 at halfword width, and countdown byte 0x9 is
 *           decremented once and its truncated value tested against zero; when
 *           it reaches zero a sixth pointer-cell read increments phase byte 0x2,
 *           which advances the group. Finally the shared helper at 0x800AF764
 *           runs and the overlay's sprite emitter at 0x801EF79C receives the
 *           object's byte 0xB with mode word 1, the mode that distinguishes this
 *           step phase from the 0 its sibling handler passes. Takes no
 *           arguments, returns nothing, and the 0x18-byte frame only keeps $ra
 *           across the four calls. Bytes 0x801EF3C4 through 0x801EF4B7 hold no
 *           relocation whose immediate is unique to this target other than the
 *           in-overlay call to 0x801EF79C, so no byte-identical sibling exists
 *           and the body was written from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByteAStepWorkPositionPairByAngleWord0CAddWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9AdvanceWorkByte2OnZeroAndDrawSpriteMode1BmagicMagic01200_801EF3C4(void) {
  u8 *work;
  s32 step;
  u8 count;

  g_battle_work[0xA]++;

  *(s32 *)(g_battle_work + 0x34) +=
      (func_801782FC(*(s32 *)(g_battle_work + 0xC)) * 0x2000) >> 12;

  step = (func_801783C8(*(s32 *)(g_battle_work + 0xC)) * 0x2000) >> 12;

  work = g_battle_work;
  *(s32 *)(work + 0x38) += step;
  *(s32 *)(work + 0x14) += *(s32 *)(work + 0x20);
  *(u16 *)(work + 0x3E) += *(s32 *)(work + 0x14);
  count = (u8)(work[9] - 1);
  work[9] = count;
  if (count == 0) {
    g_battle_work[2]++;
  }

  func_800AF764();
  func_801EF79C(g_battle_work[0xB], 1);
}
