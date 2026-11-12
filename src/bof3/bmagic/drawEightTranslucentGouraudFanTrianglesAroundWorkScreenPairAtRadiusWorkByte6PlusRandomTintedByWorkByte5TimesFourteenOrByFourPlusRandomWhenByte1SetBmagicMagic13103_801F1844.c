#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Work-record pointer cell at 0x801F24E0, the overlay's selected 0x2C-byte
 * bmagic work record. This body takes one pointer load per prelude group and
 * none inside the triangle walk, which reads the published scratchpad cells
 * instead. */
extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */

/* Scratchpad screen-pair cell at 0x1F80000C. The original takes this address
 * into a register instead of the literal address form the two surrounding
 * publications use, because the walk reads it with displacements: minus six
 * halfwords is the signed radius published at 0x1F800000 and minus three the
 * phase halfword at 0x1F800006. This body publishes the pair through one local
 * and then copies that base into the walk's own local. */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */

/* PsyQ random generator: only the low two bits of each result reach the fan,
 * once for the radius and once for the conditional centre shade. */
extern int rand(void);

/* Shared gouraud triangle seed helper, addressed by its own name in this
 * target's generated undefined function table. No in-image symbol names
 * 0x8017A97C, so it keeps its address name. */
extern void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared phase samplers of a 12-bit turn in the fixed-point scale the >> 12
 * products below divide out: 0x801782FC feeds the x vertices and 0x801783C8
 * the y vertices. Neither sample is cached; each pair takes the phase value
 * for the first and the halfword read back from 0x1F800006 for the second. */
extern s32 func_801782FC(s32 arg0); /* @source 0x801782FC */
extern s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

/* @source 0x801F1844
 * @behavior Battle-magic drawing step. Its only in-image routes are the record
 *           state handlers at 0x801F1138 and 0x801F14A0, which call it with no
 *           arguments when record bytes 0x0 and 0x2 are both non-zero, so it
 *           reads no incoming register and returns nothing. It takes the work
 *           record pointer from the cell at 0x801F24E0 and publishes the fan
 *           radius to scratchpad halfword 0x1F800000 as the record unsigned
 *           byte 0x6 plus the low two bits of a rand result, then publishes the
 *           record screen pair at halfwords 0x20 and 0x22 into scratchpad
 *           halfwords 0x1F80000C and 0x1F80000E. The first vertex shade is the
 *           record unsigned byte 0x5 times fourteen when record byte 0x1 is
 *           zero, and byte 0x5 times four plus the low two bits of a second rand
 *           result otherwise. It then emits eight semi-transparent gouraud
 *           POLY_G3 triangles, one per 0x200 step of the 0x1000 turn, so the
 *           wedges close a full disc: each packet is seeded by the shared helper
 *           at 0x8017A97C, takes the published screen pair as its first vertex
 *           with the shade in all three channels, and builds its second and
 *           third vertices from the 0x801782FC and 0x801783C8 samples of the
 *           current and next phase scaled by the signed radius halfword and
 *           shifted right twelve. Those two vertices take one in all three
 *           channels, and every packet is linked at ordering slot 2 with size
 *           0x1C. Each phase is published to scratchpad halfword 0x1F800006
 *           before the 0x801782FC sample and read back as a signed halfword for
 *           the 0x801783C8 sample of the same pair. The body keeps a 0x30-byte
 *           frame for $ra and six saved registers and writes nothing outside the
 *           scratchpad and the primitives it emits. Record bytes 0x1, 0x5 and
 *           0x6 keep offset-based roles.
 * @status partial
 * @match 95.49
 * @residual 127 of 133 original instructions match at the original 532-byte
 *           size. The one remaining site is the centre-publication block: the
 *           payload forms &D_1F80000C in a caller-saved register, copies it
 *           into the callee-saved loop base and publishes the screen pair
 *           through that copy, while this body publishes through the temporary
 *           base and copies it into the loop base after the store group, which
 *           also moves the rim constant one statement later. Probes that kept
 *           the publication through the loop base itself removed the copy and
 *           the four bytes it occupies (123 of 133), the copy written before
 *           the stores propagated away (123), a byte-typed base cast to the
 *           halfword view dropped it as well (123), and a duplicated counter
 *           initialization in both arms put the zeroing move in the untaken
 *           branch's delay slot instead of the call's (120 at the original
 *           size). Swapping the two pointer declarations and the literal
 *           scratchpad spellings left this measurement unchanged.
 */
void drawEightTranslucentGouraudFanTrianglesAroundWorkScreenPairAtRadiusWorkByte6PlusRandomTintedByWorkByte5TimesFourteenOrByFourPlusRandomWhenByte1SetBmagicMagic13103_801F1844(
    void) {
  u16 *cells;
  u16 *cell;
  POLY_G3 *prim;
  u8 i;
  s32 shade;
  s32 rim;
  s32 phase;
  s32 next;

  *(u16 *)0x1F800000 = (rand() & 3) + workRecord801F24E0[6];
  if (workRecord801F24E0[1] != 0) {
    shade = ((rand() & 3) + 4) * workRecord801F24E0[5];
  } else {
    shade = workRecord801F24E0[5] * 14;
  }

  i = 0;
  cells = &D_1F80000C;
  cells[0] = *(u16 *)(workRecord801F24E0 + 0x20);
  *(u16 *)0x1F80000E = *(u16 *)(workRecord801F24E0 + 0x22);
  rim = 1;
  cell = cells;

  for (; i < 8; i++) {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = cell[0];
    prim->y0 = cell[1];
    phase = (i << 9) & 0xFFF;
    *(s16 *)(cell - 3) = phase;
    prim->x1 = cell[0] + ((func_801782FC(phase) * *(s16 *)(cell - 6)) >> 12);
    prim->y1 =
        cell[1] + ((func_801783C8(*(s16 *)(cell - 3)) * *(s16 *)(cell - 6)) >> 12);
    next = ((i + 1) << 9) & 0xFFF;
    *(s16 *)(cell - 3) = next;
    prim->x2 = cell[0] + ((func_801782FC(next) * *(s16 *)(cell - 6)) >> 12);
    prim->y2 =
        cell[1] + ((func_801783C8(*(s16 *)(cell - 3)) * *(s16 *)(cell - 6)) >> 12);

    prim->r0 = shade;
    prim->g0 = shade;
    prim->b0 = shade;
    prim->r1 = rim;
    prim->g1 = rim;
    prim->b1 = rim;
    prim->r2 = rim;
    prim->g2 = rim;
    prim->b2 = rim;

    GpuAppendPrim(2, 0x1C);
  }
}
