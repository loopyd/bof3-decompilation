#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so each
 * access is a lui plus offset load, which is how the original re-materialises
 * the cell address for the argument group and again after the drawing call. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

void drawProjectedBeamWithRotatedEndQuadsAndGouraudBodyWorld03Area14514_801F332C(
    void* arg0, void* arg1);

/* @source 0x801F2ECC
 * @behavior Overlay per-frame step handler: loads the work-object pointer
 * published at the scratchpad pointer cell 0x1F800044, passes the addresses of
 * that object's +0x0C and +0x18 fields to the drawing helper at 0x801F332C, and
 * then re-reads the same cell and increments the object's byte at +0x01.
 * The two passed addresses are twelve bytes apart and the helper reads each
 * through the shared 3D-to-screen projector func_801AFF04, so +0x0C and +0x18
 * are the record's two three-word position vectors; the helper draws between
 * them. Byte +0x01 is the dispatch byte this overlay's own handler table
 * dispatchStateByte1Table5454World03Area14514_801F2E88 indexes with, so the
 * increment advances the record to the next handler step. The pointer cell is
 * read twice because the drawing call sits between the two reads. It takes no
 * arguments, returns nothing and reads no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkVectorPairAndAdvanceStateByte1World03Area14514_801F2ECC(void) {
  u8* work;

  work = D_1F800044;
  drawProjectedBeamWithRotatedEndQuadsAndGouraudBodyWorld03Area14514_801F332C(
      work + 0x0C, work + 0x18);
  D_1F800044[1]++;
}
