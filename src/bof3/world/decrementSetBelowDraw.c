#include "bof3/bof3.h"

extern u8 D_801454F2;

/* @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* Draws the work halfword as a signed 16-bit screen coordinate. */
void renderMarkerLayersAppend(
    s16 arg0, s16 arg1);

/* @source 0x801F360C
 * @behavior Byte-2 state handler 3 of the state-byte-2 handler table
 *           0x801F4F64 for the work record published at the scratchpad
 *           pointer cell 0x1F800044. Subtracts 0x10 from the signed halfword
 *           at record offset 0x2E and stores the truncated halfword, then
 *           clears state byte 0x02 while the stored value stays below -0x2F.
 *           Unless the shared byte 0x801454F2 is 2 it writes 1 to state byte
 *           0x02 through a second read of the pointer cell. Finally it reads
 *           the cell again and calls the local drawer 0x801F3B04 with 0x10 and
 *           the signed halfword at offset 0x2E. Takes no arguments, uses no
 *           result and saves only the return address. The 0x84 bytes are
 *           byte-identical to the lifted exact
 *           decrementHalfword2eBy10AndSetState2To0BelowMinus2fOrTo1Unless454f2Is2ThenDrawHalfword2eWorld03Area12114_801F3620
 *           in emi/world03/area121/14 and to the lifted exact
 *           decrementHalfword2eBy10AndSetState2To0BelowMinus2fOrTo1Unless454f2Is2ThenDrawHalfword2eWorld01Area06513_801F3640
 *           in emi/world01/area065/13 apart from the low byte of the internal
 *           `jal` word that encodes each overlay's own drawer, so this body
 *           follows the reviewed sibling statement order with this target's
 *           drawer. The record is read through the named scratchpad cell
 *           binding: the first draft spelled the three reads with the raw
 *           address macro and measured 25 of 33 instructions, because the
 *           compiler hoisted that literal address into one register as an
 *           extra `ori`, while the named cell keeps one load per read.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementSetBelowDraw(
    void) {
  u8* record;
  u8* draw;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x2E);
  *half = (s16)(*half - 0x10);
  if (*half < -0x2F) {
    record[2] = 0;
  }
  if (D_801454F2 != 2) {
    D_1F800044[2] = 1;
  }
  draw = D_1F800044;
  renderMarkerLayersAppend(
      0x10, FIELD_REF(s16, draw, 0x2E));
}
