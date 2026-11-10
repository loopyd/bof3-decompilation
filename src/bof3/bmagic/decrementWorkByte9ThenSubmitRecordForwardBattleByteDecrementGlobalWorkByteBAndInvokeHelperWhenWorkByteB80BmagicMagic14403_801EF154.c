#include "bof3/bof3.h"

void func_80196718(void *arg0);
void func_800AF814(u8 arg0);
void func_801E5988(void);

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF154
 * @behavior Decrements unsigned work byte 9 when nonzero, then re-reads the
 *           scratchpad work pointer at 0x1F800044 and tests work byte 0xB for
 *           equality with 0x80. On equality, submits the record pointer from
 *           0x8014638C to 0x80196718, forwards the unsigned battle byte at
 *           0x80146384 to 0x800AF814, decrements byte 0xB of the record at
 *           pointer cell 0x801EB4E0, and invokes 0x801E5988. Takes no arguments
 *           and returns nothing. The sole in-payload reference to this entry
 *           is the handler pointer at 0x801F04E0. Helper roles and record fields
 *           beyond these accesses remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSubmitRecordForwardBattleByteDecrementGlobalWorkByteBAndInvokeHelperWhenWorkByteB80BmagicMagic14403_801EF154(void)
{
    if (g_battle_work[9] != 0) {
        g_battle_work[9]--;
    }
    if (g_battle_work[0xB] == 0x80) {
        func_80196718(D_8014638C);
        func_800AF814(D_80146384);
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
