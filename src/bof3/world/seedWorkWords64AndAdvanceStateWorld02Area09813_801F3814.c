#include "bof3/bof3.h"

/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x80145ECC @kind unknown */
extern u32 D_80145ECC;
/* @source 0x1F800044 @kind data */
extern u8 *scratchWorkPointerAt1f800044;

/* @source 0x801F3814
 * @behavior Copies the shared words at 0x80145EC4 and 0x80145EC8 into
 *           scratch work offsets 0x64 and 0x68, and the word at 0x80145ECC
 *           plus 0x800000 into offset 0x6C. Sets work byte 9 to 8, clears
 *           halfword 0x2E, and advances byte 1 through the reloaded work
 *           pointer. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWords64AndAdvanceStateWorld02Area09813_801F3814(void) {
    u8 *work;
    u8 *next;
    u32 third;

    work = scratchWorkPointerAt1f800044;
    FIELD_REF(u32, work, 0x64) = D_80145EC4;
    FIELD_REF(u32, work, 0x68) = D_80145EC8;
    third = D_80145ECC;
    work[9] = 8;
    next = scratchWorkPointerAt1f800044;
    FIELD_REF(u16, work, 0x2E) = 0;
    FIELD_REF(u32, work, 0x6C) = third + 0x800000u;
    next[1] += 1;
}
