#include "bof3/bof3.h"

/* Shared work-record pointer cell of the scratchpad. The original
 * re-materialises the cell address and reloads the pointer once per access
 * group, so the fixed-address cell keeps its own loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Pose tables of this overlay's own data span at 0x801F511C. Each is an interior
 * word of that reviewed run and a D_<ADDRESS> name, so the generated
 * undefined-syms map binds it and no target-map row is needed. The bytes at
 * 0x801F515C and 0x801F515D are the two halves of one two-byte pose entry. */
extern u8 D_801F5138[];  /* @source 0x801F5138 @kind data */
extern u16 D_801F513C[]; /* @source 0x801F513C @kind data */
extern u16 D_801F514C[]; /* @source 0x801F514C @kind data */
extern u8 D_801F515C[];  /* @source 0x801F515C @kind data */
extern u8 D_801F515D[];  /* @source 0x801F515D @kind data */

/* Shared camera coordinate words of the main executable, projected by the world
 * overlays. Both lie outside this overlay's payload and are D_<ADDRESS> names,
 * so the generated undefined-syms map binds them and no target-map row is
 * needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Resident map base pointer and map row stride byte of the main executable.
 * Both outside this overlay's payload and bound by their D_<ADDRESS> names. */
extern u32 D_8014931C; /* @source 0x8014931C @kind unknown */
extern u8 D_80104000;  /* @source 0x80104000 @kind unknown */

/* @source 0x8015477C @kind unknown
 * Resident distance helper of the main executable. It takes the two coordinate
 * words of a work record and returns their signed distance as a halfword.
 */
extern s16 func_8015477C(s32 x, s32 y);

/* The overlay record draw routine this step finishes with, called with a zero
 * mode argument. It is a later code span of this same overlay. */
void func_801F381C(s32 arg0);

/* @source 0x801F3C54
 * @behavior Byte-2 state handler that the overlay's second byte-2 dispatch
 *           table (0x801F5184, indexed by the dispatcher at 0x801F3C10) lists
 *           at entry 0. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and derives the cell pose of the record
 *           from this overlay's own pose tables: it stores 8 minus the record
 *           halfword at offset 0x36 into the byte at offset 0x0A, the byte at
 *           0x801F5138 indexed by the signed halfword at offset 0x3A into the
 *           byte at offset 0x0B, the byte at 0x801F515C indexed by twice the
 *           halfword at offset 0x36 back into that halfword, and the byte at
 *           0x801F515D at that same index into the halfword at offset 0x3A, and
 *           records whether the original halfword at offset 0x36 was zero in
 *           the byte at offset 0x08. It then takes the signed distance of the
 *           record words at offsets 0x34 and 0x38 from the resident distance
 *           helper 0x8015477C, stores half its negation less the halfword at
 *           0x801F513C into the record halfword at offset 0x3E and that table
 *           halfword into the halfword at offset 0x32, clears the halfword at
 *           offset 0x30, copies the halfword at 0x801F514C into the halfword at
 *           offset 0x2E, stamps 0x52 into the map byte of the record column at
 *           offset 0x36 for the row at offset 0x3A and again for the row below
 *           through the resident map base pointer 0x8014931C scaled by the
 *           resident map row stride byte 0x80104000, and increments the record
 *           state byte at offset 0x02 so the next dispatch takes entry 3 of the
 *           same table. When the camera word 0x80145EC4 lies no further than
 *           0x8000 from that cell centre ((halfword at offset 0x36 plus 1) << 16
 *           or 0x8000) and the camera word 0x80145EC8 lies no further than
 *           0x10000 from the cell start (halfword at offset 0x3A << 16) or from
 *           the next cell start, it also stamps 0xA1 into both map bytes and
 *           stores -0x100 into the record halfword at offset 0x30. Every path
 *           finishes with the overlay record draw routine 0x801F381C called with
 *           a zero mode argument. Takes no arguments and returns nothing; the
 *           record pointer is loaded before the frame is created and the
 *           0x18-byte frame keeps $ra at 0x14 and the doubled column index at
 *           0x10 across the distance call.
 * @status partial
 * @match 95.78
 * @residual The body is 664 bytes like the original and only two scheduler
 *           placements differ, seven instructions in all. The pose row store at
 *           offset 0x3A is emitted before the argument pointer reload instead
 *           of filling that reload load delay slot, which costs one load delay
 *           nop, and the first window test materialises 0x8000 in the bgez
 *           delay slot where the original leaves a nop and loads it after the
 *           negation. Giving the camera-test column and row their own
 *           quantities closed the earlier leading hunk; separate locals for the
 *           distance arguments, a per-group record pointer, a table pointer
 *           local for the depth pair, a scalar window bound and both pose store
 *           orders were measured and moved the score by nothing.
 */
void seedRecordCellPoseFromTablesStampMapColumnAdvanceStateAndClearSpriteOffsetWhenCameraInsideWorld03Area11813_801F3C54(
    void) {
  u8* stride;
  s32 col;
  s32 index;
  s32 row;
  s32 cur;
  s32 dist;

  col = FIELD_REF(s16, D_1F800044, 0x36);
  index = col * 2;
  FIELD_REF(u8, D_1F800044, 0x0A) = 8 - col;
  FIELD_REF(u8, D_1F800044, 0x0B) =
      D_801F5138[FIELD_REF(s16, D_1F800044, 0x3A)];
  FIELD_REF(s16, D_1F800044, 0x36) = D_801F515C[index];
  FIELD_REF(s16, D_1F800044, 0x3A) = D_801F515D[index];
  FIELD_REF(u8, D_1F800044, 0x08) = col == 0;

  dist = func_8015477C(FIELD_REF(s32, D_1F800044, 0x34),
                       FIELD_REF(s32, D_1F800044, 0x38));
  FIELD_REF(s16, D_1F800044, 0x3E) = -dist / 2 - D_801F513C[col];
  FIELD_REF(s16, D_1F800044, 0x32) = D_801F513C[col];

  stride = PSX_PTR(u8, 0x80104000u);
  FIELD_REF(s16, D_1F800044, 0x2E) = D_801F514C[col];
  FIELD_REF(s16, D_1F800044, 0x30) = 0;

  *(u8*)(D_8014931C + FIELD_REF(s16, D_1F800044, 0x3A) * stride[0] +
         FIELD_REF(s16, D_1F800044, 0x36)) = 0x52;
  *(u8*)(D_8014931C + (FIELD_REF(s16, D_1F800044, 0x3A) + 1) * stride[0] +
         FIELD_REF(s16, D_1F800044, 0x36)) = 0x52;
  ++FIELD_REF(u8, D_1F800044, 0x02);

  cur = FIELD_REF(s16, D_1F800044, 0x36);
  {
    s32 diff;

    diff = D_80145EC4 - (((cur + 1) << 16) | 0x8000);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > 0x8000) {
      goto draw;
    }
  }

  row = FIELD_REF(s16, D_1F800044, 0x3A);
  {
    s32 diff;

    diff = D_80145EC8 - (row << 16);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > 0x10000) {
      diff = D_80145EC8 - ((row + 1) << 16);
      if (diff < 0) {
        diff = -diff;
      }
      if (diff > 0x10000) {
        goto draw;
      }
    }
  }

  *(u8*)(D_8014931C + row * stride[0] + cur) = 0xA1;
  *(u8*)(D_8014931C + (FIELD_REF(s16, D_1F800044, 0x3A) + 1) * stride[0] +
         FIELD_REF(s16, D_1F800044, 0x36)) = 0xA1;
  FIELD_REF(s16, D_1F800044, 0x30) = -0x100;
  FIELD_REF(u8, D_1F800044, 0x02) = 3;

draw:
  func_801F381C(0);
}
