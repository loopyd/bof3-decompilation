#include "bof3/bof3.h"

/* Per-route offset run the work record's projected row is built from. The
 * original scales the route index by eight bytes, so each route owns two words
 * and this target reads the first of the pair. */
extern s32 D_80181B98[];

/* Byte table inside this payload's trailing data run, addressed at the very
 * end of the image. It is indexed by the shared companion byte 0x801448ED and
 * only the original bytes prove the access: the table's own role beyond one
 * byte per mode is not established, so it stays address-anchored. */
extern u8 D_801F6898[];

/* Shared companion byte of the state-entry tuple 0x801448EB. Other paths in
 * the world family reuse the same cell as a record row index, which is why the
 * gate below reads it as this step's table index and not as a state id. */
extern u8 D_801448ED;

/* Pointer cell of the shared record this step arms and then counts down. The
 * record's byte 7 and halfword 0x0A are the only cells the body reaches, so the
 * byte view is the one declared; the neighbouring world lifts declare the same
 * cell as u16* for the halfword alone. */
extern u8* D_80148210;

/* Shared main-executable helper that arms the record for the level byte passed
 * as its second argument. It lives outside this overlay's image, so only a
 * prototype is declared here. */
extern void func_801ABBA8(u8* record, u8 level);

/* @source 0x801F4840
 * @behavior Overlay step handler. It builds the work record's projected row by
 *           loading the scratch work record published at the scratchpad pointer
 *           cell 0x1F800044, masking that record's byte 8 with 0x07 and adding
 *           the corresponding two-word entry of the route offset run
 *           0x80181B98 to the record's word 0x38 (`lbu`/`andi 0x7`/`sll 3`,
 *           `lw 0x38`, `addu`). The gate shifts the byte of the table
 *           0x801F6898 selected by the shared companion byte 0x801448ED left by
 *           16 and compares it against that row with `sltu`, so the comparison
 *           is unsigned and fixed-point: the step returns (`bnez` to the single
 *           `lw $ra`/`addiu $sp` epilogue) whenever the projected row is above
 *           the mode's limit. The surviving path stores 1 into byte 7 of the
 *           record published at the global pointer cell 0x80148210, passes that
 *           record with level 5 to the shared helper func_801ABBA8 and, after a
 *           fresh load of the pointer cell, subtracts 2 from the record's
 *           halfword at 0x0A. Each of the three record accesses reloads the
 *           pointer cell because the byte store through the loaded pointer may
 *           alias it and the call may write it. The 0x18-byte frame keeps only
 *           $ra, and the constant 1 is materialised in the gate's branch delay
 *           slot. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x80148210 is the shared armed-record pointer cell of the
 * world step family (armWorkWithScratchByte8AndSetByte7To2 and
 * selectSequenceByFrameParitySeedWorkByteARandom7ArmHandler1AndDecrement
 * RecordHalfword0a read the same cell), func_801ABBA8 is the shared arming
 * helper whose second argument selects the level, 0x80181B98 is the route
 * offset run the projected row is built from, and 0x801448ED is the shared
 * companion byte of the state-entry tuple. No in-image `jal` reaches 0x801F4840:
 * its only in-image reference is the handler pointer word at 0x801F67B4 inside
 * the payload's trailing pointer run, so the roles stay structural and the
 * table byte at 0x801F6898 stays address-anchored.
 *
 * Shape: 0x801F4840-0x801F48DF is one complete function - a 0x18-byte frame
 * that saves $ra, one body and the one `jr $ra` epilogue - not a fragment. No
 * byte-identical twin exists under this target's spans: the nearest sibling
 * armWorkWithScratchByte8AndSetByte7To2World00Area00013_801F2CD8 passes work
 * byte 8 to the same helper and has no gate, and the two counter steps
 * 0x801F48E0/0x801F4908 run on a different pointer pair, so both are shape
 * evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armRecordLevel5AndSetRecordByte7WithinModeLimitThenRetreatHalfword0aWorld04Area18813_801F4840(
    void) {
  u8* work;
  u32 row;

  work = SPAD_PTR_SLOT(u8, 0x44);
  row = FIELD_REF(u32, work, 0x38) + (u32)D_80181B98[(work[8] & 7) * 2];
  if (row > ((u32)D_801F6898[D_801448ED] << 16)) {
    return;
  }
  D_80148210[7] = 1;
  func_801ABBA8(D_80148210, 5);
  FIELD_REF(u16, D_80148210, 0x0A) -= 2;
}
