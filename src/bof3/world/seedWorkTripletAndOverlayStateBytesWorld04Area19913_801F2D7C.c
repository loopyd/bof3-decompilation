#include "bof3/bof3.h"

extern u8 D_8014933B; /* @source 0x8014933B @kind data */
extern u8 D_801F54D0; /* @source 0x801F54D0 @kind data */
extern u8 D_801F54D1; /* @source 0x801F54D1 @kind data */
extern u8 D_801F54D2; /* @source 0x801F54D2 @kind data */
extern u8 D_801F54D3; /* @source 0x801F54D3 @kind data */
extern u8 D_801F54D4; /* @source 0x801F54D4 @kind data */
extern u8 D_801F54D5; /* @source 0x801F54D5 @kind data */

/* @source 0x801F2D7C
 * @behavior Clears the shared overlay state byte at 0x8014933B, seeds the
 *           scratch work record published at pointer cell 0x1F800044 with
 *           word +0x34 = 0x54 and words +0x38/+0x3C = 0, then writes the six
 *           overlay state bytes 0x801F54D0..0x801F54D5 with 0x7E, 0x14, 1,
 *           0x54, 0x13 and 0. Takes no arguments, returns no value, and the
 *           work field and overlay byte roles are unknown.
 * @status partial
 * @match 77.78
 * @residual Register exchange: GCC 2.7.2 allocates this body's single
 *           scratchpad pointer quantity to $v0 and the 0x54 literal to $v1,
 *           while the original holds the pointer in $v1 and every literal in
 *           $v0. Size, frame and memory events match (108 bytes, 27
 *           instructions); the first difference is instruction 0, and the
 *           scheduled position of the 0x7E load follows from the same
 *           exchange. Local-alloc/global-alloc dumps show the pointer as a
 *           4-reference quantity and the literal as 2 references; measured
 *           shapes with the same instruction set (u8/u32/s32 views, indexed
 *           stores, per-store cell expressions, split cursor locals, reordered
 *           statements) all keep the pointer in $v0.
 */
void seedWorkTripletAndOverlayStateBytesWorld04Area19913_801F2D7C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  D_8014933B = 0;
  *(s32*)(work + 0x34) = 0x54;
  *(s32*)(work + 0x38) = 0;
  *(s32*)(work + 0x3C) = 0;
  D_801F54D0 = 0x7E;
  D_801F54D1 = 0x14;
  D_801F54D2 = 1;
  D_801F54D3 = 0x54;
  D_801F54D4 = 0x13;
  D_801F54D5 = 0;
}
