#include "bof3/bof3.h"

u8* findFreeWorkRecordAt800E515CScenarioScena0700_801F9DD4(void);
void func_801F9E10(u8* record);
s32 func_801F9EFC(void);

/* Fixed-address state cell named by this target's own map: the scratchpad
 * work-object pointer cell at 0x1F800044. The head read and the claim loop's
 * back-edge read go through the SPAD_PTR_SLOT macro of the same address; the
 * two step reads that follow the loop use this map-named binding instead, which
 * is what makes each of them re-materialize the cell address in a caller-saved
 * register after the pass call below (see the representation note under the
 * metadata block). */
extern u8* workPointerCell; /* @source 0x1F800044 @kind unknown */

/* Overlay-local pointer cell holding the shared main-RAM effect object at
 * 0x800E5148 whose draw bound this handler ramps. The cell sits in the
 * payload's final data span (the Splat textbin block labelled T_801FDE34) and
 * has no target map row; the generated undefined_syms_auto.txt row binds it.
 * The same cell is what the sibling run member
 * stateEffectObjectDistance
 * publishes into it and what tickObjectRampQuadAndAdvanceHandlerScenarioScena0700_801F77AC
 * advances by four per frame while it draws. */
/* @source 0x801FDFBC @kind data */
extern u8* D_801FDFBC;

/* Four bytes 00 00 01 03 at 0x801FDD7C, the non-pointer data that closes the
 * three-word code-pointer run based at 0x801FDD70. Its only reader is this
 * handler, which masks the work object's 32-bit countdown word with
 * `mask[step]`, so steps 0 and 1 never reach the claim below. The row sits
 * inside the reviewed data span T_801FDD74 (0x801FDD74 through 0x801FDDB3) and
 * no image word points at it. */
/* @source 0x801FDD7C @kind table */
extern u8 D_801FDD7C[];

/* Eight bytes 01 01 01 02 03 04 00 00 at 0x801FDD84, the per-step record count
 * this handler reads through the same work-object step byte: it is both the
 * amount added to the effect object's draw bound on a gated frame and the
 * number of free records claimed on it. The row follows the mask above inside
 * the reviewed data span T_801FDD74 and no image word points at it. */
/* @source 0x801FDD84 @kind table */
extern u8 D_801FDD84[];

/* @source 0x801F7480
 * @behavior Entry 1 (the word at 0x801FDD74) of this overlay's three-entry
 * in-image code-pointer run based at 0x801FDD70 (0x801F7408 / this address /
 * stateTableIdle), which
 * callWorkByte2Step4 indexes with the work
 * object's unsigned byte at offset 0x02; no in-image `jal` reaches the address
 * and that run word is the only image reference to it. It loads the scratchpad
 * work-object pointer cell at 0x1F800044 ahead of the frame reservation, reads
 * that object's unsigned byte at 0x09 (the step index) and its 32-bit word at
 * 0x0C (the countdown), and forms the frame gate
 * `countdown & D_801FDD7C[step]`. On a frame whose gate reads zero it adds the
 * per-step record count `D_801FDD84[step]` to the effect object published at
 * pointer cell 0x801FDFBC (its unsigned halfword at +0x10, the loop bound the
 * quad helper at 0x801F97E0 clamps to 0xFF) and, while the gate reads below that
 * count, claims records in a zeroed-counter `do`/`while` loop: the free-slot
 * scan at 0x801F9DD4 walks the 0x40 entries of the 0x20-byte-stride pool at
 * 0x800E515C and every non-zero record goes in $a0 to the seeder at 0x801F9E10,
 * which sets the record's lead byte to 1, its byte at +0x01 to 0 and its
 * countdown byte at +0x02 to 0x10. The claim count is re-read from the
 * scratchpad cell on each back edge (the calls clobber the registers) and the
 * unsigned loop counter is compared masked to 0xFF. It then re-reads the cell,
 * and on frames whose countdown word reads zero in its low nibble increments the
 * step byte at 0x09, so the step index advances once every sixteen countdown
 * values. It calls the per-frame pass at 0x801F9EFC over the 0x40 entries of the
 * 0x800E515C work table (entry 2 of this same run calls that pass and resets the
 * work state when it reports no live entry), re-reads the cell once more,
 * decrements the countdown word at 0x0C in place - the `sw` of the decremented
 * value lands in the branch delay slot - and, only when the decremented count
 * reads zero, increments the work object's byte at 0x02, the handler index of
 * the byte-2 state machine, so the dispatcher moves this run to its next entry.
 * Takes no arguments and returns nothing; the 0x18-byte frame keeps $ra and $s0,
 * the claim loop's counter, and the single `jr $ra` epilogue has the usual nop
 * delay slot.
 * Representation note. Reading the two post-loop step cells through the
 * SPAD_PTR_SLOT macro measured 66/80: the macro's constant address lets the
 * compiler hold 0x1F800000 in $s0 across the pass call, so the first read costs
 * `li`/`ori`/`lw` and the second reuses the register, and the delay-slot fill of
 * the loop counter then targets the re-loaded address instead. Reading both
 * through the map-named cell measured 68/81: each read re-materializes its
 * address, but the counter's zero assignment was left outside the branch delay
 * slot. The form below - map-named reading for the first step cell, a fresh
 * local for the second, and the claim counter zeroed before the guarded block
 * rather than inside it - measures 80/80 bytes and instructions.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimRampBoundSixteenth(
    void) {
  u8* work;
  u8* record;
  u8* again;
  u32 gate;
  u8 i;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  gate = *(u32*)(work + 0xC) & D_801FDD7C[work[9]];
  if (gate == 0) {
    *(u16*)(D_801FDFBC + 0x10) += D_801FDD84[work[9]];
    i = 0;

    if (gate < D_801FDD84[work[9]]) {
      do {
        record = findFreeWorkRecordAt800E515CScenarioScena0700_801F9DD4();
        if (record != 0) {
          func_801F9E10(record);
        }
        i++;
      } while (i < D_801FDD84[SPAD_PTR_SLOT(u8, 0x44u)[9]]);
    }
  }

  if ((*(u32*)(workPointerCell + 0xC) & 0xF) == 0) {
    workPointerCell[9]++;
  }

  func_801F9EFC();

  again = workPointerCell;

  if (--*(s32*)(again + 0xC) == 0) {
    again[2]++;
  }
}
