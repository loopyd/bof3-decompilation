#include "bof3/bof3.h"

/* Shared main-RAM flag bank that func_8015B5D4 tests one bit of. This overlay
 * only passes its address, so the byte view is enough here.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Scratchpad pointer cell holding the work record this overlay steps. The
 * target map binds it under this name, and the neighbouring lifts of this
 * overlay read it as a halfword record.
 * @source 0x1F800044 @kind data
 */
extern u16 *g_WorkRecordWorld03Area14013;

/* Byte +8 of the shared work-record run based at 0x80145E90: the mode byte the
 * neighbouring lifts copy into work byte 8.
 * @source 0x80145E98 @kind data
 */
extern u8 D_80145E98;

/* Byte +9 of the same run: the flag the original tests before it treats the
 * mode byte as an active direction.
 * @source 0x80145E99 @kind data
 */
extern u8 D_80145E99;

/* Byte +0x12B of the same run: the record-kind byte the shared lifts of the
 * game00 and area09113 targets publish from a per-kind table, so the value 1
 * tested here is a kind code, not a flag.
 * @source 0x80145FBB @kind data
 */
extern u8 D_80145FBB;

/* Shared work-gate halfword in main RAM at 0x80145AA8: the pressed-pad-state
 * word the ui and bmagic lifts test, and the first side of the pair this body
 * intersects with the mask at 0x80145AC0.
 * @source 0x80145AA8 @kind data
 */
extern u16 D_80145AA8;

/* Second work-gate halfword of that pair.
 * @source 0x80145AC0 @kind data
 */
extern u16 D_80145AC0;

/* Pointer cell of the shared main-RAM work record. It is outside this overlay's
 * image, and the store through the loaded pointer may alias the cell, so the
 * original reloads it for the helper argument.
 * @source 0x80148210 @kind data
 */
extern u8 *D_80148210;

/* Shared route-step byte pair in main RAM: entry n of the walk direction table
 * adds its first byte to the first coordinate and its second byte to the
 * second, and both are loaded unsigned and then sign-extended, so they are
 * signed steps held in a signed byte array.
 * @source 0x80181B00 @kind table
 */
extern s8 D_80181B00[];

/* Second byte of the same route-step pair, addressed from its own base so each
 * access re-materialises the base register as the original does.
 * @source 0x80181B01 @kind table
 */
extern s8 D_80181B01[];

/* Local step-cue pair table inside this payload, two bytes per walk direction
 * quadrant: the first byte is the resource id the move requests and the second
 * is the foot side byte the move stores at +0x2A of the work record. Its eight
 * entries read 08 00 07 00 07 01 08 01.
 * @source 0x801F4A6C @kind table
 */
extern u8 D_801F4A6C[];

/* Second byte of the same step-cue pair.
 * @source 0x801F4A6D @kind table
 */
extern u8 D_801F4A6D[];

/* Shared main-executable flag test: nonzero when the addressed bit of the flag
 * bank is set. Outside this payload, so only a prototype is declared here.
 * @source 0x8015B5D4
 */
extern u32 func_8015B5D4(void *flag_bank, u8 bit_index);

/* Shared main-executable resource request keyed by the id byte it is passed.
 * @source 0x8015BAC4
 */
extern void func_8015BAC4(u8 resource);

/* Shared main-executable helper that arms the shared work record for the level
 * byte passed as its second argument.
 * @source 0x801ABBA8
 */
extern void func_801ABBA8(u8 *record, u8 level);

/* Shared main-executable resource request that takes the resource id byte.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 resource);

/* Movement probe of this overlay: nonzero unless the projected column and row
 * reach a map cell that holds the code 0x89.
 * @source 0x801F4058
 */
extern s32 testMoveProbeAndMapCellCodeNot89World03Area14013_801F4058(s32 col,
                                                                   s32 row);

/* Local trigger-record lookup of this overlay: it scans the eight six-byte
 * records at 0x801F4A84 for one whose column byte and row byte equal the
 * masked arguments and whose third byte low nibble equals the mode byte
 * 0x80145E98, then acts on the matched record. Both arguments are compared
 * after masking to their low byte, which is why the caller passes them as
 * unsigned halfwords.
 * @source 0x801F4380
 */
extern void func_801F4380(u16 col, u16 row);

/* @source 0x801F3E30
 * @behavior Walk tick of this overlay. It runs only while flag 0x66 of the main
 *           RAM flag bank 0x80144F28 is set: it copies the shared mode byte
 *           0x80145E98 into work byte 8, projects the walk step by adding the
 *           signed route-step pair of 0x80181B00 / 0x80181B01, selected by that
 *           byte doubled, to the work halfwords +0x3A and +0x36, and keeps the
 *           projected column and row for the tail call. When the record flag
 *           byte 0x80145E99 is set and the record-kind byte 0x80145FBB reads 1,
 *           it probes the projected cell through 0x801F4058 and, when that
 *           probe reports the blocked code and the projected column lies within
 *           0x44..0x50 and the projected row within 0x06..0x10, stores 2 into
 *           byte 7 of the record published through 0x80148210 and arms that
 *           record for work byte 8. Otherwise it requests the resource named by
 *           work byte 8 through 0x8015BAC4. Then, when the work-gate halfwords
 *           0x80145AA8 and 0x80145AC0 share a set bit, it sets bit 0x40 of the
 *           shared record's byte 0 and bit 0x08 of work byte 7, requests the
 *           step cue of the facing quadrant from the local pair table at
 *           0x801F4A6C, stores that pair's second byte at +0x2A of the work
 *           record and passes the projected cell to the local trigger lookup at
 *           0x801F4380 as unsigned halfwords. Every other path, including a
 *           clear flag 0x66 and a clear gate pair, only adds 4 to the halfword
 *           at +0x0A of the record published through 0x80148210. Takes no
 *           arguments and returns nothing.
 *
 *           Name evidence: the body owns the projected cell (the two additions
 *           above), the local trigger lookup it drives and the +0x0A halfword
 *           it advances, and no evidence names that halfword's role, so it
 *           stays address-anchored in the name.
 *
 *           Shape evidence: the projected coordinates are held in signed
 *           halfword locals. That width is what the body shows: the probe call
 *           sign-extends each one with sll 16 / sra 16, the trigger call masks
 *           each with andi 0xFFFF, and the range tests subtract the bound
 *           before masking, so no wider value is ever read. It is also the
 *           width that reproduces the route-step loads, which are lbu followed
 *           by sll 24 / sra 24: as s32 locals they compiled to a single lb. The
 *           exact battle15 siblings func_8009B700 and func_8009B794 add the
 *           same signed-byte table element to halfword cells with that same
 *           three-instruction extension.
 *
 *           A little-endian scan of every payload under out/binaries with the
 *           jal immediate words masked found this 0x228 byte span only at
 *           0x801F3E30 of this payload, so clone was declined;
 *           the body also carries its own table base 0x801F4A6C as a data
 *           immediate, which no twin could share. It opens on addiu sp,-0x28 and
 *           returns through jr $ra at 0x801F4050, so the data lever did not
 *           apply, and this target's own data-candidate ranking lists no range
 *           at all. The span starts at its own declared boundary 0x801F3E30 and
 *           ends at the next declared entry 0x801F4058, so carve had no fragment
 *           boundary to regroup. No jal and no table word of this payload names
 *           this address; the only reference is the word at 0x801F4A20 of the
 *           local pointer run 0x801F4A1C..0x801F4A24, beside the already
 *           claimed spans 0x801F3DAC and 0x801F3D4C, so the body is reached as
 *           a handler rather than by a call site.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectFacingStepAndDispatchLocalTriggerAtCellElseAdvanceSharedRecordHalfword0aWorld03Area14013_801F3E30(void) {
    u32 offset;
    u32 index;
    s16 col;
    s16 row;

    if (func_8015B5D4(&D_80144F28, 0x66) != 0) {
        ((u8 *)g_WorkRecordWorld03Area14013)[8] = D_80145E98;
        offset = ((u8 *)g_WorkRecordWorld03Area14013)[8] * 2;
        col = g_WorkRecordWorld03Area14013[0x1B] + D_80181B00[offset];
        row = g_WorkRecordWorld03Area14013[0x1D] + D_80181B01[offset];
        if (D_80145E99 != 0 && D_80145FBB == 1) {
            if ((u8)testMoveProbeAndMapCellCodeNot89World03Area14013_801F4058(
                    col, row) == 0) {
                if ((u16)(col - 0x44) < 0xD) {
                    if ((u16)(row - 6) < 0xB) {
                        D_80148210[7] = 2;
                        func_801ABBA8(D_80148210,
                                      ((u8 *)g_WorkRecordWorld03Area14013)[8]);
                    }
                }
            }
        } else {
            func_8015BAC4(((u8 *)g_WorkRecordWorld03Area14013)[8]);
        }
        index = (((u8 *)g_WorkRecordWorld03Area14013)[8] & 7) >> 1;
        if ((D_80145AA8 & D_80145AC0) != 0) {
            D_80148210[0] |= 0x40;
            ((u8 *)g_WorkRecordWorld03Area14013)[7] |= 8;
            func_8014D6B8(D_801F4A6C[index * 2]);
            ((u8 *)g_WorkRecordWorld03Area14013)[0x2A] = D_801F4A6D[index * 2];
            func_801F4380((u16)col, (u16)row);
            return;
        }
    }
    *(u16 *)(D_80148210 + 0x0A) += 4;
}
