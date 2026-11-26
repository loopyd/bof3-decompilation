#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;

/* The local layer drawer narrows both arguments with `sll`/`sra 16` in its own
 * prologue and this call site consumes no result. */
void drawMarkerSpriteLayers(
    s16 arg0, s16 arg1);

/* @source 0x801F34CC
 * @behavior Byte-2 state handler 3 of the work-byte-2 handler run based at
 *           0x801F4D74 in this overlay, whose four words read 0x801F33D0,
 *           0x801F3420, 0x801F347C and 0x801F34CC; the same word also sits at
 *           0x801F4D80, index 10 of the in-image code-pointer run from
 *           0x801F4D58 through 0x801F4D90, and no `jal` in the payload names
 *           this address, so that run is its only entry. It subtracts 0x10 from
 *           the signed halfword at offset 0x2E of the work record published at
 *           the scratchpad pointer cell 0x1F800044 and stores the truncated
 *           halfword, then clears state byte 0x02 while the stored value stays
 *           below -0x2F. Unless the shared byte 0x801454F2 reads 2 it writes 1
 *           to state byte 0x02 through a second read of the cell. Finally it
 *           reads the cell again and calls the local drawer 0x801F39C4 with
 *           0x10 and the signed halfword at offset 0x2E. Each of the three
 *           reads of the named cell 0x1F800044 emits its own `lui`/`lw` base
 *           pair; a fixed-address spelling of that cell instead materialises
 *           the absolute address once in `$a0` with an `ori`, which measured
 *           25 of 33 instructions. It takes no arguments,
 *           uses no result, and its 0x18-byte frame holds the return address
 *           only. One payload cmp measured its 0x84 bytes equal to the reviewed
 *           decrementHalfword2eBy10AndSetState2To0BelowMinus2fOrTo1Unless454f2Is2ThenDrawHalfword2eWorld03Area12114_801F3620
 *           span of emi/world03/area121/14 at payload offset 0xA20 apart from
 *           the `jal` word at offset 0x6C, which carries each overlay's own
 *           drawer address (0x801F39C4 here, 0x801F3B18 there) and shifts by
 *           the 0x154 delta between the two spans, so that exact body supplied
 *           this statement order, operand forms and name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementDrawHalfword2E(
    void) {
  u8* record;
  u8* draw;
  s16* half;

  record = workRecordCursor1F800044;
  half = (s16*)(record + 0x2E);
  *half = (s16)(*half - 0x10);
  if (*half < -0x2F) {
    record[2] = 0;
  }
  if (D_801454F2 != 2) {
    workRecordCursor1F800044[2] = 1;
  }
  draw = workRecordCursor1F800044;
  drawMarkerSpriteLayers(
      0x10, FIELD_REF(s16, draw, 0x2E));
}
