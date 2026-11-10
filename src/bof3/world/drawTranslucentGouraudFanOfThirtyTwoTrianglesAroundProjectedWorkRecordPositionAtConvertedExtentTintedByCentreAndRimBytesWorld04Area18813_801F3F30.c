#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared frame counter of the resident main executable, bound by the generated
 * undefined_syms_auto row. */
extern s32 D_80143E6C;

/* 0x801AFF04 - shared projector: writes the projected halfword pair of the
 * three-word point handed as arg0 into arg1. */
void func_801AFF04(const void* arg0, void* arg1);

/* 0x801AFFD8 - shared size-pair converter: converts the halfword pair handed
 * as arg1 at the depth of the same point and writes the result into arg2. */
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);

/* 0x801783C8 and 0x801782FC - shared 12-bit angular helpers, the x and the y
 * companion of the rotation pair the sibling fan drawer
 * drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20
 * of overlay sce15ef1 also calls for every ring angle. */
s32 func_801783C8(s32 arg0);
s32 func_801782FC(s32 arg0);

/* 0x8017A97C - packet opener: the sibling fan drawer of the same family calls
 * it on the primitive cursor before it fills a POLY_G3, and the resident
 * SetPolyG4 sits four entries later at 0x8017A9CC. */
void func_8017A97C(void* arg0);

/* One 8-byte screen entry of this body's projection scratch: the projector and
 * the size-pair converter write the halfword pair at +0x00/+0x02 and this body
 * reads back only that pair, so the trailing four bytes stay unmodelled. */
typedef struct Area18813ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} Area18813ScreenPoint;

/* @source 0x801F3F30
 * @behavior Overlay record render pass: the per-frame record pass 0x801F3E7C
 * hands it one 0x18-byte record of the 8-entry record table at 0x800E4800 that
 * this overlay shares with its claim and clear passes, and it draws that
 * record as a translucent Gouraud triangle fan around the record position.
 * It first arms the primitive list exactly as the rest of this overlay does:
 * GetTPage(0, 1, 0x3C0, 0) yields the texture page, SetDrawMode installs it on
 * the packet published at g_PrimCursor with dfe 0 and dtd 1, and
 * GpuAppendPrim(2, 0xC) queues the 0xC-byte draw-mode packet under ordering
 * table slot 2. It then converts the record halfword at 0x14 - the extent the
 * record pass decrements by 0x10 per frame alongside the 1 it subtracts from
 * the countdown byte 2, so the two expire together - at the depth of the
 * position triple at 0x04 through func_801AFFD8 and projects that same triple
 * through func_801AFF04, which is the centre every triangle shares. The
 * per-frame counter bit (D_80143E6C & 1) is added to the converted extent
 * before use, so the fan breathes by one sub-pixel unit on alternating frames.
 *
 * The loop runs 32 passes of one semi-transparent POLY_G3 emission at the
 * primitive cursor, its phase advancing by 0x80 over the 0x1000-unit turn, so
 * the last pass closes the disc exactly one turn after the first. Each pass
 * takes the projected centre as vertex 0, the centre plus the converted extent
 * rotated by the phase as vertex 1 through func_801783C8 for x and
 * func_801782FC for y with both products shifted down by twelve, and the same
 * centre plus the extent rotated by the already advanced phase as vertex 2, so
 * consecutive triangles share an arc edge. The first colour triplet takes the
 * record byte 0x16 in red and green with blue clear, and the second and third
 * take record byte 0x17 the same way, so the record spawn that writes 0x40
 * there and 0 at 0x17 lights the shared centre instead of the rim. Each packet
 * is appended under ordering table slot 2 with the 0x1C-byte POLY_G3 size.
 *
 * Evidence: payload asm at 0x801F3F30 - addiu $sp,$sp,-0x48 with sw
 * $ra,0x44, $s4,0x40, $s3,0x3C, $s2,0x38, $s1,0x34 and $s0,0x30, the four
 * addiu argument loads for GetTPage with sw $s0,0x30 in its delay slot, then
 * addu $a1,$zero,$zero, addiu $a2,$zero,0x1, andi $a3,$v0,0xFFFF and the
 * g_PrimCursor load, jal SetDrawMode with sw $zero,0x10 on its stack argument
 * slot, and jal 0x8014E5A0 with addiu $a0,$zero,0x2 and addiu $a1,$zero,0xC.
 * Next addiu $s0,$s4,0x4, addiu $a1,$sp,0x18, lhu $v0,0x14($s4), addiu
 * $a2,$sp,0x20, jal 0x801AFFD8 with sh $v0,0x18 in its delay slot, then jal
 * 0x801AFF04 with addiu $a1,$sp,0x28. The radius block is lui/lw of
 * 0x80143E6C, lhu $v0,0x20($sp), andi $v1,$v1,0x1, addu, sll 0x10 and sra
 * $s2,$v0,0x10, the signed conversion the signed mult operands need. The loop
 * is lui/lw of g_PrimCursor into $s0, jal 0x8017A97C with addu $a0,$s0, jal
 * SetSemiTrans with addiu $a1,$zero,0x1, lhu 0x28($sp) and sh 0x8($s0), andi
 * $s1,$s3,0xFFFF, lhu 0x2A($sp) and sh 0xA($s0), then the four
 * jal 0x801783C8 / jal 0x801782FC pairs each followed by mult $s2,$v0, mflo,
 * sra $v1,$t0,0xC and addu onto a fresh lhu of 0x28($sp) or 0x2A($sp), with
 * addiu $s3,$s3,0x80 and andi $s1,$s3,0xFFFF between the second and the third
 * pair and the stores at 0x10/0x12 and 0x18/0x1A folded into the two jal delay
 * slots. The colour block is lbu 0x16($s4) once feeding sb 0x4/0x5 with
 * sb $zero,0x6, and lbu 0x17($s4) once feeding sb 0xC/0xD/0x14/0x15 with
 * sb $zero,0xE and sb $zero,0x16. The back edge is sltiu $s1,$s1,0x1000 and
 * bnez to the loop head, so the loop is one unsigned phase bound, and the
 * epilogue reloads $ra, $s4, $s3, $s2, $s1 and $s0 and returns with jr $ra.
 * The colour block store sequence is this span's own: $s4 byte 0x16 is loaded
 * once, the blue byte of the first triplet is cleared, red and green take that
 * value; $s4 byte 0x17 is then loaded once and fills blue of the second and
 * third triplets with the two clears before it. One statement order reproduces
 * that emission and it was measured, not assumed: the natural red, green, blue
 * order for all three triplets matched 103 of 113 instructions and was
 * replaced by the order the bytes show.
 * The 0x08/0x0A, 0x10/0x12, 0x18/0x1A coordinate slots, the 0x04, 0x0C and
 * 0x14 colour triplets and the 0x1C append size are the POLY_G3 packet the
 * exact sibling pass drawEffectEntriesThenAdvanceAngle24By10World04Area17213_801F3FAC
 * and the sibling fan drawer at 0x801D1A20 both emit.
 *
 * Frame: the 0x48-byte frame carries SetDrawMode's outgoing stack argument at
 * sp+0x10, three 8-byte screen entries from sp+0x18 to sp+0x2F and the saved
 * group $s0..$s4 plus $ra from sp+0x30 to sp+0x47, so one address-taken array
 * of that entry type keeps the three entries together across the calls.
 *
 * Clone lever: a byte-identical scan of this 452-byte body over every payload
 * under out/binaries returns three hits - its own offset 0x1330 in
 * emi/world04/area188/13.bin, offset 0x27BC of emi/world02/area098/13.bin and
 * offset 0xF10 of emi/scenario/sce15ef1/00.bin, the last two being that
 * payload's still-unlifted fan drawers func_801F53BC and func_801D1B10. Both
 * twins are still asm boundaries, so there is no lifted source to clone, and
 * no address inside either payload is referenced by the body: every callee and
 * both fixed cells live in the resident main executable, which is why the same
 * bytes serve all three overlays. The clones stay a proof of shared shape and
 * the seed comes from the nearest lifted sibling, whose differences are the
 * converter call, the texture-page x, and the drawn record fields.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfThirtyTwoTrianglesAroundProjectedWorkRecordPositionAtConvertedExtentTintedByCentreAndRimBytesWorld04Area18813_801F3F30(
    u8* record) {
  Area18813ScreenPoint screen[3];
  POLY_G3* prim;
  s16 radius;
  u16 phase;
  u8 centre;
  u8 rim;
  s32 turn;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3C0, 0), 0);
  GpuAppendPrim(2u, 0x0cu);

  screen[0].x = FIELD_REF(u16, record, 0x14u);
  func_801AFFD8(record + 4, &screen[0], &screen[1]);
  func_801AFF04(record + 4, &screen[2]);

  radius = (s16)(screen[1].x + (D_80143E6C & 1));
  turn = 0;

  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = screen[2].x;
    prim->y0 = screen[2].y;
    phase = (u16)turn;
    prim->x1 = (u16)(screen[2].x + ((radius * func_801783C8(phase)) >> 12));
    prim->y1 = (u16)(screen[2].y + ((radius * func_801782FC(phase)) >> 12));
    turn += 0x80;
    phase = (u16)turn;
    prim->x2 = (u16)(screen[2].x + ((radius * func_801783C8(phase)) >> 12));
    prim->y2 = (u16)(screen[2].y + ((radius * func_801782FC(phase)) >> 12));
    centre = FIELD_REF(u8, record, 0x16u);
    prim->b0 = 0;
    prim->g0 = centre;
    prim->r0 = centre;
    rim = FIELD_REF(u8, record, 0x17u);
    prim->b2 = 0;
    prim->b1 = 0;
    prim->g2 = rim;
    prim->r2 = rim;
    prim->g1 = rim;
    prim->r1 = rim;
    GpuAppendPrim(2u, 0x1cu);
  } while (phase < 0x1000u);
}
