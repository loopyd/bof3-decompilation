#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed. The sibling lifts of this target read the same record through this
 * pointer: decrementWorkByte9ThenSeedWorkHeaderAndAdvanceWorkByte2
 * BmagicMagic15403_801EF1EC copies its words 0x34, 0x38 and 0x3C, and the
 * shared helper lifts of the neighbouring overlays decrement its global work
 * byte 0xB exactly as this body does. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. This target's map binds no name there,
 * so the cell is reached through its address-anchored name. The original
 * carries the loaded pointer through the word-0x44 decrement and the tint
 * channel 0x5F test, so this body reads the cell once at entry. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared no-argument helper of the main executable. This target's map and its
 * generated undefined_funcs_auto.txt both bind 0x801E5988, and the sibling
 * lifts name the same address through the same prototype. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF454
 * @behavior Work byte 1 step handler of this overlay's work object, word 5 (the
 *           last) of the six-word code pointer run at 0x801EF6CC (0x801EF104,
 *           0x801EF4B4, 0x801EF1EC, 0x801EF380, 0x801EF3E4, 0x801EF454) that
 *           the dispatcher dispatchStateByte1Tablef6ccBmagicMagic15403
 *           _801EF0C0 indexes with work-object byte 0x1 scaled by four, so the
 *           payload holds no jal to this address. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once before its frame
 *           exists, takes the object's 16.16 motion word at 0x44 down by 0x1000
 *           with an immediate add (lw / addiu -0x1000 / sw) and then reads the
 *           object's signed tint channel 0x5F through that same loaded pointer,
 *           so the decrement and the test share one cell read. When the channel
 *           reads exactly -0x80, the floor the tint-fade siblings step towards,
 *           it decrements the shared battle work record's global work byte 0xB
 *           by one at byte width (lbu / addiu -1 / sb) through the pointer at
 *           0x801EB4E0 and invokes the shared main-executable helper at
 *           0x801E5988, the byte 0xB store filling the call's delay slot. The
 *           0x18-byte frame keeps only $ra at 0x10 across that one call, which
 *           is why the leading pointer load is scheduled before the frame
 *           setup. It takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMotionTintIs(
    void) {
  u8 *work;

  work = D_1F800044;
  *(s32 *)(work + 0x44) -= 0x1000;
  if (((s8 *)work)[0x5F] == -0x80) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
