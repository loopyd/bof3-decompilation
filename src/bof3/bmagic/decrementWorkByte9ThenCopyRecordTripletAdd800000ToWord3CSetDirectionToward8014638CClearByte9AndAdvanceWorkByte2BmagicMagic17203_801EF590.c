#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, declared the way this
 * target's exact sibling
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes2And0SetBmagicMagic17203_801EF518
 * declares it. The original re-materialises the cell with its own lui 0x1F80
 * plus lw 0x44($vN) pair at every read, because a store through the loaded
 * pointer may alias the cell, so the cell is named rather than written as a
 * bare scratchpad constant: this body loads it four times, once before frame
 * creation for the countdown byte, once for the position copy, and twice after
 * the angle call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image. This
 * target's generated undefined_syms_auto.txt binds the address, so the name
 * needs no map row. Words 0x34, 0x38 and 0x3C read here are the same position
 * triplet at the same offsets that the exact bmagic sibling
 * decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionAndAngle
 * Toward8014638CRearmByte9AndAdvanceWorkByte2BmagicMagic00603_801EEF28 copies
 * out of the record it reads through this cell, so the record layout is shared
 * and the address is not this target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Record pointer at 0x8014638C in main RAM, outside this overlay image. This
 * lift reads words 0x34 and 0x38 of the record it points to as the aim target;
 * the role of that record is not proven, so the name stays address-anchored,
 * exactly as the exact bmagic sibling above declares it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Two-argument angle helper: the exact bmagic sibling above calls it with the
 * word differences 0x34 and 0x38 of the same two records and stores the answer
 * into work word 0xC, the result this body reproduces. Bound by this target's
 * generated undefined_funcs_auto.txt row. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EF590
 * @behavior Entry index 7 of the handler pointer run that starts at 0x801F0B74
 *           in this target's textbin segment: that run's word at 0x801F0B90
 *           holds this address, and this target's exact sibling for index 1 at
 *           0x801EF518 reads the same run through the work object's dispatch
 *           byte 2 scaled by four, so the body is reached through the table and
 *           has no jal caller. It reads the scratchpad work object through the
 *           pointer cell at 0x1F800044 and decrements its unsigned byte 9 in
 *           place (lbu / addiu -1 / sb / andi 0xFF / bnez), testing the stored
 *           byte truncated to eight bits rather than a reload, so the rest of
 *           the body runs only when that countdown reaches zero. The first cell
 *           load precedes frame creation because the decrement needs no stack;
 *           the 0x18-byte frame keeps $ra at 0x10 across the single call and
 *           saves no other register.
 *           On zero it reads the shared battle record pointer at 0x801EB4E0 and
 *           copies that record's words 0x34 and 0x38 into the same offsets of
 *           the work object, then stores the record's word 0x3C there plus
 *           0x800000; the addition is materialised with lui 0x80 because the
 *           constant does not fit an addiu. The three stores go through one cell
 *           load held in a register across them, and the aim record pointer at
 *           0x8014638C is loaded before the argument differences are formed.
 *           It then aims the work object at the record published at 0x8014638C:
 *           the first func_8017A138 takes the word differences 0x34 and 0x38
 *           between that record and the work object, reading the work words back
 *           through the same base the copy used, and its answer is stored into
 *           work word 0xC. Last it clears byte 9 through a fresh cell read,
 *           takes a second cell read that the following direction store does not
 *           use, and advances dispatch byte 2 through that second read, so the
 *           next frame runs entry 8 at 0x801EF650. The two reads are visible in
 *           the payload as two lui 0x1F80 plus lw 0x44 pairs with the direction
 *           store between them, and the store fills the load delay slot of the
 *           second pair, which is why the direction store sits between that
 *           load and the byte 2 read; a single shared read drops that slot to a
 *           nop and measures 43 of 48 instructions at 196 bytes. Takes no
 *           arguments and returns nothing
 *           through the jr $ra at 0x801EF648. The roles of work bytes 9 and 2
 *           and of word 0xC are not established beyond what this body does to
 *           them, so they stay offset-anchored, and the 0x800000 bias is written
 *           as the literal constant it is.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionToward8014638CClearByte9AndAdvanceWorkByte2BmagicMagic17203_801EF590(void) {
    u8 *record;
    u8 *work;
    u8 *aim;
    u8 *motion;
    u8 *step;
    s32 direction;

    if (--g_battle_work[9] == 0) {
        record = D_801EB4E0;
        work = g_battle_work;
        *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
        *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
        *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C) + 0x800000;
        aim = D_8014638C;
        direction = func_8017A138(*(s32 *)(aim + 0x34) - *(s32 *)(work + 0x34),
                                  *(s32 *)(aim + 0x38) - *(s32 *)(work + 0x38));
        motion = g_battle_work;
        motion[9] = 0;
        step = g_battle_work;
        *(s32 *)(motion + 0xC) = direction;
        step[2]++;
    }
}
