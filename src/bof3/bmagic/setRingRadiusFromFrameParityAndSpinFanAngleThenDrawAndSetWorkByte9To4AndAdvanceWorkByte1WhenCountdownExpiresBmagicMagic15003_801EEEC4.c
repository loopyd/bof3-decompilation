#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, shared by all overlays and
 * bound as g_battle_work by this family's map row in the sibling target
 * emi/bmagic/magic151/03. The original re-materializes the cell base for each
 * access (lui $vN,0x1F80 / lw $vN,68($vN) before the countdown read, before the
 * byte-9 store and before the byte-1 update), so the cell stays the named
 * symbol rather than a pointer local that would fold those reads into one. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared frame counter at 0x80143E6C, outside this overlay's image and owned by
 * the shared map. Its low bit is the per-frame phase that the frame-parity lifts
 * of other bmagic overlays read from the same word, and this span shifts it into
 * bit 4 of the ring radius it publishes. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Effect-transform state in this overlay's own data run at 0x801EFA9C. The
 * overlay init 0x801EECA4 zeroes the three halfwords after publishing the
 * effect origin into 0x801EFAA0 and 0x801EFAA4, so 0x801EFAAC and 0x801EFAB0 are
 * the geometry this handler seeds.
 *
 * 0x801EFAAC is the ring radius: the sixteen-vertex ring draw at 0x801EF040
 * loads it signed (lh 0x10($s2) off the 0x801EFA9C base), multiplies it by each
 * rotator answer and adds that scaled offset to the origin halfwords, and the
 * adjacent handlers 0x801EEE24 and 0x801EEF80 ramp the same halfword by 0x40.
 * This span only stores it. */
extern s16 D_801EFAAC; /* @source 0x801EFAAC @kind data */

/* 0x801EFAB0 is the fan angle: the burst draw at 0x801EF4E0 loads it unsigned
 * and hands it to the rotators 0x801783C8 and 0x801782FC, advancing it by 0x80
 * per particle. This span loads it unsigned (lhu) and stores the decremented
 * value back, so it is declared unsigned. */
extern u16 D_801EFAB0; /* @source 0x801EFAB0 @kind data */

/* Draw setup of this overlay at 0x801EF254, still an asm row of this target's
 * Splat listing and therefore already defined by the link. It binds the tpage,
 * installs the draw mode on the primitive cursor, then runs the two draw passes
 * 0x801EF040 and 0x801EF4E0 against the state above, so this call is the frame's
 * whole draw and it takes no arguments and returns nothing. */
void func_801EF254(void); /* @source 0x801EF254 */

/* Shared helper of the main executable at 0x800A9820, outside this overlay's
 * image and bound through this target's generated undefined_funcs_auto.txt. It
 * consumes the zero-extended battle byte at 0x80146384, which is how the lift
 * signatures of the surrounding magic overlays pass it. */
void func_800A9820(u8 arg0); /* @source 0x800A9820 */

/* Cue submitter of the main executable at 0x8015DF18, outside this overlay's
 * image and bound through this target's generated undefined_funcs_auto.txt. It
 * takes the cue word this span supplies as 0x102 and returns nothing. */
void func_8015DF18(u32 arg0); /* @source 0x8015DF18 */

/* Shared battle byte at 0x80146384, outside this overlay's image, so it keeps
 * its address-qualified name. The original loads it zero-extended (lbu). */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEEC4
 * @behavior Magic-150 effect handler, one of the words of the overlay handler
 *           table at 0x801EEC04 that the dispatcher 0x801EEC20 copies into its
 *           frame and indexes by unsigned work byte 1, so it runs for one frame
 *           per step. It publishes the ring radius at 0x801EFAAC as 0x100, or
 *           0x110 when bit 0 of the shared frame counter 0x80143E6C is set, that
 *           is on every other frame the radius is one sixteenth larger. It then
 *           subtracts 0x200 from the fan angle at 0x801EFAB0, an unsigned load,
 *           one addiu and a halfword store, an eighth of a turn in the
 *           0x1000-unit angle the rotators use, and calls the overlay draw setup
 *           0x801EF254 with no arguments so the frame is drawn once from the
 *           state it just wrote. After the draw it decrements work byte 9 with
 *           the stored byte masked to 0xFF for the test, and returns untouched
 *           while that byte is nonzero. When the countdown reaches zero it
 *           stores 4 into work byte 9, re-reads the work pointer and increments
 *           work byte 1, which moves the dispatcher on to the next handler
 *           entry, then calls 0x800A9820 with the shared battle byte 0x80146384
 *           and submits cue 0x102 through 0x8015DF18. Those are its only stores
 *           and it reads no other cell. The 0x18-byte frame keeps $ra at 0x10
 *           across the three calls, and the function takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRingRadiusFromFrameParityAndSpinFanAngleThenDrawAndSetWorkByte9To4AndAdvanceWorkByte1WhenCountdownExpiresBmagicMagic15003_801EEEC4(
    void) {
  D_801EFAAC = ((D_80143E6C & 1) << 4) + 0x100;
  D_801EFAB0 -= 0x200;
  func_801EF254();
  if (--g_battle_work[9] == 0) {
    g_battle_work[9] = 4;
    g_battle_work[1]++;
    func_800A9820(D_80146384);
    func_8015DF18(0x102);
  }
}
