#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two separate
 * reads each re-materialise the base. The plain cell is kept rather than the
 * volatile one several siblings use: every access goes through a freshly loaded
 * pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address, so
 * no target-map row is needed. The exact sibling
 * decrementWorkTintChannel5DByTwoThenDecrementGlobalWorkByteBAndInvokeHelperWhenZero
 * BmagicMagic15103_801F0CB0 decrements the same record byte 0xB and reaches the
 * same helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F013C
 * @behavior Per-frame fade step of the overlay work object. The scratchpad
 *           work-object pointer cell at 0x1F800044 is read once, before the
 *           0x20-byte frame is created, and the object's tint channel byte at
 *           offset 0x5D is loaded through the signed byte view the siblings
 *           decrementWorkTintChannel5DThenIncrementWorkByte2WhenChannelIs8
 *           BmagicMagic15103_801F00E8 uses; only when that channel is nonzero is
 *           it decremented by one with a byte-width store. The cell is then read
 *           a second time and the second tint channel byte at offset 0x5E is
 *           decremented by one with a byte-width read-modify-write, the value it
 *           just stored truncated to the byte written (sll 24, no third load of
 *           the cell) and tested for zero. Only when channel 0x5E reaches zero
 *           does it read the shared battle work record pointer 0x801EB4E0,
 *           decrement that record's byte 0xB by one and invoke the shared helper
 *           at 0x801E5988, the store filling the call delay slot. Frame 0x20
 *           with $ra only, because of the call; takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkTintChannel5DWhenSetAndChannel5EThenDecrementGlobalWorkByteBAndInvokeHelperWhenChannel5EIsZeroBmagicMagic15103_801F013C(void) {
  u8 *work;
  u8 *next;
  s8 channel;
  s8 tint;

  work = g_battle_work;
  channel = ((s8 *)work)[0x5D];
  if (channel != 0) {
    ((s8 *)work)[0x5D] = channel - 1;
  }
  next = g_battle_work;
  tint = next[0x5E] - 1;
  next[0x5E] = tint;
  if (tint == 0) {
    u8 *object;

    object = D_801EB4E0;
    object[0xB]--;
    func_801E5988();
  }
}
