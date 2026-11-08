#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident helpers this overlay calls, declared the way its exact siblings
 * declare them: func_801AFF04 projects the three-word point handed as arg0 into
 * the screen halfword pair written at arg1, func_801AFFD8 converts the pair
 * handed at arg1 in place into arg2 for the same point, and func_80155A08 links
 * one primitive at the passed coordinate pair with its kind and size class. */
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* One 8-byte projected screen entry: func_801AFF04 and func_801AFFD8 write the
 * halfword pair at +0x00/+0x02 and the trailing four bytes stay unwritten. The
 * original keeps four such entries in the one address-taken workspace, which is
 * what reproduces its 0x40-byte frame; only the first two are addressed here. */
typedef struct Area12513ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} Area12513ScreenPoint;

/* The 0x28-byte work record of the 0x10-entry table at 0x800E4800 that this
 * overlay's sweep hands over: the byte at +0x03 is the ramp value the sweep
 * raises by 4 and lowers by 4, the halfword at +0x04 is the value it advances by
 * 8, and +0x08..+0x10 is the three-word point it moves by the velocity words at
 * +0x18/+0x1C and whose third word it writes as the scaled pair result. The
 * field names follow the sweep's own exact lift; the roles this body does not
 * exercise stay offset-anchored. */
typedef struct Area12513QuadRecord {
  u8  flags_00;
  u8  state_01;
  u8  counter_02;
  u8  intensity_03;
  u16 angle_04;
  u8  pad_06[2];
  s32 position_08;
  s32 position_0c;
  s32 scaled_10;
} Area12513QuadRecord;

/* @source 0x801F2FC4
 * @behavior Per-record emitter of this overlay's sweep: called once for each
 *           active record of the 0x10-entry, 0x28-byte-stride table at
 *           0x800E4800 by
 *           rampIntensityIntegrateAndRenderActiveWorkRecordsWorld03Area12513_801F33B0,
 *           the only caller in this payload, after that sweep has advanced the
 *           record intensity, its halfword at +0x04 and its position words. It
 *           takes the shared primitive cursor 0x8014598C, initialises one
 *           POLY_FT4 with SetPolyFT4 and marks it translucent with
 *           SetSemiTrans(prim, 1), projects the record point at +0x08 through
 *           func_801AFF04, writes the record halfword at +0x04 into both
 *           halfwords of a second screen entry and converts that pair in place
 *           through func_801AFFD8, then places the four quad vertices around
 *           the projected point: the pair edges are the converted pair, the
 *           first vertex sits half a pair above and left of the projected
 *           point, and the second introduces the full pair while the third and
 *           fourth repeat the two axes, so the point is the quad centre. The
 *           texels are the 0x20 by 0x20 cell at (0xE0, 0x30), the CLUT is
 *           GetClut(0xA0, 0x1E3), the texture page is
 *           GetTPage(0, 1, 0x2C0, 0x100), and the three colour bytes come from
 *           the ramp byte at +0x03: red is that byte, green three quarters of
 *           it and blue half of it. It closes by queueing the primitive through
 *           func_80155A08 with the record position pair, kind 3 and size 0x28.
 *           Takes the record pointer as its only argument, returns nothing, and
 *           keeps a 0x40-byte frame for $ra and $s0..$s2 over the screen-point
 *           workspace at 0x10..0x2F.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkRecordSemiTransparentQuadWorld03Area12513_801F2FC4(void* arg0) {
  Area12513ScreenPoint                screen[4];
  volatile Area12513QuadRecord* record;
  const void*                         object;
  POLY_FT4*                           prim;
  u16                                 size;

  record = (volatile Area12513QuadRecord*)arg0;
  object = (const void*)((const u8*)arg0 + 8u);
  prim = (POLY_FT4*)g_PrimCursor;

  SetPolyFT4(prim);
  SetSemiTrans(prim, 1);

  func_801AFF04(object, &screen[0]);
  size = record->angle_04;
  screen[1].x = size;
  size = record->angle_04;
  screen[1].y = size;
  func_801AFFD8(object, &screen[1], &screen[1]);

  prim->x0 = screen[0].x - ((s16)screen[1].x >> 1);
  prim->y0 = screen[0].y - ((s16)screen[1].y >> 1);
  prim->x1 = (screen[0].x - ((s16)screen[1].x >> 1)) + (s16)screen[1].x;
  prim->y1 = screen[0].y - ((s16)screen[1].y >> 1);
  prim->x2 = screen[0].x - ((s16)screen[1].x >> 1);
  prim->y2 = (screen[0].y - ((s16)screen[1].y >> 1)) + (s16)screen[1].y;
  prim->x3 = (screen[0].x - ((s16)screen[1].x >> 1)) + (s16)screen[1].x;
  prim->y3 = (screen[0].y - ((s16)screen[1].y >> 1)) + (s16)screen[1].y;
  setUVWH(prim, 0xe0, 0x30, 0x1f, 0x1f);
  prim->clut = GetClut(0xa0, 0x1e3);
  prim->tpage = GetTPage(0, 1, 0x2c0, 0x100);

  /* The original re-reads the live ramp byte once per colour channel, so the
   * record is read through its volatile view and the three channel bytes of the
   * POLY_FT4 at +0x04..+0x06 are written in the order the original stores them.
   */
  ((volatile u8*)prim)[4] = record->intensity_03;
  ((volatile u8*)prim)[5] = (record->intensity_03 >> 2) * 3;
  ((volatile u8*)prim)[6] = record->intensity_03 >> 1;

  func_80155A08(record->position_08, record->position_0c, 3, 0x28);
}
