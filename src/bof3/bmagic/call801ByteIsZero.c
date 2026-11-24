#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the bmagic overlays. The
 * original loads the cell once (lui $v0,0x1F80 / lw $v0,0x44($v0)) before the
 * frame is created and reads one work byte through that value, so the access
 * needs no local of its own. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Main-executable helper at 0x801E5988, below this overlay's image, so the
 * payload reaches it only through the call immediate and the generated
 * undefined-function table binds the raw address spelling. */
void func_801E5988(void);

/* @source 0x801EF190
 * @behavior Work byte 2 entry 2 of the five-entry code pointer run this
 *           overlay's leading data block holds at 0x801EEC04
 *           (0x801EEFE4, 0x801EF110, 0x801EF190, 0x801EF1C8, 0x801EF2E4),
 *           which the dispatcher callCopiedStepTable
 *           copies into its own frame and indexes with work byte 2, so this
 *           entry is invoked with no arguments. The body reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once, before the 0x18-byte
 *           frame is created, in a 0x38-byte span, and tests the work object's
 *           unsigned byte 0x0B; only when that byte is zero does it fall
 *           through to the main-executable helper at 0x801E5988 with no
 *           arguments passed. Nothing is stored at any width and nothing is
 *           returned; the frame keeps only $ra at 0x10 and the body leaves
 *           through the jr $ra at 0x801EF1C0. The role of work byte 0x0B is not
 *           established, so it stays offset-anchored. The statement order, the
 *           single cell read hoisted above the frame and the operand spelling
 *           were taken from the same overlay's
 *           callIsZeroGlobal,
 *           whose gate measured exact when this body was written and which
 *           carries the same guard over the same cell and byte with a wider
 *           taken arm; it is the shape seed, not a byte-identical copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
void call801ByteIsZero(void) {
  if (g_battle_work[0x0B] == 0) {
    func_801E5988();
  }
}
