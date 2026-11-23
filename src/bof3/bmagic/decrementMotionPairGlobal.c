#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds no name there,
 * so the cell is reached through its address-anchored name, and the original
 * reads it once at entry (lui $a0,0x1F80 / lw $a0,68($a0)) for every access of
 * the body, the same one-read form the exact sibling
 * decrementWorkMotionWord44By1000ThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenTintChannel5FIsMinus80BmagicMagic15403_801EF454 uses. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared helper of the main executable. This target's generated
 * undefined_funcs_auto.txt binds 0x801E5988, and the exact sibling
 * decrementWorkMotionWord44By1000ThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenTintChannel5FIsMinus80BmagicMagic15403_801EF454 calls the same address
 * through the same prototype. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF3E4
 * @behavior Per-frame step of the bmagic work object and word 4 (index 4) of
 *           the six-word code pointer run at 0x801EF6CC (0x801EF104,
 *           0x801EF4B4, 0x801EF1EC, 0x801EF380, this address, 0x801EF454),
 *           which this overlay's dispatcher
 *           callStateTablef6 indexes with
 *           work-object byte 0x1 scaled by four, so the payload holds no jal to
 *           this address. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 once at entry, before its frame exists, and takes the
 *           object's 16.16 motion pair down: word 0x40 loses 0x4000 with an
 *           immediate add (lw / addiu -0x4000 / sw) and word 0x44 loses 0x8000
 *           through a register-carried constant (lw / addiu $v1,$zero,-0x8000 /
 *           addu / sw), the register-carried form the joining 0x8000 step of
 *           the exact sibling incrementWorkByte9AndAdvanceMotionPairThenAdvance
 *           WorkByte2At4BmagicMagic15403_801EF380 takes with ori; both stores are
 *           unconditional and the 0x40 store lands first. In the same pointer it
 *           decrements the object's countdown byte 0x9 at byte width (lbu /
 *           addiu -1 / sb) and keeps the decremented value in a byte local, so
 *           the truncation rather than a reload supplies the compared value
 *           (andi 0xFF before the test). When that new value is zero it
 *           decrements the shared battle work record's global work byte 0xB at
 *           byte width (lbu / addiu -1 / sb) through the pointer at 0x801EB4E0
 *           and invokes the shared main-executable helper at 0x801E5988, the
 *           byte 0xB store filling the call's delay slot. The 0x18-byte frame
 *           keeps only $ra at 0x10 across that one call, which is why the
 *           leading pointer load is scheduled before the frame setup. It takes
 *           no arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMotionPairGlobal(
    void) {
  u8 *work;
  u8 counter;
  s32 word40;
  s32 word44;

  work = D_1F800044;
  counter = work[9] - 1;
  word40 = *(s32 *)(work + 0x40) - 0x4000;
  word44 = *(s32 *)(work + 0x44) - 0x8000u;
  work[9] = counter;
  *(s32 *)(work + 0x40) = word40;
  *(s32 *)(work + 0x44) = word44;
  if (counter == 0) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
