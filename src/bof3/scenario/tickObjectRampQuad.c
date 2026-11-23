#include "bof3/bof3.h"

/* Scratchpad pointer cell at 0x1F800044 holding this overlay's work object: a
 * 0x140-byte-stride state record whose byte at +0x02 is the byte-2 handler
 * index, byte at +0x09 the ramp step index and word at +0x0C the countdown. It
 * is the target map's own cell (symbols.txt), read once per block so each read
 * keeps its own `lui`/`lw` pair. */
extern u8* scenarioScena0700Work; /* @source 0x1F800044 @kind unknown */

/* Overlay-local per-step bit-mask table at 0x801FDD9C: one byte per ramp step
 * index (the work object's unsigned byte at +0x09) tested against the work
 * object's countdown word at +0x0C. The leading bytes 0x03, 0x01, 0x00 mean
 * step 0 ramps twice per four countdown ticks, step 1 once per two and the
 * later steps on every tick. The address sits inside the overlay's final data
 * span (the Splat textbin block T_801FDD74), which defines no label for it, so
 * the generated undefined_syms_auto.txt row binds it. */
extern u8 D_801FDD9C[]; /* @source 0x801FDD9C @kind data */

/* Overlay-local per-step ramp amount table at 0x801FDDA4: one byte per ramp
 * step index, added to the shared effect object's unsigned halfword at +0x10
 * when that step's mask bit is clear. The leading bytes are 0x01, 0x01, 0x01,
 * 0x02, 0x03, 0x04, so the ramp accelerates as the step index grows. Bound the
 * same way, from the generated undefined_syms_auto.txt row. */
extern u8 D_801FDDA4[]; /* @source 0x801FDDA4 @kind data */

/* Overlay-local pointer cell holding the shared 0x800E5148 work structure that
 * the sibling run member
 * stateEffectObjectDistance
 * publishes into it. The cell sits in the payload's final data span (the Splat
 * textbin block labelled T_801FDE34) and has no target map row; the generated
 * undefined_syms_auto.txt row binds it. */
/* @source 0x801FDFBC @kind data */
extern u8* D_801FDFBC;

/* Quad helper at 0x801F97E0: reads three table pointers at +0x00, +0x04 and
 * +0x08 of its argument, picks one 0x20-byte-stride entry from each with
 * func_801783C8 / func_801782FC, fetches the texture page with GetTPage, and
 * submits a semi-transparent Gouraud quad at the shared primitive cursor with
 * SetDrawMode / SetPolyG4 / SetSemiTrans, walking the argument's unsigned
 * halfword at +0x10 (clamped to 0xFF) as its loop bound. Its result is a scaled
 * word the caller at 0x801F7638 stores; this caller discards it. */
s32 func_801F97E0(u8* obj);

/* @source 0x801F76D0
 * @behavior Per-frame tick of the shared effect object published at 0x801FDFBC
 * and word 1 of the four-word in-image code-pointer run labelled D_801FDD8C at
 * 0x801FDD8C, which dispatchWorkByte2HandlerScenarioScena0700_801F75F4 indexes
 * with the scratchpad work object's unsigned byte at +0x02, so it runs while
 * that byte reads one. It loads the work-object pointer cell at 0x1F800044
 * before it creates its frame, reads that object's unsigned byte at +0x09 (the
 * ramp step index) and its countdown word at +0x0C, masks the countdown with
 * the byte that step selects from the overlay-local mask table at 0x801FDD9C
 * and, while that mask result is zero, adds the byte the same step selects from
 * the overlay-local amount table at 0x801FDDA4 to the published object's
 * unsigned halfword at +0x10, which raises the ramp the quad helper walks.
 * Through a second read of the pointer cell it advances the step index once
 * every sixteenth countdown tick (countdown masked with 0xF reaching zero),
 * then calls the quad helper at 0x801F97E0 with a fresh read of the published
 * object pointer as its only argument, so the object is drawn once per tick.
 * Through a third read of the pointer cell it decrements that object's
 * countdown word and, when the decremented value reaches zero, reloads it with
 * 0x10 and increments the work object's unsigned byte at +0x02, the handler
 * index of the byte-2 state machine, so the expired countdown advances the run.
 * It takes no arguments, returns nothing, and its 0x18-byte frame only keeps
 * $ra across the one call. The bytes are not identical to the sibling run
 * member tickObjectRampDrawQuadAndAdvanceHandlerScenarioScena0700_801F77AC at
 * 0x801F77AC, which draws the same object, ticks the same countdown and
 * advances the same handler index with a constant four-step ramp and no
 * countdown reload, so that sibling supplied the statement order, operand forms
 * and offset spelling of the shared call/advance tail and this body's own
 * tables, mask test, step advance and 0x10 reload come from its original bytes.
 * Probes before this gate, every one of them on this same body: the
 * SPAD_PTR_SLOT(u8,0x44) spelling measured 39/55 instructions with 228 bytes,
 * because three reads of the cell kept 0x1F800044 in one saved register and
 * emitted a `lui`/`ori` base; the map's named cell scenarioScena0700Work with
 * one function-scope base reused for all three reads measured 46/55 at the
 * original 220 bytes, where the two hunks that differed were the register the
 * first and second base took (`a0` against the original `v0` and `v1`); giving
 * each of the three reads its own base local measured 55/55 at 220 bytes, and
 * the inlined-cell and block-scoped-base spellings of that change tied it, so
 * the per-read base is the lever and its spelling is not.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickObjectRampQuad(
    void) {
  u8* work;
  u8* next;
  u8* pending;
  u8 step;
  u8* obj;

  work = scenarioScena0700Work;
  step = work[9];
  if ((*(s32*)(work + 0x0C) & D_801FDD9C[step]) == 0) {
    obj = D_801FDFBC;
    *(u16*)(obj + 0x10) += D_801FDDA4[step];
  }
  next = scenarioScena0700Work;
  if ((*(s32*)(next + 0x0C) & 0xF) == 0) {
    next[9]++;
  }
  func_801F97E0(D_801FDFBC);
  pending = scenarioScena0700Work;
  if (--*(s32*)(pending + 0x0C) == 0) {
    *(s32*)(pending + 0x0C) = 0x10;
    pending[2]++;
  }
}
