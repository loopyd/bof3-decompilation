#include "bof3/bof3.h"

/* Scratchpad work-record cursor cell 0x1F800044. Declaring it as a named
 * object rather than a literal-address dereference keeps the original
 * per-store-group reloads: gcc may not carry a global pointer value across a
 * store through an unknown pointer, while a literal-address dereference is
 * treated as invariant and hoisted into a callee-saved register. */
/* @source 0x1F800044 @kind unknown */
extern u8* stateWorkCursor;

/* Shared main-executable lookup helper: its exact lift at 0x8014DD3C
 * (src/bof3/world/applyMainLookupRecord.c) scans the published main lookup
 * table and copies the record whose 16-bit key matches into the active work
 * record. The overlay reaches it through undefined_funcs_auto as
 * func_8014DD3C, so the address-anchored name stays here. */
void func_8014DD3C(u16 entry); /* @source 0x8014DD3C */

/* @source 0x801F3098
 * @behavior Prepares the scratch work record for the overlay's byte-1 state 1:
 *           stores 0x80 (bit 7) in work byte 0x24, applies main lookup entry
 *           0x53 through the resident 0x8014DD3C helper, stores 5 in work byte
 *           0x29, clears work byte 0x2A and the work colour triplet at
 *           0x5D/0x5E/0x5F, and finally arms the state byte at 0x01 with 1 so
 *           the next dispatch through the local table 0x801F5288 runs the
 *           state-1 handler. The scratchpad pointer cell 0x1F800044 is re-read
 *           before every store group, matching the original's per-store reloads.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePrepareRecordArm(void) {
  stateWorkCursor[0x24] = 0x80;
  func_8014DD3C(0x53);
  stateWorkCursor[0x29] = 5;
  stateWorkCursor[0x2A] = 0;
  stateWorkCursor[0x5D] = 0;
  stateWorkCursor[0x5E] = 0;
  stateWorkCursor[0x5F] = 0;
  stateWorkCursor[0x01] = 1;
}
