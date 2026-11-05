#include "bof3/battle/battle03_internal.h"

/* @source 0x801EB494 @kind table */
extern u8 D_801EB494[];
void func_8014D6B8(u8 request);

/* @source 0x801E6E54
 * @behavior Clears scratch byte +0x0B, selects the next enabled 0x58 group bit,
 * and returns if the selected index or its request-table byte is 0xFF.
 * Otherwise initializes the scratch work bytes, halfword and motion words,
 * derives its queued-record position, requests the selected table byte through
 * the resident helper, and advances scratch byte +1 by two.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeGroupFlagScratchAndRequest(void) {
    u8 index;
    Battle03LocalWork* work;
    Battle03LocalWork* next_work;
    Battle03LocalWork* motion_work;

    ((u8*)D_1F800044)[0x0b] = 0;
    index = func_801E8FA8();
    if (index == 0xffu) {
        return;
    }
    if (D_801EB494[index] == 0xffu) {
        return;
    }
    ((u8*)D_1F800044)[0x0b] = index;
    ((u8*)D_1F800044)[0x29] = 2;
    ((u8*)D_1F800044)[0x25] = 5;
    ((u8*)D_1F800044)[0x26] = 0xf0;
    ((u8*)D_1F800044)[0x24] = 0x80;
    ((u8*)D_1F800044)[0x27] = 0xff;
    ((u8*)D_1F800044)[0x28] = 0;
    work = D_1F800044;
    ((u8*)work)[0x2b] = 0;
    next_work = D_1F800044;
    FIELD_REF(u16, work, 0x2c) = 1;
    next_work->unk_48 = 0;
    motion_work = D_1F800044;
    motion_work->unk_44 = 0x10000;
    motion_work->unk_40 = 0x10000;
    motion_work->unk_18 = 0x666;
    motion_work->unk_0c = 0;
    ((u8*)motion_work)[0x0a] = 0x3c;
    func_801E8DD8();
    func_8014D6B8(D_801EB494[index]);
    D_1F800044->unk_01 += 2;
}
