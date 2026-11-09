#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. Kept as the named cell symbol the
 * exact siblings on this target use. The original takes the cell once for the
 * byte 0xA update and takes it again, with its own lui/lw pair, for the byte 2
 * increment on the taken path - a fresh load after the first base is dead, so
 * the arm reaches the plain cell symbol rather than the local base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF308
 * @behavior Per-frame step of the overlay work object, reached through the
 *           eighth word (index 7) of the in-image code pointer run based at
 *           0x801EEC04 rather than by a call: it reads the scratchpad work
 *           object pointer cell at 0x1F800044 once and adds four to the
 *           object's byte at offset 0xA (lbu / addiu 4 / sb), then zero-extends
 *           the stored byte with andi 0xFF and tests it against 0x20 by
 *           equality (addiu $v0,$zero,0x20 / bne). Only when that byte equals
 *           0x20 does it re-read the cell with its own lui/lw pair and
 *           increment the object's state byte at offset 0x2 (lbu / addiu 1 /
 *           sb), advancing the overlay work byte 2 handler sequence. Leaf with
 *           no frame; takes no arguments, returns nothing and reads or writes
 *           no other state.
 *
 *           Shape: the stored-then-masked byte test is the shape of the exact
 *           sibling incrementWorkByteAThenInvokeHelperAt8BmagicMagic07803_801F02B4,
 *           which keeps the incremented byte in a u8 local so GCC 2.7.2 stores
 *           the unmasked sum and only then emits the andi for the comparison;
 *           this function's own constant 4 and compared literal 0x20 replace
 *           its 1 and 8. The arm re-reads the cell instead of reusing the first
 *           base, the reload-in-arm-only shape of
 *           incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic04103_801EF524
 *           on this target. 0x801EF308..0x801EF357 match no byte-identical
 *           span in any other bmagic overlay image, so no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByteAThenAdvanceWorkByte2At20BmagicMagic04103_801EF308(void) {
  u8 *work = g_battle_work;
  u8 next = work[0xA] + 4;

  work[0xA] = next;
  if (next == 0x20) {
    g_battle_work[2]++;
  }
}
