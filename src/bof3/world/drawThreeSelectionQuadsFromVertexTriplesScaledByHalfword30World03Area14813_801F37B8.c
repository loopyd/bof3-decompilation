#include "bof3/bof3.h"

/* Scratchpad vector deck the four quad corners are built in and handed to
 * RotTransPers4, at the four 8-byte vectors starting at 0x1F800014.
 * @source 0x1F800014 @kind unknown
 */
extern SVECTOR D_1F800014[];
/* Pointer cell of the work record this overlay publishes and every record
 * handler of the target reads.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* Shared primitive cursor of the main executable, the primitive each of the
 * three quads is seeded in.
 * @source 0x8014598C @kind unknown
 */
extern u8* g_PrimCursor;

/* Signed scale bytes of the packed overlay drawing tables, read at the low bit
 * of the selection index and multiplied by the record halfword at 0x30. The
 * four bytes sit immediately before the vertex triples at 0x801F5038.
 * @source 0x801F5034 @kind unknown
 */
extern volatile u8 D_801F5034[];
/* Packed vertex triples of the overlay drawing tables: three s16 per vertex
 * and four vertices per selection row, so each row is 0x18 bytes. The three
 * halfwords of a triple bind at their own addresses, which are the three words
 * the table base reaches.
 * @source 0x801F5038 @kind unknown
 */
extern volatile u8 D_801F5038[];
/* @source 0x801F503A @kind unknown */
extern volatile u8 D_801F503A[];
/* @source 0x801F503C @kind unknown */
extern volatile u8 D_801F503C[];
/* Texture words of the overlay drawing tables, three per selection row.
 * @source 0x801F5080 @kind unknown
 */
extern volatile u32 D_801F5080[];

void func_80155560(u32 arg0, void* arg1, s32 arg2);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F37B8
 * @behavior Overlay draw routine of the payload, called from three of its
 *           state handlers (0x801F361C with modes -2 and 1, 0x801F3720 with
 *           modes -1 and 1, and 0x801F3788 with mode 0). Its first argument is
 *           a selection index and its second a signed byte mode. It runs three
 *           times over the selection rows of the overlay drawing tables: for
 *           row i it seeds the shared primitive cursor g_PrimCursor as a
 *           POLY_FT4, clears its shade texture and fills the four scratchpad
 *           corner vectors at 0x1F800014 from that row's four packed vertex
 *           triples at 0x801F5038 + row * 0x18, keeping each triple's y and z
 *           and rewriting x as the triple's x plus ((selection & 1) << 7) minus
 *           the record halfword at 0x30 times the signed byte at 0x801F5034 +
 *           (selection & 1). RotTransPers4 projects the four corners into the
 *           primitive's four vertex pairs, func_80155560 fills the texture
 *           state from the row's word in 0x801F5080, and func_80155A08 links
 *           the record's fixed-point position pair at 0x34/0x38 with the mode
 *           as its kind and 0x28 as its size class. Takes the selection index
 *           and the mode, returns nothing, and keeps a 0x58-byte frame whose
 *           saved registers are $s0-$s7 and $ra plus the two long cells the
 *           projection writes.
 *
 * Name evidence: the body's own constants and loop bound fix the name - three
 * iterations over selection rows (`slti 3`), four corners per row built from
 * six-byte vertex triples at 0x801F5038, each row's x scaled by the signed byte
 * 0x801F5034[selection & 1] times the record halfword at 0x30, and the
 * POLY_FT4/SetShadeTex/RotTransPers4 seeding the siblings 0x801F4634 and
 * 0x801F40F0 also record as quad drawing.
 *
 * Spelling evidence: the corner stores, the y and z corner loads, the scale
 *           byte and the texture word are qualified volatile because the
 *           original reloads them and keeps their order around the corner
 *           store; each qualifier was measured - the unqualified forms fold the
 *           scale byte into a signed load and let the y and z loads cross the
 *           corner store.
 *
 * Shape evidence: no other payload carries these 472 bytes - the prologue
 * signature and two interior 32-byte signatures occur once each across the 396
 * overlay payloads, and the span hash differs from every neighbour - so neither
 * clone eligibility nor a byte-identical twin applies. The nearest exact
 * sibling drawRecordCellTexturedQuadWorld03Area11813_801F4634 supplies the
 * declaration style, the scratchpad deck spelling and the two call shapes; its
 * body is single-quad and reads different record cells, so it is shape evidence
 * only.
 * @status partial
 * @match 91.53
 * @residual The instruction set differs only in two reordered groups. The
 *           original materialises the texture word base 0x801F5080 inside each
 *           iteration and adds the selection row term 12 * selection before the
 *           column term 4 * i, while this body folds i into the index first, so
 *           the address arithmetic orders differently. The entry block is also
 *           scheduled in another order: the original initialises the selection
 *           and its 7-bit bias before the scratchpad deck base and the row
 *           offset. Every remaining word is an address-register or schedule
 *           choice; the stored halfwords, calls and constants agree.
 */
void drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
    s32 arg0, s32 arg1) {
  SVECTOR* v;
  POLY_FT4* prim;
  u8* work;
  s32 i;
  s32 off;
  s32 j;
  s32 dst;
  s32 src;
  s32 sel;
  long depth;
  long flag;

  i = 0;
  sel = arg0 & 1;
  v = D_1F800014;
  off = 0;
  do {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    work = D_1F800044;
    j = 0;
    src = off;
    dst = 0;
    do {
      FIELD_REF(volatile s16, D_1F800014, dst) =
          FIELD_REF(u16, D_801F5038, src) + (sel << 7) -
          FIELD_REF(u16, work, 0x30) * (s8)D_801F5034[sel];
      FIELD_REF(volatile s16, D_1F800014, dst + 2) = FIELD_REF(volatile u16, D_801F503A, src);
      FIELD_REF(volatile s16, D_1F800014, dst + 4) = FIELD_REF(volatile u16, D_801F503C, src);
      dst += 8;
      src += 6;
      j++;
    } while (j < 4);
    RotTransPers4(v, v + 1, v + 2, v + 3, (long*)&prim->x0, (long*)&prim->x1,
                  (long*)&prim->x2, (long*)&prim->x3, &depth, &flag);
    off += 0x18;
    func_80155560(D_801F5080[3 * arg0 + i], (void*)prim, 1);
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                  FIELD_REF(s32, D_1F800044, 0x38), (s8)arg1, 0x28);
    i++;
  } while (i < 3);
}
