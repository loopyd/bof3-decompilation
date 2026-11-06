#include "bof3/bof3.h"

/* @source 0x801F3718
 * @behavior Byte-2 state handler 2 of the local handler table based at
 *           0x801F3924, the word that table stores at 0x801F392C, for the
 *           work record published at the scratchpad pointer cell 0x1F800044.
 *           Adds 0x20 to the record's halfword at 0x30 and, once that stored
 *           signed halfword reaches 0x100, advances the record's state byte at
 *           0x02 by one. The cell is loaded once and that one base serves the
 *           halfword arms and the byte store. Takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By20AndAdvanceState2At100World04Area15713_801F3718(void) {
  u8* record;
  s16* half;

  record = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x20);
  if (*half >= 0x100) {
    record[2]++;
  }
}
