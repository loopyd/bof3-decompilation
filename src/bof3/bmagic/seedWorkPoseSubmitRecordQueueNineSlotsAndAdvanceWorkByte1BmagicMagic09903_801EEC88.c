#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind data */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC333[]; /* @source 0x801EC333 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */
extern void func_80196718(void *record); /* @source 0x80196718 */
extern s32 func_8019651C(void *record, s32 x, s32 y, s32 z, s32 mode); /* @source 0x8019651C */
extern u32 func_801E590C(u32 kind, u32 mode); /* @source 0x801E590C */
extern void func_8015D71C(u16 value); /* @source 0x8015D71C */

/* @source 0x801EEC88
 * @behavior First entry of the five-word handler run at 0x801EEC04, selected
 *           by work byte 1. Copy byte 8 and position words 0x34/0x38/0x3C
 *           from the record in 0x8014638C to the scratchpad work object.
 *           Submit that record to 0x80196718 and 0x8019651C with arguments
 *           0/0/0/1, storing the latter result in work byte A. Claim nine
 *           action slots through 0x801E590C(1, 12), each of stride 0x78:
 *           the first clears byte 1, the next four set byte 1 to 1, byte 3
 *           to ordinal 0..3 and byte B to ordinal times four, and the last
 *           four set byte 1 to 2 and byte 3 to ordinal 0..3. Each slot gets
 *           the current work pointer at word 0x74. Invoke 0x8015D71C(0x100),
 *           set work byte 9 to 8, clear byte B and advance byte 1. The
 *           helper roles and slot-field meanings remain address/offset anchored.
 * @status partial
 * @match 74.59
 * @residual 91/122 instructions at 488/488 bytes. First difference +0x94:
 *           the allocator result is masked into v0 rather than v1. Scaled
 *           slot offsets and the loop store scheduling also differ.
 */
void seedWorkPoseSubmitRecordQueueNineSlotsAndAdvanceWorkByte1BmagicMagic09903_801EEC88(void) {
    u8 *record;
    u8 *work;
    u8 *first;
    u8 *band1;
    u8 *band2;
    u32 offset;
    u32 i;

    record = D_8014638C;
    g_battle_work[8] = record[8];
    work = g_battle_work;
    ((u32 *)work)[0xD] = ((u32 *)record)[0xD];
    ((u32 *)work)[0xE] = ((u32 *)record)[0xE];
    ((u32 *)work)[0xF] = ((u32 *)record)[0xF];
    func_80196718(record);
    g_battle_work[0xA] = func_8019651C(record, 0, 0, 0, 1);
    offset = (u8)func_801E590C(1, 12) * 0x78u;
    first = g_battle_work;
    D_801EC331[offset] = 0;
    *(u8 **)(D_801EC3A4 + offset) = first;
    for (i = 0; i < 4u; i++) {
        offset = (u8)func_801E590C(1, 12) * 0x78u;
        band1 = g_battle_work;
        D_801EC333[offset] = i;
        D_801EC331[offset] = 1;
        D_801EC33B[offset] = i * 4;
        *(u8 **)(D_801EC3A4 + offset) = band1;
    }
    for (i = 0; i < 4u; i++) {
        offset = (u8)func_801E590C(1, 12) * 0x78u;
        band2 = g_battle_work;
        D_801EC333[offset] = i;
        D_801EC331[offset] = 2;
        *(u8 **)(D_801EC3A4 + offset) = band2;
    }
    func_8015D71C(0x100);
    g_battle_work[9] = 8;
    g_battle_work[0xB] = 0;
    g_battle_work[1]++;
}
