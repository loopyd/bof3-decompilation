#include "bof3/bof3.h"

/* Shared main-executable lookup helper, outside this payload. Its exact lift
 * applyMainLookupRecord (0x8014DD3C) scans the published main-lookup table for
 * the 16-bit record id it is given and copies that record into the active work
 * record, so the argument here is a record id. This target's generated
 * undefined_funcs_auto row binds the address-anchored name, so the call keeps
 * it. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Shared main-executable resource request helper, outside this payload; the
 * sibling plchar, bmagic and boss lifts declare the same prototype and pass a
 * resource id, and this target's generated undefined_funcs_auto row binds the
 * address-anchored name. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The original re-materialises the cell address with its own lui
 * before every one of the three loads, including the one that follows the byte
 * +0x2A store with no call in between, so each access names the cell instead of
 * keeping a pointer in a local. The exact siblings
 * setFlag40UntilWorkByte9ExpiresThenClearSelectedRecordsWorld02Area08513_801F35A0
 * and invokeHelperWithWorkWorld02Area08513_801F3BA4 of this target use the same
 * spelling. */
extern u8* D_1F800044;

/* @source 0x801F3AF8
 * @behavior Overlay state handler. It clears bit 0 of byte +0x24 of the work
 *           record published at the scratchpad pointer cell 0x1F800044, applies
 *           main lookup record 0x184 through the shared main-executable helper
 *           0x8014DD3C, requests resource 0 through the shared helper
 *           0x8014D6B8, sets that work record's byte +0x2A to 1, and sets bit
 *           0x10 of its byte 0. It reads or writes no other state, takes no
 *           arguments and returns nothing. The 0x18-byte frame only keeps $ra
 *           at 0x10 across the two calls; the 0xFE mask store fills the first
 *           call's delay slot, the second call's delay slot materialises its
 *           zero argument as `addu $a0,$zero,$zero`, the load-delay slots after
 *           the two cell loads that feed a byte access stay nops, and the
 *           `lw $ra` restore is scheduled ahead of the final flag store. No
 *           `jal` of this payload names the body. Its own address is the word at
 *           0x801F42B0, index 17 of the twenty-three-word in-image pointer run
 *           based at 0x801F426C inside the reviewed `textbin` row D_801F3D2C,
 *           and no instruction of this payload references that run's base, so
 *           its consumer lies outside the lifted set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyMainLookupRecord184ClearWorkByte24Bit0RequestResource0ThenSetWorkByte2AAndByte0Flag10World02Area08513_801F3AF8(
    void) {
  D_1F800044[0x24] &= 0xFE;
  func_8014DD3C(0x184);
  func_8014D6B8(0);
  D_1F800044[0x2A] = 1;
  D_1F800044[0] |= 0x10;
}
