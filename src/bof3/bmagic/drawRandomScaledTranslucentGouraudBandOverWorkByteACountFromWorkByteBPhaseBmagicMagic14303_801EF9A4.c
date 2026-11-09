#include "bof3/bof3.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's map
 * and shared by the overlay's dispatchers and handlers. The original
 * re-materialises the cell with its own lui/lw pair at each use because the
 * intervening calls may republish it, so the plain named cell suffices. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad band block this step owns. The first word is the radial scale the
 * sample helper's result is multiplied by, the second the phase handed to that
 * helper, the third the random shade the two vertex colours start from and the
 * fourth the number of primitives the loop emits. The halfwords at 0x1F800014
 * hold the sample the two projections consume. */
extern volatile s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern volatile s32 D_1F800004; /* @source 0x1F800004 @kind data */
extern volatile s32 D_1F800008; /* @source 0x1F800008 @kind data */
extern volatile s32 D_1F80000C; /* @source 0x1F80000C @kind data */
extern SVECTOR D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared main-executable helper that queues the primitive the shared cursor
 * holds; it takes the band origin's x and y, the output list and the primitive
 * size. Its arguments are the only reason this step reads the work record's
 * word fields 0x34 and 0x38. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Shared radial sample helper: turns the scaled phase into the amplitude the
 * band's y halfword keeps. */
s32 func_801782FC(s32 arg0);

/* Shared primitive header helper: stamps the code, length and tpage words of
 * the primitive the shared cursor holds. */
void func_8017AA94(void *arg0);

/* @source 0x801EF9A4
 * @behavior The argument-free band step of this overlay's magic effect. It
 *           takes no arguments and reads the work record the scratchpad cell at
 *           0x1F800044 publishes. The step seeds its own scratchpad band block:
 *           0x80 as the radial scale at 0x1F800000, the record's low nibble of
 *           byte 0xB plus two shifted up eight bits as the phase at 0x1F800004,
 *           the record's byte 0xA as the primitive count at 0x1F80000C and
 *           rand() & 0x3F as the shade at 0x1F800008; it then samples the first
 *           radial vertex at 0x1F800014 (x zero, z zero, y the sample helper's
 *           result times the scale shifted right twelve) before the loop.
 *
 *           Each iteration first derives the band origin from the sample it is
 *           about to project: the previous vertex's x and y, each sign-extended
 *           from the unsigned halfword and shifted right three, plus the work
 *           record's word fields 0x34 and 0x38. It picks the draw-mode tpage
 *           from GetGraphType (0x2A5 for graphic types 1 and 2, 0xB5 otherwise)
 *           and seeds SetDrawMode on the shared primitive cursor at 0x8014598C
 *           with dfe 0, dtd 1 and a null tw, appends a 0xC-byte primitive for
 *           output list 2 through the helper at 0x80155A08 from that origin,
 *           and stamps the header of the primitive the cursor now holds. It
 *           projects the seeded vertex through RotTransPers into primitive
 *           bytes 0x08, then re-randomises the radial scale to 0xA0 plus or
 *           minus 0..0x1F and re-derives the phase from the record's byte 0xB
 *           plus the index plus two, keeping the shifted phase at 0x1F800004,
 *           zeroing the sample's x and sampling the next vertex (y from the
 *           product of the sample helper's result and the scale shifted right
 *           twelve, z the negated index shifted left six) before projecting it
 *           into primitive bytes 0x10.
 *
 *           It then writes the shade into primitive bytes 0x04/0x05 and the
 *           re-randomised (rand() + 0x40) & 0x3F into bytes 0x0C/0x0D, keeping
 *           0x20 in bytes 0x06/0x0E, appends the 0x14-byte primitive through the
 *           same helper from the origin it derived at the top of the iteration
 *           and repeats while the index is below the count the band block holds.
 *           Every store's opcode fills the delay slot of the call that follows
 *           it, and the 0x40-byte frame keeps $ra and the seven saved registers;
 *           nothing is returned and no other state is read or written.
 *
 *           Shape and naming evidence: this is the same effect as the exact
 *           main-executable sibling func_801D218C in emi/scenario/sce10eff/00,
 *           which seeds the same four scratchpad words, samples the same helper
 *           at 0x801782FC, selects the same 0x2A5/0xB5 tpage over GetGraphType
 *           and stamps the same primitive byte pattern; this overlay's copy
 *           differs in its 0x80/0xA0 scales, its 0xA0 +/- (rand & 0x1F) radius
 *           walk and its origin-taking append through 0x80155A08. Statements,
 *           operand forms and declaration style follow that sibling. A byte scan
 *           of all 122 bmagic payloads finds no copy of this 0x2D0-byte span
 *           (sha256 ce502c918b53c14117d624f8a14c91522ed5c391173eca4c55c68c4e208c18c9),
 *           not even on its first 0x40 bytes, and the sce10eff sibling differs
 *           in constants and callees, so the clone lever was declined and the
 *           body is seeded from the original bytes.
 * @status partial
 * @match 93.89
 * @residual The original reaches the seed store to 0x1F800014 through $s2 and
 *           copies that address into $s3 in the loop-entry branch delay slot.
 *           This build keeps the address in $s3 from the seed store on, so the
 *           four-byte copy is missing (716 of the original 720 bytes) and every
 *           later instruction carries the shifted offset; the remaining body,
 *           including both projections, the tpage selection and the two
 *           appends, is instruction-identical. Fourteen clean-C shapes over the
 *           cell's declaration view (named SVECTOR cell, block-local pointer,
 *           two pointer locals), the seed-store order (before and after the
 *           index and shade seeds), the loop-base derivation (pre-loop,
 *           in-loop, `base + 1` spelling) and the sampled product's local all
 *           measured 169/180 at this same first difference, so the residual is
 *           the entry-block address copy, not statement order.
 */
void drawRandomScaledTranslucentGouraudBandOverWorkByteACountFromWorkByteBPhaseBmagicMagic14303_801EF9A4(
    void) {
  s32* base;
  s32* stage;
  u8*  prim;
  s32  i;
  s32  graph_value;
  s32  count_or_ready;
  s32  rotation_or_product;
  s32  sample_or_scale;
  s32  phase;
  s32  projected;
  s32  depth;
  s32  origin_x;
  s32  origin_y;

  base = (s32*)&D_1F800000;
  *base = 0x80;
  D_1F800004 = (s32)((s32)(((((u8*)g_battle_work)[0x0b]) + 2) & 0x0fu) << 8);
  D_1F80000C = ((u8*)g_battle_work)[0x0a];
  *(s32*)&D_1F800008 = rand() & 0x3f;
  i = 1;
  D_1F800014.vx = 0;
  rotation_or_product = func_801782FC(D_1F800004) * *base;
  D_1F800014.vz = 0;
  stage = (s32*)(base + 1);
  count_or_ready = stage[2];
  count_or_ready = i < count_or_ready;
  sample_or_scale = rotation_or_product >> 12;
  D_1F800014.vy = (s16)sample_or_scale;
  while (count_or_ready) {
    origin_x = ((s32)D_1F800014.vx >> 3) + FIELD_REF(s32, g_battle_work, 0x34u);
    origin_y = ((s32)D_1F800014.vy >> 3) + FIELD_REF(s32, g_battle_work, 0x38u);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      graph_value = 0x2a5;
    } else {
      graph_value = 0xb5;
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, graph_value, 0);
    func_80155A08(origin_x, origin_y, 2, 0x0c);

    prim = g_PrimCursor;
    func_8017AA94(prim);
    SetSemiTrans(prim, 1);

    RotTransPers(&D_1F800014, (long*)(prim + 8), (long*)&projected,
                 (long*)&depth);

    if (rand() & 1) {
      D_1F800000 = (rand() & 0x1f) + 0xa0;
    } else {
      D_1F800000 = 0xa0 - (rand() & 0x1f);
    }

    phase = (s32)((s32)(((((u8*)g_battle_work)[0x0b]) + i + 2) & 0x0fu) << 8);
    D_1F800014.vx = 0;
    stage[0] = phase;
    rotation_or_product = func_801782FC(phase);
    sample_or_scale = stage[-1];
    D_1F800014.vy = (s16)((rotation_or_product * sample_or_scale) >> 12);
    D_1F800014.vz = (s16)(-(i << 6));

    RotTransPers(&D_1F800014, (long*)(prim + 0x10), (long*)&projected,
                 (long*)&depth);

    prim[4] = stage[1];
    prim[5] = stage[1];
    prim[6] = 0x20;

    stage[1] = (rand() + 0x40) & 0x3f;
    prim[0xc] = stage[1];
    prim[0xd] = stage[1];
    prim[0xe] = 0x20;

    func_80155A08(origin_x, origin_y, 2, 0x14);

    i++;
    count_or_ready = stage[2];
    count_or_ready = i < count_or_ready;
  }
}
