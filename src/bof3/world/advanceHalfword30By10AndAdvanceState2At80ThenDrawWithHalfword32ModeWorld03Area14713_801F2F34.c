#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void func_801F313C(s8 arg0);

/* @source 0x801F2F34
 * @behavior Byte-2 state handler 2 of the local handler table based at
 *           0x801F358C for the work record published at the scratchpad pointer
 *           cell 0x1F800044. Adds 0x10 to the record's signed halfword at 0x30
 *           and stores the truncated halfword; once that stored value reaches
 *           0x80 it advances the record's state byte at 0x02 by one. It then
 *           re-reads the pointer cell and passes mode -1 to the overlay draw
 *           routine at 0x801F313C when the record halfword at 0x32 is zero, or
 *           mode 0 when it is not. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceState2At80ThenDrawWithHalfword32ModeWorld03Area14713_801F2F34(
    void) {
  u8* record;
  u8* emitter;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x80) {
    record[2]++;
  }
  emitter = D_1F800044;
  if (*(s16*)(emitter + 0x32) != 0) {
    func_801F313C(0);
  } else {
    func_801F313C(-1);
  }
}
