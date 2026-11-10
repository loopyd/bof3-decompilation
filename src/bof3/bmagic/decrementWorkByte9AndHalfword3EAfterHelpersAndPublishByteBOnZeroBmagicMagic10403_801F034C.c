#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801EFA54(void); /* @source 0x801EFA54 @kind function */
extern void func_801F03C4(void); /* @source 0x801F03C4 @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F034C
 * @behavior Handler published by table word 0x801F0904. Calls helpers at
 *           0x801EFA54 and 0x801F03C4, then PopMatrix. Decrements work byte
 *           0x9 modulo 256 and subtracts 0x40 from unsigned halfword 0x3E,
 *           storing the byte before the halfword. When the stored byte is
 *           zero, reloads the work pointer, copies byte 0xB to byte 0xB of
 *           the record at 0x801EB4E0, and calls the helper at 0x801E5988.
 *           Takes no arguments and returns nothing. Field roles remain
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndHalfword3EAfterHelpersAndPublishByteBOnZeroBmagicMagic10403_801F034C(void)
{
    u8 *work;
    u8 countdown;

    func_801EFA54();
    func_801F03C4();
    PopMatrix();
    work = g_battle_work;
    countdown = work[9] - 1;
    FIELD_REF(u16, work, 0x3E) -= 0x40;
    work[9] = countdown;
    if (countdown == 0) {
        D_801EB4E0[0xB] = g_battle_work[0xB];
        func_801E5988();
    }
}
