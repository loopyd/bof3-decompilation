#include "bof3/bof3.h"

/* Work record the main program publishes through the scratchpad pointer cell
 * 0x1F800044. The original takes the pointer into a register once for the whole
 * field block below and re-reads the cell for each of the three accesses that
 * follow it, so the source names a local for the block and evaluates the cell
 * again afterwards. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program readiness helper; its result is dropped here, the same
 * way the exact sibling accumulateEnemyThenRearmScratchState3BossBoss02213_800C1938
 * drops it. */
void func_8014DAEC(void); /* @source 0x8014DAEC */

/* Shared main-program rectangle clear, carried by the main executable as
 * clearRenderRect(x, y, width, height). */
void func_8014E564(s16 x, s16 y, s16 w, s16 h); /* @source 0x8014E564 */

/* Shared main-program sound-cue dispatcher, carried by the main executable as
 * dispatchSoundCue(cue_id). */
void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 */

/* This payload's rotated-quad draw pass: it samples the texture page at
 * (0x340, 0x100) through GetTPage, takes the drawn quad's size from record
 * words +0x0C/+0x10, its anchor from +0x18/+0x1C and its spin from the byte it
 * is handed, and gives the quad the 0xBF-by-0x68 texture UV rectangle. */
void func_800C23D4(u8 spin); /* @source 0x800C23D4 */

/* @source 0x800C1A78
 * @behavior Rotating-quad step of this overlay: it runs the shared readiness
 * helper at 0x8014DAEC with its result dropped, clears the 0xC0-by-0x100
 * rectangle at (0x340, 0x100) through the shared rectangle clear at 0x8014E564
 * -- the same texture page the draw pass it ends with samples -- and then seeds
 * the scratch work record published through the pointer cell 0x1F800044. It
 * first lifts that record's staged anchor halfwords +0x2E and +0x30, sign
 * extending both into the drawn-quad anchor words +0x18 and +0x1C, and then
 * overwrites them: the quad size words +0x0C and +0x10 take 0xBE and 0x68, byte
 * +0x0A takes 1, word +0x20 takes zero, and the staged anchor halfwords take
 * 0x64 and 0x50. Bits 0x8 and 0x80 of record byte +0x24 are raised, sound cue
 * 0x602 goes through the shared cue dispatcher at 0x8015DF18, the draw pass at
 * 0x800C23D4 runs on record byte +0x09 as the quad's spin, and byte +0x03 of
 * the record is incremented to advance the step state. Both staged-anchor
 * stores read before they are overwritten, so the copy is a save of the anchor
 * this step draws with rather than a re-read of the new values. The flag update
 * re-reads the pointer cell into `record` after the field stores, which is the
 * load this body keeps in $a1 while the field block keeps $v0. Roles of words
 * +0x0C/+0x10 as the quad's size and of +0x18/+0x1C as its anchor come from the
 * draw pass itself. No word of this payload holds this function's address and
 * no `jal` reaches it, so the main program reaches it through the overlay entry
 * point; prologue and return path alone place its boundary. Same body shape as
 * the still-unlifted 0x800C1978 of BIN/BOSS/BOSS012.EMI#16, which clears 0x80
 * instead of 0xC0, cues 0x601 instead of 0x602, stores its staged anchor pair
 * 0x32 and 0x41 and leaves record byte +0x03 at 2 instead of incrementing it.
 * @status partial
 * @match 98.00
 * @residual One instruction of 50 is displaced: the original schedules the
 * staged-anchor load of halfword +0x30 next to the +0x2E load as its second
 * instruction (original 0x800C1AAC, `lh $a0,0x30($v0)`), while this body emits
 * it four instructions later, in the load-delay slot of the pointer re-read at
 * original 0x800C1AD4. Instruction selection, the 50-instruction size and every
 * register ($v0 base, $v1 constants, $a2/$a0 anchors, $a1 re-read) already
 * agree; the residue is one scheduling step of the pinned PsyQ 2.7.2 pre- and
 * post-reload passes. Seventeen source shapes were measured (anchor reads
 * before, after and interleaved with the field stores, read through the record
 * pointer, direct stores without locals, swapped read order, and both orders of
 * the +0x20 and +0x30 stores) and each left a single displaced instruction, the
 * anchor load this body delays or that store pair.
 */
void seedQuadSizeAndAnchorThenDrawRotatedQuadAndAdvanceStateBossBoss03416_800C1A78(
    void) {
  u8 *work;
  u8 *record;
  s32 anchor_x;
  s32 anchor_y;

  func_8014DAEC();
  func_8014E564(0x340, 0x100, 0xC0, 0x100);

  work = D_1F800044;
  anchor_x = FIELD_REF(s16, work, 0x2Eu);
  anchor_y = FIELD_REF(s16, work, 0x30u);
  FIELD_REF(u32, work, 0x0Cu) = 0xBEu;
  FIELD_REF(s32, work, 0x10u) = 0x68;
  FIELD_REF(s16, work, 0x2Eu) = 0x64;
  FIELD_REF(u8, work, 0x0Au) = 1u;
  record = D_1F800044;
  FIELD_REF(u32, work, 0x20u) = 0u;
  FIELD_REF(s16, work, 0x30u) = 0x50;
  FIELD_REF(s32, work, 0x18u) = anchor_x;
  FIELD_REF(s32, work, 0x1Cu) = anchor_y;
  FIELD_REF(u8, record, 0x24u) |= 0x88u;
  func_8015DF18(0x602u);
  func_800C23D4(D_1F800044[9]);
  D_1F800044[3]++;
}
