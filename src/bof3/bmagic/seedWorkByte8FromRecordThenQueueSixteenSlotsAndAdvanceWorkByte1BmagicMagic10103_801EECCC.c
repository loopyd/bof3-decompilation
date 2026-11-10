#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern u8 D_80146374; /* @source 0x80146374 @kind data */
extern u8 *D_8014637C; /* @source 0x8014637C @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC334[]; /* @source 0x801EC334 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EECCC
 * @behavior First handler in the five-word run at 0x801EEC04, dispatched by
 *           func_801EEC18 using work byte 1. Copies record byte 8 from the
 *           pointer at 0x8014637C into work byte 8 when flag 0x40 at 0x80146384
 *           is set and byte 0x80146374 is at least 3, or when that flag is clear
 *           and the byte is below 3; otherwise XORs the copied byte with 2.
 *           Calls func_800AFD9C, clears work bytes B and A, sets byte 9 to 0x30
 *           and advances byte 1. Claims sixteen action slots with (1, 0x1B),
 *           scaling each low-byte result by 0x78. Clears slot byte 1, sets byte
 *           4 to the ordinal and byte 9 to (ordinal / 4) * 12 + 1, publishes
 *           the work pointer in slot word 0x74 and increments work byte B.
 *           Finally calls func_8015D71C(0x100). Takes no arguments and returns
 *           nothing. Unknown helper and flag roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte8FromRecordThenQueueSixteenSlotsAndAdvanceWorkByte1BmagicMagic10103_801EECCC(void) {
    u32 index;
    u8 slot;
    u32 slotOffset;
    u8 *published;
    u8 *record;

    if (D_80146384 & 0x40) {
        if (D_80146374 >= 3) {
            g_battle_work[8] = D_8014637C[8];
        } else {
            g_battle_work[8] = D_8014637C[8] ^ 2;
        }
    } else {
        if (D_80146374 < 3) {
            g_battle_work[8] = D_8014637C[8];
        } else {
            g_battle_work[8] = D_8014637C[8] ^ 2;
        }
    }

    func_800AFD9C();
    g_battle_work[0xB] = 0;
    g_battle_work[9] = 0x30;
    g_battle_work[0xA] = 0;
    g_battle_work[1]++;
    for (index = 0; index < 16; index++) {
        slot = (u8)func_801E590C(1u, 0x1Bu);
        slotOffset = slot << 4;
        slotOffset -= slot;
        slotOffset <<= 3;
        published = g_battle_work;
        D_801EC331[slotOffset] = 0;
        D_801EC334[slotOffset] = index;
        D_801EC339[slotOffset] = (index >> 2) * 12 + 1;
        record = g_battle_work;
        *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
        record[0xB]++;
    }
    func_8015D71C(0x100);
}
