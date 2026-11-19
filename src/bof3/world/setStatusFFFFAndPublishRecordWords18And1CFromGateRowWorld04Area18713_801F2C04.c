#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 *D_80148208; /* @source 0x80148208 @kind unknown */
extern u8 D_801F35CC[]; /* @source 0x801F35CC @kind table */
extern u8 D_801F35CD[]; /* @source 0x801F35CD @kind table */

/* @source 0x801F2C04
 * @behavior Entry 0 of this payload's handler table at 0x801F35D8: reads the
 * shared gate byte D_801490C7, resets the shared status halfword D_801490A8 to
 * 0xFFFF, and copies the two bytes of the gate-selected row of the 0x801F35CC
 * row table into record words +0x18 and +0x1C of the pointer held at
 * D_80148208. Rows have a byte stride of two, so the second copy takes the
 * 0x801F35CD byte of the same row. The gate byte is read again for that second
 * row; the original folds the re-read into its own `lui`/`lb` pair instead of
 * reusing the first value, which the fixed-address accessor at that use site
 * reproduces. The row table holds six rows whose leading bytes are 8, 1, 4, 5,
 * 7 and 9 and whose trailing bytes are all zero. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: this body is byte-identical to the one
 * lifted at emi/world04/area192/13 0x801F2CFC apart from the two `lbu` `%lo`
 * words that carry each payload's own row-table base, which a payload `cmp` of
 * the two 0x58-byte spans measured as its only two differing words. Six other
 * overlays carry the same body, differing in those two words alone
 * (emi/world04/area193/13 0x801F2D70, emi/world04/area191/13 0x801F2F94,
 * emi/world04/area154/13 0x801F2C04, emi/world04/area153/13 0x801F2C04,
 * emi/world02/area090/13 0x801F2C04, emi/world01/area053/13 0x801F2C04); none
 * of those rows carried source when this file was written, so this entry keeps
 * its own address-anchored binding of the row table.
 */
void setStatusFFFFAndPublishRecordWords18And1CFromGateRowWorld04Area18713_801F2C04(
    void) {
  u8 *record;

  record = D_80148208;
  D_801490A8 = 0xFFFF;
  *(u32 *)(record + 0x18) = D_801F35CC[D_801490C7 * 2];
  *(u32 *)(record + 0x1C) = D_801F35CD[PSX_REF(s8, 0x801490C7u) * 2];
}
