#include "bof3/bof3.h"

/* One record of the shared battle record table the main program indexes at
 * 0x801468FC. The scaled index of the original is a 0x98-byte stride (shift
 * left twice, add the index, shift left twice, subtract the index, shift left
 * three), the element size the reviewed D_8014688E battle scan records of the
 * main executable carry. Only the record address is used here: the argument
 * goes to the shared routine at 0x8015C264 and no field of the record is read
 * in this body. */
typedef struct Boss03316OwnerRecord {
  u8 bytes[0x98];
} Boss03316OwnerRecord;

extern Boss03316OwnerRecord D_801468FC[]; /* @source 0x801468FC @kind data */

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and the finish tail reaches that record byte 5 through this
 * cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle flag byte: the finish tail reads it as a byte and writes it
 * back with bit 0x2 raised. */
extern u8 D_80146328; /* @source 0x80146328 @kind unknown */

/* Shared battle accumulator: the main program adds the current enemy work
 * halfwords 0x86 and 0x84 into it while it finishes a battler. */
extern u32 D_8014632C; /* @source 0x8014632C @kind unknown */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and this target siblings read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared main-program readiness helper; its result is dropped here. */
extern void func_8014DAEC(void); /* @source 0x8014DAEC */

/* Shared main-program battle record scan: returns the index of the first
 * 0x98-byte record whose leading byte is 7 and whose byte 0x8C matches the
 * given owner id, or 0xFF when no record matches. */
extern u8 func_800AC7D0(u8 id); /* @source 0x800AC7D0 */

/* Shared main-program record routine at 0x8015C264: it takes a record address
 * plus six word-sized values and returns a status the caller tests for zero.
 * The roles of the six values are not established here. */
extern s32 func_8015C264(Boss03316OwnerRecord *record, s32 arg1, s32 arg2,
                         s32 arg3, s32 arg4, s32 arg5,
                         s32 arg6); /* @source 0x8015C264 */

/* Shared main-program battler pending-bit clear: clears one bit of the pending
 * bitset at 0x801463C2. */
extern void clearPendingBit(u32 bit); /* @source 0x801DE1B0 */

/* Shared main-program battle work reset: clears bytes 0x00 through 0x04 of the
 * scratch work area. */
extern void func_800ACFF4(void); /* @source 0x800ACFF4 */

/* @source 0x800C1BD4
 * @behavior Byte-1 handler entry 16 of this overlay code-pointer run based at
 * 0x800C275C, the table the overlay entry dispatcher func_800C1804 indexes with
 * work byte 1 scaled by four; run word 16 at 0x800C279C holds this address and
 * no jal in this payload names it. It runs the shared readiness helper at
 * 0x8014DAEC with its result dropped, resolves the owner-0 scan record index
 * through the shared battle record scan at 0x800AC7D0, and hands that index
 * 0x98-byte record of the table at 0x801468FC to the shared record routine at
 * 0x8015C264 together with 0, -0xE, 0x40, -0x400, 0xFF and 0. Only when that
 * routine returns zero does the finish tail run, in this order: it clears the
 * pending bit of byte +5 of the work object held by the scratchpad pointer cell
 * 0x1F800044, raises bit 0x2 of the shared battle flag byte D_80146328, adds
 * the current enemy work record halfword at byte offset 0x86, read through the
 * pointer published at 0x801EB4E8, into the shared accumulator D_8014632C, and
 * calls the shared battle work reset at 0x800ACFF4. The head and the tail are
 * the exact same shape the main executable lift of the boss overlay sibling at
 * 0x800C191C carries, which omits this accumulation; that sibling names the
 * pending-bit clear and the reset as the shared battle finish path. The roles
 * of the six values handed to 0x8015C264 and of the accumulator are not
 * established. It takes no arguments and returns nothing; the 0x28-byte frame
 * only keeps the return address at 0x20 above the three stack arguments of the
 * seven-argument call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runOwnerZeroRecordThenClearPendingBitSetFlagTwoAccumulateEnemyWorkHalfword86AndResetWorkBossBoss03316_800C1BD4(
    void) {
  u8 *flags;

  func_8014DAEC();

  if (func_8015C264(&D_801468FC[func_800AC7D0(0)], 0, -0xE, 0x40, -0x400, 0xFF,
                    0) == 0) {
    clearPendingBit(D_1F800044[5]);
    flags = &D_80146328;
    *flags |= 0x2;
    D_8014632C += FIELD_REF(u16, D_801EB4E8, 0x86u);
    func_800ACFF4();
  }
}
