#include "bof3/battle/battle03_internal.h"

/* @source 0x801E72F4
 * @behavior Copies selected queued-work fields to scratch work, setting scratch
 * flag 0x20, then runs func_8014D290. Clears queued-slot bytes when the queued
 * work lacks flag 1 and independently when the global mode byte is not 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyQueuedWorkToScratchAndRun(void) {
  volatile Battle03LocalWork* first;
  Battle03LocalWork* first_scratch;
  volatile Battle03LocalWork* second;
  Battle03LocalWork* second_scratch;
  volatile Battle03LocalWork* position;
  Battle03LocalWork* position_scratch;

  D_1F800044->flags_00 = D_801EB4E0->flags_00 | 0x20;
  D_1F800044->unk_48 = D_801EB4E0->unk_48;
  first = D_801EB4E0;
  first_scratch = D_1F800044;
  first_scratch->unk_40 = first->unk_40;
  first_scratch->unk_44 = first->unk_44;
  FIELD_REF(u16, first_scratch, 0x58) = FIELD_REF(volatile u16, first, 0x58);
  FIELD_REF(u16, first_scratch, 0x5a) = FIELD_REF(volatile u16, first, 0x5a);
  FIELD_REF(u8, first_scratch, 0x4b) = FIELD_REF(volatile u8, first, 0x4b);
  FIELD_REF(u8, D_1F800044, 0x4a) = FIELD_REF(volatile u8, D_801EB4E0, 0x4a);
  FIELD_REF(u8, D_1F800044, 0x49) = FIELD_REF(volatile u8, D_801EB4E0, 0x49);
  second = D_801EB4E0;
  second_scratch = D_1F800044;
  FIELD_REF(u32, second_scratch, 0x4c) = FIELD_REF(volatile u32, second, 0x4c);
  FIELD_REF(u32, second_scratch, 0x50) = FIELD_REF(volatile u32, second, 0x50);
  FIELD_REF(u32, second_scratch, 0x54) = FIELD_REF(volatile u32, second, 0x54);
  FIELD_REF(u8, second_scratch, 0x27) = FIELD_REF(volatile u8, second, 0x27);
  FIELD_REF(u8, D_1F800044, 0x28) = FIELD_REF(volatile u8, D_801EB4E0, 0x28);
  D_1F800044->unk_2a = D_801EB4E0->unk_2a;
  position = D_801EB4E0;
  position_scratch = D_1F800044;
  position_scratch->unk_34 = position->unk_34;
  position_scratch->unk_38 = position->unk_38;
  FIELD_REF(u32, position_scratch, 0x3c) = FIELD_REF(volatile u32, position, 0x3c);
  func_8014D290();
  if ((D_801EB4E0->flags_00 & 1) == 0) {
    clearQueuedSlotBytes();
  }
  if (D_801462E0 != 1) {
    clearQueuedSlotBytes();
  }
}
