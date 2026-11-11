#include "bof3/battle/battle15_internal.h"

void func_800AF764(void);

/* @source 0x800AE82C
 * @behavior Decrements work byte +0x09 modulo 256. On zero it copies the
 *   coordinate triplet (words 0, 4 and 8) of the action record selected by
 *   work byte +0x0B into work words 0x34, 0x38 and 0x3C: the 0x140-byte party
 *   action array at 0x80145EC4 while that byte is below 3, otherwise the
 *   0x118-byte enemy action array at 0x801EB664 at the byte minus three. The
 *   third word is staged across the two arms and stored at the join. It then
 *   calls func_800AF764, sets work word +0x10 to -14, clears byte +0x09,
 *   subtracts 24 from halfword +0x2E and byte +0x0A from halfword +0x30, sets
 *   byte +0x0A to 1 and advances work byte +0x01. The original re-reads work
 *   byte +0x0B before each record load and the work pointer cell before each
 *   later access group, because a store through the loaded pointer may alias
 *   them.
 * @status partial
 * @match 98.04
 * @residual 100 of 102 instructions at 408 bytes. The post-call block
 *   subtracts and stores both halfwords before the work-cell reload that feeds
 *   the byte +0x0A store, while the original stores halfword +0x2E, reloads
 *   the cell, then subtracts and stores halfword +0x30. Same instruction set
 *   and size; statement order in the post-call group only.
 */
void countdownWorkByte9SeedAxesFromBattlerRecordAndAdvanceStage(void)
{
    u8 *entry;
    u8 *work;
    u32 axis;
    u16 first;
    u8 step;
    u16 second;

    if (--g_battle_work[0x09] != 0) {
        return;
    }
    entry = g_battle_work;
    if (entry[0x0B] < 3) {
        *(u32 *)(entry + 0x34) = D_80145EC4[entry[0x0B]].unk_00;
        *(u32 *)(entry + 0x38) = D_80145EC4[entry[0x0B]].unk_04;
        axis = D_80145EC4[entry[0x0B]].unk_08;
    } else {
        *(u32 *)(entry + 0x34) = D_801EB664[entry[0x0B] - 3].unk_00;
        *(u32 *)(entry + 0x38) = D_801EB664[entry[0x0B] - 3].unk_04;
        axis = D_801EB664[entry[0x0B] - 3].unk_08;
    }
    *(u32 *)(entry + 0x3C) = axis;
    func_800AF764();
    work = g_battle_work;
    FIELD_REF(s32, work, 0x10) = -14;
    first = FIELD_REF(u16, work, 0x2E);
    step = work[0x0A];
    second = FIELD_REF(u16, work, 0x30);
    work[9] = 0;
    FIELD_REF(u16, work, 0x2E) = first - 24;
    FIELD_REF(u16, work, 0x30) = second - step;
    g_battle_work[0x0A] = 1;
    g_battle_work[1]++;
}
