#include "bof3/bof3.h"

/* Work-record pointer cell in main RAM, outside this overlay's image. The
 * opening access reads it as the helper's record argument and the store after
 * the call re-reads the cell, which is what the original does because a store
 * through the loaded pointer may alias the cell. The neighbour world lifts
 * declare the same cell as u16*; this step only reaches its byte 7, so this
 * target declares the byte view.
 * @source 0x80148210 @kind unknown
 */
extern u8* D_80148210;

/* Shared main-executable helper that arms the work record for the level byte
 * passed as its second argument. It is outside this overlay's image, so only a
 * prototype is declared here.
 */
extern void func_801ABBA8(u8* record, u8 level);

/* @source 0x801F2CD8
 * @behavior Arms the shared work record published through the global pointer
 *           cell 0x80148210 for the current step: it loads the scratch work
 *           record published at the scratchpad pointer cell 0x1F800044 once,
 *           passes the work-record pointer as the first argument and that
 *           record's byte 8 as the second to the shared func_801ABBA8 helper,
 *           and then reloads the 0x80148210 pointer cell and stores 2 into byte
 *           7 of the record. The scratch cell is read before the record cell
 *           because the original loads it ahead of the record, so one u8* local
 *           holds the scratch record and the record pointer is passed inline.
 *           Work byte 0 is the flag byte the helper sets bit 0x10 of, and byte 8
 *           is the level index the neighbouring incrementWorkByte8Modulo8
 *           (0x801F2E18) cycles 0..7, so byte 7 and byte 8 keep their original
 *           offsets. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armWorkWithScratchByte8AndSetByte7To2World00Area00013_801F2CD8(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  func_801ABBA8(D_80148210, work[8]);
  D_80148210[7] = 2;
}
