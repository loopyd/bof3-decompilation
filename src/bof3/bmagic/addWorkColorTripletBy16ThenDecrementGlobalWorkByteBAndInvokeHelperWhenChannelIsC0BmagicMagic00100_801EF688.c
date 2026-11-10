#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
/* @source 0x801E5988 @kind function */
extern void func_801E5988(void);

/* @source 0x801EF688
 * @behavior Adds 0x10 to work-object colour bytes 0x5D, 0x5E and 0x5F,
 *           reading the scratchpad pointer cell at 0x1F800044 before each
 *           update. Re-reads the cell and tests byte 0x5D as signed; when it
 *           equals -0x40 (0xC0), decrements byte 0xB of the record read through
 *           0x801EB4E0 and invokes the no-argument helper at 0x801E5988.
 *           Registered at 0x801EEC38 in the leading handler pointer run.
 *           Takes no arguments and returns nothing. The 0x18-byte frame
 *           preserves the return address across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAndInvokeHelperWhenChannelIsC0BmagicMagic00100_801EF688(void) {
    D_1F800044[0x5D] += 0x10;
    D_1F800044[0x5E] += 0x10;
    D_1F800044[0x5F] += 0x10;

    if (((s8 *)D_1F800044)[0x5D] == -0x40) {
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
