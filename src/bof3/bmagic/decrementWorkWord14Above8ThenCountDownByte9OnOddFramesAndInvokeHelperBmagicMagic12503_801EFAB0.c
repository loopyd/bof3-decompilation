#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EFAB0
 * @behavior Decrements the scratchpad work object's signed word at 0x14 when
 * it exceeds 8. On odd shared frame-counter values, reloads the work pointer
 * and decrements byte 9. When that byte becomes zero, decrements byte 0xB of
 * the record pointed to by 0x801EB4E0 and invokes the helper at 0x801E5988.
 * This is entry 7 of the handler table at 0x801F05D8. Takes no arguments
 * and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkWord14Above8ThenCountDownByte9OnOddFramesAndInvokeHelperBmagicMagic12503_801EFAB0(void) {
    s32 *fields;
    u8 *work;

    fields = (s32 *)g_battle_work;
    if (fields[5] >= 9) {
        fields[5]--;
    }
    if (D_80143E6C & 1) {
        work = g_battle_work;
        if (--work[9] == 0) {
            D_801EB4E0[0xB]--;
            func_801E5988();
        }
    }
}
