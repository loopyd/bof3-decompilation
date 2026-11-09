#include "bof3/bof3.h"

/* Shared main-executable helper at 0x800AF764. It is outside every payload
 * lifted here, so it keeps its raw name and no target-map row covers it; the
 * exact bmagic siblings declare it with this same no-argument shape. */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-reads the cell with its own lui/lw pair for the step
 * dispatch, again after the shared helper and once more for each of the three
 * tint groups, so every group keeps its own load instead of one live pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Word 0x801F24FC of the code pointer run whose own row begins at 0x801F24A4:
 * the work byte 2 step handler table this span indexes, 0x801F11E0 at entry 0
 * and 0x801F129C at entry 1. The ten-entry table at 0x801F24F0 and the
 * three-entry tail at 0x801F250C of the exact siblings index other words of the
 * same run, so this base is bound at the word the scaled index is added to
 * rather than at the run base, and the generated undefined_syms_auto.txt row
 * carries the address. */
extern void (*D_801F24FC[])(void); /* @source 0x801F24FC @kind data */

/* Pushes the GTE matrix stack and installs the work-object transform. It stays
 * an asm boundary of this target, so it keeps its raw name and needs no
 * declaration header. */
void func_801F1408(void);

/* Shaded four-vertex polygon drawer of this overlay: it sets the draw mode,
 * draws one semi-transparent SetPolyG4 primitive and rotates its vertices with
 * RotTransPers4. Each call here passes its own three constant arguments. */
void func_801F14D0(s32 arg0, s32 arg1, s32 arg2);

/* @source 0x801F10E8
 * @behavior Frame wrapper around the work byte 2 step handlers of the scratchpad
 *           work object published at the pointer cell 0x1F800044. The object
 *           byte at offset 0x2, scaled by four, selects one entry of the step
 *           handler table based at 0x801F24FC and that handler is invoked with
 *           no arguments; the cell load precedes frame creation. The shared
 *           main-executable helper func_800AF764 is then called unconditionally.
 *           After it the cell is read again and only when the object byte at
 *           offset 0x2 and the byte at offset 0x0 are both non-zero does the body
 *           run: func_801F1408 pushes the GTE matrix stack and installs the
 *           object transform, then work byte 0xB is raised by four through its
 *           own cell read and func_801F14D0 draws one tinted polygon, the byte
 *           is raised by four again through another cell read before the second
 *           draw, lowered by eight through a fourth cell read before PopMatrix
 *           restores the matrix func_801F1408 pushed. The three draw calls pass
 *           the argument triples 0x20/0x20/0x7, 0x10/0x40/0xF and
 *           0x10/0x60/0x1F, and the tint byte returns to its entry value, so the
 *           ramp lasts for that triple only. Frame 0x18 with $ra only, because of
 *           the calls; it takes no arguments, returns nothing and reads or writes
 *           no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2StepThenInvokeSharedHelperThenDrawTintedQuadTripleAndPopWhenWorkBytes2And0SetBmagicMagic15103_801F10E8(
    void) {
  u8 *gate;
  u8 *first_tint;
  u8 *second_tint;
  u8 *final_tint;

  D_801F24FC[g_battle_work[2]]();
  func_800AF764();
  gate = g_battle_work;
  if (gate[2] != 0 && gate[0] != 0) {
    func_801F1408();
    func_801F14D0(0x20, 0x20, 0x7);
    first_tint = g_battle_work;
    first_tint[0xB] += 4;
    func_801F14D0(0x10, 0x40, 0xF);
    second_tint = g_battle_work;
    second_tint[0xB] += 4;
    func_801F14D0(0x10, 0x60, 0x1F);
    final_tint = g_battle_work;
    final_tint[0xB] -= 8;
    PopMatrix();
  }
}
