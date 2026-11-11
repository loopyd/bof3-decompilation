#include "bof3/bof3.h"

/* Work-area record pointer cell of the shared main executable, immediately
 * before the D_80146888 work-area array. It is outside this overlay's image, so
 * the neighbouring lifts of this overlay family read the same cell.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;

/* Selector byte +0x79 of the first 0x140-stride work record of the shared run
 * at 0x80145E90. It is outside this overlay's image, and the shared-executable
 * battle overlay names the same byte as that record's kind byte.
 * @source 0x80145F09 @kind data
 */
extern u8 D_80145F09;

/* Flag bank of the shared main executable that the shared bit helper below
 * writes. It is outside this overlay's image, so the exact lift of the
 * neighbouring overlay family that reaches the same helper keeps the same cell.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28[];

/* Scratchpad work-record pointer cell. This target's other exact lifts of the
 * same overlay read the same cell under this name.
 * @source 0x1F800044 @kind data
 */
extern u16* g_WorkRecordWorld03Area14013;

/* Shared bit helper of the main executable: writes the bit its second argument
 * selects in the flag bank its first argument addresses.
 * @source 0x8015B580
 */
extern void func_8015B580(void* bitmap, s32 bit_index);

/* Shared two-argument request helper of the main executable. This body passes
 * two small positive constants, so the arguments stay anonymous.
 * @source 0x8015507C
 */
extern void func_8015507C(s32 arg0, s32 arg1);

/* Clears bit 0 of the work record byte at +0x74, then gates one teardown run on
 * the shared work-record kind byte.
 *
 * Shape evidence: overlay World02 area108 target emi/world02/area108/14 holds
 * func_801F4A98 at src/bof3/world/
 * clearGlobalWorkFlagBit0AndResetScratchModeWhenMappedState5World02Area10814_801F4A98.c,
 * an exact lift whose body is this one statement for statement: the record
 * pointer cell load hoisted above the frame creation, the +0x74 read, the 0xFE
 * mask and the store back, one gated block with the shared flag bank call, the
 * two-argument request call and the scratchpad byte store, then the single
 * epilogue. Only its gate expression and its passed constants differ, so that
 * source supplied the statement order, the pointer-local spelling and the tail,
 * while every literal here is written from this span's own bytes: flag bit 0x54
 * rather than 0x30, request arguments 2 and 7 rather than 1 and 0x0B, and its
 * `D_80181B10[D_80145F09] == 5` table test replaced by this span's own three
 * equality tests of the kind byte.
 *
 * The gate reads the kind byte three times in source, but the original holds
 * one `lbu` of it, so common-subexpression elimination collapses the three
 * reads to that one load. The two tests against 5 and 6 fold into the original
 * `addiu -5` plus `sltiu 2` range check on the loaded value; the test against 7
 * takes the separate `bne` whose compared value is zero-extended by the `andi
 * 0xFF` in the first branch delay slot. That mask belongs to the comparison
 * chain on a byte lvalue, not to a source mask: overlay World00 area030 target
 * emi/world00/area030/04 holds the same idiom at func_801DC93C, an `lbu` into
 * the range check followed by the same `andi 0xFF` and equality chain, and it
 * is why the hand-written spellings that mask a wide local or cast one measure
 * the mask away.
 *
 * Clone declined: a little-endian scan of all 396 extracted payloads under
 * out/binaries for this 0x84-byte body, unmasked and again with its two `jal`
 * immediate words masked so relocation-different siblings could match, returned
 * only this span's own offset in its own payload.
 *
 * The caller is the pointer word at 0x801F4A1C of this payload's script blob, so
 * no `jal` reaches this body and its argument roles stay structural.
 *
 * @source 0x801F3DAC
 * @behavior Clears bit 0 of the byte at +0x74 of the work record published by
 *           the shared pointer cell 0x80146884, then tests the shared
 *           work-record kind byte at 0x80145F09 against 5, 6 and 7. When it
 *           reads any of those three values it sets bit 0x54 of the shared flag
 *           bank at 0x80144F28 through the shared helper 0x8015B580, calls the
 *           shared request helper 0x8015507C with 2 and 7, and clears byte 0 of
 *           the work record published by the scratchpad pointer cell
 *           0x1F800044. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte74Bit0AndSetSharedFlagBankBit54ThenResetScratchWorkByteWhenWorkRecordKind5To7World03Area14013_801F3DAC(void) {
    u8* record;

    record = D_80146884;
    record[0x74] &= ~1u;
    if (D_80145F09 == 5 || D_80145F09 == 6 || D_80145F09 == 7) {
        func_8015B580(D_80144F28, 0x54);
        func_8015507C(2, 7);
        ((u8*)g_WorkRecordWorld03Area14013)[0] = 0;
    }
}
