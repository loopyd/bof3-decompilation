#include "bof3/bof3.h"

/* One record of the shared battle record table the main program indexes at
 * 0x801468FC. The scaled index of the original is a 0x98-byte stride
 * (sll 2, add index, sll 2, sub index, sll 3), the same element size the
 * reviewed D_8014688E battle scan records of the main executable carry. Only
 * the record address is used here: the argument goes to the shared routine at
 * 0x8015C264 and no field of the record is read in this body. */
typedef struct Boss00216OwnerRecord {
  u8 bytes[0x98];
} Boss00216OwnerRecord;

extern Boss00216OwnerRecord D_801468FC[]; /* @source 0x801468FC @kind data */

/* Work-object pointer cell in scratchpad; the original reads the cell once, in
 * the body below the shared routine's success test. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle flag byte: the original reads it as a byte and writes it back
 * with bit 0x2 raised. */
extern u8 D_80146328; /* @source 0x80146328 @kind unknown */

/* Shared main-program readiness helper; its result is dropped here. */
void func_8014DAEC(void); /* @source 0x8014DAEC */

/* Shared main-program battle record scan: returns the index of the first
 * 0x98-byte record whose leading byte is 7 and whose byte +0x8C matches the
 * given owner id, or 0xFF when no record matches. */
u8 func_800AC7D0(u8 id); /* @source 0x800AC7D0 */

/* Shared main-program record routine at 0x8015C264. Its role is not
 * established here: it takes a record address plus six word-sized values and
 * returns a status the caller tests for zero. The same record, the same
 * argument set 0, -0xE, 0x40, -0x400, 0xFF, 0 and the same success test appear
 * at 0x800C1BD4 of BIN/BOSS/BOSS033.EMI#16, and an overlay of the game
 * directory family at 0x801A9944 passes script bytes through the same six
 * slots, so only the argument shape is known. */
s32 func_8015C264(Boss00216OwnerRecord *record, s32 arg1, s32 arg2, s32 arg3,
                  s32 arg4, s32 arg5, s32 arg6); /* @source 0x8015C264 */

/* Shared main-program battler pending-bit clear: clears one bit of the pending
 * bitset at 0x801463C2. */
void clearPendingBit(u32 bit); /* @source 0x801DE1B0 */

/* Shared main-program battle work reset: clears bytes 0x00 through 0x04 of the
 * scratch work area. */
void func_800ACFF4(void); /* @source 0x800ACFF4 */

/* @source 0x800C191C
 * @behavior Handler word of this overlay battle work state machine, referenced
 * only from the handler word 0x800C1FF0 inside the payload data row 0x800C1F9C
 * and reached by no jal in the payload, so the prologue and the return path
 * alone place its boundary. It runs the shared readiness helper at 0x8014DAEC
 * with its result dropped, resolves the owner-0 scan record index through the
 * shared battle record scan at 0x800AC7D0, and hands that index's 0x98-byte
 * record of the table at 0x801468FC to the shared record routine at 0x8015C264
 * together with 0, -0xE, 0x40, -0x400, 0xFF and 0. Only when that routine
 * returns zero does the tail run: it clears the pending bit of byte +5 of the
 * work object held by the scratchpad pointer cell 0x1F800044, raises bit 0x2
 * of the shared battle flag byte D_80146328 and calls the shared battle work
 * reset at 0x800ACFF4. That tail is the same triple the exact main-program
 * lift at 0x801E51DC runs when it finishes a battler, so it is read here as the
 * shared battle finish path; roles of the six values handed to 0x8015C264 are
 * not established. It takes no arguments and returns nothing. The body is the
 * same shape as the still-unlifted handler at 0x800C1BD4 of
 * BIN/BOSS/BOSS033.EMI#16, which carries the identical head and tail and only
 * inserts an accumulation of the current enemy work halfword into 0x8014632C
 * before the reset call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runOwnerZeroRecordThenClearPendingBitAndSetFlagTwoBossBoss00216_800C191C(
    void) {
  u8 *flags;

  func_8014DAEC();

  if (func_8015C264(&D_801468FC[func_800AC7D0(0)], 0, -0xE, 0x40, -0x400,
                    0xFF, 0) == 0) {
    clearPendingBit(D_1F800044[5]);
    flags = &D_80146328;
    *flags |= 0x2;
    func_800ACFF4();
  }
}
