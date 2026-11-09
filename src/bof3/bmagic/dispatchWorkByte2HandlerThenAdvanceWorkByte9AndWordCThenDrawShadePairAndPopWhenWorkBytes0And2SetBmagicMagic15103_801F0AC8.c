#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original re-reads the
 * cell with its own lui/lw pair before the step dispatch, after it, and once
 * more for each of the two read-modify-write groups, so the cell is bound by
 * name and reloaded instead of being kept live across them. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Ten-entry code pointer table at 0x801F24F0 holding the work byte 2 step
 * handlers of this overlay: 0x801F0BA8, 0x801F0C50, 0x801F0CB0, 0x801F11E0,
 * 0x801F129C, 0x801F1318, 0x801F1380, 0x801F1CCC, 0x801F1D28 and 0x801F1D60,
 * the same handlers the exact sibling
 * dispatchWorkByte2HandlerThenDrawQuadPairAndPopWhenWorkBytes0And2SetBmagicMagic15103_801F1C3C
 * dispatches from the three-entry tail at 0x801F250C. The table sits inside the
 * pointer run whose own row begins at 0x801F24A4, so its base is bound by this
 * target's map. */
extern void (*workByte2StepTable[])(void); /* @source 0x801F24F0 @kind data */

/* Sixteen two-byte entries at 0x801F24D0, 0x2C bytes into the same run. The
 * entry work byte 4 selects is passed to the draw routine at 0x801F0D0C: its
 * low byte added to work byte 9 is the first argument and its high byte the
 * second. */
extern u8 workShadePairTable[]; /* @source 0x801F24D0 @kind data */

/* Installs the work object transform: pushes the GTE matrix stack, builds the
 * translation and rotation of work words 0x34 and 0x38 with the work halfword
 * 0x3E and sets both GTE matrices. */
void projectWorkObjectPositionAndInstallTransformWithZeroTranslationBmagicMagic15103_801F01BC(
    void);

/* Draws the tinted primitive of this overlay work object from one shade pair
 * entry and the work byte 9 offset the first argument carries. */
void func_801F0D0C(u8 shade, u8 page);

/* @source 0x801F0AC8
 * @behavior Dispatches one frame of the overlay work handler of the scratchpad
 *           work object published at the pointer cell 0x1F800044: the object
 *           byte at offset 0x2, scaled by four, selects one of the ten work
 *           step handlers at 0x801F24F0 (0x801F0BA8, 0x801F0C50, 0x801F0CB0,
 *           0x801F11E0, 0x801F129C, 0x801F1318, 0x801F1380, 0x801F1CCC,
 *           0x801F1D28, 0x801F1D60) and that handler is invoked with no
 *           arguments. After it returns the cell is read a second time and only
 *           when the object bytes at offset 0x0 and offset 0x2 are both
 *           non-zero does the body run: the transform installer at 0x801F01BC
 *           is called, which pushes the GTE matrix stack and installs the
 *           object transform, then a third cell read adds two to work byte
 *           0x9 and one to work word 0xC, and a fourth cell read supplies the
 *           shade pair 0x801F24D0 indexed by twice work byte 0x4 whose low
 *           byte added to work byte 0x9 is passed clamped to eight bits to
 *           func_801F0D0C with that pair high byte as the second argument.
 *           PopMatrix restores the matrix the installer pushed. Frame 0x18
 *           with $ra only, because of the calls; the first cell load precedes
 *           frame creation. Takes no arguments, returns nothing and reads or
 *           writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenAdvanceWorkByte9AndWordCThenDrawShadePairAndPopWhenWorkBytes0And2SetBmagicMagic15103_801F0AC8(
    void) {
  u8 *step_record;
  u8 *advance_record;
  u8 *shade_record;
  u32 *words;
  s32 shade;

  workByte2StepTable[g_battle_work[2]]();
  step_record = g_battle_work;
  if (step_record[0] != 0 && step_record[2] != 0) {
    projectWorkObjectPositionAndInstallTransformWithZeroTranslationBmagicMagic15103_801F01BC();
    advance_record = g_battle_work;
    advance_record[9] += 2;
    words = (u32 *)advance_record;
    words[3] += 1;
    shade_record = g_battle_work;
    shade = shade_record[4] * 2;
    func_801F0D0C(workShadePairTable[shade] + shade_record[9],
                  workShadePairTable[shade + 1]);
    PopMatrix();
  }
}
