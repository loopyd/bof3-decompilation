#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF1B8
 * @behavior Decrement scratchpad work byte 9, add two to work byte 0xB,
 * then increment work byte 0xA, all modulo 256. Reload the pointer cell
 * at 0x1F800044 for each update group because the byte stores can alias it.
 * When the truncated byte-A result equals 0x10, set byte 0xB of the record
 * pointed to by 0x801EB4E0 to 0xFF and call the helper at 0x801E5988.
 * Takes no arguments and returns nothing. A pointer word at 0x801EFC70
 * names this handler. Its 136 original bytes equal the span at 0x801EF1B8
 * in magic091/03 (payload cmp at offset 0x5B8).
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AddTwoToWorkByteBIncrementWorkByteAThenSetGlobalWorkByteBToFFAndInvokeHelperAt10BmagicMagic05503_801EF1B8(void)
{
    u8 **slots;
    u8 *work;
    u8 value;

    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x09] -= 1;
    slots[0x11][0x0B] += 2;
    work = slots[0x11];
    value = work[0x0A] + 1;
    work[0x0A] = value;
    if (value == 0x10) {
        D_801EB4E0[0x0B] = 0xFF;
        func_801E5988();
    }
}
