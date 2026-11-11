#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-record pointer cell, bound by this target's map. The original
 * re-materialises the cell address and reloads the published pointer once per
 * tint lookup, because the colour stores between them may alias the cell. */
extern u8* workRecordCursor1F800044;

/* Shared frame counter at 0x80143E6C: this body reads its low bit as the
 * one-unit screen jitter the two rotated end squares are seeded with. The
 * sibling targets declare the same cell the same way. */
extern s32 D_80143E6C;

/* Three-byte tint record run at 0x801F4D5C, the head of this overlay's
 * textbin data row: the original scales the work record's byte +0x06 by three
 * and reads the three bytes as one colour, so the entries are three bytes
 * wide with no padding. */
typedef struct World01Area048Tint {
  u8 r;
  u8 g;
  u8 b;
} World01Area048Tint;

/* @source 0x801F4D5C @kind data */
extern World01Area048Tint D_801F4D5C[];

/* One 8-byte projected entry: the shared projector writes the halfword pair at
 * +0x00/+0x02 and the trailing four bytes stay unwritten here, and the shared
 * pair converter at 0x801AFFD8 reads and writes the same first pair. */
typedef struct World01Area048ScreenPair {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} World01Area048ScreenPair;

/* @source 0x801AFE18 - shared projection setup consuming a 0x20-byte scratch
 * area, declared with the same prototype by the sibling world targets. */
extern void func_801AFE18(void* arg0);

/* @source 0x801AFF04 - shared projector: projects the three-word point passed
 * as arg0 into the screen halfword pair written through arg1. */
extern void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801AFFD8 - shared pair converter: scales the size pair passed as
 * arg1 by the depth of the three-word point passed as arg0 and writes the
 * screen result through arg2. */
extern void func_801AFFD8(const void* arg0, const void* arg1, void* arg2);

/* @source 0x8017A138 - shared heading query: returns the 12-bit phase of the
 * delta pair passed as its two arguments (the sibling overlays read it as the
 * heading between two points). */
extern s32 func_8017A138(s32 delta_y, s32 delta_x);

/* @source 0x8017AA80 - shared primitive stamper, the slot where the SDK's
 * SetLineF2 sits (between SetTile at 0x8017AA6C and SetLineF3 at 0x8017AAA8 in
 * the shipped SDK map): it stamps the length, code and tpage words of the
 * line packet the shared cursor holds. */
extern void func_8017AA80(void* arg0);

/* @source 0x8014E5A0 - engine primitive-append helper, shared with the sibling
 * emitters of this overlay; takes the size class and the allocated size. */
extern void func_8014E5A0(u32 sizeClass, u32 primitiveSize);

/* Rotated end-square emitter at 0x801F3274, unlifted in this overlay. Its own
 * bytes stamp a four-corner packet through the shared stamper at 0x8017A97C,
 * mark it semi-transparent, write the passed centre into the packet halfwords
 * +0x08/+0x0A and lay the four corners on the sine and cosine of the passed
 * phase stepped by 0x100 times the sign-extended radius shifted right 12, so
 * the roles below follow that arithmetic: centre pair, signed radius, phase. */
extern void func_801F3274(s16 centre_x, s16 centre_y, s16 radius, u16 phase);

/* Connector emitter at 0x801F3434, unlifted in this overlay. Its own bytes
 * stamp a POLY_G4 through SetPolyG4, mark it semi-transparent, write the two
 * passed centres into the packet vertex halfwords and read the last argument
 * back as an unsigned halfword, so it takes both centres with their radii and
 * phases in the order this body passes them. */
extern void func_801F3434(s16 centre_x0, s16 centre_y0, s16 radius0, u16 phase0,
                          s16 centre_x1, s16 centre_y1, s16 radius1,
                          u16 phase1);

/* @source 0x801F302C
 * @behavior Line emitter of this overlay's map-panel group, taking the two
 *           three-word points it links and returning nothing.
 *           It appends a 0xC-byte draw-mode packet through the shared append
 *           helper with size class 3, seeds the projection scratch through
 *           0x801AFE18, and stamps the shared cursor through 0x8017AA80 as a
 *           0x10-byte line packet, with semi-transparency cleared, whose
 *           tpage comes from GetTPage(0, 1, 0x3C0, 0) through the draw-mode
 *           packet that is appended first with the same size class.
 *           It projects both points through 0x801AFF04 straight into the
 *           packet's two vertex pairs, tints the red, green and blue bytes
 *           from the three-byte record the work record's byte +0x06 selects in
 *           the 0x801F4D5C run, and appends the line with size 0x10.
 *           It then takes the heading of the packet's own vertex delta through
 *           0x8017A138, converts the fixed size pair {0x40, 0} through
 *           0x801AFFD8 at each point to get that end's screen radius (plus the
 *           frame-parity bit of 0x80143E6C), and emits a rotated square of that
 *           radius at each end through 0x801F3274, the first quarter-turn
 *           phased and the second phased by three quarter-turns off the shared
 *           heading. Finally it hands both centres, both radii and both phases
 *           to the connector emitter 0x801F3434, which draws the gouraud quad
 *           between them.
 *           Its frame holds the 0x20-byte projection scratch at +0x30, the
 *           fixed size pair at +0x20 and the converted pair at +0x28, the
 *           draw-mode argument slot at +0x10 and $ra with $s5..$s0 at
 *           +0x68..+0x50.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTintedLineBetweenPointsWithRotatedEndSquaresAndGouraudConnectorWorld01Area04813_801F302C(
    const void* point0, const void* point1) {
  World01Area048ScreenPair size;
  World01Area048ScreenPair converted;
  u8 scratch[0x20];
  LINE_F2* line;
  s32 heading;
  s16 radius0;
  s16 radius1;
  u16 tpage;

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(3, 0xC);
  func_801AFE18(scratch);
  line = (LINE_F2*)g_PrimCursor;
  func_8017AA80(line);
  SetSemiTrans(line, 0);
  func_801AFF04(point0, &line->x0);
  func_801AFF04(point1, &line->x1);
  line->r0 = D_801F4D5C[workRecordCursor1F800044[6]].r;
  line->g0 = D_801F4D5C[workRecordCursor1F800044[6]].g;
  line->b0 = D_801F4D5C[workRecordCursor1F800044[6]].b;
  func_8014E5A0(3, 0x10);

  heading = func_8017A138(line->y1 - line->y0, line->x1 - line->x0);
  size.x = 0x40;
  size.y = 0;
  func_801AFFD8(point0, &size, &converted);
  radius0 = converted.x + (D_80143E6C & 1);
  func_801F3274(line->x0, line->y0, radius0, heading + 0x400);
  func_801AFFD8(point1, &size, &converted);
  radius1 = converted.x + (D_80143E6C & 1);
  func_801F3274(line->x1, line->y1, radius1, heading + 0xC00);
  func_801F3434(line->x0, line->y0, radius0, heading + 0x400, line->x1,
                line->y1, radius1, heading + 0xC00);
}
