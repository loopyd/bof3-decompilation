#include "bof3/bof3.h"

/* Published work-record pointer cell. The original re-materialises the cell
 * address and reloads the pointer after the frame-byte store, so both loads
 * stay in the body. */
/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801469E6 @kind unknown */
extern u16 workSeedHalfword801469E6;
/* @source 0x801469E8 @kind unknown */
extern u16 workSeedHalfword801469E8;
/* @source 0x801469EA @kind unknown */
extern u16 workSeedHalfword801469EA;

/* @source 0x801F322C
 * @behavior Overlay step handler: entry 3 of the seven-word handler run based
 * at 0x801F3ED8 and entry 0 of the run based at 0x801F3EE4, the table this
 * target's exact dispatcher
 * dispatchStateByte1Table3ee4World03Area13013_801F31E8 indexes with the work
 * record's state byte at +1. Arms the record's frame byte at +0x09 with 0x14,
 * clears the record's words at +0x0C and +0x10, copies the shared halfwords at
 * 0x801469E6/0x801469E8/0x801469EA into the record's halfwords at
 * +0x2E/+0x30/+0x32, and advances the state byte at +0x01 by one so the next
 * frame dispatches entry 1. Takes no arguments and returns nothing. The roles
 * of the record's halfword at +0x32 and of the three shared halfwords stay
 * described by offset.
 *
 * Matching note: the shared halfword at 0x801469EA is read before the frame
 * byte at +0x09 is armed, so that load sits ahead of the arm store while the
 * store that consumes it is scheduled after the cell reload; spelling the
 * store inline instead measured 88->96 bytes with a first difference at
 * +0x0000, because the compiler then reloads the cell again for the later
 * stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownByte9ClearWords0cAnd10SeedHalfwords2eTo32ThenAdvanceWorkByte1World03Area13013_801F322C(
    void) {
  u8* work;
  u8* state;
  u16 seed32;

  work = workRecordCursor1F800044;
  seed32 = workSeedHalfword801469EA;
  work[9] = 0x14;
  state = workRecordCursor1F800044;
  *(u16*)(work + 0x32) = seed32;
  *(s32*)(state + 0x0C) = 0;
  *(s32*)(state + 0x10) = 0;
  *(u16*)(state + 0x2E) = workSeedHalfword801469E6;
  *(u16*)(state + 0x30) = workSeedHalfword801469E8;
  state[1] += 1;
}
