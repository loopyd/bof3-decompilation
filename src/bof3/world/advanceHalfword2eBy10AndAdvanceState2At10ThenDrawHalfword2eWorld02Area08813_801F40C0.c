#include "bof3/bof3.h"

void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld02Area08813_801F411C(void);

/* @source 0x801F40C0
 * @behavior Advances the work halfword at offset 0x2E, published through the
 * scratchpad pointer cell 0x1F800044, by 0x10 and stores the truncated
 * halfword. When the stored signed value is at least 0x10 it increments work
 * state byte 0x02. Then runs the state-2 handler at 0x801F411C, which draws
 * halfword 0x2E. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld02Area08813_801F40C0(void) {
  u8* cursor;
  s16* half;

  cursor = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(cursor + 0x2E);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x10) {
    cursor[2]++;
  }
  setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld02Area08813_801F411C();
}
