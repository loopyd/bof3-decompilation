#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 *D_80148208; /* @source 0x80148208 @kind unknown */
extern u8 D_801F41D0[]; /* @source 0x801F41D0 @kind table */
extern u8 D_801F41D1[]; /* @source 0x801F41D1 @kind table */

/* @source 0x801F2F94
 * @behavior Entry 2 of the handler pointer run the payload holds at 0x801F4168:
 * reads the shared gate byte D_801490C7, resets the shared status halfword
 * D_801490A8 to 0xFFFF, and copies the two bytes of the row that gate byte
 * selects from the 0x801F41D0 row table into the record words at +0x18 and
 * +0x1C of the pointer held at D_80148208. Rows have a byte stride of two, so
 * the second copy takes the 0x801F41D1 byte of the same row. The gate byte is
 * read again for that second row instead of being reused, and the fixed-address
 * accessor at that site reproduces the original's own `lui`/`lb` pair. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: this 0x58-byte span differs from the lifted twins at
 * emi/world04/area192/13 0x801F2CFC and emi/world04/area187/13 0x801F2C04 in
 * two words alone, the %lo immediates of the two row-table byte loads (payload
 * offsets 0x28 and 0x4C, the second one the first plus one); every other word is
 * equal, so the statement order and operand forms here follow that twin and this
 * file writes its own row-table addresses. The row table sits in this payload's
 * reviewed data span, so both row externs carry this target's own anchors.
 */
void setStatusFFFFAndPublishRecordWords18And1CFromGateRowWorld04Area19113_801F2F94(
    void) {
  u8 *record;

  record = D_80148208;
  D_801490A8 = 0xFFFF;
  *(u32 *)(record + 0x18) = D_801F41D0[D_801490C7 * 2];
  *(u32 *)(record + 0x1C) = D_801F41D1[PSX_REF(s8, 0x801490C7u) * 2];
}
