#include "bof3/bof3.h"

/* Shared world/state id that indexes the world record table below. */
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;

/* World record table indexed by the shared world/state id. */
/* @source 0x8017F974 @kind table */
extern void* D_8017F974[];

/* Pointer cell of the shared record whose step halfword at 0x0A this handler
 * both reads and advances. */
/* @source 0x80148210 @kind data */
extern u8* D_80148210;

/* Scratchpad cell holding the work record this overlay publishes. The cell is
 * materialised as `lui` plus an offset load for every access, so the source
 * keeps the named-symbol form and re-reads the cell instead of caching it. */
/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

/* Per-entry pair of signed byte offsets, twelve entries, at the end of this
 * payload's textbin data blob. */
/* @source 0x801F6B60 @kind table */
extern s8 D_801F6B60[][2];

/* Resident overlay dispatcher at 0x801FD34C, outside this payload. Its own
 * bytes take two signed-byte arguments, a signed-halfword fourth argument and
 * three further arguments from its caller's stack frames (signed halfword,
 * unsigned byte, unsigned byte), then re-dispatch on scratchpad work byte 0x04
 * through the mode table at 0x801FE46C. It returns the dispatched routine's
 * answer sign-extended from a byte. */
/* @source 0x801FD34C */
extern s8 func_801FD34C(u8* arg0, s8 arg1, s8 arg2, s16 arg3, s16 arg4, u8 arg5, u8 arg6);

/* Shared main-executable distance helper: takes the two position words of a
 * record and answers the planar distance between them. */
/* @source 0x8015477C */
extern s32 func_8015477C(s32 x, s32 y);

/* Shared main-executable resource request helper; the sibling world, plchar
 * and bmagic lifts pass a resource id and declare the same prototype. */
/* @source 0x8014D6B8 */
extern void func_8014D6B8(u32 arg0);

/* @source 0x801F60C4
 * @behavior State handler of this overlay, reached only through the code
 *           pointer word at 0x801F6AC8 inside the state-pointer run of the
 *           payload's D_801F63C8 textbin blob; no jal in the payload names it.
 *           It indexes the world record table 0x8017F974 with the shared
 *           world/state id 0x80143F00, takes that record's word at offset 0x10
 *           as a table of pointers, picks the entry with byte 0x03 of the
 *           record published through the pointer cell 0x80148210, adds that
 *           record's halfword 0x0A as a byte offset and reads the byte at
 *           offset 0x02 of the result. That byte selects one of the twelve
 *           signed-byte offset pairs of the table at 0x801F6B60, and the pair,
 *           the record pointer, the literal 0x40 and the fixed arguments
 *           (-0x400, 4, 0) go to the resident dispatcher 0x801FD34C. When the
 *           dispatcher answers nonzero the handler steps the record's halfword
 *           0x0A back by two; otherwise, when the selected entry index is 4, it
 *           first takes the distance between the scratchpad work record's
 *           position words at 0x34 and 0x38 through func_8015477C, stores that
 *           distance in halfword 0x3E of the record re-read from the scratchpad
 *           cell 0x1F800044 and requests resource 0 through func_8014D6B8, and
 *           then steps the halfword 0x0A forward by one. The pointer cell
 *           0x80148210 is re-read after the dispatcher call, so the step it
 *           advances is the one the callee left behind. Takes no arguments and
 *           returns nothing; the 0x30-byte frame keeps $ra, $s1, which holds the
 *           4 the sixth argument needs, and $s0, which holds the selected entry
 *           index.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes, with the four live measurements of this
 * lift. The first body wrote the shared scratchpad cell through the
 * SPAD_PTR_SLOT helper and reused one pointer variable for the entry record and
 * for the tail step; it measured 55/70 instructions and 276 bytes. Building the
 * cell address from the constant forced `lui` plus `ori` plus a zero-offset
 * load where the original has the map-bound cell's `lui` plus offset load, and
 * naming that cell through the target's existing 0x1F800044 row
 * (workRecordCursor1F800044) instead restored the offset-load form: 62/70 at
 * 280 bytes. The tail then measured 69/70 once its re-read record pointer was
 * held in a variable of its own rather than in the entry pointer, because the
 * entry pointer is forced into $a0 at the dispatcher call and one shared
 * variable kept the reload there, where the original's separate variable lands
 * in $v1. Splitting the per-state load into a base local, `base =
 * (u8*)states[work[3]]` followed by `entry = base + *(u16*)(work + 0xA)`, closed
 * the last word: the single expression scheduled the halfword load one slot
 * ahead of the per-state load. Spellings that were measured and did not move
 * the result: a typed record struct, a u16* view of the cell and a hoisted
 * halfword local each measured identically to the 62/70 baseline, and an
 * inlined nested index expression and an explicit `entry + 2` dereference each
 * measured identically to the 69/70 baseline, so none of them is a lever here.
 */
void dispatchSelectedOffsetPairThenStepHalfword0aAndStoreDistanceAtEntry4World02Area07714_801F60C4(
    void) {
  u32* states;
  u8* work;
  u8* base;
  u8* record;
  u8* entry;
  u8 index;
  s32 value;
  u8* scratch;
  s32 distance;

  work = D_80148210;
  states = *(u32**)((u8*)D_8017F974[D_80143F00] + 0x10);
  base = (u8*)states[work[3]];
  entry = base + *(u16*)(work + 0xA);
  index = entry[2];
  if (func_801FD34C(work, D_801F6B60[index][0], D_801F6B60[index][1], 0x40, -0x400,
                    4, 0)) {
    record = D_80148210;
    value = *(u16*)(record + 0xA) - 2;
  } else {
    if (index == 4) {
      scratch = workRecordCursor1F800044;
      distance = func_8015477C(*(s32*)(scratch + 0x34), *(s32*)(scratch + 0x38));
      *(u16*)(workRecordCursor1F800044 + 0x3E) = distance;
      func_8014D6B8(0);
    }
    record = D_80148210;
    value = *(u16*)(record + 0xA) + 1;
  }
  *(u16*)(record + 0xA) = value;
}
