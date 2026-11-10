#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident engine helpers this overlay calls, declared the way its siblings
 * declare them: 0x8017AA80 stamps the LINE_F2 packet the shared cursor points
 * at (the shipped SDK map carries no row for it; it starts where its SetTile
 * row at 0x8017AA6C ends and the next row, SetLineF3, starts at 0x8017AAA8),
 * and 0x801AFF04 projects the three-word point handed as arg0 into the screen
 * halfword pair written through arg1. */
/* @source 0x8017AA80 */
extern void func_8017AA80(void* arg0);
/* @source 0x801AFF04 */
extern void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801F4568
 * @behavior Overlay line emitter: takes the addresses of two points and one
 *           tint shade and returns nothing. It reads the shared primitive
 *           cursor 0x8014598C once into $s0, stamps that cursor through
 *           0x8017AA80 as a line packet and flags it semi-transparent through
 *           SetSemiTrans, then projects the first point and the second point
 *           through 0x801AFF04 into one four-byte stack pair at $sp+0x10,
 *           writing the first projection into the halfwords at packet offsets
 *           0x08 and 0x0A and the second into the halfwords at 0x0C and 0x0E.
 *           The shade argument fills both the red and the green bytes at
 *           offsets 0x04 and 0x05 while the blue byte at 0x06 is cleared, so
 *           the line is drawn in one yellow-family shade, and the packet is
 *           appended with size class 2 and size 0x10 through 0x8014E5A0. The
 *           same five-instruction stamp/corner/tint/append body is the shape
 *           of the flat line emitter drawSemiTransFlatLinePrimWorld04Area17013_801F3774,
 *           which differs only in taking its two corners already projected.
 *           Evidence from the original bytes at 0x801F4568: addiu $sp,$sp,-0x30
 *           with sw $s0,0x18, $s1,0x1C, $s2,0x20, $s3,0x24 and $ra,0x28, the
 *           lui 0x8014 / lw $s0 with $lo(g_PrimCursor), the three register
 *           copies of $a0, $a1 and $a2 into $s1, $s2 and $s3, jal 0x8017AA80
 *           with addu $a0,$s0,$zero in its delay slot, jal SetSemiTrans with
 *           addiu $a1,$zero,1 in its delay slot, the two jal 0x801AFF04 calls
 *           that each pass addiu $a1,$sp,0x10, the lhu/sh pairs that move
 *           0x10($sp) and 0x12($sp) into 0x08/0x0A($s0) and 0x0C/0x0E($s0),
 *           sb $s3,0x4($s0), sb $s3,0x5($s0), sb $zero,0x6($s0), and jal
 *           0x8014E5A0 with addiu $a0,$zero,2 and addiu $a1,$zero,0x10. No
 *           caller in this overlay: the 0x3CA4-byte payload holds no jal to
 *           0x801F4568 and no pointer word for it, so the consumer lies
 *           outside this overlay and the role stays anchored here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSemiTransFlatLineBetweenProjectedPointsTintedShadeWorld04Area18813_801F4568(
    const void* startPoint, const void* endPoint, u8 shade) {
  LINE_F2* line;
  u16 sxy[2];

  line = (LINE_F2*)g_PrimCursor;
  func_8017AA80(line);
  SetSemiTrans(line, 1);

  func_801AFF04(startPoint, sxy);
  line->x0 = sxy[0];
  line->y0 = sxy[1];
  func_801AFF04(endPoint, sxy);
  line->x1 = sxy[0];
  line->y1 = sxy[1];
  line->r0 = shade;
  line->g0 = shade;
  line->b0 = 0;
  func_8014E5A0(2, 0x10);
}
