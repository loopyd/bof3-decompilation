#include "bof3/bof3.h"

/* State handler at 0x801F35EC: writes 3 to work state byte 0x02 when the shared
   mode byte 0x801454F2 is 2, then draws the work halfword at offset 0x2E. */
void stateHalfword2eGate(
    void);

/* @source 0x801F3590
 * @behavior Advances the work halfword at offset 0x2E of the record published
 *           through the scratchpad pointer slot 0x44 by 0x10 and stores the
 *           truncated halfword. When the stored signed value is at least 0x10
 *           it increments work state byte 0x02. Then runs the state-2 handler
 *           at 0x801F35EC, which draws halfword 0x2E. It takes no arguments,
 *           returns nothing and saves only the return address. Its 0x5C
 *           original bytes (span sha1 82caef2333cd610237c3e2008505bedd6f454050)
 *           differed from the emi/world02/area088/13 lift
 *           advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld02Area08813_801F40C0,
 *           which measured 100.00 then, in exactly two bytes, offsets 0x44 and
 *           0x45, the low half of the internal `jal` word, by the 0xB30 the two
 *           spans sit apart; the emi/world00/area033/13 and emi/world03/area121/14
 *           twins differed from this span in that same word's low byte alone, so
 *           one body was proved across the four spans and this source follows
 *           that sibling's statement order with this target's own handler
 *           address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAdvanceHalfword2e2(
    void) {
  u8* cursor;
  s16* half;

  cursor = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(cursor + 0x2E);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x10) {
    cursor[2]++;
  }
  stateHalfword2eGate();
}
