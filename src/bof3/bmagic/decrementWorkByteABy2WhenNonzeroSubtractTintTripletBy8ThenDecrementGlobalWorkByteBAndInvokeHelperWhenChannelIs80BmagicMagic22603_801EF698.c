#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
/* @source 0x801E5988 @kind function */
extern void func_801E5988(void);

/* @source 0x801EF698
 * @behavior Subtracts 2 from work byte 0xA when it is nonzero, then subtracts
 *           8 from tint bytes 0x5D, 0x5E and 0x5F. Reloads the scratchpad
 *           work pointer at 0x1F800044 before each subsequent access group.
 *           When the signed tint byte 0x5D equals -0x80, decrements byte 0xB
 *           of the record read through 0x801EB4E0 and invokes the helper at
 *           0x801E5988. All updates wrap at byte width. Registered by the
 *           handler pointer at 0x801F0C54. Takes no arguments and returns
 *           nothing; preserves the return address in a 0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteABy2WhenNonzeroSubtractTintTripletBy8ThenDecrementGlobalWorkByteBAndInvokeHelperWhenChannelIs80BmagicMagic22603_801EF698(void) {
    if (g_battle_work[0xA] != 0) {
        g_battle_work[0xA] -= 2;
    }
    g_battle_work[0x5D] -= 8;
    g_battle_work[0x5E] -= 8;
    g_battle_work[0x5F] -= 8;

    if (((s8 *)g_battle_work)[0x5D] == -0x80) {
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
