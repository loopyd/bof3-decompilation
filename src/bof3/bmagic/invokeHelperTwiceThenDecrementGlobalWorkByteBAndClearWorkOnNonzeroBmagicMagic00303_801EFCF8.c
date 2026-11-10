#include "base/types.h"

extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void clearWorkBytesBmagicMagic00303_801F0A28(void); /* @source 0x801F0A28 @kind function */

/* @source 0x801EFCF8
 * @behavior Calls helper 0x8014DAEC twice and discards the first result.
 *           When the second returned byte is nonzero, reads the shared work
 *           pointer, decrements byte 0xB with wraparound, and calls the owned
 *           helper that clears scratchpad work bytes 0 through 4.
 *           Takes no arguments and returns nothing; field roles are structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperTwiceThenDecrementGlobalWorkByteBAndClearWorkOnNonzeroBmagicMagic00303_801EFCF8(void) {
    func_8014DAEC();
    if (func_8014DAEC() != 0) {
        u8 *work = D_801EB4E0;

        work[0xB]--;
        clearWorkBytesBmagicMagic00303_801F0A28();
    }
}
