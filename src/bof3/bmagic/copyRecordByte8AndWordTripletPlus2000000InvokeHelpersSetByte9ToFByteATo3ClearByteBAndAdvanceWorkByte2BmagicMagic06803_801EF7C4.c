#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name the exact bmagic lifts of this family use for the
 * same main-executable cell. The original materialises the cell with its own
 * lui / lw pair before each dereference, because every read after the first is
 * separated from the previous one by a store through the pointer just loaded;
 * the body therefore leaves every read at its own source site instead of
 * caching the cell in one local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pointer cell at 0x801EB4E0 of the main executable: a word holding the
 * address of the record whose byte 8 and words 0x34, 0x38 and 0x3C this step
 * copies into the work object. It sits outside this overlay image and is spelled
 * in its raw address form, so it binds without a row in this target's map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Main-executable helper this step calls with no arguments before it seeds the
 * work bytes. Outside the payload, so it keeps a raw origin-tagged prototype and
 * no map row, and its role stays address-anchored. */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */

/* Second main-executable helper, taking the single immediate 0x100. Its role is
 * unestablished for the same reason as the helper above. */
extern void func_8015D71C(u16 arg0); /* @source 0x8015D71C @kind function */

/* @source 0x801EF7C4
 * @behavior First handler of this overlay's work-byte-2 step dispatch and second
 *           entry of its work-byte-1 dispatch. The payload holds no jal word for
 *           this address: the only in-image references are the dispatch words at
 *           0x801EFCCC (entry 0 of the run the dispatcher at 0x801EF780 indexes
 *           with work byte 2) and 0x801EFCC8 (entry 1 of the run the dispatcher
 *           at 0x801EF73C indexes with work byte 1), both read from the data
 *           blob at the end of the image. Copies byte 8 of the record published
 *           by the pointer cell 0x801EB4E0 into work byte 8, then re-reads both
 *           pointer cells and copies the record words 0x34, 0x38 and 0x3C into
 *           the same three work words, adding 0x2000000 to the third copy. Both
 *           pointer cells are re-read at their own sites because the byte store
 *           in between may alias them, and each region keeps its own pair of
 *           locals: the original loads the first pair into $v0 and $v1 and the
 *           second into $v1 and $a0, which one shared pair of locals does not
 *           reproduce (it measured 42 of 48 instructions, the first six register
 *           names only). Calls 0x800AF764 with no arguments and then 0x8015D71C
 *           with 0x100, ignoring both results, clears work byte 0xB, sets work
 *           byte 9 to 0xF and work byte 0xA to 3, and increments work byte 2, the
 *           selector the 0x801EF780 dispatcher reads. Frame 0x18 keeps only $ra;
 *           takes no arguments, returns nothing. The roles of record byte 8, of
 *           record words 0x34/0x38/0x3C beyond being copied (the 0x2000000 bias
 *           the third copy takes is unknown) and of both helpers stay
 *           unestablished, as do work bytes 9, 0xA and 0xB beyond the literals
 *           stored here. No byte-identical copy of this 192-byte span exists in
 *           any payload under out/binaries: a payload scan found the span only in
 *           this target's own binary, so no sibling seeds it and the statement
 *           order is read from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordByte8AndWordTripletPlus2000000InvokeHelpersSetByte9ToFByteATo3ClearByteBAndAdvanceWorkByte2BmagicMagic06803_801EF7C4(void) {
    u8 *record;
    u8 *work;
    u8 *src;
    u8 *dst;

    record = D_801EB4E0;
    work = g_battle_work;
    work[8] = record[8];
    src = D_801EB4E0;
    dst = g_battle_work;
    *(u32 *)(dst + 0x34) = *(u32 *)(src + 0x34);
    *(u32 *)(dst + 0x38) = *(u32 *)(src + 0x38);
    *(u32 *)(dst + 0x3C) = *(u32 *)(src + 0x3C) + 0x2000000;
    func_800AF764();
    func_8015D71C(0x100);
    g_battle_work[0xB] = 0;
    g_battle_work[9] = 0xF;
    g_battle_work[0xA] = 3;
    g_battle_work[2]++;
}
