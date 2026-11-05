#include "bof3/world/area03004_internal.h"

/* @source 0x801D1B54
 * @behavior initializes the AREA030 work words at offsets 0x0C, 0x10 and
 * 0x1C to zero, copies the unsigned first byte of the record selected by
 * D_801E320C into work word 0x18 and advances work state byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkWordsAndAdvanceStateWorld00Area03004_801D1B54(void) {
  u32 value;
  u8* work;
  u8 state;

  value = (u8)D_801E320C[0];
  work = D_1F800044;
  state = work[1];
  *(u32*)(work + 0x0C) = 0;
  *(u32*)(work + 0x10) = 0;
  *(u32*)(work + 0x1C) = 0;
  *(u32*)(work + 0x18) = value;
  work[1] = state + 1;
}
