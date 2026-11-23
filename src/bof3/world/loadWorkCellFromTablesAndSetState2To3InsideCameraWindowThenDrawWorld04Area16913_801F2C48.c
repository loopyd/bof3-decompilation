#include "bof3/bof3.h"

/* Shared work-record pointer cell of the scratchpad. The original
 * re-materialises the cell address and reloads the pointer once per access
 * group, so the fixed-address cell keeps its own loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Shared camera coordinate words of the main executable, projected by the
 * world overlays. Both lie outside this overlay's payload and are D_<ADDRESS>
 * names, so the generated undefined-syms map binds them and no target-map row
 * is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Per-index cell tables of this overlay's own data span at 0x801F3A90. Each is
 * an interior word of that reviewed run and a D_<ADDRESS> name, so the
 * generated undefined-syms map binds it and no target-map row is needed. The
 * bytes at 0x801F3AA8 and 0x801F3AA9 are the two halves of one two-byte cell
 * entry, and 0x801F3AA4 is the halfword table the same index selects. */
extern u8 D_801F3AA8[];  /* @source 0x801F3AA8 @kind data */
extern u8 D_801F3AA9[];  /* @source 0x801F3AA9 @kind data */
extern u16 D_801F3AA4[]; /* @source 0x801F3AA4 @kind data */

/* Local drawing routine of this overlay. */
void drawMirroredWorkRecordQuadPairForGraphTypeWorld04Area16913_801F30E8(void);

/* @source 0x801F2C48
 * @behavior State-0 handler of this overlay's work-record step dispatch: it
 *           is word 0 of the handler run based at 0x801F3A90, which the
 *           dispatcher at 0x801F2C04 indexes with the record's state byte at
 *           +2, so no jal names it.
 *           It reads the record published at the scratchpad pointer cell
 *           0x1F800044, stores the route byte at +8 as the zero test of the
 *           record's halfword 0x3A, and takes twice the signed halfword 0x36
 *           as the index of the cell tables of this overlay's data blob: the
 *           byte at 0x801F3AA8 at that index goes into the halfword at 0x36,
 *           the byte at 0x801F3AA9 at that index into the halfword at 0x3A,
 *           and the halfword at 0x801F3AA4 at that index into the unsigned
 *           depth halfword at 0x3E. In the same pass the state byte at +2
 *           increments so the next dispatch takes entry 1, and the progress
 *           halfword at 0x30 is cleared.
 *           The route byte then selects the axis the window test uses: when
 *           it is non-zero the shared camera word D_80145EC8 is compared with
 *           the new halfword 0x3A, and when it is zero D_80145EC4 is compared
 *           with the new halfword 0x36. Both build that halfword's cell
 *           centre (((halfword + 1) << 16) | 0x8000) and take the absolute
 *           difference; when it exceeds 0x8000 the camera is outside and the
 *           record keeps state 1. Otherwise the other axis decides, through
 *           its own halfword and camera word: only when the absolute
 *           difference from both the halfword's cell start (halfword << 16)
 *           and the next cell's start ((halfword + 1) << 16) exceeds 0x10000
 *           does the record keep state 1, and otherwise the progress halfword
 *           takes 0xC0 and the state byte takes 3, the endpoint the step
 *           handler at 0x801F2F0C counts that progress up to, so a camera
 *           already inside the cell skips the approach.
 *           It ends by calling this overlay's draw routine at 0x801F30E8
 *           either way. The cell is re-read for every access group: the first
 *           read supplies the route byte and the index, the second the table
 *           stores and the state increment, and the third the window test and
 *           the progress it can set. The depth halfword is read before that
 *           increment and stored after the third read, in the second read's
 *           base. Takes no arguments and returns nothing; the 0x18-byte frame
 *           holds $ra at 0x10 across the call.
 *
 *           Representation notes from the measured shapes: one pointer
 *           variable serving all three reads moved the index to $v1 and the
 *           pointer to $a1 (59/97 instructions), two pointer variables with
 *           the third read placed before the two progress stores dropped a
 *           cell read (53/97), writing 0x10000 as two literals instead of the
 *           per-arm window variable broke the window comparison's operand
 *           form (42/97), and storing the depth halfword in the table group
 *           instead of through its own temporary put that store one
 *           instruction early (96/97).
 * @status exact
 * @match 100.00
 * @residual none
 */
void loadWorkCellFromTablesAndSetState2To3InsideCameraWindowThenDrawWorld04Area16913_801F2C48(
    void) {
  u8* work;
  u16 halfword3e;
  u8* gate;
  s32 index;
  s32 route;
  s32 value;
  s32 base;
  s32 limit;

  route = FIELD_REF(s16, D_1F800044, 0x3A) == 0;
  index = FIELD_REF(s16, D_1F800044, 0x36);
  FIELD_REF(u8, D_1F800044, 0x08) = route;

  work = D_1F800044;
  FIELD_REF(u16, work, 0x36) = D_801F3AA8[index * 2];
  FIELD_REF(u16, work, 0x3A) = D_801F3AA9[index * 2];
  halfword3e = D_801F3AA4[index];
  ++FIELD_REF(u8, work, 0x02);
  gate = D_1F800044;
  FIELD_REF(u16, work, 0x30) = 0;
  FIELD_REF(u16, work, 0x3E) = halfword3e;

  if (FIELD_REF(u8, gate, 0x08) != 0) {
    s32 diff;

    diff = D_80145EC8 - (((FIELD_REF(s16, gate, 0x3A) + 1) << 16) | 0x8000);
    diff = __builtin_abs(diff);
    if (diff > 0x8000) {
      goto draw;
    }
    limit = 0x10000;
    value = FIELD_REF(s16, gate, 0x36);
    base = D_80145EC4;
  } else {
    s32 diff;

    diff = D_80145EC4 - (((FIELD_REF(s16, gate, 0x36) + 1) << 16) | 0x8000);
    diff = __builtin_abs(diff);
    if (diff > 0x8000) {
      goto draw;
    }
    limit = 0x10000;
    value = FIELD_REF(s16, gate, 0x3A);
    base = D_80145EC8;
  }

  {
    s32 diff;

    diff = base - (value << 16);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > limit) {
      diff = base - ((value + 1) << 16);
      if (diff < 0) {
        diff = -diff;
      }
      if (diff > limit) {
        goto draw;
      }
    }
  }

  FIELD_REF(u16, gate, 0x30) = 0xC0;
  FIELD_REF(u8, gate, 0x02) = 3;

draw:
  drawMirroredWorkRecordQuadPairForGraphTypeWorld04Area16913_801F30E8();
}
