#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches each active queued battle slot through its selected
 * handler while publishing the slot and associated local-work pointers.
 * @source 0x801E5824
 * @status partial
 */
void runQueuedSlotHandlers(void) {
  Battle03FourDispatchTable handlers = D_801D0CC0;
  Battle03Handler* table = handlers.handlers;
  s32 i = 0;
  u8* slot = (u8*)D_801EC330;
  s32 offset = 0;

  for (; i < 48; i++) {
    if (*(u8*)((u8*)D_801EC330 + offset) != 0u) {
      volatile Battle03LocalWork* work = (volatile Battle03LocalWork*)
          *(u32*)((u8*)D_801EC330 + offset + 0x74);

      D_801EC2E0 = (Battle03QueuedSlot*)slot;
      D_1F800044 = (Battle03LocalWork*)slot;
      D_801EB4E0 = work;
      table[slot[6]]();
    }
    slot += 0x78;
    offset += 0x78;
  }
}
