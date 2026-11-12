#include "bof3/world/area03004_internal.h"

/* Engine primitive initialiser of the main executable: it writes the tag and the
 * code byte of the 0x10-byte packet that g_PrimCursor points at. This payload
 * only calls it, and the address lies outside every EMI payload.
 * @source 0x8017AA80 @kind function */
extern void func_8017AA80(void* primitive);

/* AREA030 per-slot panel records based at 0x80146888, stride 0x98 (152 bytes).
 * The original reaches four of their byte and word fields and one signed
 * halfword through their own %hi/%lo symbol pair, each access re-reading the
 * slot index byte D_801E3208, so the fields stay separate address-anchored
 * objects rather than one typed struct view. */
extern u8  D_80146889[];   /* @source 0x80146889 @kind table */
extern u8  D_8014688C[];   /* @source 0x8014688C @kind table */
extern u8  D_80146892[];   /* @source 0x80146892 @kind table */
extern u8  D_80146898[];   /* @source 0x80146898 @kind table */
extern u8  D_80146916[];   /* @source 0x80146916 @kind table */

/* Lever record for this span. A little-endian scan of every payload under
 * out/binaries found the 0x480-byte span only at 0x801D2DC4 of this payload, so
 * clone was declined; the two byte-identical replicas this manifest declares
 * (BIN/WORLD02/AREA089.EMI#4 and BIN/WORLD03/AREA129.EMI#4) have no target rows
 * and so no lifted source to copy. Scanning every generated asm row that calls
 * submitTpageDrawMode found this shared panel shape only in this target, and no
 * sibling carries the D_80146892 / D_80146916 / D_80145E92 combination, so the
 * seed was written from the original bytes. The span opens on addiu sp,-0x30,
 * returns through jr $ra at 0x801D3240 and ends exactly at its own declared
 * boundary, so the data and carve levers did not apply. */

/* @source 0x801D2DC4
 * @behavior entry 2 of the local handler table at 0x801E1D80, which the
 * dispatcher func_801D2BF0 indexes with the work record's unsigned step byte at
 * +0x1: it first selects draw mode (0, 2) and submits panel sprite 0x11 at
 * (0x87, 0xB4), then advances the work record's halfword at +0x30 towards the
 * signed byte that the slot index D_801E3208 selects from D_80146892 (stride
 * 0x98). A zero direction word at +0x10 arms the approach: while the halfword
 * is farther from that byte than 4 * D_801E320C[3] + 0xC the record's byte at
 * +0x9 counts up to 0xF and the direction word is then latched to -1 or 1 by
 * the sign of the gap, while a halfword already inside that band clears byte
 * +0x9. Every pass adds the direction word to the halfword, clamps the sum into
 * [4 * D_801E320C[3] - 0x24, 0x24 - 4 * D_801E320C[3]] and stores it back.
 * It then emits a grey 0x10-byte line at x 0xB9 from y 0xC9 to y 0xD5, draws
 * the panel row through func_801D3244 (origin halfword 0xAD below the row, 0xCB,
 * (8 * D_801E320C[3] + 0x18) & 0xF8, work byte 0xA low three bits doubled, 2,
 * and a flag that is 1 when the row start falls outside the 8 * D_801E320C[3] +
 * 0x18 span above it), and draws the panel cell through
 * drawMirroredPanelCellQuadWorld00Area03004_801D2C34 (the D_80146892 byte plus
 * 0xB9 clamped to 0x90..0xE4, 0xCB, whether the D_80146898 word's top bit is
 * clear, 2). When D_80146889's byte is 4 it first emits a second 0x10-byte line
 * in the 0x80 red at y 0xD3 whose length is the signed halfword at D_80146916
 * scaled by 96/256.
 * Finally, when D_80145E92 is 4, D_8014688C's byte is not 3, the clamped cell
 * origin falls outside the span above it and the frame counter D_80143E6C low
 * nibble is clear, it requests sound cue 0x207 through func_8015DF18.
 * @status partial
 * @match 70.83
 * @residual 204 of 288 instructions match at 1124 of 1152 bytes; the prologue,
 * epilogue, both call groups, every stated constant and the tail control flow
 * agree, and the first difference is instruction 14, where the scratch-record
 * pointer of the approach block lands in $a3 instead of the original $a0. The
 * rest of the gap is the allocation and scheduling that follows from it, plus
 * three unproven spellings: the D_80146892 byte at +0x0A is read here with a
 * single lb where the original loads lbu and re-signs with sll 24/sra 24, the
 * clamp's default arm re-reads +0x30 as a plain lhu where this build forwards
 * the stored value, and the two func_801D3244 calls hoist their shared
 * arguments where the original sets them per arm.
 */
void slideWorkHalfword30TowardSlotTargetAndDrawPanelWorld00Area03004_801D2DC4(void) {
  u8*      work;
  LINE_F2* prim;
  s32      b;
  s32      cur;
  s32      target;
  s32      sum;
  s32      x;
  s32      keep;
  s32      v;
  s32      y;
  s32      span;
  s8       level;
  s16      clamped;

  submitTpageDrawMode(0, 2);
  func_801E0DCC(0x11, 2, 0x87, 0xB4);

  work = D_1F800044;
  if ((*(s32*)(work + 0x10) == 0) &&
      ((((s32)*(s16*)(work + 0x30) + ((u8*)D_801E320C)[3] * 4) + 0xC) <
           (s8)D_80146892[D_801E3208 * 0x98] ||
       (s8)D_80146892[D_801E3208 * 0x98] <
           ((s32)*(s16*)(work + 0x30) - ((u8*)D_801E320C)[3] * 4 - 0xC))) {
    if (work[9] >= 0xF) {
      if ((s8)D_80146892[D_801E3208 * 0x98] < *(s16*)(work + 0x30)) {
        *(s32*)(work + 0x10) = -1;
      } else if (*(s16*)(work + 0x30) < (s8)D_80146892[D_801E3208 * 0x98]) {
        *(s32*)(work + 0x10) = 1;
      }
    } else {
      work[9] = work[9] + 1;
    }
  } else {
    work[9] = 0;
  }

  work = D_1F800044;
  sum = (s32)*(u16*)(work + 0x30) + *(s32*)(work + 0x10);
  *(u16*)(work + 0x30) = (u16)sum;
  x = (s16)*(u16*)(work + 0x30);
  b = ((u8*)D_801E320C)[3];
  keep = x < b * 4 - 0x24
             ? b * 4 - 0x24
             : (0x24 - b * 4 < x ? 0x24 - b * 4 : (s32)*(u16*)(work + 0x30));
  *(u16*)(work + 0x30) = (u16)keep;

  prim = (LINE_F2*)g_PrimCursor;
  func_8017AA80(prim);
  prim->x0 = 0xB9;
  prim->x1 = 0xB9;
  prim->y0 = 0xC9;
  prim->y1 = 0xD5;
  prim->r0 = 0x80;
  prim->g0 = 0x80;
  prim->b0 = 0x80;
  func_8014E5A0(2, 0x10);

  b = ((u8*)D_801E320C)[3];
  work = D_1F800044;
  level = D_80146892[D_801E3208 * 0x98];
  cur = *(u16*)(work + 0x30);
  span = b * 8 + 0x18;
  v = cur - b * 4 + 0xAD;
  x = v + b * 8 + 0x18;
  y = (s32)level + 0xB9;
  if (((s16)x < (s16)y) || ((s16)y < (s16)v)) {
    func_801D3244((s16)v, 0xCB, span & 0xF8, (work[0xA] & 7) * 2, 2, 1);
  } else {
    func_801D3244((s16)v, 0xCB, span & 0xF8, (work[0xA] & 7) * 2, 2, 0);
  }

  if (D_80146889[D_801E3208 * 0x98] == 4) {
    prim = (LINE_F2*)g_PrimCursor;
    func_8017AA80(prim);
    prim->x0 = 0x88;
    prim->x1 = 0x88 + (*(s16*)(D_80146916 + D_801E3208 * 0x98) * 96) / 256;
    prim->y0 = 0xD3;
    prim->y1 = 0xD3;
    prim->r0 = 0x80;
    prim->g0 = 0;
    prim->b0 = 0;
    func_8014E5A0(2, 0x10);

    target = (s8)(~(*(u32*)(D_80146898 + D_801E3208 * 0x98)) >> 31);
    if ((s16)y < 0x90) {
      clamped = 0x90;
    } else if ((s16)y < 0xE5) {
      clamped = (s16)y;
    } else {
      clamped = 0xE4;
    }
    drawMirroredPanelCellQuadWorld00Area03004_801D2C34(clamped, 0xCB, (s8)target, 2);

    if ((D_80145E92 == 4) && (D_8014688C[D_801E3208 * 0x98] != 3) &&
        (((s16)x < clamped) || (clamped < (s16)v)) &&
        ((D_80143E6C & 0xF) == 0)) {
      func_8015DF18(0x207);
    }
  }
}
