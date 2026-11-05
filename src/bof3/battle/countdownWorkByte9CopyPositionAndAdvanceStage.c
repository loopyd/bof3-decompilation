#include "bof3/battle/battle15_internal.h"

void func_800AF764(void);

/* @source 0x800AE574
 * @behavior Decrements work byte +0x09 modulo 256. On zero, copies the
 *   current local record's words +0x34 through +0x3C to scratch work, calls
 *   func_800AF764, sets counter +0x10 to -14, clears byte +0x09, subtracts
 *   24 from halfword +0x2E and byte +0x0A from halfword +0x30, sets byte
 *   +0x0A to 1, and advances work byte +0x01.
 * @status partial
 * @match 82.69
 * @residual 43/52 instructions match at 208 versus 204 bytes. Post-call
 *   halfword/byte loads move before the counter store, adding a load-delay
 *   nop; halfword temporaries and stores also differ in allocation/order.
 */
void countdownWorkByte9CopyPositionAndAdvanceStage(void)
{
    u8 *entry;
    u8 counter;
    u32 *source;
    u32 *destination;
    u8 *work;
    u16 first;
    u8 step;
    u16 second;

    entry = g_battle_work;
    counter = entry[9] - 1;
    entry[9] = counter;
    if (counter == 0) {
        source = (u32 *)D_801EB4E0;
        destination = (u32 *)g_battle_work;
        destination[13] = source[13];
        destination[14] = source[14];
        destination[15] = source[15];
        func_800AF764();
        work = g_battle_work;
        FIELD_REF(s32, work, 0x10) = -14;
        first = FIELD_REF(u16, work, 0x2E) - 24;
        step = work[0x0A];
        second = FIELD_REF(u16, work, 0x30) - step;
        work[9] = 0;
        FIELD_REF(u16, work, 0x2E) = first;
        FIELD_REF(u16, work, 0x30) = second;
        g_battle_work[0x0A] = 1;
        g_battle_work[1]++;
    }
}
