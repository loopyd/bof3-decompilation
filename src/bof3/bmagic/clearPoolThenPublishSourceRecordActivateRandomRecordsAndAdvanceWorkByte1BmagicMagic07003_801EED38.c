#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The compiled original re-materializes and
 * reloads the cell (lui $vN,0x1F80 / lw $vN,0x44($vN)) before every access, so
 * the cell is reached through this address-derived symbol rather than through a
 * constant address, which the compiler materializes once and reuses. The exact
 * siblings of this target, setWorkByte9To8AndIncrementWorkByte1WhenWorkByteB
 * Below5BmagicMagic07003_801EF15C and decrementWorkByteARecordCountersAndWork
 * Byte9ThenSubmitRecordAndAdvanceWorkByte1OnZeroBmagicMagic07003_801EF1A4,
 * reach the same cell through the same name. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* The overlay's record pool: 128 records of 0x2C bytes based at 0x801F01B4,
 * spanning through 0x801F17B3 and ending exactly at the record pointer cell
 * 0x801F17B4 that the frame loop at 0x801EEC28 publishes a selected record
 * into. The original addresses the pool as a flat byte base plus a byte offset
 * scaled by 0x2C, so every field access below folds its own offset into the
 * relocation and each store re-materializes the base. The reserve helper
 * func_801F0000 indexes the same pool the same way, and the lifted sibling of
 * emi/bmagic/magic162/03 spells that shape as
 * reserveFirstFreeRecordBmagicMagic16203_801EFFFC. The trailing data block
 * carries no map row for it, so the reference binds through the target's
 * generated undefined-symbol table. */
extern u8 D_801F01B4[]; /* @source 0x801F01B4 @kind data */

/* Per-kind record count and per-kind level divisor, both byte tables indexed by
 * the work object's byte 0x4. The original loads each one as a byte through its
 * own base scaled by that byte, so both stay flat byte tables. */
extern u8 D_801F0198[]; /* @source 0x801F0198 @kind table */
extern u8 D_801F01A0[]; /* @source 0x801F01A0 @kind table */

/* Battle record published at 0x8014638C. This body copies its byte 0x8 and its
 * words 0x34, 0x38 and 0x3C into the work object; its role is not proven, so the
 * name stays address-anchored, exactly as the exact sibling
 * decrementWorkByteARecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWork
 * Byte1OnZeroBmagicMagic07003_801EF1A4 declares it. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared helper at 0x800AF764, called with no arguments here and declared with
 * the same no-argument shape by the sibling lifts of emi/bmagic that call it
 * (decayWorkHalfword2EThenDrawByWorkByteBAndRaiseFlag4WhenByte9ExpiresBmagic
 * Magic08000_801EECE4, dispatchWorkByte2ThenCallUpdatePassesBmagicMagic05003
 * _801EEF4C). */
void func_800AF764(void);

/* Shared helper at 0x8015DF18: include/bof3/battle/battle03_internal.h declares
 * `void func_8015DF18(u16 arg0)`, and the original passes the stage tag 0x100
 * with one `addiu $a0,$zero,0x100` in the call's delay slot. */
void func_8015DF18(u16 arg0);

/* Reserve helper of this overlay. It walks the same 0x801F01B4 pool for the
 * first record whose flag byte has bit 0 clear, sets that bit and returns the
 * record index, or 0xFF once all 128 records are taken; the call below masks the
 * result to a byte before comparing it against 0xFF. */
s32 func_801F0000(void);

/* PsyQ C runtime. Only the low two bits of the result are used. */
extern int rand(void);

/* @source 0x801EED38
 * @behavior Stage 0 of this overlay's entry-stage table: the nine-word pointer
 *           run at 0x801EEC04 carries 0x801EED38 as its first word, and the
 *           frame dispatcher func_801EEC28 calls that word through the work
 *           object's byte 0x1, so this body is the overlay's first step. It
 *           clears bytes 0x00, 0x01 and 0x02 of all 128 records of the
 *           0x2C-byte-stride pool at 0x801F01B4 with one `sb` per byte, stepping
 *           a byte offset by 0x2C and testing the masked counter against 0x80 at
 *           the loop bottom. It then clears work byte 0x4 and copies the battle
 *           record reached through the pointer at 0x8014638C into the work
 *           object: byte 0x8 and words 0x34, 0x38 and 0x3C, the last store
 *           filling the delay slot of the call to func_800AF764. It clears work
 *           byte 0xB, stores 8 into work byte 0x9, clears work byte 0xA and
 *           increments work byte 0x1, the entry-stage index the dispatcher
 *           selects with, so the overlay moves to stage 1. While the count
 *           D_801F0198[work->4] is nonzero it activates that many pool records:
 *           each iteration reserves one record through func_801F0000, skips it
 *           when the returned byte is 0xFF, and otherwise clears the record
 *           bytes at 0x01 and 0x02, publishes the work-object pointer into the
 *           record word at 0x28, stores the low two bits of rand() into byte
 *           0x03, copies work byte 0x4 into byte 0x04, stores the iteration
 *           ordinal into byte 0x07, stores D_801F01A0[work->4] scaled by the
 *           ordinal shifted right two, plus one, into byte 0x05, and increments
 *           work byte 0xB. The loop re-reads the count each time round. It
 *           finishes by calling func_8015DF18 with 0x100. Takes no arguments,
 *           returns nothing, and keeps a 0x20-byte frame for $ra and the two
 *           saved registers it uses. The first loop's byte offset is a local of
 *           its own and the activation loop's is `offset`: sharing one local
 *           would keep a single callee-saved register live across the call,
 *           while the original schedules the clearing loop's offset in a
 *           temporary. The activation loop binds the work-object pointer to
 *           `slot_work` before the two clears so its cell load is scheduled into
 *           the offset calculation, which is the order the original encodes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearPoolThenPublishSourceRecordActivateRandomRecordsAndAdvanceWorkByte1BmagicMagic07003_801EED38(
    void) {
  u8 *work;
  u8 *slot_work;
  u8 *source;
  u8 i;
  u8 index;
  u32 offset;
  u32 clear;

  i = 0;
  do {
    clear = i * 0x2C;
    i++;
    D_801F01B4[clear] = 0;
    D_801F01B4[clear + 1] = 0;
    D_801F01B4[clear + 2] = 0;
  } while (i < 0x80);
  D_1F800044[4] = 0;
  source = D_8014638C;
  D_1F800044[8] = source[8];
  work = D_1F800044;
  ((u32 *)work)[0xD] = ((u32 *)source)[0xD];
  ((u32 *)work)[0xE] = ((u32 *)source)[0xE];
  ((u32 *)work)[0xF] = ((u32 *)source)[0xF];
  func_800AF764();
  D_1F800044[0xB] = 0;
  D_1F800044[9] = 8;
  D_1F800044[0xA] = 0;
  D_1F800044[1]++;
  for (i = 0; i < D_801F0198[D_1F800044[4]]; i++) {
    index = func_801F0000() & 0xFF;
    if (index != 0xFF) {
      offset = index * 0x2C;
      slot_work = D_1F800044;
      D_801F01B4[offset + 1] = 0;
      D_801F01B4[offset + 2] = 0;
      *(u32 *)(D_801F01B4 + offset + 0x28) = (u32)slot_work;
      D_801F01B4[offset + 3] = rand() & 3;
      D_801F01B4[offset + 4] = D_1F800044[4];
      D_801F01B4[offset + 7] = i;
      D_801F01B4[offset + 5] = (D_801F01A0[D_1F800044[4]] * (i >> 2)) + 1;
      D_1F800044[0xB]++;
    }
  }
  func_8015DF18(0x100);
}
