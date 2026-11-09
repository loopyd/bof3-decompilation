#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original re-materialises the cell with its own lui 0x1F80
 * plus lw 68($vN) pair at every read, because a store through the loaded pointer
 * may alias the cell, so the cell is named rather than written as a bare
 * scratchpad constant. The exact same-target siblings
 * copyWorkWordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic00603_801F05A4 and
 * decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagic
 * Magic00603_801EF41C declare the same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image. This
 * target's generated undefined_syms_auto.txt binds the address because the
 * remaining asm spans func_801EEC90, func_801EF298 and func_801F0684 still load
 * it, so the name needs no map row. Words 0x34, 0x38 and 0x3C read here are the
 * position triplet the exact bmagic siblings
 * copyWorkWordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic00603_801F05A4 and
 * copyWorkWordPairAndHalfwordClearBytes9AThenAdvanceWorkByte2BmagicMagic10303
 * _801EEF94 copy out of the record they read through 0x8014638C: the record
 * layout is shared and the address is not this target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Record pointer at 0x8014638C in main RAM, outside this overlay image. This
 * lift reads words 0x34 and 0x38 and halfwords 0x2E and 0x30 of the record it
 * points to as the aim target; its role is not proven, so the name stays
 * address-anchored, exactly as the exact bmagic siblings
 * copyWorkWordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic00603_801F05A4 and
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC declare it. The address keeps its
 * generated undefined_syms_auto.txt row because the remaining asm spans
 * func_801EF118 and func_801F0100 still load it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Overlay frame-loop helper reached here with no argument set up by this body,
 * the form the exact bmagic siblings
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC and drawBattlerRecordPointSnap
 * PositionWhenSlotUnavailableAndOnRangeHitRaiseFlag2000ForwardSlotSetHeading
 * TowardBattleRecordAndAdvanceWorkByte2BmagicMagic01200_801EF188 call it with. */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* Two-argument angle helper: the exact bmagic sibling
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC calls it with the word
 * differences 0x34 and 0x38 of the same two records and stores the answer into
 * work word 0xC, the result this body reproduces at word and halfword width. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* One-argument overlay helper; every caller in this tree passes an encoded
 * constant, here 0x100. Bound by this target's generated
 * undefined_funcs_auto.txt row. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EEF28
 * @behavior Per-frame handler selected by the scratchpad work object's dispatch
 *           byte 0x2: entry index 0 of the thirteen-word code-pointer run
 *           0x801EEC04..0x801EEC34 in this target's tabledata segment, the
 *           table func_801EEE50 copies to its stack frame and indexes with
 *           lbu 0x2($v0) before jalr, so the body is reached through that table
 *           and has no jal caller. It reads the scratchpad work object through
 *           the pointer cell at 0x1F800044 and decrements its unsigned byte 9 in
 *           place (lbu / addiu -1 / sb / andi 0xFF / bnez), testing the stored
 *           byte truncated to eight bits rather than a reload, so the rest of
 *           the body runs only when that countdown reaches zero. The first cell
 *           load precedes frame creation because the decrement needs no stack;
 *           the 0x18-byte frame keeps $ra at 0x14 and the saved record pointer
 *           at 0x10 across the three calls.
 *           On zero it reads the shared battle record pointer at 0x801EB4E0 and
 *           copies that record's words 0x34 and 0x38 into the same offsets of
 *           the work object, then stores the record's word 0x3C there plus
 *           0x800000; the addition is materialised with lui 0x80 because the
 *           constant does not fit an addiu, and its store fills the call delay
 *           slot of the shared helper func_800AF764.
 *           It then aims the work object at the record published at 0x8014638C,
 *           whose pointer is loaded into a saved register after that call and
 *           reused by both angle calls. The first func_8017A138 takes the word
 *           differences 0x34 and 0x38 between that record and the work object
 *           and its answer replaces work word 0xC, work word 0x10 is cleared,
 *           and the cell is re-read after those two stores, so the second call
 *           reads its work halfwords 0x2E and 0x30 through the fresh base. The
 *           second func_8017A138 takes the halfword differences 0x2E and 0x30,
 *           and its answer minus 0x200 masked to 0xFFF is stored into work word
 *           0x14 as the argument 0x100 is set up and the shared helper
 *           func_8015D71C is called, that store filling the delay slot.
 *           Last it re-arms the countdown to 0x20, re-reading the cell for that
 *           store, and advances the dispatch byte 0x2, again through a fresh
 *           cell read, so the next frame runs entry 1 at 0x801EF04C. Takes no
 *           arguments, returns nothing through the jr $ra at 0x801EF044 and
 *           reads or writes no other state. The roles of work bytes 9 and 2 and
 *           of words 0xC, 0x10, 0x14, 0x34, 0x38 and 0x3C are not established
 *           beyond what this body does to them, so they stay offset-anchored,
 *           and the 0x800000 bias is written as the literal constant it is.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionAndAngleToward8014638CRearmByte9AndAdvanceWorkByte2BmagicMagic00603_801EEF28(void) {
  u8 *record;
  u8 *work;
  u8 *aim;
  u8 *target;
  u8 *motion;
  s32 direction;
  s32 angle;

  if (--g_battle_work[9] == 0) {
    record = D_801EB4E0;
    work = g_battle_work;
    *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
    *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
    *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C) + 0x800000;
    func_800AF764();
    aim = D_8014638C;
    target = g_battle_work;
    direction = func_8017A138(*(s32 *)(aim + 0x34) - *(s32 *)(target + 0x34),
                              *(s32 *)(aim + 0x38) - *(s32 *)(target + 0x38));
    motion = g_battle_work;
    *(s32 *)(motion + 0xC) = direction;
    *(s32 *)(motion + 0x10) = 0;
    angle = func_8017A138(*(s16 *)(aim + 0x2E) - *(s16 *)(motion + 0x2E),
                          *(s16 *)(aim + 0x30) - *(s16 *)(motion + 0x30));
    *(s32 *)(g_battle_work + 0x14) = (angle - 0x200) & 0xFFF;
    func_8015D71C(0x100);
    g_battle_work[9] = 0x20;
    g_battle_work[2]++;
  }
}
