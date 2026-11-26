#include "bof3/bof3.h"

void func_801F7484(void);
void stateIndexedBaseService(
    void);

/* @source 0x801F72A0
 * @behavior Work-object step handler of this overlay, the word at 0x801FBF04:
 * entry 5 of the twelve-word work-object handler run D_801FBEF0
 * (0x801FBEF0..0x801FBF1C) that the frame dispatcher func_801F6EA4 indexes with
 * byte 0x01 of the scratchpad work object published at pointer slot 0x1F800044,
 * and entry 2 of the run the second byte-1 dispatcher func_801F7198 indexes from
 * the same base run at 0x801FBEFC, so both dispatchers reach this body with no
 * argument and no result consumed. It takes no arguments and returns nothing. It
 * first reads the work pointer cell once (`lui $v0,0x1F80` / `lw $v0,0x44($v0)`,
 * before the frame is created), takes the object countdown byte 0x09 (`lbu
 * $v1,0x9($v0)`), subtracts one (`addiu $v1,$v1,-0x1`) and stores it back (`sb
 * $v1,0x9($v0)`). It then re-reads the cell and branches on that byte: while it
 * stays non-zero the handler ramps the object tint triplet at 0x5D, 0x5E and
 * 0x5F up by 2 each (`lbu $v0`/`addiu 0x2`/`sb $v0`), re-reading the cell before
 * the 0x5E and 0x5F channels so each of those two groups keeps its own pointer
 * load, and the 0x5F store sits in the `j` delay slot. When the byte reads zero
 * it instead stores the 0x1E the `beqz` delay slot loads back into that same
 * byte through the pointer the test already holds, re-reads the cell and stores
 * 3 into the object dispatch byte 0x01, which hands the next frame to entry 3
 * of D_801FBEF0 (0x801F71DC, the step that re-clears the triplet, latches the
 * shared front-end phase byte and moves byte 0x01 to 1). Both arms rejoin at the
 * two overlay helper calls func_801F7484 and
 * stateIndexedBaseService,
 * made in that order with no arguments and with any result discarded, so the
 * pair runs on every call and their `jal` delay slots stay nops. The 0x18-byte
 * frame keeps only $ra at 0x10 across them.
 *
 * Statement order, operand forms, offset spelling and declaration style come
 * from the exact same-target step 0x801F6F24
 * (decrementWorkCountdownRamp),
 * whose 0xAC-byte walk of this record differs only in the size and direction of
 * its triplet step, the state byte it stores on expiry and its two callees, and
 * from the exact cross-target mirror 0x801F3A6C
 * (fadeWorkTintTripletBy6AndAdvanceWorkByte1To6WhenCountdownExpiresWorld02Area10814_801F3A6C).
 * This span adds the countdown reload to 0x1E in its expiry arm, the 16 bytes
 * the sibling size difference accounts for, and writes 3 rather than 2 or 6 into
 * the dispatch byte. A scan of the 398 extracted payloads under out/binaries
 * found the `lbu $v1,0x9($v0)`-to-`sb $v0,0x5F($v1)` region of this span in this
 * payload only, so the +2 step, the 0x1E reload and the 3 dispatch store are
 * this span own immediates and no byte-identical twin exists. The pattern is
 * written with the `slot_offset` local so each pointer-cell read keeps its own
 * `lui $vN,0x1F80` plus folded `lw $vN,0x44($vN)` pair; the tint step and the
 * expiry reload share one cursor local and only the two later channels and the
 * dispatch store re-read the cell, exactly as the original does.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTintRampStep(void) {
  u32 slot_offset;
  u8 *countdown;
  u8 *work;

  slot_offset = 0x44u;
  countdown = PSX_REF(u8 *, SPAD_BASE + slot_offset);
  countdown[9]--;
  work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
  if (work[9] != 0) {
    work[0x5D] += 2;
    work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    work[0x5E] += 2;
    work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    work[0x5F] += 2;
  } else {
    work[9] = 0x1E;
    work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    work[1] = 3;
  }
  func_801F7484();
  stateIndexedBaseService();
}
