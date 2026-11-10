#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 D_801F049C[]; /* @source 0x801F049C @kind data */
extern u8 D_801F04A5[]; /* @source 0x801F04A5 @kind data */
extern u8 D_801F04AF[]; /* @source 0x801F04AF @kind data */
extern u8 D_801F0518[]; /* @source 0x801F0518 @kind data */
extern u8 D_801F04AD[]; /* @source 0x801F04AD @kind data */
extern u32 func_801F03D4(void); /* @source 0x801F03D4 */
extern int rand(void);

/* @source 0x801EF4A8
 * @behavior Increments work byte 9 and acts only when it reaches 16. For each
 *           ordinal below the byte table at 0x801F049C indexed by work byte 4,
 *           claims an entry through 0x801F03D4. Unless the returned byte is
 *           0xFF, scales it by 0x78, clears the byte at 0x801F04A5 plus that
 *           offset, writes the ordinal at 0x801F04AF, publishes the record
 *           pointer at 0x801F0518, and writes (rand() & 15) + 16 at 0x801F04AD.
 *           Increments record byte 0xB for each successful claim, then advances
 *           work byte 2 after the loop. Callback stored at table word
 *           0x801F0490; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenAllocateTableCountEntriesAndAdvanceWorkByte2BmagicMagic07603_801EF4A8(void) {
    u8 i;
    u8 slot;
    u32 offset;
    u8 *record;
    u8 *recordCount;

    if (++g_battle_work[9] == 0x10) {
        for (i = 0; i < D_801F049C[g_battle_work[4]]; i++) {
            slot = func_801F03D4();
            if (slot != 0xFF) {
                offset = slot * 0x78;
                record = D_801EB4E0;
                D_801F04A5[offset] = 0;
                D_801F04AF[offset] = i;
                *(u32 *)&D_801F0518[offset] = (u32)record;
                D_801F04AD[offset] = (rand() & 0xF) + 0x10;
                recordCount = D_801EB4E0;
                recordCount[0xB]++;
            }
        }
        g_battle_work[2]++;
    }
}
