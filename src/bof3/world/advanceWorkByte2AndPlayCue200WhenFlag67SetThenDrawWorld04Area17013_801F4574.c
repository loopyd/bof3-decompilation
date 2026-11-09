#include "bof3/bof3.h"

/* Shared main-RAM flag bank of the main executable, outside this overlay's
 * payload. Only its address is used, as the base argument of the shared flag
 * tester func_8015B5D4. Splat binds the address through this target's
 * generated undefined-syms map, so no target-map row is needed. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Front-end phase byte of the main executable, outside this overlay's
 * payload. The cue below only fires while the front end holds phase 0. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared flag tester of the main executable, outside this overlay's payload:
 * reports whether the flag whose index is the second argument is set in the
 * bank the first argument points at. The sibling world targets declare the
 * same prototype. */
s32 func_8015B5D4(u32 bank, s32 flag);

/* Sound-cue dispatcher of the main executable, outside this overlay's
 * payload. */
void func_8015DF18(u16 cue);

/* Draw routine of this overlay, still an `asm` span of this target at
 * 0x801F4640. Its argument is stored once and tested with `beqz`, so a
 * non-zero value enables a second rendering pass; the sibling dispatch
 * entries at 0x801F453C and 0x801F45E4 pass zero. */
void func_801F4640(s32 variant);

/* @source 0x801F4574
 * @behavior Byte-2 state handler gated on flag 0x67. Asks the shared flag
 *           tester func_8015B5D4 whether flag 0x67 is set in the main-RAM flag
 *           bank D_80144F28. When the flag is clear the handler only draws.
 *           When it is set, the front-end phase byte D_80143BB0 gates sound
 *           cue 0x200 through func_8015DF18 and the record's step byte at
 *           offset 0x02 of the work record published at the scratchpad pointer
 *           cell 0x1F800044, the byte the overlay dispatcher indexes, is
 *           advanced by one so the next dispatch takes the following entry.
 *           The pointer cell is re-read after the cue gate, as `lui` plus the
 *           offset load. The draw routine at 0x801F4640 then runs with
 *           argument 1, the variant flag its own body tests to add its second
 *           pass; that call runs on every call, including the frames that
 *           leave the flag clear. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra at 0x10.
 *
 * Shape evidence: entry 4 of the same handler table at 0x801F4080 gates the
 * same byte-2 advance on a halfword countdown instead of a flag and draws with
 * 0x801F40F0; entry 1 at 0x801F3DE8 keeps the D_80143BB0 cue gate, the cue
 * 0x200 and the reloaded pointer cell around the increment. Its neighbour
 * 0x801F453C at 0x801F453C advances the same byte against no gate at all and
 * passes zero to the same draw routine. The near twin
 * emi/world03/area140/13 0x801F2FEC runs the identical statement order with
 * flag 0x54, cue 0x200 and its own draw callee, but stores the constant 2
 * instead of incrementing; emi/world03/area148/13 0x801F36A0 runs the gate
 * with flag 0x5A and cue 0x208 and does increment, then takes two calls. All
 * three differ in at least the stored literals and a call, so they are shape
 * evidence, not clone proof, and none of them is a lifted `c` span to clone.
 * The byte 0x67 immediate appears in only two spans of this payload and in
 * one unrelated bmagic span repo-wide, so this body has no byte-identical
 * sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndPlayCue200WhenFlag67SetThenDrawWorld04Area17013_801F4574(
    void) {
  if (func_8015B5D4((u32)&D_80144F28, 0x67) != 0) {
    u8* cursor;

    if (D_80143BB0 == 0) {
      func_8015DF18(0x200);
    }
    /* The original reloads the cursor cell after the cue call and keeps that
     * pointer in its own register, so the increment takes a local of its own
     * instead of a pointer hoisted above the gate. */
    cursor = *(u8**)SPAD_ADDR(u8, 0x44u);
    cursor[2]++;
  }
  func_801F4640(1);
}
