#include "bof3/bof3.h"

/* Pointer cell of the scratchpad work record. The original loads the cell once
 * and then reaches the work phase byte through that loaded pointer, so the
 * pointer is one local of this body and the cell keeps the named form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Signed 16-entry phase table of this overlay's own image, the interior word
 * 0x801F43B0 of the pointer run the Splat row D_801F3F10 places. Its bytes
 * repeat -1, 0, 1, 0 four times, so the low nibble of the work phase byte
 * selects one of four equal quadrants of the step. Bound at the word the scaled
 * index adds to; the run keeps its single reviewed row.
 * @source 0x801F43B0 @kind table
 */
extern s8 D_801F43B0[];

/* Front-end record table of the shared main executable: 0x74-byte rows based at
 * 0x80143FC8, whose byte 0 is the occupancy flag and whose byte 5 is the handler
 * selector at 0x80143FCD. The original re-materialises each field's own base for
 * every store, so both fields keep their address-derived symbols.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];

/* Pointer cell of the shared record whose halfword 0x0A this step consumes.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Shared state byte of the main executable's scenario area, the byte after the
 * scenario progress word g_ScenarioProgress at 0x80146864 that the scenario
 * overlays' shared-state clears reset. This body reads it as a one-shot request
 * flag and clears it, so the byte stays address-anchored.
 * @source 0x80146865 @kind unknown
 */
extern u8 D_80146865;

/* Front-end record allocator of the shared main executable, outside this
 * payload: it answers the index of the next free 0x74-byte row of the table at
 * 0x80143FC8, or 0xFF when every row is occupied. This body compares that answer
 * without storing it, so the overlay sees the byte-wide return type. */
u8 func_8019601C(void);

/* @source 0x801F3388
 * @behavior Per-frame step of the work record published through the scratchpad
 *           pointer cell 0x1F800044. Each of the three accumulator updates reads
 *           the record's phase byte at 0x0A through the loaded pointer, masks it
 *           to its low nibble and takes that entry of the signed phase table at
 *           0x801F43B0: the entry scaled by 0x800 is added to word 0x34 and
 *           subtracted from word 0x38, so the pair moves in opposite directions
 *           along the table's axis, and the entry scaled by 0x10 is added to the
 *           halfword at 0x3E, the angle the record steers by. The phase byte is
 *           then decremented, and two units of the halfword at 0x0A of the record
 *           published through the global pointer cell 0x80148210 are consumed.
 *           The body ends on the shared state byte 0x80146865: while that byte
 *           reads nonzero it claims a front-end record through the allocator,
 *           seeds row byte 0 with 1 (the occupancy flag) and row byte 5 with the
 *           handler selector 0xAC unless the allocator reported 0xFF, clears the
 *           state byte and restarts the signed level triple at 0x5F/0x5E/0x5D at
 *           -0x40, the value the neighbouring world handlers arm it with; while
 *           the byte reads zero it instead steps each of the three signed level
 *           bytes by 8, so a triple armed at -0x40 walks to zero in eight frames.
 *           The 0x30-byte frame keeps $ra at 0x2C and $s0 at 0x28, the one saved
 *           register the address of the state byte stays in across the call, and
 *           the span returns through one shared epilogue.
 *
 *           The third phase read sits before the phase decrement, so the angle
 *           step is computed into an s32 local ahead of that decrement and added
 *           to the halfword afterwards: the original reads the phase byte for the
 *           third table index before it stores the decremented byte, so the angle
 *           step uses the pre-decrement nibble, and the shared record pointer is
 *           loaded into its own local between the decrement and the halfword
 *           update, which is where its `lui`/`lw` pair fills the slot the add
 *           leaves. Spelling the angle step and the halfword add as one inline
 *           statement instead measured 92/104 instructions with 424 bytes, because
 *           the shipped compiler then sinks the shift below the halfword load and
 *           needs a load-delay `nop` for the shared pointer; keeping the local but
 *           reading the shared pointer after the halfword store measured 93/105
 *           with 420 bytes.
 *           The three level-triple blocks of the second arm each re-read the
 *           pointer cell, while one load of the cell serves the whole triple on
 *           the arming arm, matching the level-triple idioms of
 *           advanceWorkCounterTripleBy2OrArmScratchWorkMin40LevelTriple
 *           World04Area17413_801F4B0C, which recorded the same arming order
 *           0x5F/0x5E/0x5D and the same single-load group, and of
 *           advanceScratchWorkLevelTripleAndAccumulatorsAtColorMarker
 *           World04Area17213_801F47F0, which recorded one cell load per
 *           conditional level block. The 0x34/0x38 word-pair and 0x3E halfword
 *           steps with the independently re-read nibble follow
 *           decrementPhaseStepWorkWordPairConsumeCountdownOrClearHandler
 *           World04Area17513_801F426C, and the shared halfword consume is the
 *           statement advanceWorkCounterTripleBy2OrArmScratchWorkMin40LevelTriple
 *           World04Area17413_801F4B0C closes on. The claim-and-seed group takes
 *           its statement order and 0x74 row stride from the exact sibling
 *           publishFrontEndRecordIndexToWorkByteBThenSeedHandlerA7Record
 *           FromWorkPositionWorld04Area19813_801F3854 of this same target, which
 *           recorded the occupancy flag at row byte 0 and the handler selector at
 *           row byte 5 and writes this body's own selector 0xAC.
 *
 *           Name evidence: the phase table plus the 0x34/0x38/0x3E accumulator
 *           family names the first half of the body, the signed level triple
 *           0x5D/0x5E/0x5F names the second, and the front-end record claim names
 *           the third. Nothing in this payload reads selector 0xAC back, so the
 *           value stays described by number. No `jal` in the payload names
 *           0x801F3388; the only in-image reference is the word 0x801F4280,
 *           entry 220 of the pointer run the Splat row D_801F3F10 places, so the
 *           body is reached from outside its own code spans rather than by call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkAccumulatorsByPhaseTableAndStepLevelTripleThenClaimRecordHandlerAcWorld04Area19813_801F3388(
    void) {
  volatile u8* work;
  u8* levels;
  u8* flag;
  u8 slot;
  s32 angle;
  u16* shared;

  work = D_1F800044;
  *(s32*)(work + 0x34) += D_801F43B0[work[0x0A] & 0x0F] << 11;
  *(s32*)(work + 0x38) -= D_801F43B0[work[0x0A] & 0x0F] << 11;
  angle = D_801F43B0[work[0x0A] & 0x0F] << 4;
  work[0x0A] = work[0x0A] - 1;
  shared = D_80148210;
  *(u16*)(work + 0x3E) += angle;
  shared[5] -= 2;

  flag = &D_80146865;
  if (*flag != 0) {
    slot = func_8019601C();
    if (slot != 0xFFu) {
      D_80143FC8[(u32)slot * 0x74u] = 1;
      D_80143FCD[(u32)slot * 0x74u] = 0xAC;
    }
    *flag = 0;
    levels = D_1F800044;
    *(s8*)(levels + 0x5F) = -0x40;
    *(s8*)(levels + 0x5E) = -0x40;
    *(s8*)(levels + 0x5D) = -0x40;
  } else {
    if (((s8*)D_1F800044)[0x5D] != 0) {
      ((s8*)D_1F800044)[0x5D] += 8;
    }
    if (((s8*)D_1F800044)[0x5E] != 0) {
      ((s8*)D_1F800044)[0x5E] += 8;
    }
    if (((s8*)D_1F800044)[0x5F] != 0) {
      ((s8*)D_1F800044)[0x5F] += 8;
    }
  }
}
