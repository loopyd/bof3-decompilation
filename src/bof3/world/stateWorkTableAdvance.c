#include "bof3/bof3.h"

void stateWorkTableAt800(void);

/* @source 0x801F2C48
 * @behavior Entry 0 of this overlay's nine-word state-handler run at
 *           0x801F36CC, the table the exact dispatcher
 *           callStateTable36World03 indexes with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044, so this is the entry the
 *           run starts from. It calls the overlay's work-table clear at
 *           0x801F3384, which zeroes the lead byte of the first 0x10 entries of
 *           the 0x28-byte-stride record table at 0x800E4800, then loads that
 *           pointer cell once and advances the same work byte at +0x1 by one, so
 *           the next frame dispatches entry 1 at 0x801F2C80. The pointer-cell
 *           load sits after the call because the byte store through the loaded
 *           pointer may alias the cell itself. The function takes no arguments
 *           and returns nothing; its 0x18-byte frame exists only to hold the
 *           return address across the call. It is byte-identical apart from the
 *           call target to the lift of 0x801F2C48 in emi/world00/area028/13,
 *           which was exact when this span was written and additionally clears
 *           the halfword at +0x2E, so the statement order and operand forms
 *           below are that family's with this span's own callee and without that
 *           store; the neighbouring entries 0x801F2D3C and 0x801F2E58 show the
 *           same load-once pointer cell and work byte advance.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkTableAdvance(
    void) {
  u8* work;

  stateWorkTableAt800();
  work = SPAD_PTR_SLOT(u8, 0x44);
  work[1]++;
}
