#include "bof3/bof3.h"

/* Pose tables of this overlay's own data span at 0x801F511C. Each is an interior
 * word of that reviewed run and a D_<ADDRESS> name, so the generated
 * undefined-syms map binds it and no target-map row is needed. The bytes at
 * 0x801F515C and 0x801F515D are the two halves of one two-byte route entry. */
extern u8 D_801F5138[];  /* @source 0x801F5138 @kind data */
extern u16 D_801F513C[]; /* @source 0x801F513C @kind data */
extern u16 D_801F514C[]; /* @source 0x801F514C @kind data */
extern u8 D_801F515C[];  /* @source 0x801F515C @kind data */
extern u8 D_801F515D[];  /* @source 0x801F515D @kind data */

/* Shared flag bank of the main executable, probed at the record cursor byte
 * 0x0B and bound by its D_<ADDRESS> name. */
extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */

/* Shared camera coordinate words of the main executable, projected by the world
 * overlays. Both lie outside this overlay's payload and are D_<ADDRESS> names,
 * so the generated undefined-syms map binds them and no target-map row is
 * needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Work record pointer cell of the scratchpad. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Resident map base pointer of the main executable. It lies outside this
 * overlay's payload and is a D_<ADDRESS> name, so the generated undefined-syms
 * map binds it and no target-map row is needed. */
extern u32 D_8014931C; /* @source 0x8014931C @kind unknown */

/* @source 0x8015477C @kind unknown
 * Resident distance helper of the main executable. It takes the two coordinate
 * words of a work record and returns their signed distance as a halfword.
 */
extern s16 func_8015477C(s32 x, s32 y);

/* Shared flag tester of the main executable, the same probe the neighbouring
 * handlers of this overlay call. */
/* @source 0x8015B5D4 @kind unknown */
extern s32 func_8015B5D4(u32 arg0, s32 arg1);

/* The overlay record draw routine this step finishes with, called with a zero
 * mode argument. It is a later code span of this same overlay. */
void func_801F381C(s32 arg0);

/* @source 0x801F3058
 * @behavior Byte-2 state handler that the first byte-2 dispatch table of this
 *           overlay (0x801F516C, indexed by the dispatcher at 0x801F3014) lists
 *           at entry 0. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044, stores the record row halfword at offset
 *           0x3A plus one into the step byte at offset 0x0A, the byte at
 *           0x801F5138 indexed by that row into the flag cursor byte at offset
 *           0x0B, and then routes the record along this overlay's own route
 *           tables indexed by twice the original column halfword at offset 0x36:
 *           the byte at 0x801F515C back into that halfword, the byte at
 *           0x801F515D into the row halfword at offset 0x3A, and whether that
 *           original column was zero into the route byte at offset 0x08. It
 *           then takes the signed distance of the record words at offsets 0x34
 *           and 0x38 from the resident distance helper 0x8015477C, stores half
 *           its negation less the halfword at 0x801F513C into the record
 *           halfword at offset 0x3E and that table halfword into the halfword at
 *           0x32, copies the halfword at 0x801F514C into the halfword at offset
 *           0x2E and clears the halfword at offset 0x30. It then probes flag
 *           bank 0x80144ED0 at the cursor byte. When the flag is clear it stamps
 *           0x52 into the map byte of the record column at offset 0x36 for the
 *           row at offset 0x3A and again into the neighbour cell chosen by the
 *           route byte (the row below when that byte is zero, otherwise the
 *           column advanced by it), addressed through the resident map base
 *           pointer 0x8014931C scaled by the resident map row stride byte
 *           0x80104000, and stores 5 into the record state byte at offset 0x02.
 *           When the flag is set it stamps the same two cells with 0xA1,
 *           increments that state byte, and then, when the camera word for the
 *           route-selected axis lies no further than 0x8000 from the cell centre
 *           (((halfword + 1) << 16) | 0x8000) and the other camera word lies no
 *           further than 0x10000 from that axis halfword's own cell start
 *           (halfword << 16) or from the next cell start, stamps nothing more
 *           and stores -0x100 into the record halfword at offset 0x30 together
 *           with 3 into the state byte. Every path finishes with the overlay
 *           record draw routine 0x801F381C called with a zero mode argument.
 *           Takes no arguments and returns nothing; the record pointer is
 *           loaded before the frame is created, the 0x18-byte frame keeps $ra at
 *           0x14 and the doubled column index at 0x10 across the distance call,
 *           and the record cell is reloaded once per access group.
 * @status partial
 * @match 61.97
 * @residual Behavior, tables, constants and instruction count class agree, but this
 *           source matches 132 of the original 211 instructions and emits 852
 *           bytes against 844. The first divergence is instruction 13: the
 *           original loads the column halfword into the dying record pointer
 *           register (`lh v1,54(v1)`) while this shape loads it into $a2 and
 *           keeps the following access group's record pointer in $v1. That single
 *           allocation tie pushes the 0x08 store ahead of the 0x3A store load and
 *           costs the two extra instructions. The stamp arms then differ in
 *           register placement only: one rematerialised 0x52/0xA1 register ($t0)
 *           covers both stores where the original reloads the byte per store, the
 *           multiplier result lands in $t1 instead of $a3, the row-stride pointer
 *           stays in $a3 instead of $a1, the neighbour column sum uses $a2
 *           instead of $v1, and the flag-set arm reuses $s0 for the record
 *           pointer. The state-3 window tail materialises 0x8000 in the bgez
 *           delay slot, the same placement the sibling 0x801F3590 records.
 *           Measured and rejected shapes: inline record cell per reference
 *           (126/213), scratchpad pointer-slot macro (92/211), one local
 *           reassigned per access group (107/211), per-arm record pointer locals
 *           (76/214), per-arm stride-byte locals (119/211), per-arm record
 *           pointer variable (99/211), column read after the flag-cursor group
 *           (130/213), column and row declarations reordered (132/213 each).
 */
void routeRecordToNextCellStampMapCellsByAreaFlagThenSetStateFromCameraWindowWorld03Area11813_801F3058(
    void) {
  u8* work;
  u8* record;
  u8* stride;
  s32 col;
  s32 dist;
  s32 row;
  s32 value;
  s32 base;

  work = D_1F800044;
  FIELD_REF(u8, work, 0x0A) = FIELD_REF(u8, work, 0x3A) + 1;

  col = FIELD_REF(s16, work, 0x36);

  record = D_1F800044;
  FIELD_REF(u8, record, 0x0B) = D_801F5138[FIELD_REF(s16, record, 0x3A)];

  FIELD_REF(s16, D_1F800044, 0x36) = D_801F515C[col * 2];
  FIELD_REF(u8, D_1F800044, 0x08) = col == 0;
  FIELD_REF(s16, D_1F800044, 0x3A) = D_801F515D[col * 2];

  dist = func_8015477C(FIELD_REF(s32, D_1F800044, 0x34), FIELD_REF(s32, D_1F800044, 0x38));

  FIELD_REF(s16, D_1F800044, 0x3E) = -dist / 2 - D_801F513C[col];
  FIELD_REF(s16, D_1F800044, 0x32) = D_801F513C[col];
  FIELD_REF(s16, D_1F800044, 0x2E) = D_801F514C[col];
  FIELD_REF(s16, D_1F800044, 0x30) = 0;

  if (func_8015B5D4((u32)&D_80144ED0, FIELD_REF(u8, D_1F800044, 0x0B)) == 0) {
    stride = PSX_PTR(u8, 0x80104000u);
    *(u8*)(D_8014931C + FIELD_REF(s16, D_1F800044, 0x3A) * stride[0] +
           FIELD_REF(s16, D_1F800044, 0x36)) = 0x52;

    col = FIELD_REF(s16, D_1F800044, 0x36) + FIELD_REF(u8, D_1F800044, 0x08);
    row = FIELD_REF(s16, D_1F800044, 0x3A);
    if (FIELD_REF(u8, D_1F800044, 0x08) == 0) {
      row = row + 1;
    }
    *(u8*)(D_8014931C + row * stride[0] + col) = 0x52;

    FIELD_REF(u8, D_1F800044, 0x02) = 5;
    goto draw;
  }

  stride = PSX_PTR(u8, 0x80104000u);
  *(u8*)(D_8014931C + FIELD_REF(s16, D_1F800044, 0x3A) * stride[0] +
         FIELD_REF(s16, D_1F800044, 0x36)) = 0xA1;

  col = FIELD_REF(s16, D_1F800044, 0x36) + FIELD_REF(u8, D_1F800044, 0x08);
  row = FIELD_REF(s16, D_1F800044, 0x3A);
  if (FIELD_REF(u8, D_1F800044, 0x08) == 0) {
    row = row + 1;
  }
  *(u8*)(D_8014931C + row * stride[0] + col) = 0xA1;

  ++FIELD_REF(u8, D_1F800044, 0x02);

  if (FIELD_REF(u8, D_1F800044, 0x08) != 0) {
    s32 diff;

    diff = D_80145EC8 - (((FIELD_REF(s16, D_1F800044, 0x3A) + 1) << 16) | 0x8000);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > 0x8000) {
      goto draw;
    }
    value = FIELD_REF(s16, D_1F800044, 0x36);
    base = D_80145EC4;
  } else {
    s32 diff;

    diff = D_80145EC4 - (((FIELD_REF(s16, D_1F800044, 0x36) + 1) << 16) | 0x8000);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > 0x8000) {
      goto draw;
    }
    value = FIELD_REF(s16, D_1F800044, 0x3A);
    base = D_80145EC8;
  }

  {
    s32 diff;

    diff = base - (value << 16);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > 0x10000) {
      diff = base - ((value + 1) << 16);
      if (diff < 0) {
        diff = -diff;
      }
      if (diff > 0x10000) {
        goto draw;
      }
    }
  }

  FIELD_REF(s16, D_1F800044, 0x30) = -0x100;
  FIELD_REF(u8, D_1F800044, 0x02) = 3;

draw:
  func_801F381C(0);
}
