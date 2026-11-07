#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x80145ECC @kind unknown */
extern u32 D_80145ECC;

/* Fan drawer of this payload at 0x801F2DB4: projects the three-word descriptor
 * it is handed and emits the 32-step Gouraud triangle fan around that projected
 * centre, each step scaling the projected extent by the cosine and sine of its
 * ring angle. Its second argument is the halfword it stores into the descriptor
 * before that projection, and the third and fourth are the centre and rim
 * colour bytes. It takes no result the caller uses. */
void func_801F2DB4(VECTOR* pos, s16 arg1, u8 arg2, u8 arg3);

/* @source 0x801F31F4
 * @behavior Effect step that draws this overlay's soft radial disc. It selects
 *           the texture page GetTPage(0, 1, 0x380, 0x100) for the current
 *           primitive cursor, appends the 0xC-byte draw-mode primitive for it
 *           through func_8014E5A0, then builds the shared three-word world
 *           position descriptor on its own frame: D_80145EC4, D_80145EC8 and
 *           D_80145ECC plus 0x800000. It hands that descriptor to
 *           func_801F2DB4 together with its signed argument, the centre colour
 *           0x60 and the rim colour 0, so the fan fades from flat grey at the
 *           projected centre to black at the rim. Takes its signed halfword
 *           argument in $a0 and returns nothing; the callee loads its own
 *           descriptor pointer from the frame slot, which is why the argument
 *           passes as a VECTOR rather than as three scalars.
 *
 * Evidence notes: both callers prove the argument is a signed halfword.
 * func_801F2CC8 calls it with the constant 0x80 (`addiu $a0, $zero, 0x80` in
 * the `jal` delay slot at 0x801F2CD4) and func_801F2D44 with the countdown
 * word 0x0C of the record published at the scratchpad cell 0x1F800044 scaled
 * by eight (`sll $a0, $a0, 19` then `sra $a0, $a0, 16` before the `jal` at
 * 0x801F2D80), so the same 0x80..0x08 extent serves both a fixed and a decaying
 * radius. Both callers also ignore the result, which agrees with a void
 * prototype. The three cells the descriptor is built from are the shared
 * position words the exact sibling lift
 * updateWorkTableRecordsAndReportActiveWorld01Area04713_801F330C copies into
 * each work record one statement run later, so they stay address-anchored at
 * their shared main-RAM binding, and the D_80145ECC plus 0x800000 offset is
 * the same one that sibling stores. The frame is 0x30 bytes with $s0 at 0x28
 * and $ra at 0x2C, and the descriptor occupies 0x18 through 0x27, which is the
 * 0x10-byte VECTOR the projection helper func_801AFF04 reads. The two ring
 * colour constants are byte stores in the callee, so they pass as bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSharedPositionGouraudDiscWorld01Area04713_801F31F4(s16 arg0) {
  VECTOR pos;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  GpuAppendPrim(1u, 0x0cu);
  pos.vx = D_80145EC4;
  pos.vy = D_80145EC8;
  pos.vz = D_80145ECC + 0x800000u;
  func_801F2DB4(&pos, arg0, 0x60, 0);
}
