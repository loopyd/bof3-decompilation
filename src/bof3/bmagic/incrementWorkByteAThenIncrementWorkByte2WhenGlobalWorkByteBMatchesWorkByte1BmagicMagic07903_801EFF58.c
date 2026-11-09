#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_801EB4E0 = 0x801EB4E0, so no target-map row is needed, and every access
 * here is byte-addressed, so the local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the lifted siblings on this target use. The original loads the cell twice: a
 * first lui/lw pair feeds the byte-0xA increment, and a second lui/lw pair
 * feeds both the guard byte 0x1 and the taken path increment of byte 0x2. The
 * byte-0xA store between the two pairs can alias the cell, so the statements
 * below stay on the named cell instead of sharing one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFF58
 * @behavior Per-frame step of an overlay work-object handler. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and increments
 *           that object's unsigned byte at offset 0xA by one at byte width
 *           (lbu $v0,10($v1) / addiu $v0,$v0,1 / sb $v0,10($v1)). It then
 *           reads the shared battle work pointer at 0x801EB4E0 once, loads the
 *           pointer cell again with its own lui $a0,1F80 / lw $a0,68($a0) pair
 *           and tests byte 0xB of the shared record against byte 0x1 of the
 *           work object (lbu $v1,11($v0) / lbu $v0,1($a0) / bne with an empty
 *           delay slot). Only when those two bytes are equal does it increment
 *           the object's byte at offset 0x2 at byte width (lbu $v0,2($a0) /
 *           addiu $v0,$v0,1 / sb $v0,2($a0)); the taken path reuses the guard
 *           base register $a0 because no store separates the two reads. Leaf
 *           with no frame: takes no arguments, returns nothing, reads or
 *           writes no other state. Address 0x801EFF58 is index 6 of the
 *           eight-address code-pointer run at 0x801F035C inside the reviewed
 *           textbin row D_801F02A4, and no in-image jal reaches it, so it is
 *           published for a consumer outside the demonstrated set. Its
 *           reviewed range measured 21 of 22 instructions identical to the
 *           already-exact sibling decrementWorkByteAThenIncrementWorkByte2WhenGlobalWorkByteBMatchesWorkByte1BmagicMagic07903_801EFD2C,
 *           differing only in the byte-0xA immediate (-1 there, +1 here), so
 *           the clone lever was ineligible for want of byte identity and the
 *           sibling supplied shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenIncrementWorkByte2WhenGlobalWorkByteBMatchesWorkByte1BmagicMagic07903_801EFF58(void) {
  g_battle_work[0xA]++;
  if (D_801EB4E0[0xB] == g_battle_work[1]) {
    g_battle_work[2]++;
  }
}
