#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF890
 * @behavior Decrements the scratchpad work object's unsigned countdown byte 9.
 * On zero, copies the shared battle record's words 0x34, 0x38 and 0x3C to the
 * work object, sets its tint bytes 0x5D through 0x5F to 15, sets word 0x14 to
 * 0x100 and word 0x20 to 0x21, reloads byte 9 to 0x10 and byte 0xA to 0x1E,
 * and increments byte 2. The source record comes from the pointer at
 * 0x801EB4E0; the work pointer comes from the scratchpad cell at 0x1F800044.
 * The word-0x3C load precedes the first tint store and its store follows it.
 * Later store groups re-read the work pointer because object stores may alias
 * that cell. This is a frame-free leaf with no arguments or return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyRecordTripletSetTintAndWorkFieldsAndAdvanceWorkByte2BmagicMagic12503_801EF890(void) {
    u32 *source;
    u32 *work;
    u32 value;
    u32 *fields;

    if (--g_battle_work[9] == 0) {
        source = (u32 *)D_801EB4E0;
        work = (u32 *)g_battle_work;
        work[13] = source[13];
        work[14] = source[14];
        value = source[15];
        ((u8 *)work)[0x5D] = 15;
        work[15] = value;
        g_battle_work[0x5E] = 15;
        g_battle_work[0x5F] = 15;
        fields = (u32 *)g_battle_work;
        fields[5] = 0x100;
        fields[8] = 0x21;
        ((u8 *)fields)[9] = 0x10;
        g_battle_work[0xA] = 0x1E;
        g_battle_work[2]++;
    }
}
