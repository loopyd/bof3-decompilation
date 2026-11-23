#include "bof3/bof3.h"

/* Point rows of the table this overlay bases at 0x801F3D80: a 0x10-byte stride,
 * three signed words that this step reads and writes, and a fourth word that
 * the eight literal rows at the table base leave zero. func_801F34FC walks the
 * first fourteen rows of that table and hands each one to the shared
 * three-word projector at 0x801AFF04. Only the part this step proves is
 * modelled.
 * @source 0x801F3D80 @kind table
 */
typedef struct World03Area14413Point {
  s32 x;
  s32 y;
  s32 z;
  s32 unk_0C;
} World03Area14413Point;

extern World03Area14413Point D_801F3D80[];

/* @source 0x801F376C
 * @behavior Componentwise midpoint of three point rows of this overlay's own
 *           table at 0x801F3D80. It masks the three byte arguments, scales each
 *           by the 0x10-byte row stride and keeps the scaled indices in the
 *           argument registers, then for each of the row's first three words
 *           loads the word of the row named by arg0 and the word of the row
 *           named by arg1, adds them as signed 32-bit values and stores the
 *           arithmetic half of the sum into the same word of the row named by
 *           arg2. The fourth word of every row is left alone. The table base is
 *           materialised per access through the lui/addu pair against the
 *           scaled index, so each read is at the row offset 0x00, 0x04 or 0x08
 *           of the table base and each store goes back through the same form.
 *           It is a leaf: no call, no stack frame, no saved register, and the
 *           return delay slot stays empty.
 *
 *           Both ends of the argument range are set by its only caller, the
 *           step-0 bootstrap at 0x801F2E18, which issues six calls through the
 *           jal sites at 0x801F2F04, 0x801F2F5C, 0x801F2FB0, 0x801F2FF8,
 *           0x801F304C and 0x801F307C with the index triplets (0,5,8), (0,3,9),
 *           (2,4,10), (2,7,11), (4,7,12) and (3,5,13), so the rows it derives
 *           are the six rows 8 through 13 of the fourteen func_801F34FC
 *           projects. No other payload under out/binaries carries this body: a
 *           scan for its first four words and for its three-word shift-and-add
 *           run finds only this span, so no sibling seeded it and the shape is
 *           written from these original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateMidpointPointThird(
    u8 arg0, u8 arg1, u8 arg2) {
  D_801F3D80[arg2].x = (D_801F3D80[arg0].x + D_801F3D80[arg1].x) >> 1;
  D_801F3D80[arg2].y = (D_801F3D80[arg0].y + D_801F3D80[arg1].y) >> 1;
  D_801F3D80[arg2].z = (D_801F3D80[arg0].z + D_801F3D80[arg1].z) >> 1;
}
