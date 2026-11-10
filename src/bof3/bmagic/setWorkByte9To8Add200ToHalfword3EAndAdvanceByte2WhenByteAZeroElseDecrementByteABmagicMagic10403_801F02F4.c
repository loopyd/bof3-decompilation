#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F02F4
 * @behavior Handler published by the table word at 0x801F0900. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and tests byte
 *           0xA before changing it. If zero, stores 8 into byte 0x9, adds
 *           0x200 to unsigned halfword 0x3E, and increments byte 0x2 through
 *           a reloaded work pointer. Otherwise decrements byte 0xA modulo
 *           256. Takes no arguments and returns nothing. Field roles remain
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8Add200ToHalfword3EAndAdvanceByte2WhenByteAZeroElseDecrementByteABmagicMagic10403_801F02F4(void) {
    u8 *work;
    u8 *step;
    u16 value;

    work = g_battle_work;
    if (work[0xA] == 0) {
        work[9] = 8;
        value = FIELD_REF(u16, work, 0x3E);
        step = g_battle_work;
        FIELD_REF(u16, work, 0x3E) = value + 0x200;
        step[2]++;
    } else {
        work[0xA]--;
    }
}
