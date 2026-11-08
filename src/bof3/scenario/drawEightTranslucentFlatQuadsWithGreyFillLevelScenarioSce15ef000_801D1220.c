#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

void func_8014E5A0(u8 arg0, u8 arg1);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A9A4(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801D1220
 * @behavior Draws eight semi-transparent flat quads, the overlay's whole-screen
 * grey fill: it runs one 0x18-byte POLY_F4 emission eight times and keeps a 0x68
 * byte frame with $ra at 0x64, $s0-$s4 at 0x50-0x60 and the one outgoing stack
 * argument of SetDrawMode at 0x10. Before the loop it seeds two 16-byte VECTOR
 * locals at sp+0x18 and sp+0x28 with fixed fixed-point coordinates, (0x2C8000,
 * 0x718000, 0) and (0x2C8000, 0x718000, 0xFD000000), so the two points share x
 * and y and are 0x3000000 apart in z only. Each iteration takes the texture page
 * from GetTPage(0, 1, 0x3C0, 0) and writes it into a draw-mode packet at
 * g_PrimCursor through SetDrawMode with dfe 0, dtd 1 and no texture window,
 * links that 0xC-byte packet at the first point's x and y through the shared
 * grid-cell linker func_80155A08 with kind 0, re-seeds the cursor as a flat quad
 * through func_8017A9A4, sets that quad semi-transparent, projects the two points
 * into its first corner pair and its second corner pair through the shared
 * projector func_801AFF04, advances both points by 0x10000 in x and projects them
 * again into its third and fourth corner pairs, so a quad spans one 0x10000 step
 * of x between the near and far point and the next iteration starts at the x the
 * previous one ended on. The quad then takes the caller's level byte as its r0,
 * g0 and b0, which is a grey fill, and is linked at OT slot 1 with the 0x18-byte
 * POLY_F4 size through func_8014E5A0.
 *
 * Evidence: the overlay's step-3 handler at 0x801D0E5C and step-4 handler at
 * 0x801D0F44 are the only two in-payload `jal` sites of this address (decoded
 * from the original bytes at 0x801D0EA4 and 0x801D0F74), and both hand the work
 * object's unsigned byte at +0x5E to it as the fill level, the second after
 * subtracting 0x40 and the first substituting 0xFF on wrap, which fixes the
 * argument as a grey level and its `sb $s2, 0x4/0x5/0x6($s0)` triple as the r0,
 * g0 and b0 bytes of the POLY_F4 at the cursor. The four 4-byte writes at
 * cursor offsets +0x08, +0x0C, +0x10 and +0x14 are the quad's four short corner
 * pairs and not a textured packet's u/v fields, because the same cursor takes
 * only the +0x04/+0x05/+0x06 colour bytes and is linked with size 0x18. The two
 * `lw`/`addu`/`sw` triples between the second and third projection are the
 * in-place x advance of both points, and the loop tail's `andi 0xFF` plus
 * `sltiu 8` is the unsigned byte counter of a do-while. Its own callees, its
 * g_PrimCursor reference and the 0x80155A08 row all resolve in the shared main
 * executable at the same addresses every scenario overlay reaches, so no payload
 * local of this span is bound. The 12 other payloads that hold this body's first
 * three instructions (a shared 0x68-byte prologue) differ from this span in 80
 * or 81 of its 84 words, so the clone lever has no byte-identical sibling and
 * the body was seeded from clean C. The range has a prologue, a `jr $ra` return
 * path and a loop, and the target's own data-candidate report ranks only the
 * 0x801D1FA8 numeric table, so the data lever is closed; its single declared
 * `asm` row starts at this entry and ends at the next declared entry 0x801D1370,
 * so the carve lever has no fragment boundary to regroup.
 *
 * Frame: the original's 0x68-byte frame carries a 0x18-byte local area the body
 * never touches, between the two points that end at sp+0x37 and the saved group
 * at sp+0x50. The declared `POLY_F4 scratch` reproduces exactly that reserved
 * size and no use, which is the orphan declared-local lever; the bytes constrain
 * only the size, so its name and type are a reconstruction choice flagged for
 * review, and removing it leaves this body with a 0x50-byte frame and every
 * saved-register and local offset shifted down by 0x18.
 * @status partial
 * @match 97.62
 * @residual 82 of 84 instructions match at the original 336 bytes with frame,
 * saved group, S register numbering, point and colour store order and loop shape
 * all reproduced. The two differing slots are the texture page's zero extension
 * `andi $a3,$v0,0xFFFF` and the loop counter's `addiu $s1,$s1,1`: the original
 * keeps both in the argument-setup slots before the SetDrawMode call, while this
 * candidate places the mask after the `lui`/`lw` pair that reloads g_PrimCursor
 * and the increment after the next cursor reload, so the pair is permuted by one
 * slot each. Statement-order probes (increment first, increment in the condition,
 * for-loop header, block-scope and function-scope u16/s32 texture page locals,
 * explicit mask, byte-typed cursor with offsets, array and scalar locals of the
 * same reserved size, wrapped and interleaved point initialisers) all measured
 * the same two slots, so the residue is a scheduler tie in one argument-setup
 * window rather than a recovered declaration.
 */
void drawEightTranslucentFlatQuadsWithGreyFillLevelScenarioSce15ef000_801D1220(
    u8 level) {
  VECTOR near_point;
  VECTOR far_point;
  POLY_F4* prim;
  u8 i;
  POLY_F4 scratch;

  near_point.vx = far_point.vx = 0x2C8000;
  near_point.vy = far_point.vy = 0x718000;
  near_point.vz = 0;
  far_point.vz = 0xFD000000;
  i = 0;
  do {
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3C0, 0), 0);
    func_80155A08(near_point.vx, near_point.vy, 0, 0xC);
    prim = (POLY_F4*)g_PrimCursor;
    func_8017A9A4(prim);
    SetSemiTrans(prim, 1);
    func_801AFF04(&near_point, &prim->x0);
    func_801AFF04(&far_point, &prim->x1);
    near_point.vx += 0x10000;
    far_point.vx += 0x10000;
    func_801AFF04(&near_point, &prim->x2);
    func_801AFF04(&far_point, &prim->x3);
    prim->r0 = level;
    prim->g0 = level;
    prim->b0 = level;
    func_8014E5A0(1, 0x18);
    i++;
  } while (i < 8);
}
