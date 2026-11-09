#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
extern s32 D_80143E6C;   /* shared frame counter */

void func_8014E5A0(u8 arg0, u8 arg1);
void func_8017AA30(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801D1C1C
 * @behavior Draws the overlay's tinted trail between the caller's work position
 * and its target: a pair of overlapping Gouraud quads plus a run of eight thin
 * segments. It first arms the draw mode with the texture page
 * GetTPage(0, 1, 0x2C0, 0x100), dfe 0 and dtd 1, submits that 0xC-byte
 * draw-mode packet under OT slot 2, then builds two 0x24-byte POLY_G4 at the
 * primitive cursor cell 0x8014598C. Both quads are stamped with SetPolyG4,
 * marked semi-transparent through SetSemiTrans with one, and take their four
 * corner pairs from the shared projector func_801AFF04 run over the caller's
 * two fixed-point points. The first quad offsets the first point's projected
 * pair by (+1, +2) into vertices 0 and 1 and the second point's by (-1, -2) into
 * vertices 2 and 3 while keeping the unoffset pairs in vertices 1 and 3, and the
 * second quad swaps the offset and unoffset pairs, so each quad spans the same
 * one-pixel band from the opposite side. The first quad's bright vertices 1 and
 * 3 carry the caller's level in the red and green channels and the second quad's
 * bright vertices 0 and 2 carry it in green only, so the two quads alternate a
 * yellow and a green edge along the shared axis. The body then derives the
 * per-step offset (target minus position, arithmetic shift right six) and seeds
 * a moving point at the caller position advanced by that offset times the low
 * three bits of the shared frame counter at 0x80143E6C, which phases the trail
 * against the animation frame. It walks up to eight times: as soon as the moving
 * point's y is below the target y the walk ends, otherwise it takes the cursor,
 * prepares the 0xC-class segment through func_8017AA30, marks it
 * semi-transparent, projects the moving point through func_801AFF04 into the
 * segment's halfwords +0x08 and +0x0A (the y taken one less), paints its colour
 * bytes black, level and black, submits it under OT slot 2 and advances the
 * moving point by the offset times eight, so the eight segments cover the span
 * from the phase offset to the target. Takes the work position pointer, the
 * target pointer and the tint level, returns nothing, and keeps a 0x58-byte
 * frame with $ra at 0x54, $s0-$s4 at 0x40-0x50 and the one outgoing stack
 * argument of SetDrawMode at 0x10.
 *
 * Evidence: the six in-payload `jal` sites of this address are 0x801D0D28,
 * 0x801D0DC8, 0x801D0EC0, 0x801D0F90, 0x801D1034 and 0x801D1124, one in each of
 * this overlay's step handlers from step 1 at 0x801D0CEC to step 6 at
 * 0x801D11B4, and every one of them loads the scratchpad work object through
 * 0x1F800044, passes `work + 0x34` in $a0, the work object itself in $a1 and the
 * unsigned byte at +0x5D in $a2, which fixes both pointer arguments as the work
 * position pair and the third as the tint level whose stores land only in the
 * green or red-and-green channels. The 0x24 sizes and the SetPolyG4/SetSemiTrans
 * pair fix the two packets as POLY_G4 and the x/y stores land on its corner pairs
 * +0x08/+0x0A, +0x10/+0x12, +0x18/+0x1A and +0x20/+0x22 with the colour triples
 * at +0x04, +0x0C, +0x14 and +0x1C; the 0xC sizes fix the draw-mode packet and
 * the eight segments, which take only the +0x04/+0x05/+0x06 colour bytes and the
 * +0x08/+0x0A pair exactly as the sibling lifts
 * drawBlinkingTintedEffectRecordsAdvanceByVelocityTickCountdown9ThenSetProgress43AndAdvanceWorkByte1ScenarioSce15ef100_801D34F0
 * of the neighbouring payload write that same 0xC-class packet. The `lhu` reads
 * of the projected pair fix it as unsigned halfwords; the `lw`/`subu`/`sra 6`
 * differences and the `mult`/`mflo` products fix the two points as 32-bit
 * fixed-point words and the phase as a signed word. The stop test is `slt` of the
 * moving y against the target y followed by a branch to the epilogue, so the walk
 * ends once the moving point drops below the target's y.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Levers: the data lever is declined because the range opens with the
 * `addiu $sp, $sp, -0x58` prologue, ends with `jr $ra` and holds eight `jal`
 * sites; the carve lever is declined because the span is one declared row from
 * this entry to the next declared entry (0x801D1F88) with its own prologue and
 * return path and no shared epilogue; the clone lever is declined because a scan
 * of all 396 payload binaries under out/binaries found this 876-byte span only
 * once, at its own address, so no byte-identical twin exists. The body was
 * seeded directly from the original bytes and the already-lifted siblings of
 * this overlay.
 *
 * Decisive clean-C shape: the eight-step walk must be a counted `for` loop whose
 * stop test is a `break` at the top of the body. Writing it as
 * `do { ... } while (i < 8)` with the same `break` made GCC duplicate the stop
 * test at the back edge and measured 203 of 224 instructions at 896 bytes, first
 * difference 0x801D1E90; the `for` form measured 219 of 219 at the original 876
 * bytes. The frame-phase statement must also follow the three step differences
 * rather than precede them: with it first the body measured 200 of 224 with the
 * first difference at 0x801D1E24, and masking the counter inline or through an
 * unmasked local measured the same 203 of 224, so only the statement's position
 * mattered.
 */
void drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C(
    VECTOR* arg0, VECTOR* arg1, u8 level) {
  POLY_G4* prim;
  VECTOR current;
  VECTOR step;
  u16 projected[2];
  u8* segment;
  u8 i;
  s32 phase;
  u16 tpage;

  tpage = GetTPage(0, 1, 0x2C0, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(2, 0xC);

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(arg0, projected);
  prim->x0 = projected[0] + 1;
  prim->y0 = projected[1] + 2;
  prim->x1 = projected[0];
  prim->y1 = projected[1];
  func_801AFF04(arg1, projected);
  prim->x2 = projected[0] - 1;
  prim->y2 = projected[1] - 2;
  prim->x3 = projected[0];
  prim->y3 = projected[1];
  prim->r0 = 0;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = level;
  prim->g1 = level;
  prim->b1 = 0;
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = level;
  prim->g3 = level;
  prim->b3 = 0;
  func_8014E5A0(2, 0x24);

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(arg0, projected);
  prim->x0 = projected[0];
  prim->y0 = projected[1];
  prim->x1 = projected[0] + 1;
  prim->y1 = projected[1] + 2;
  func_801AFF04(arg1, projected);
  prim->x2 = projected[0];
  prim->y2 = projected[1];
  prim->x3 = projected[0] - 1;
  prim->y3 = projected[1] - 2;
  prim->r0 = 0;
  prim->g0 = level;
  prim->b0 = 0;
  prim->r1 = 0;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = 0;
  prim->g2 = level;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(2, 0x24);

  step.vx = (arg1->vx - arg0->vx) >> 6;
  step.vy = (arg1->vy - arg0->vy) >> 6;
  step.vz = (arg1->vz - arg0->vz) >> 6;
  phase = D_80143E6C & 7;
  current.vx = arg0->vx + step.vx * phase;
  current.vy = arg0->vy + step.vy * phase;
  current.vz = arg0->vz + step.vz * phase;
  for (i = 0; i < 8; i++) {
    if (current.vy < arg1->vy) {
      break;
    }
    func_801AFF04(&current, projected);
    segment = g_PrimCursor;
    func_8017AA30(segment);
    SetSemiTrans(segment, 1);
    *(u16*)(segment + 0x08) = projected[0];
    *(u16*)(segment + 0x0A) = projected[1] - 1;
    segment[4] = 0;
    segment[5] = level;
    segment[6] = 0;
    func_8014E5A0(2, 0xC);
    current.vx += step.vx * 8;
    current.vy += step.vy * 8;
    current.vz += step.vz * 8;
  }
}
