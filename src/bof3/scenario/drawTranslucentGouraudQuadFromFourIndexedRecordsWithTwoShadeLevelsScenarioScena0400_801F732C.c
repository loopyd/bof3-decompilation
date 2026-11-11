#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Overlay-local vertex table the quad stages read: the eight 16-byte records at
 * 0x801FA240 that the drawing entry at 0x801F7084 seeds with the corners of the
 * two points it is handed, each record holding a 16.16 point in its first three
 * words. The record index this body receives is multiplied by 16, so a record
 * is one table element. */
extern u8 D_801FA240[];

/* Shared frame counter in the main executable: this body reads its low bit to
 * offset the quad's colour bytes. */
extern s32 D_80143E6C;

/* Resident projector shared by this overlay's scenario and world drawing
 * bodies: it reads the three-word point it is handed and writes the projected
 * screen pair to its second argument. */
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801F732C
 * @behavior Emits one translucent Gouraud quad at the packet published by
 *           g_PrimCursor: SetPolyG4 stamps the 0x24-byte POLY_G4 there and
 *           SetSemiTrans marks it semi-transparent with abe 1, then the four
 *           index arguments select records of this overlay's vertex table at
 *           0x801FA240 by `(index & 0xFF) << 4` and func_801AFF04 projects each
 *           selected record's three-word point into the quad's corner pairs at
 *           cursor +0x08, +0x10, +0x18 and +0x20 in argument order.
 *           The two shade levels arrive on the stack as the fifth and sixth
 *           arguments. Each is shifted left by seven and the low bit of the
 *           shared frame counter D_80143E6C is folded in at bit 2, giving the
 *           value this body stores in the r and g bytes of the first two
 *           vertices for the first level and of the last two for the second;
 *           the b byte of every one of those four vertices takes the low byte
 *           of that same value shifted right by one. So the first level lights
 *           the quad's first edge and the second lights its second edge, and a
 *           level of zero leaves that edge black apart from the counter's bit.
 *           The finished packet is linked under OT slot 1 with GpuAppendPrim(1,
 *           0x24). It takes six arguments, returns no value, and its 0x38-byte
 *           frame holds $s0-$s7 plus $ra.
 *
 * Evidence: the only `jal` sites of 0x801F732C in this payload are the five at
 * 0x801F72A0, 0x801F72BC, 0x801F72D8, 0x801F72F4 and 0x801F7310, all inside
 * the exact drawing entry at 0x801F7084, which seeds the eight 16-byte records
 * of 0x801FA240 from its own point record and hands this body the tuples
 * (0, 1, 2, 3), (0, 1, 4, 5), (2, 0, 6, 4), (1, 3, 5, 7) and (2, 3, 6, 7)
 * with the level pairs 1/1, 1/0, 1/0, 1/0 and 1/0; no word of this payload
 * holds 0x801F732C as data, so those five calls are its only route. That
 * caller indexes records 0..7, which fixes the `<< 4` as the 16-byte record
 * stride of that table and the four masked arguments as record indices.
 * The packet layout is this body's own store schedule: an r/g/b triple at
 * cursor +0x04, +0x0C, +0x14 and +0x1C around the projected corner pairs at
 * +0x08, +0x10, +0x18 and +0x20, four 8-byte vertices after the tag, which is
 * the POLY_G4 layout of <libgpu.h> and the size 0x24 the link call passes.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Levers: the data lever is declined because the span opens with
 * `addiu $sp, $sp, -0x38` and ends with `jr $ra` and holds its own six `jal`
 * sites; the carve lever is declined because the span is one declared row with
 * its own prologue and return path from 0x801F732C to the next declared entry
 * 0x801F747C; the clone lever is declined because a scan of the 396 payloads
 * under out/binaries finds this 0x150-byte span only in its own payload, and
 * none at all once every `lui`, `jal` and `j` word and the three `%lo`
 * immediates of its local cells are wildcarded.
 *
 * Frame: the 0x38-byte frame holds $s0 at sp+0x10, $s1 at sp+0x14, $s2 at
 * sp+0x18, $s3 at sp+0x1C, $s4 at sp+0x20, $s5 at sp+0x24, $s6 at sp+0x28, $s7
 * at sp+0x2C and $ra at sp+0x30, and needs no outgoing argument slot because
 * its widest call passes two registers.
 */
void drawTranslucentGouraudQuadFromFourIndexedRecordsWithTwoShadeLevelsScenarioScena0400_801F732C(
    u8 first, u8 second, u8 third, u8 fourth, s32 level0, s32 level1) {
  u8* prim;
  u8 shade0;
  u8 shade1;
  s32 cycle;

  prim = g_PrimCursor;
  SetPolyG4((POLY_G4*)prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(D_801FA240 + (first << 4), prim + 8);
  func_801AFF04(D_801FA240 + (second << 4), prim + 0x10);
  func_801AFF04(D_801FA240 + (third << 4), prim + 0x18);
  func_801AFF04(D_801FA240 + (fourth << 4), prim + 0x20);
  cycle = (D_80143E6C & 1) << 2;
  shade0 = level0 << 7;
  shade0 |= cycle;
  shade1 = level1 << 7;
  shade1 |= cycle;
  prim[4] = shade0;
  prim[5] = shade0;
  prim[6] = (shade0 & 0xFF) >> 1;
  prim[0xC] = shade0;
  prim[0xD] = shade0;
  prim[0xE] = (shade0 & 0xFF) >> 1;
  prim[0x14] = shade1;
  prim[0x15] = shade1;
  prim[0x16] = (shade1 & 0xFF) >> 1;
  prim[0x1C] = shade1;
  prim[0x1D] = shade1;
  prim[0x1E] = (shade1 & 0xFF) >> 1;
  GpuAppendPrim(1, 0x24);
}
