#include "bof3/bof3.h"

/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;

/* The local drawer narrows both arguments with `sll`/`sra 16` in its own
 * prologue and neither call site consumes a result. */
void func_801F3A34(s16 arg0, s16 arg1);

/* @source 0x801F34EC
 * @behavior Byte-2 step tail of this overlay. When the shared mode byte at
 *           0x801454F2 reads 2 it writes 3 to byte 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. It then
 *           reloads that cell and calls the local drawer 0x801F3A34 with 0x10
 *           and the signed halfword at record offset 0x2E, discarding any
 *           result. Its only in-image caller, 0x801F3490, first advances that
 *           same halfword by 0x10, bumps work byte 0x02 once the advanced
 *           signed value reaches 0x10, and then calls this function. It takes
 *           no arguments, uses no result, saves only the return address and
 *           touches no other state. The 0x50 bytes match the already reviewed
 *           setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area12114_801F35D0
 *           in emi/world03/area121/14 apart from the single internal `jal`
 *           word at offset 0x38, which carries each overlay's own drawer
 *           address (0x801F3A34 here, 0x801F3B18 there), so this body takes
 *           that sibling's statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld00Area03313_801F34EC(
    void) {
  u8* record;

  if (D_801454F2 == 2) {
    SPAD_PTR_SLOT(u8, 0x44)[2] = 3;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  func_801F3A34(0x10, FIELD_REF(s16, record, 0x2E));
}
