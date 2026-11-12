#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell. This body reads the cell twice, once for
 * the byte-9 update and once for the byte-0x5D update, so it stays the named
 * cell and each access keeps its own lui/lw pair; the target map binds the
 * name. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The word-typed declaration and the derived byte view follow this target's
 * other sources; the target map binds the name. */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F058C
 * @behavior Entry index 7 of the eight-word dispatcher code-pointer run at
 *           0x801F1BF4 that 0x801F0100 indexes with the scratchpad work
 *           object's byte 2 scaled by four. Adds 2 to the work-object byte at
 *           offset 0x9 at byte width, then re-reads the pointer cell at
 *           0x1F800044 and subtracts 4 from the work-object byte at offset
 *           0x5D at byte width (lbu / addiu -4 / sb) and tests that stored
 *           value truncated to eight bits, the signed-byte view, so no second
 *           cell read supplies the guard. Only when that byte reaches zero does
 *           it read the shared battle work record pointer at 0x801EB4E0,
 *           decrement that record's byte 0x0B by one and invoke the shared
 *           helper at 0x801E5988, the store filling the call delay slot. The
 *           first cell load precedes frame creation; the 0x18-byte frame only
 *           keeps $ra at 0x10 across the call. Takes no arguments, returns
 *           nothing and reads or writes no other state. The byte at 0x5D is the
 *           same signed work-object tint channel this target's seeder
 *           0x801F04C0 initializes to 0x10; the drawing row 0x801F06B4 reads it
 *           with lb and scales it by eight into the scratchpad halfwords at
 *           0x1F800008, 0x1F80000A and 0x1F80000C, where 0x10 * 8 fills the byte
 *           range, and the shared draw path 0x801F7134 binds work byte 0x5D to
 *           the sprite's r0 tint byte, so the channel role holds here too.
 *           Shape came from the exact
 *           sibling
 *           decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F0CB0,
 *           which carries the same byte-width channel update, signed
 *           truncation and guarded record-byte decrement but not this body's
 *           leading byte-9 step.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9DecrementWorkTintChannel5DBy4ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic13103_801F058C(void) {
    u8 *work;
    s8 channel;

    g_battle_work[9] += 2;
    work = g_battle_work;
    channel = work[0x5D] - 4;
    work[0x5D] = channel;
    if (channel == 0) {
        u8 *object;

        object = (u8 *)record801EB4E0;
        object[0xB]--;
        func_801E5988();
    }
}
