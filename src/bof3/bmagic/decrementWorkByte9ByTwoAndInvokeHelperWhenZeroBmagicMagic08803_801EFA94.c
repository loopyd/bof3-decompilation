#include "bof3/bof3.h"

/* Shared battle helper in main RAM, outside this overlay's image. This target's
 * generated undefined_funcs_auto.txt binds the address, so no map row is
 * needed; the call site passes no argument and reads no return value. */
void func_801E5988(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map as g_battle_work. The original materialises the cell twice - once for the
 * byte-9 store group and once again for the zero test - because the preceding
 * byte store can alias the pointer cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFA94
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044, subtracts two
 *           from the object's countdown byte 0x9 at byte width (lbu / addiu -2
 *           / sb), then re-reads the cell and loads byte 0x9 again, so the
 *           reloaded byte - not the stored register - supplies the tested value
 *           (lbu $v0,0x9($v0) / bnez). Only when that byte is zero does it call
 *           the shared helper at 0x801E5988, with the call's delay slot left
 *           empty. The pointer-cell load precedes frame creation; the 0x18-byte
 *           frame only keeps $ra at 0x10 across the call. Takes no arguments,
 *           returns nothing and reads or writes no other state.
 *
 *           No jal in this payload reaches it. Its address is the fourth
 *           pointer word, zero-based entry 3, of the four-entry handler run at
 *           0x801F005C (0x801EF990, 0x801EF9DC, 0x801EFA2C, 0x801EFA94) that
 *           ends the reviewed textbin row D_801EFFE0, and no instruction in
 *           this payload materialises that run's base, so its caller is the
 *           overlay loader or a script-invoked callback.
 *
 *           The first eight instructions through the byte-9 store are
 *           byte-identical to the opening of the exact sibling
 *           decrementWorkByte9ByTwoThenDecrementGlobalWorkByteBAndInvokeHelper
 *           WhenZeroBmagicMagic17203_801EF4BC, which this body's statement
 *           order and operand forms follow. That sibling differs at the test
 *           site (it tests the stored register with andi/bnez) and carries one
 *           extra store of the shared global work byte 0xB that this span does
 *           not have. A comparison of this 84-byte span against every
 *           out/binaries payload found no byte-identical copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ByTwoAndInvokeHelperWhenZeroBmagicMagic08803_801EFA94(void) {
  g_battle_work[9] -= 2;
  if (g_battle_work[9] == 0) {
    func_801E5988();
  }
}
