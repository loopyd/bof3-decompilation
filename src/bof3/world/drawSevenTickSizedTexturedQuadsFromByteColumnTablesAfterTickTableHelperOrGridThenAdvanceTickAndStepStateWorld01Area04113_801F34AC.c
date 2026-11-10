#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The scratchpad vector the loop fills before each projection. The body keeps
 * its base in a saved register and writes the three halfword lanes through it. */
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind data */

/* The scratchpad screen pair RotTransPers fills: the projected x at
 * 0x1F800034 and the projected y at 0x1F800036. The body takes the address of
 * the pair for the projector and reads the two halfwords back by name. Every
 * corner group re-reads the pair after the stores that write the primitive, so
 * both halves stay volatile: the plain spelling let the compiler keep the y
 * halfword live and fold the second read into a register, which is the one
 * spelling this body does not have. */
extern u16 D_1F800034; /* @source 0x1F800034 @kind data */
extern volatile u16 D_1F800036; /* @source 0x1F800036 @kind data */

/* Scratchpad work-record pointer cell. Every store through the record re-reads
 * this cell, because a byte store through the record may alias it. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Per-tick selector table: the seven leading entries are nonzero, the rest
 * zero, and the tick byte at 0x09 of the work record indexes it. */
extern u8 D_801F4334[]; /* @source 0x801F4334 @kind data */

/* The seven-cell column byte tables. Both are walked with a stride of two, so
 * entry k of the pair is 0x801F4344[2k] with 0x801F4345[2k] as its second
 * column. */
extern u8 D_801F4344[]; /* @source 0x801F4344 @kind data */
extern u8 D_801F4345[]; /* @source 0x801F4345 @kind data */

/* The seven-cell signed spread tables, read with the same stride of two. */
extern s8 D_801F4354[]; /* @source 0x801F4354 @kind data */
extern s8 D_801F4355[]; /* @source 0x801F4355 @kind data */

/* Local rotated textured quad band at a turn fraction of six times the
 * argument. Its first act is to negate the argument and store it as the band's
 * half turn at frame offset 0x32. */
extern void func_801F387C(s32 arg0); /* @source 0x801F387C */

/* Local three-by-two textured grid drawer this overlay already lifts. */
extern void
drawThreeByTwoTexturedQuadGridFromByteColumnsWithCellCommandTableWorld01Area04113_801F3704(
    void); /* @source 0x801F3704 */

void func_80155560(u32 arg0, void* arg1, s32 arg2);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F34AC
 * @behavior Per-frame overlay draw and step: reads the tick byte at 0x09 of the
 *           work record published at the scratchpad pointer cell 0x1F800044
 *           and indexes the local selector table 0x801F4334 with it. A nonzero
 *           entry calls the local rotated quad helper 0x801F387C with that byte
 *           shifted left four; a zero entry calls the local three-by-two
 *           textured grid drawer 0x801F3704. Either way it then draws seven
 *           textured quads over one loop whose index steps by two: each step
 *           seeds a POLY_FT4 at the shared primitive cursor g_PrimCursor with
 *           SetPolyFT4 and SetShadeTex off, fills the scratchpad vector at
 *           0x1F800014 with the record position words at 0x34 and 0x38 plus the
 *           cell byte from 0x801F4344 (x) or 0x801F4345 (y) times 128 plus the
 *           signed spread byte from 0x801F4354 (x) or 0x801F4355 (y) times the
 *           tick byte times eight, takes the record depth halfword at 0x3E as z
 *           and projects the vector with RotTransPers into the scratchpad
 *           screen pair 0x1F800034/0x1F800036. It writes the primitive corners
 *           x0 = x2 = projected x minus the tick byte, y0 = y1 = projected y
 *           minus the tick byte, x1 = x3 = projected x plus the tick byte and
 *           y2 = y3 = projected y plus the tick byte, hands the primitive to
 *           0x80155560 with 0xBA009124 plus (0x0D minus the tick byte) shifted
 *           left nineteen and mode 1, and queues it through 0x80155A08 with
 *           both record position words raised by 0x4000 and shifted left nine,
 *           selector 6 and size 0x28. After the loop it increments the tick
 *           byte and, once that byte reaches 0x0E, bumps the record state byte
 *           at 0x02, the handler index the overlay's state-byte-2 dispatcher
 *           uses. Takes no arguments and returns nothing; the 0x30-byte frame
 *           keeps $ra, $s3 through $s0 and the two projector words RotTransPers
 *           writes.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 150/150 instructions, 600 bytes. Every access to the work
 * record re-reads the pointer cell through its own local, because the byte
 * stores through the record may alias the cell; folding those re-reads into
 * fewer locals left the head and tail of the loop in different registers,
 * 123/150, while one local per re-read measured exact.
 */
void drawSevenTickSizedTexturedQuadsFromByteColumnTablesAfterTickTableHelperOrGridThenAdvanceTickAndStepStateWorld01Area04113_801F34AC(
    void) {
  SVECTOR* vec;
  POLY_FT4* prim;
  long* sxy;
  u8* work;
  u8* step;
  u8* check;
  u8* queue;
  u8* tick;
  s32 i;
  u8 angle;
  long p;
  long flag;

  angle = D_801F4334[D_1F800044[9]];
  if (angle != 0) {
    func_801F387C(angle << 4);
  } else {
    drawThreeByTwoTexturedQuadGridFromByteColumnsWithCellCommandTableWorld01Area04113_801F3704();
  }
  vec = D_1F800014;
  sxy = (long*)&D_1F800034;
  for (i = 0; i < 0xE; i += 2) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    work = D_1F800044;
    vec[0].vx = *(s32*)(work + 0x34) + D_801F4344[i] * 128 +
                (D_801F4354[i] * work[9]) * 8;
    vec[0].vy = *(s32*)(work + 0x38) + D_801F4345[i] * 128 +
                (D_801F4355[i] * work[9]) * 8;
    vec[0].vz = *(u16*)(work + 0x3E);
    RotTransPers(vec, sxy, &p, &flag);
    step = D_1F800044;
    prim->x0 = prim->x2 = *(u16*)sxy - step[9];
    prim->y0 = prim->y1 = D_1F800036 - step[9];
    prim->x1 = prim->x3 = *(u16*)sxy + step[9];
    prim->y2 = prim->y3 = D_1F800036 + step[9];
    func_80155560(0xBA009124 + ((0xD - step[9]) << 19), (void*)prim, 1);
    queue = D_1F800044;
    func_80155A08((*(s32*)(queue + 0x34) + 0x4000) << 9,
                  (*(s32*)(queue + 0x38) + 0x4000) << 9, 6, 0x28);
  }
  tick = D_1F800044;
  tick[9] = tick[9] + 1;
  check = D_1F800044;
  if (check[9] >= 0xE) {
    check[2] = check[2] + 1;
  }
}
