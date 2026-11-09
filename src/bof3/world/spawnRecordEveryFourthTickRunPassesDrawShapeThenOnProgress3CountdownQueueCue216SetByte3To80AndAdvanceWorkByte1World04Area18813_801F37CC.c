#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 for every store group below, so
 * the cell is its own fixed-address symbol rather than the constant-address
 * accessor, whose shared base would stay live across the calls in a saved
 * register and widen the frame. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared scenario progress byte the world overlays branch on. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 */

void claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0(void);

/* Per-frame passes this overlay's progress handlers run, still anchored to
 * their addresses. 0x801F3E7C is the record pass over the 0x800E4800 table
 * that ticks and draws the active records; 0x801F44B4 is the 0x800E48C0-table
 * effect draw pass. */
void func_801F3E7C(void);
void func_801F44B4(void);

/* Overlay quad renderer of this area: it primes a semi-transparent four-vertex
 * primitive through the shared primitive cursor, takes the record's three-word
 * triple at +0x18 as arg0 and the position triple at +0x34 as arg1, and uses
 * the byte handed as arg2 as the channel mask colour it stores into the
 * primitive's colour bytes. */
void func_801F3ADC(void* arg0, void* arg1, u8 channels);

/* Shared EXE-side cue dispatcher; the constant argument seeds a caller's
 * channel refresh, so the call-site width is the narrow constant form. */
void func_8015DF18(u16 arg0);

/* @source 0x801F37CC
 * @behavior Overlay step handler, entry 7 of the state-byte-1 handler table at
 * 0x801F4E04 that the dispatcher 0x801F32C0 indexes. It reads the frame byte
 * at +0x06 of the work record published at the scratchpad pointer cell
 * 0x1F800044, stores that byte plus one back so the byte counts the frames
 * this handler runs, and on every frame whose pre-increment value has its low
 * two bits clear it calls the work-record spawn 0x801F3DE0. It then runs this
 * overlay's two per-frame passes 0x801F3E7C and 0x801F44B4 in that order and
 * renders the record through the overlay renderer 0x801F3ADC with the record's
 * three-word triple at +0x18, its position triple at +0x34 and the channel
 * mask 0x40. While the shared scenario progress byte 0x80146864 does not read
 * 3 it returns; otherwise it queues the front-end cue 0x216 through
 * func_8015DF18 on the frame the countdown byte at +0x09 still reads 0x10,
 * decrements that countdown byte and, on the frame it reaches zero, stores
 * 0x80 into the work byte at +0x03 and advances the state byte at +0x01 by
 * one. It takes no arguments and its 0x18-byte frame only holds $ra at 0x10
 * across the four calls.
 *
 * Evidence: payload asm at 0x801F37CC - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in
 * its delay slot, jal 0x801F3DE0 only on the fall-through, then jal 0x801F3E7C
 * and jal 0x801F44B4 with nop delay slots, a fresh lui/lw pair of the same
 * cell into $a1 with addiu $a2,$zero,0x40 and addiu $a0,$a1,0x18, jal
 * 0x801F3ADC with addiu $a1,$a1,0x34 in its delay slot, lui/lbu of
 * g_ScenarioProgress, addiu $v0,$zero,3 and bne straight to the epilogue, then
 * inside the progress-3 block a lui/lw pair plus lbu 0x9 compared against
 * 0x10, then jal 0x8015DF18 with addiu $a0,$zero,0x216 in its delay slot, a third lui/lw pair with lbu 0x9, addiu $v0,$v0,-1 and sb $v0,0x9($v1)
 * followed by andi $v0,$v0,0xFF and bnez to the epilogue, and on the expiry
 * path a fourth lui/lw pair storing 0x80 at 0x3 and a fifth reading, advancing
 * and storing back the byte at 0x1, then lw $ra,0x10($sp), jr $ra with addiu
 * $sp,$sp,0x18 in its delay slot.
 *
 * Name evidence: the only in-image reference to this address is the word at
 * 0x801F4E20, inside the nine-word code-pointer run at 0x801F4E04 that also
 * holds 0x801F3304, 0x801F3368, 0x801F3400, 0x801F3484, 0x801F3598,
 * 0x801F364C, 0x801F36C4 and 0x801F38C0; that run is the byte-1 dispatch table
 * of the dispatcher 0x801F32C0, which indexes it with the work record's state
 * byte, and no in-payload code reads it as data, so the run fixes this
 * function as the state-7 progress handler and the caller stays outside the
 * payload. The every-fourth-frame cadence and the two-pass order are the ones
 * this target's exact claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0
 * records for its four step-handler callers, and the renderer 0x801F3ADC is
 * this overlay's own quad renderer, whose 0x18/0x34 argument pair reads the
 * same record triples that 0x801F3DE0 seeds. Work byte 0x09 is the countdown
 * the sibling handler 0x801F364C arms with 0x20, the literal 0x10 is the value
 * this handler tests before queueing its cue, and 0x216 continues the cue
 * series whose 0x214 and 0x215 the sibling handlers 0x801F3368 and 0x801F3484
 * queue. Nothing in this payload reads the work byte at +0x03, so its role
 * stays described by offset rather than named.
 * @status exact
 * @match 100.00
 * @residual none
 */
void spawnRecordEveryFourthTickRunPassesDrawShapeThenOnProgress3CountdownQueueCue216SetByte3To80AndAdvanceWorkByte1World04Area18813_801F37CC(
    void) {
  u8 tick;

  tick = D_1F800044[6]++;
  if ((tick & 3) == 0) {
    claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0();
  }
  func_801F3E7C();
  func_801F44B4();
  func_801F3ADC(D_1F800044 + 0x18, D_1F800044 + 0x34, 0x40);
  if (g_ScenarioProgress == 3) {
    if (D_1F800044[9] == 0x10) {
      func_8015DF18(0x216);
    }
    if (--D_1F800044[9] == 0) {
      D_1F800044[3] = 0x80;
      D_1F800044[1]++;
    }
  }
}
