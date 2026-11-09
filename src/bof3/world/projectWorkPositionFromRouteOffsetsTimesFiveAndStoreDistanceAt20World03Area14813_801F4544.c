#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay's work record. This body reads
 * the cell once, before it creates its frame, keeps that answer for its record
 * reads and writes below and reloads the cell once more after its single call. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the work route byte at
 * +0x08. The original reads that byte once per table, which is why each of the
 * two statements below names it: it scales the byte by eight, element index
 * route * 2, and multiplies each selected offset by five before adding it.
 * @source 0x80181B94 @kind table */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Distance helper of the shared EXE image, outside this payload: it answers the
 * distance of the supplied 16.16 point pair. The front-end lifts at 0x801CF050,
 * 0x801CEF14 and 0x801CF62C declare the same prototype. */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F4544
 * @behavior Position step of this overlay, called by the plain `jal` at
 *           0x801F42F4 from the state-byte-1 handler based on the handler table
 *           0x801F6160; no code-pointer word of this payload holds this
 *           address. It reads the work record through the scratchpad pointer
 *           cell 0x1F800044, selects the eight-byte record of the route byte at
 *           +0x08 from the offset tables 0x80181B94 and 0x80181B98, multiplies
 *           the selected first-coordinate and second-coordinate offsets by
 *           five and adds them to the record words +0x34 and +0x38, storing
 *           those two derived position words into +0x18 and +0x1C. It then
 *           passes exactly those two stored words to the shared distance helper
 *           0x8015477C and stores the answer shifted left by 16 into the record
 *           word +0x20, reloading the scratchpad cell to reach that last store.
 *           Takes no arguments, returns nothing and keeps a 0x18-byte frame
 *           whose only saved register is ra at +0x10; the scratchpad record
 *           pointer cell is loaded before the frame is created.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkPositionFromRouteOffsetsTimesFiveAndStoreDistanceAt20World03Area14813_801F4544(
    void) {
  u8* work;

  work = D_1F800044;
  *(s32*)(work + 0x18) = *(s32*)(work + 0x34) + D_80181B94[work[0x08] * 2] * 5;
  *(s32*)(work + 0x1C) = *(s32*)(work + 0x38) + D_80181B98[work[0x08] * 2] * 5;
  *(s32*)(D_1F800044 + 0x20) =
      func_8015477C(*(s32*)(work + 0x18), *(s32*)(work + 0x1C)) << 16;
}
