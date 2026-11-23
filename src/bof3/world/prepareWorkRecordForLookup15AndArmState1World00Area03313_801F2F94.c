#include "bof3/bof3.h"

/* Scratchpad work-record cursor cell 0x1F800044. The original folds the cell's
 * address with lui %hi and reads it with an offset lw before every store, so
 * the body below spells the cell as a named object and re-reads it per store. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared main-executable lookup helper. This target's generated
 * undefined_funcs_auto row binds the address-anchored name, and the sibling
 * seedFacingStepPairAndStepPositionThenAdvanceModeAndRequestResourceWorld00Area03313_801F422C
 * of this target declares the same one-argument prototype. */
extern void func_8014DD3C(u16 entry);

/* @source 0x801F2F94
 * @behavior State-handler word 0 of this overlay's handler run based at
 *           0x801F4F64, which the overlay update func_801F2E20 indexes with
 *           work-record byte 0x01. It stores 0x80 in work byte 0x24, applies
 *           main lookup entry 0xF through the resident func_8014DD3C helper,
 *           stores 5 in work byte 0x29, clears work bytes 0x2A and 0x5D..0x5F,
 *           and finally arms the state byte at 0x01 with 1 so the next dispatch
 *           runs the state-1 handler. The scratchpad pointer cell 0x1F800044 is
 *           re-read before every store, matching the original's seven reloads.
 *           Takes no arguments, returns nothing and keeps only the return
 *           address in its 0x18-byte frame.
 *           Its 0x8C bytes differ from the exact
 *           prepareWorkRecordForLookup158AndArmMode1World03Area12114_801F3078
 *           lift in emi/world03/area121/14 (span sha1
 *           54e29c2ad9774f5075a90e287288d119372ed2e6) in the 0x158/0xF lookup-id
 *           immediate alone, one cmp of the two payload spans; that sibling
 *           supplied this body's statement order, operand forms and name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void prepareWorkRecordForLookup15AndArmState1World00Area03313_801F2F94(void) {
  D_1F800044[0x24] = 0x80;
  func_8014DD3C(0xF);
  D_1F800044[0x29] = 5;
  D_1F800044[0x2A] = 0;
  D_1F800044[0x5D] = 0;
  D_1F800044[0x5E] = 0;
  D_1F800044[0x5F] = 0;
  D_1F800044[0x01] = 1;
}
