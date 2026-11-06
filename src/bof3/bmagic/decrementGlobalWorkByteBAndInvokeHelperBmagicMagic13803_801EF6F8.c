#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF6F8
 * @behavior Overlay header handler, index 8 of the handler-pointer run at
 *           0x801EEC04 (the payload word at 0x801EEC24, its only in-image
 *           reference). Reads the shared battle work record pointer at
 *           0x801EB4E0 once, decrements that record's byte 0xB by one at byte
 *           width (lbu 0xB / addiu -1 / sb 0xB) and then invokes the shared
 *           helper at 0x801E5988 unconditionally, the store filling the call's
 *           delay slot. The pointer load precedes frame creation because the
 *           decrement needs no stack; the 0x18-byte frame only keeps $ra across
 *           the call. Takes no arguments, returns nothing and reads or writes
 *           no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperBmagicMagic13803_801EF6F8(void) {
  D_801EB4E0[0xB]--;
  func_801E5988();
}
