#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address (config/targets/shared/symbols.txt), so this target needs
 * no row for it; its low bit is the per-frame phase the sibling lifts of the
 * bmagic overlays read, the global word being relocated as %hi/%lo here. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the name the exact
 * lifts copyWorkWordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic00603
 * _801F05A4 and add4ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic00603_801F05F4
 * of this same target used. The original re-materializes the cell with its own
 * lui $vN,0x1F80 / lw $vN,0x44($vN) pair per read, so the cell stays named
 * rather than written as a bare constant, and it needs no volatile: the store
 * through the loaded pointer separates the two reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F020C
 * @behavior Per-frame step of the overlay's scratchpad work object. Tests bit 0
 *           of the shared frame counter at 0x80143E6C (lui %hi / lw %lo / andi
 *           0x1) and, when it is set, reads the work-object pointer cell at
 *           0x1F800044 with its own lui/lw pair and increments the object's byte
 *           0xA modulo 256 (lbu / addiu 1 / sb). On every frame it then reloads
 *           the cell and adds 0x10 to the object's byte 0x9 (lbu / addiu 0x10 /
 *           sb); the value just stored is zero-extended by andi 0xFF, with no
 *           second load, and compared against 0x80, which is materialized as
 *           addiu $v0,$zero,0x80, so only when byte 0x9 ramps to 0x80 does the
 *           third cell read increment the object's byte 0x2 modulo 256, which
 *           advances the object one step of its work-byte-2 handler sequence.
 *           Leaf with no frame and no saved registers; takes no arguments,
 *           returns nothing through the jr $ra at 0x801F0288 and reads or
 *           writes no other state. Reached by no in-image jal: the word at
 *           0x801EEC20 is entry 7 of the 13-word code-pointer run
 *           0x801EEC04..0x801EEC34 in this target's tabledata segment, so that
 *           handler-table entry proves the function-entry boundary at
 *           0x801F020C. The roles of work-object bytes 0xA, 0x9 and 0x2 are not
 *           established, so they stay offset-anchored. Measured against the
 *           reviewed bytes: the 132-byte span holds no byte-identical copy
 *           anywhere in the overlay binaries, and the last 80 bytes
 *           0x801F0240..0x801F028F compare equal at their own payload offsets to
 *           the tail of the exact lift
 *           add16ToWorkByte9AndAdvanceWorkByte2At80BmagicMagic03903_801EF690,
 *           whose statement form is taken unchanged, while the leading 52 bytes
 *           repeat the parity-guarded byte-0xA increment of the exact lift
 *           advanceWorkByteAOnFrameParityInvokePointHelperCountdownWorkByte9
 *           DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4 byte for
 *           byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAOnFrameParityThenAdd16ToWorkByte9AndAdvanceWorkByte2At80BmagicMagic00603_801F020C(void) {
  if (D_80143E6C & 1u) {
    g_battle_work[0xA]++;
  }

  if ((g_battle_work[9] += 0x10) == 0x80) {
    g_battle_work[2]++;
  }
}
