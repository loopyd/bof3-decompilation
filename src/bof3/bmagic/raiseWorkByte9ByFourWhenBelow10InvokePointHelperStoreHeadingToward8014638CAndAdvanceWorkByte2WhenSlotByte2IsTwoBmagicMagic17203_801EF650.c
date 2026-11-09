#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original
 * re-materialises the cell with its own lui 0x1F80 plus lw 0x44 pair at each of
 * its three reads, because a store through the loaded pointer may alias the
 * cell. Reading the cell through this named row at every access, without a
 * pointer local, is what reproduced the original allocation: the loaded pointer
 * lands in $a0 for the first group, and a `u8 *work` local held across the same
 * statements measured 54 of 72 instructions with the pointer in $a1 instead.
 * This target's exact siblings
 * decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionToward
 * 8014638CClearByte9AndAdvanceWorkByte2BmagicMagic17203_801EF590 and
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes2And0Set
 * BmagicMagic17203_801EF518 read the same cell through the same row. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pointer cell at 0x8014638C in main RAM, outside this overlay image,
 * which this target's generated undefined_syms_auto.txt binds. The body reads
 * words 0x34, 0x38 and 0x3C of the record as the position it hands to the point
 * helper and as the origin of the heading difference. The role of the record is
 * not proven here, so the name stays address-anchored, as in the exact sibling
 * above and in the sibling lift
 * decrementWorkByte9ThenCopyRecordTripletAdd800000ToWord3CSetDirectionAnd
 * AngleToward8014638CRearmByte9AndAdvanceWorkByte2BmagicMagic00603_801EEF28. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Flat 0x78-byte action-slot store record table at 0x801EC332, addressed at the
 * record byte +0x02 of the entry selected by a work index. This target's
 * generated undefined_syms_auto.txt binds the address and the bmagic sibling
 * copyQueuedSlotPositionPairAndDistanceThenAdvanceWorkByte2WhenSlotByte2IsTwo
 * BmagicMagic04500_801EF608 declares and indexes it the same way, as a byte
 * table scaled by 0x78. */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */

/* Point helper of the main executable, bound by this target's generated
 * undefined_funcs_auto.txt. The bmagic sibling lifts declare the same prototype
 * and pass a stack point triple plus an element they never set and a size
 * literal: drawBattlerRecordPointSnapPositionWhenSlotUnavailableAndOnRangeHit
 * RaiseFlag2000ForwardSlotSetHeadingTowardBattleRecordAndAdvanceWorkByte2
 * BmagicMagic01200_801EF188 passes 0x40 exactly as this body does. */
void func_800AF3A0(s32 x, s32 y, s32 z, s32 depth, s32 size);

/* Two-argument angle helper of the main executable, bound by this target's
 * generated undefined_funcs_auto.txt. The exact sibling above calls it with the
 * word 0x34 and 0x38 differences of the same two records and stores the answer
 * into work word 0xC, which is what this body does. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EF650
 * @behavior Work-step handler reached with dispatch byte 2 equal to 8, the value
 *           the exact sibling at 0x801EF590 writes one frame earlier, so the
 *           body has no jal caller and is entered through the handler table. It
 *           first reads the scratchpad work object through the pointer cell at
 *           0x1F800044 and raises its byte 9 by four, but only when that byte is
 *           below 0x10: the payload compares with sltiu 0x10, materialises the
 *           sum in the branch delay slot and stores it with sb, so the stored
 *           byte is the truncated sum. That cell read precedes frame creation
 *           because the raise needs no stack, and the frame that follows is 0x30
 *           bytes with $ra at 0x2C and $s0 at 0x28.
 *           It then loads the record pointer cell at 0x8014638C once into $s0
 *           and builds a stack point triple from that record: word 0x34 and word
 *           0x38 shifted right by nine and biased by minus 0x4000, and word 0x3C
 *           plus 0x1000000 shifted right by 17, the 0x1000000 materialised with
 *           lui 0x100 because the constant does not fit an addiu. The three words
 *           sit at 0x18, 0x1C and 0x20 of the frame. The triple is passed to the
 *           point helper func_800AF3A0 together with the fourth frame word at
 *           0x24, which the body never writes, and the size literal 0x40 stored at
 *           0x10 as the helper fifth argument.
 *           After the call it re-reads the cell and copies work word 0xC into
 *           work word 0x10, the stored heading into the previous-heading slot,
 *           while the same group re-reads work words 0x34 and 0x38 into argument
 *           registers before that store. It then computes the heading toward the
 *           0x8014638C record through func_8017A138 from that record word 0x34
 *           minus work word 0x34 and record word 0x38 minus work word 0x38, the
 *           second difference filling the call delay slot. A third cell read
 *           follows its own delay nop: it lifts work byte 4 with lbu, stores the
 *           returned heading into work word 0xC, scales that index by 0x78 with
 *           sll 4, subu, sll 3, and compares the byte at 0x801EC332 of the
 *           selected action-slot record with 2. When that byte is 2 it advances
 *           work dispatch byte 2 by one through the same pointer, so the next
 *           frame runs the following handler. Takes no arguments and returns
 *           nothing through the jr $ra at 0x801EF76C. The roles of work bytes 9,
 *           4 and 2, of work words 0xC and 0x10, of the 0x78-byte action-slot
 *           record and of the 0x8014638C record field set are not established
 *           beyond what this body does to them, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorkByte9ByFourWhenBelow10InvokePointHelperStoreHeadingToward8014638CAndAdvanceWorkByte2WhenSlotByte2IsTwoBmagicMagic17203_801EF650(void) {
    s32 point[4];
    u8 *record;
    s32 heading;

    if (g_battle_work[9] < 0x10) {
        g_battle_work[9] += 4;
    }
    record = D_8014638C;
    point[0] = (*(s32 *)(record + 0x34) >> 9) - 0x4000;
    point[1] = (*(s32 *)(record + 0x38) >> 9) - 0x4000;
    point[2] = (*(s32 *)(record + 0x3C) + 0x1000000) >> 17;
    func_800AF3A0(point[0], point[1], point[2], point[3], 0x40);
    *(s32 *)(g_battle_work + 0x10) = *(s32 *)(g_battle_work + 0xC);
    heading = func_8017A138(*(s32 *)(record + 0x34) - *(s32 *)(g_battle_work + 0x34),
                            *(s32 *)(record + 0x38) - *(s32 *)(g_battle_work + 0x38));
    *(s32 *)(g_battle_work + 0xC) = heading;
    if (D_801EC332[g_battle_work[4] * 0x78] == 2) {
        g_battle_work[2]++;
    }
}
