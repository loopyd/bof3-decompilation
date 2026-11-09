#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Scratchpad work-object pointer cell at 0x1F800044. The original
 * re-materialises the cell with its own lui/lw pair at every use, including
 * once per sampled field inside both loops, so the cell is bound by name and
 * reloaded rather than kept live across the sampler calls. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The scratchpad halfword block at 0x1F800000 that this body works in: eight
 * cells at 0x00 through 0x0E holding the dash radius at 0x00, the published
 * phase at 0x02, the fixed angle at 0x04, the shade at 0x06, and the walking
 * centre at 0x08 and its inner end at 0x0C. Every access the original makes to
 * these cells is a halfword load or store, and the 0x00 and 0x04 cells are the
 * two it reads signed, so they carry s16 and the rest u16. The cells are
 * volatile because the original reloads them: the shade cell is read three
 * times for the three colour bytes with no call in between, the radius cell is
 * read back after its own store, and the 0x02 cell is stored and never read,
 * which a non-volatile spelling drops from the body entirely. */
typedef struct {
  s16 m00;
  u16 m02;
  s16 m04;
  u16 m06;
  u16 m08;
  u16 m0A;
  u16 m0C;
  u16 m0E;
} Magic12103Scratchpad;

typedef struct {
  u16 x;
  u16 y;
} Magic12103Pair;

/* The dash end pair at 0x1F800014 and 0x1F800016 and the walk anchor pair at
 * 0x1F80001C and 0x1F80001E are reached through their own bases in the original,
 * each held in a saved register across the sampling calls, so they are pair
 * objects of their own rather than members of the block above. */

extern volatile Magic12103Scratchpad D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the >> 12 products below. This body takes the
 * 0x801782FC sample for every x offset and the 0x801783C8 sample for every y
 * offset. */
s32 func_801782FC(s32 angle);
s32 func_801783C8(s32 angle);

/* Shared flat-line primitive stamper, called on the fresh cursor primitive
 * before this body writes the line ends and the colour bytes. It takes the
 * packet pointer and returns nothing. */
void func_8017AA80(void *primitive);

/* @source 0x801F06E8
 * @behavior Draw routine of the MAGIC121 work object, reached by the three
 *   calls at 0x801EEFDC, 0x801EEFFC and 0x801EF01C inside this overlay's
 *   per-frame function func_801EEEAC, each with one work byte as the count:
 *   4 - work[0x5D], work[0x5F] + 8 and 0xC - work[0x5E].
 *   It picks the draw-mode tpage value by battle kind (0xA5 for kinds 1 and 2,
 *   0x35 otherwise), queues a 0xC-byte draw-mode packet with
 *   SetDrawMode(g_PrimCursor, 0, 1, tpage, 0) and GpuAppendPrim(3, 0xC), and
 *   copies the work object halfwords 0x2E and 0x30 into the scratchpad centre
 *   pair 0x1F800008 and 0x1F80000A. It then walks that centre by (count + 1)
 *   steps of the work word 0x14, each step 12 fixed-point units on both axes,
 *   so the count scales the pre-drift of the whole effect.
 *   The fixed angle of every dash is (work word 0x14 - 0x400) & 0xFFF, which it
 *   publishes into 0x1F800004: the inner end 0x1F80000C and 0x1F80000E is the
 *   centre offset 0x20 units along that angle, the base radius 0x1F800000 is
 *   the 0x801782FC sample of phase zero shifted right 8, and the first outer
 *   end 0x1F800014 and 0x1F800016 is the inner end plus that radius along the
 *   same angle. The shade 0x1F800006 is ((rand() & 7) + 5) times work byte 0x04.
 *   It then draws 47 semi-transparent 0x10-byte LINE_F2 dashes, the counter
 *   running from 1 while it stays below 0x30. Each iteration stamps the cursor
 *   primitive, takes a 0..7 random walk step of the work angle 0x14 on the
 *   centre, or, once the walk has accumulated 0xC, re-seats the centre 12 units
 *   along that angle from the walk anchor 0x1F80001C and 0x1F80001E and moves
 *   the anchor there. It re-derives the inner end from the centre along the
 *   fixed angle, publishes (counter & 0x1F) << 6 into 0x1F800002, takes the
 *   0x801782FC sample of that phase times (rand() & 3) + 0x10 as the signed
 *   dash radius, draws the dash from the previous outer end to the new outer
 *   end - the inner end plus the radius along the fixed angle - and writes the
 *   shade's low byte twice and the shade divided by three once as the colour
 *   bytes. The 0x48-byte frame keeps ra, all eight saved registers and the
 *   fifth SetDrawMode argument at 0x10.
 * @status partial
 * @match 60.17
 * @residual 213/354 instructions at 1416 bytes against the original 1360.
 *   First difference is the saved-register group at body offset 0x0C: the
 *   walk sum occupies s7 instead of s5. The pre-loop centre uses the block
 *   base rather than 0x1F800008, and signed field reads retain lhu followed
 *   by sign extension where the original uses lh. Pair pointers are
 *   initialized immediately before their first use; every read follows a
 *   valid initialization. Register allocation and access scheduling remain
 *   unresolved; byte equality has not been established.
 */
void drawFortySevenSemiTransDashesAlongPerpendicularWorkAngleFromDriftingScreenPairTintedByWorkByte4ShadeBmagicMagic12103_801F06E8(
    u32 arg0) {
  volatile Magic12103Pair *anchor;
  volatile Magic12103Pair *outer;
  LINE_F2 *primitive;
  u8 *work;
  s32 angle;
  s32 count;
  s32 i;
  s32 jitter;
  s32 radius;
  s32 sum;
  s32 tpage;

  sum = 0;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(3, 0xC);

  work = g_battle_work;
  D_1F800000.m08 = FIELD_REF(u16, work, 0x2E);
  D_1F800000.m0A = FIELD_REF(u16, work, 0x30);

  count = (s32)(arg0 & 0xFF) + 1;
  for (i = 0; i < count; i++) {
    D_1F800000.m08 = D_1F800000.m08 + ((func_801782FC(FIELD_REF(u32, g_battle_work, 0x14u)) * 12) >> 12);
    D_1F800000.m0A = D_1F800000.m0A + ((func_801783C8(FIELD_REF(u32, g_battle_work, 0x14u)) * 12) >> 12);
  }

  anchor = SPAD_ADDR(volatile Magic12103Pair, 0x1Cu);
  anchor->x = D_1F800000.m08;
  anchor->y = D_1F800000.m0A;
  angle = (FIELD_REF(s32, g_battle_work, 0x14u) - 0x400) & 0xFFF;
  D_1F800000.m04 = angle;
  D_1F800000.m0C = D_1F800000.m08 + ((func_801782FC(angle) * 0x20) >> 12);
  D_1F800000.m0E = D_1F800000.m0A + ((func_801783C8(D_1F800000.m04) * 0x20) >> 12);
  D_1F800000.m00 = (u16)((u32)func_801782FC(0) >> 8);
  outer = SPAD_ADDR(volatile Magic12103Pair, 0x14u);
  outer->x = D_1F800000.m0C + ((func_801782FC(D_1F800000.m04) * D_1F800000.m00) >> 12);
  outer->y = D_1F800000.m0E + ((func_801783C8(D_1F800000.m04) * D_1F800000.m00) >> 12);
  D_1F800000.m06 = (u16)(((rand() & 7) + 5) * FIELD_REF(u8, g_battle_work, 0x04u));

  i = 1;
  do {
    primitive = (LINE_F2 *)g_PrimCursor;
    func_8017AA80(primitive);
    SetSemiTrans(primitive, 1);
    jitter = rand() & 7;
    sum += jitter;
    D_1F800000.m00 = jitter;
    if (sum < 0xC) {
      D_1F800000.m08 = D_1F800000.m08 + ((func_801782FC(FIELD_REF(u32, g_battle_work, 0x14u)) * D_1F800000.m00) >> 12);
      D_1F800000.m0A = D_1F800000.m0A + ((func_801783C8(FIELD_REF(u32, g_battle_work, 0x14u)) * D_1F800000.m00) >> 12);
    } else {
      sum = 0;
      D_1F800000.m08 = anchor->x + ((func_801782FC(FIELD_REF(u32, g_battle_work, 0x14u)) * 12) >> 12);
      D_1F800000.m0A = anchor->y + ((func_801783C8(FIELD_REF(u32, g_battle_work, 0x14u)) * 12) >> 12);
      anchor->x = D_1F800000.m08;
      anchor->y = D_1F800000.m0A;
    }
    D_1F800000.m0C = D_1F800000.m08 + ((func_801782FC(D_1F800000.m04) * 0x20) >> 12);
    D_1F800000.m0E = D_1F800000.m0A + ((func_801783C8(D_1F800000.m04) * 0x20) >> 12);
    D_1F800000.m02 = (u16)((i & 0x1F) << 6);
    radius = (func_801782FC((i & 0x1F) << 6) * ((rand() & 3) + 0x10)) >> 12;
    D_1F800000.m00 = radius;
    primitive->x0 = outer->x;
    primitive->y0 = outer->y;
    outer->x = D_1F800000.m0C + ((func_801782FC(D_1F800000.m04) * D_1F800000.m00) >> 12);
    primitive->x1 = outer->x;
    outer->y = D_1F800000.m0E + ((func_801783C8(D_1F800000.m04) * D_1F800000.m00) >> 12);
    primitive->y1 = outer->y;
    primitive->r0 = (u8)D_1F800000.m06;
    primitive->g0 = (u8)D_1F800000.m06;
    primitive->b0 = (u8)((s16)D_1F800000.m06 / 3);
    GpuAppendPrim(3, 0x10);
    i++;
  } while (i < 0x30);
}
