#include "bof3/bof3.h"

/* Shared record pointer cell at 0x1F800044. Every store this body makes to the
 * work record goes through the pointer it holds: one cell read for the three
 * byte stores of the +0x5F/+0x5E/+0x5D triplet, then a fresh cell read for the
 * +0x09 byte and another for the +0x01 handoff, because the stores that precede
 * each of them go through the loaded pointer and may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared halfword at 0x8014932C. This body stores 0x1C80 into it, and the
 * scenario overlays publish camera channels through the same cell
 * (dispatchSubStateByteTable6C6CScenarioScena1400_801F7FB8 stores 1900/0xF0
 * ramp values there), so the role stays address-anchored here. Store width
 * comes from the original's single sh.
 * @source 0x8014932C @kind data
 */
extern u16 D_8014932C;

/* Shared mode byte at 0x80149333. This body stores 2; the scenario overlays
 * publish the same mode value through it, and its role in this overlay stays
 * address-anchored. Store width comes from the original's single sb.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared record flag byte at 0x80145ED8. This body stores 1 into each of the
 * three flag bytes below and reads none of them.
 * @source 0x80145ED8 @kind data
 */
extern u8 D_80145ED8;

/* @source 0x80146018 @kind data */
extern u8 D_80146018;

/* @source 0x80146158 @kind data */
extern u8 D_80146158;

/* @source 0x801F2C48
 * @behavior Entry 0 of this overlay's byte-1 handler run based at 0x801F37D0,
 *           the four-word pointer table whose words are 0x801F2C48, 0x801F2CB4,
 *           0x801F2CD4 and 0x801F2DAC in that order and which the exact
 *           dispatcher dispatchByte1Table37d0
 *           indexes with the unsigned work byte at +0x01 of the record
 *           published at the scratchpad pointer cell 0x1F800044, so this body
 *           is the step that dispatcher takes while that byte reads 0. It
 *           stores 0x1C80 into the shared halfword 0x8014932C, publishes 2 in
 *           the shared mode byte 0x80149333, stores 1 into the shared record
 *           flag bytes 0x80145ED8, 0x80146018 and 0x80146158, fills the work
 *           record's bytes +0x5F, +0x5E and +0x5D with 0x7F, arms the record
 *           countdown byte +0x09 with 0x96, and stores 1 into the record
 *           dispatch byte +0x01, which hands the record to entry 1 (0x801F2CB4)
 *           on the next frame. Takes no arguments, calls nothing and returns
 *           nothing; the body is a leaf with no frame, the record pointer is
 *           loaded once for the three byte stores of the triplet and re-read
 *           before each of the two later byte stores because the stores that
 *           precede them go through the loaded pointer, and the trailing sb
 *           fills the return-branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateChannelFlagsTriple(
    void) {
  u8* work;

  D_8014932C = 0x1C80;
  work = D_1F800044;
  D_80149333 = 2;
  D_80145ED8 = 1;
  D_80146018 = 1;
  D_80146158 = 1;
  work[0x5F] = 0x7F;
  work[0x5E] = 0x7F;
  work[0x5D] = 0x7F;
  D_1F800044[9] = 0x96;
  D_1F800044[1] = 1;
}
