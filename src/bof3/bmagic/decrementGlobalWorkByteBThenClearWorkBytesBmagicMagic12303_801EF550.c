#include "bof3/bof3.h"

void clearWorkBytesBmagicMagic12303_801EFD04(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, and the name is its own
 * D_<address> form, so the binding needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF550
 * @behavior Reads the shared battle work record pointer at 0x801EB4E0 once,
 *           decrements that record's byte 0xB by one at byte width (lbu 0xB /
 *           addiu -1 / sb 0xB) and then invokes this overlay's work-byte clear
 *           routine at 0x801EFD04 unconditionally, the store filling the call's
 *           delay slot. The pointer load precedes frame creation because the
 *           decrement needs no stack; the 0x18-byte frame only keeps $ra across
 *           the call. Takes no arguments, returns nothing and reads or writes
 *           no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBThenClearWorkBytesBmagicMagic12303_801EF550(void) {
  D_801EB4E0[0xB]--;
  clearWorkBytesBmagicMagic12303_801EFD04();
}
