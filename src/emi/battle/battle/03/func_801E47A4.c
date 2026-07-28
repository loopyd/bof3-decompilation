#include "internal.h"

/* @source 0x801E47A4
 * @behavior initializes current scratch work state fields.
 */
void func_801E47A4(void)
{
  Battle03LocalWork** scratch_slots;
  Battle03LocalWork*  work;

  scratch_slots = SPAD_PTR_TABLE(Battle03LocalWork);
  work = scratch_slots[0x11];
  work->unk_48 = 2;
  work = scratch_slots[0x11];
  work->unk_44 = 0x10000u;
  work->unk_40 = 0x10000u;
  work->unk_18 = 0x666u;
  work->unk_0c = 0;
  work->unk_03 = 1;
}
