#include "bof3/bof3.h"

/* Shared overlay state halfword pair. This target reads the two halfwords of
 * the pair separately, each against the small value 5, while the neighbouring
 * lift at 0x801F3B28 read the pair as one 32-bit word against 0x00050005, so
 * this file binds the two halves as halfwords of its own.
 * @source 0x80143B90 @kind data
 */
extern u16 D_80143B90;

/* @source 0x80143B92 @kind data */
extern u16 D_80143B92;

/* Byte in the main executable that the width ramp scales. It sits outside this
 * payload, so the address stays this target's binding; the battle target's own
 * header spells the same word BATTLE_GLOBAL_BYTE_6308 as an address-anchored
 * role there.
 * @source 0x80146308 @kind data
 */
extern u8 D_80146308;

/* Scratchpad work-record pointer cell. The original materialises the cell
 * address with `lui` plus an offset load for every access and reloads it after
 * each store through the loaded record, so the source keeps one pointer local
 * per access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Quad drawer of this target at 0x801F3F5C, lifted after this handler, so only
 * its prototype is declared here. Its head adds the third argument to the work
 * record byte at 0x0A, and its body builds a textured four-point polygon
 * through SetPolyFT4, SetShadeTex and RotTransPers4. Its fourth argument is
 * subtracted from the horizontal and vertical extents it derives from the work
 * halfwords 0x36 and 0x3A. */
void drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
    u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @source 0x801F3B84
 * @behavior Step 2 of the work-byte-2 handler run this overlay dispatches
 *           through the local table at 0x801F43E8. While the shared overlay
 *           state halfword at 0x80143B90 reads 5 and the partner halfword at
 *           0x80143B92 reads anything else, it decrements work record byte 0x02
 *           through the scratchpad pointer cell 0x1F800044, which sends the
 *           next frame back to step 1, the handler at 0x801F3B28. It then
 *           folds the record pointer held at work offset 0x0C: the 32-bit word
 *           that pointer addresses is masked to 0x00FFFF00 and matched against
 *           five scene states, and a match rewrites work byte 0x02 with the
 *           step that state resumes at, 4 for 0x20800, 6 for 0x10600, 6 for
 *           0x20600, 5 for 0x30600 and 3 for 0x40600, each matched through a
 *           fresh load of the scratchpad pointer cell. A masked word of 0x10300
 *           instead replaces the first quad argument, otherwise 0x80, with six
 *           times the shared main-executable byte at 0x80146308, plus 0x3F and
 *           truncated to a multiple of 8. It then calls the quad drawer at
 *           0x801F3F5C with the fixed arguments (0x40, 1, 0) beside that
 *           first argument. Takes no arguments, returns nothing, and its
 *           0x18-byte frame holds only the saved return address at 0x10. The
 *           roles of the masked scene states beyond the steps they select stay
 *           described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void redirectWorkByte2StepFromMaskedSceneStateThenDrawQuadWorld03Area12013_801F3B84(
    void) {
  u8* work;

  if (D_80143B90 == 5) {
    if (D_80143B92 != 5) {
      SPAD_PTR_SLOT(u8, 0x44)[2]--;
    }
  }
  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) == 0x20800) {
    work[2] = 4;
  }
  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) == 0x10600) {
    work[2] = 6;
  }
  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) == 0x20600) {
    work[2] = 6;
  }
  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) == 0x30600) {
    work[2] = 5;
  }
  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) == 0x40600) {
    work[2] = 3;
  }
  drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
      (*(u32*)*(u32*)(D_1F800044 + 0xC) & 0xFFFF00) == 0x10300
          ? ((D_80146308 * 6) + 0x3F) & 0xF8
          : 0x80,
      0x40, 1, 0);
}
