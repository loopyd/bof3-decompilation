#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay's work record. The cell is
 * reloaded for every access, which is what the original bytes show. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Step-table builders of this overlay. The builder at 0x801F4544 is lifted in
 * this target and is declared here with its canonical name; the one at
 * 0x801F45C8 is still an asm subsegment, so its role stays address-anchored. */
void projectWorkPositionFromRouteOffsetsTimesFiveAndStoreDistanceAt20World03Area14813_801F4544(
    void);
void func_801F45C8(void);

/* @source 0x801F42C0
 * @behavior Handler at word 1 of the state-byte-1 dispatch run based at
 *           0x801F6160. Copies the work record byte at +0xB into the record
 *           byte at +8, adds 0xC0 to the record halfword at +0x3E, calls the
 *           position step at 0x801F4544 that this target lifts under its own
 *           name, advances the record byte at +8 by 3 modulo 8, calls the
 *           second step-table builder at 0x801F45C8 and then writes 2 to the
 *           record byte at +6, 0x50 to the record byte at +9 and 1 to the
 *           record byte at +1, which keeps this overlay in state 1 of the same
 *           dispatch run. Takes no arguments, returns nothing and keeps a
 *           0x18-byte frame whose only saved register is ra at +0x10; the
 *           scratchpad record pointer cell 0x1F800044 is reloaded for every
 *           access.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkByteBToWorkByte8AdvanceWorkHalfword3eByC0AndSetState1World03Area14813_801F42C0(
    void) {
  D_1F800044[8] = D_1F800044[0xB];
  FIELD_REF(u16, D_1F800044, 0x3E) += 0xC0;
  projectWorkPositionFromRouteOffsetsTimesFiveAndStoreDistanceAt20World03Area14813_801F4544();
  D_1F800044[8] = (D_1F800044[8] + 3) & 7;
  func_801F45C8();
  D_1F800044[6] = 2;
  D_1F800044[9] = 0x50;
  D_1F800044[1] = 1;
}
