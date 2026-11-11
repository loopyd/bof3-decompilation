#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original
 * per-access re-materialization (lui $v1,0x1F80 / lw $v1,68($v1)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Unsigned halfword row table of this payload's own textbin tail at 0x801F4A68:
 * row k is the term the body subtracts from the halved position distance before
 * storing record halfword +0x3E. Four rows of two bytes, so the body scales the
 * row index by two.
 * @source 0x801F4A68 @kind data
 */
extern u16 D_801F4A68[];

/* Word row table of the same tail at 0x801F4A70: row k is the value the body
 * stores into record word +0x20.
 * @source 0x801F4A70 @kind data
 */
extern u32 D_801F4A70[];

/* First byte column of the stride-2 byte pair table of the same tail at
 * 0x801F4A80: row k is the pair (0x801F4A80[2k], 0x801F4A81[2k]), and the body
 * republishes it into the record halfwords +0x36 and +0x3A.
 * @source 0x801F4A80 @kind data
 */
extern u8 D_801F4A80[];

/* Second byte column of the same stride-2 byte pair table.
 * @source 0x801F4A81 @kind data
 */
extern u8 D_801F4A81[];

/* Distance helper of the shared main executable: it takes the record word pair
 * at +0x34 and +0x38 and answers a signed halfword, which the original
 * sign-extends with the `sll`/`sra` pair before negating it.
 * @source 0x8015477C
 */
extern s16 func_8015477C(s32 x, s32 y);

/* Render helper of this payload, still an `asm` span at 0x801F31BC at the time
 * of this lift: the handlers of the state-byte-2 runs of this payload end in
 * it, and it takes no arguments and answers nothing.
 * @source 0x801F31BC
 */
extern void func_801F31BC(void);

/* @source 0x801F3624
 * @behavior Entry 0 of the state-byte-2 handler run based at 0x801F4AA0 of this
 *           payload, whose dispatcher is 0x801F35E0. Reads the work record
 *           published through the scratchpad pointer cell 0x1F800044: takes the
 *           signed halfword at +0x3A and stores its zero test into record byte
 *           +0x08, and takes the signed halfword at +0x36 as the row index.
 *           Republishes record halfwords +0x36 and +0x3A from the stride-2 byte
 *           pair table at 0x801F4A80 and 0x801F4A81, both at two times the row
 *           index. Hands the record position pair at +0x34 and +0x38 to the
 *           shared distance helper func_8015477C, halves the returned signed
 *           distance with the sign-fixup sequence, subtracts the unsigned row
 *           term D_801F4A68[index] and stores that into record halfword +0x3E.
 *           Stores the word row term D_801F4A70[index] into record word +0x20,
 *           clears record halfword +0x30, increments record byte +0x02 so the
 *           next dispatch of the same run takes its entry 1 at 0x801F36FC, and
 *           calls the payload render helper 0x801F31BC on every call. Takes no
 *           arguments and returns nothing. The 0x20-byte frame only keeps $ra,
 *           $s1 and $s0, the first cell load runs before the frame is created,
 *           and the cell is re-read after the byte store and after the distance
 *           call.
 *
 * Name evidence: 0x801F4AA0 is the handler run of the state-byte-2 dispatcher
 * 0x801F35E0 and its word 0 is this body, so the handler is the first
 * state-byte-2 handler of that run. The record halfwords +0x36 and +0x3A are the
 * halfword pair this payload's exact sources leave address-anchored, and
 * 0x801F4A80/0x801F4A81 are the stride-2 byte columns of one row table in this
 * payload's own textbin tail, the shape the exact sibling
 * setWork36And3aFromRowTableStorePositionDistanceQueueSharedUiElementPairThenAdvanceStepWorld04Area16813_801F44F4
 * reads from its own tail tables. The record words +0x34 and +0x38 are the
 * fixed-point position pair of that same scratchpad record, 0x8015477C is the
 * distance helper the plchar and world families declare as a signed distance
 * over a coordinate pair, and the exact world02/area103/13 source
 * storeWorkPositionDistanceAt3eWorld02Area10313_801F57FC names the value it
 * stores at +0x3E a work position distance, which is the vocabulary this name
 * uses for the term derived here. 0x801F4A70 is the word row table of the same
 * tail, so the value it republishes into +0x20 stays a row word. The +0x20 and
 * +0x30 roles are not established, so the name leaves them address-anchored.
 *
 * Shape evidence: the exact sibling 0x801F44F4 of world04/area168/13 shares this
 * body from its pre-frame cell load to the +0x20 store, differing only in its
 * payload-local table words, and its in-target asm sibling 0x801F3424 repeats
 * the same prefix; this span then ends in the render call instead of that
 * sibling's byte +0x0B store and shared marker queues, which is the one store
 * (plus two calls) only the sibling carries. No byte-identical twin exists: a
 * 216-byte scan of every payload under out/binaries finds no equal span, so the
 * clone lever is declined and this body is written from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWork36And3aFromRowTableStorePositionDistanceAndRowWordThenAdvanceWorkByte2World04Area16713_801F3624(
    void) {
  s32 y;
  s32 x;
  s16 distance;

  y = FIELD_REF(s16, D_1F800044, 0x3A);
  x = FIELD_REF(s16, D_1F800044, 0x36);
  D_1F800044[8] = (y == 0);
  FIELD_REF(u16, D_1F800044, 0x36) = D_801F4A80[x * 2];
  FIELD_REF(u16, D_1F800044, 0x3A) = D_801F4A81[x * 2];
  distance = func_8015477C(FIELD_REF(u32, D_1F800044, 0x34),
                           FIELD_REF(u32, D_1F800044, 0x38));
  FIELD_REF(u16, D_1F800044, 0x3E) = -distance / 2 - D_801F4A68[x];
  FIELD_REF(u32, D_1F800044, 0x20) = D_801F4A70[x];
  FIELD_REF(u16, D_1F800044, 0x30) = 0;
  D_1F800044[2]++;
  func_801F31BC();
}
