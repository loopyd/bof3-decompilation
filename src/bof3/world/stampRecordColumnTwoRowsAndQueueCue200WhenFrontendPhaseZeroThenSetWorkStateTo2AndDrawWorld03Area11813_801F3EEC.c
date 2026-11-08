#include "bof3/bof3.h"

/* Shared work-record pointer cell of the scratchpad. The original
 * re-materialises the cell address and reloads the pointer once per access
 * group, so the fixed-address cell keeps its own loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Shared state byte and resident map base pointer of the main executable. Both
 * lie outside this overlay's payload and are D_<ADDRESS> names, so the
 * generated undefined-syms map binds them and no target-map row is needed. */
extern u8 D_8014933B;  /* @source 0x8014933B @kind data */
extern u32 D_8014931C; /* @source 0x8014931C @kind unknown */

/* Front-end phase byte of the main executable; the cue below only fires while
 * that phase holds zero. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* The overlay record draw routine this step finishes with, called with mode 1.
 * It is a later code span of this same overlay. */
void func_801F381C(s32 arg0);

/* @source 0x801F3EEC
 * @behavior Byte-2 state handler that the overlay's second byte-2 dispatch
 *           table (0x801F5184, indexed by the dispatcher at 0x801F3C10) lists
 *           at entry 1. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and compares its byte at offset 0x0A with
 *           the shared state byte 0x8014933B: when they differ the body is
 *           skipped and only the draw call runs. Otherwise the front-end phase
 *           byte 0x80143BB0 gates sound cue 0x200 through the resident cue
 *           dispatcher at 0x8015DF18, which fires only while that phase byte is
 *           zero. It then stamps 0xA1 into the map byte of the record's column
 *           at offset 0x36 for the row at offset 0x3A and again for the row
 *           below, addressed through the resident map base pointer 0x8014931C
 *           scaled by the resident map row stride byte 0x80104000, and stores 2
 *           into the record state byte at offset 0x02 so the next dispatch
 *           takes entry 2 of the same table. Every path finishes with the
 *           overlay record draw routine 0x801F381C called with mode 1. Takes no
 *           arguments and returns nothing; the record pointer is loaded before
 *           the frame is created, the 0x18-byte frame only keeps $ra at 0x10,
 *           the record cell is reloaded for each of the four uses, and the row
 *           stride byte is addressed through one pointer local so a single base
 *           serves both stamps.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stampRecordColumnTwoRowsAndQueueCue200WhenFrontendPhaseZeroThenSetWorkStateTo2AndDrawWorld03Area11813_801F3EEC(
    void) {
  u8* stride;

  if (D_8014933B == FIELD_REF(u8, D_1F800044, 0x0A)) {
    if (D_80143BB0 == 0) {
      dispatchSoundCue(0x200);
    }

    stride = PSX_PTR(u8, 0x80104000u);
    *(u8*)(D_8014931C + FIELD_REF(s16, D_1F800044, 0x3A) * stride[0] +
           FIELD_REF(s16, D_1F800044, 0x36)) = 0xA1;
    *(u8*)(D_8014931C + (FIELD_REF(s16, D_1F800044, 0x3A) + 1) * stride[0] +
           FIELD_REF(s16, D_1F800044, 0x36)) = 0xA1;
    FIELD_REF(u8, D_1F800044, 0x02) = 2;
  }

  func_801F381C(1);
}
