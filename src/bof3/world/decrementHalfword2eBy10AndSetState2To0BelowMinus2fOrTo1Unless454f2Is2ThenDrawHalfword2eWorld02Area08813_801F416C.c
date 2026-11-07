#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;

/* The local drawer at 0x801F4664 narrows both arguments with `sll`/`sra 16`
 * in its own prologue and neither call site consumes a result. */
void func_801F4664(s16 arg0, s16 arg1);

/* @source 0x801F416C
 * @behavior Byte-2 state handler 3 of the byte-2 handler table 0x801F61BC for
 * the work record published at the scratchpad pointer cell 0x1F800044.
 * Subtracts 0x10 from the signed halfword at record offset 0x2E and stores the
 * truncated halfword, then clears state byte 0x02 while the stored value stays
 * below -0x2F. Unless the shared byte 0x801454F2 is 2 it writes 1 to state
 * byte 0x02 through a second read of the pointer cell. Finally it reads the
 * cell again and calls the local drawer at 0x801F4664 with 0x10 and the signed
 * halfword at offset 0x2E. Every group carries its own read of the pointer
 * cell through the named slot symbol, so the cell address is re-materialised
 * per access. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword2eBy10AndSetState2To0BelowMinus2fOrTo1Unless454f2Is2ThenDrawHalfword2eWorld02Area08813_801F416C(
    void) {
  u8* record;
  u8* draw;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x2E);
  *half = (s16)(*half - 0x10);
  if (*half < -0x2F) {
    record[2] = 0;
  }
  if (D_801454F2 != 2) {
    D_1F800044[2] = 1;
  }
  draw = D_1F800044;
  func_801F4664(0x10, FIELD_REF(s16, draw, 0x2E));
}
