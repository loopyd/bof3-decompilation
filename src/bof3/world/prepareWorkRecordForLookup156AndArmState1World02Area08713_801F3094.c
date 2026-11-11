#include "bof3/bof3.h"

/* Scratchpad work-record cursor cell 0x1F800044, bound by this target's own
 * map row. Binding it as a named object keeps the original's per-store address
 * folding: written as a literal-address dereference the build held the whole
 * 0x1F800044 base in a saved register (lui + ori, then lw 0(base)) and grew the
 * frame, while the named cell emits the original's lui %hi plus offset lw at
 * every store. */
extern u8* D_1F800044;

/* Shared main-executable lookup helper. Its lifted body at 0x8014DD3C scans
 * the published main lookup table and copies the record whose 16-bit key
 * matches into the active work record. The overlay reaches it through
 * undefined_funcs_auto as func_8014DD3C, so the address-anchored name stays
 * here, and the body below discards its result. */
void func_8014DD3C(u16 entry); /* @source 0x8014DD3C */

/* @source 0x801F3094
 * @behavior Overlay state-1 handler: entry 0 of the handler table at
 *           0x801F5734, which the overlay update func_801F2F20 indexes with the
 *           work-record byte at 0x01. Stores 0x80 in work byte 0x24, applies
 *           main lookup entry 0x156 through the resident 0x8014DD3C helper,
 *           stores 5 in work byte 0x29, clears work bytes 0x2A and 0x5D..0x5F,
 *           and finally arms the state byte at 0x01 with 1 so the next dispatch
 *           runs the state-1 handler. The scratchpad pointer cell 0x1F800044 is
 *           re-read before every store group, matching the original's per-store
 *           reloads. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void prepareWorkRecordForLookup156AndArmState1World02Area08713_801F3094(void) {
  D_1F800044[0x24] = 0x80;
  func_8014DD3C(0x156);
  D_1F800044[0x29] = 5;
  D_1F800044[0x2A] = 0;
  D_1F800044[0x5D] = 0;
  D_1F800044[0x5E] = 0;
  D_1F800044[0x5F] = 0;
  D_1F800044[0x01] = 1;
}
