#include "bof3/bof3.h"

/* Scratchpad work-record cursor cell 0x1F800044, bound by this target's own map
 * row. Naming the cell keeps the original's per-store address folding: written
 * as a constant-address dereference the build materialized 0x1F800044 once into
 * a callee-saved register (lui + ori, then lw 0(base)) and grew the frame, while
 * the named cell emits the original's lui %hi plus offset lw at every store. */
extern u8* D_1F800044;

/* Shared main-executable lookup helper: its exact lift at 0x8014DD3C
 * (src/bof3/world/applyMainLookupRecord.c) scans the published main lookup
 * table and copies the record whose 16-bit key matches into the active work
 * record. This overlay reaches it through undefined_funcs_auto as
 * func_8014DD3C, so the address-anchored name stays here, and the body below
 * discards the result. */
void func_8014DD3C(u16 entry); /* @source 0x8014DD3C */

/* @source 0x801F3BC4
 * @behavior Overlay state-1 handler: entry 0 of the handler table at
 *           0x801F61A0, which the overlay update at 0x801F3A50 indexes with
 *           work byte 0x01. Stores 0x80 in work byte 0x24, applies main lookup
 *           entry 0x157 through the resident 0x8014DD3C helper, stores 5 in
 *           work byte 0x29, clears work byte 0x2A and the triplet at
 *           0x5D/0x5E/0x5F, and finally arms work byte 0x01 with 1 so the next
 *           dispatch runs entry 1 of the same table, the 0x801F3C50 handler.
 *           The scratchpad pointer cell 0x1F800044 is re-read before every
 *           store, matching the original's per-store reloads. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void prepareWorkRecordForLookup157AndArmState1World02Area08813_801F3BC4(void) {
  D_1F800044[0x24] = 0x80;
  func_8014DD3C(0x157);
  D_1F800044[0x29] = 5;
  D_1F800044[0x2A] = 0;
  D_1F800044[0x5D] = 0;
  D_1F800044[0x5E] = 0;
  D_1F800044[0x5F] = 0;
  D_1F800044[0x01] = 1;
}
