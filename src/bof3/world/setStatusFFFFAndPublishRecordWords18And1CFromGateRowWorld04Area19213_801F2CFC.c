#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 *D_80148208; /* @source 0x80148208 @kind unknown */
extern u8 D_801F380C[]; /* @source 0x801F380C @kind table */
extern u8 D_801F380D[]; /* @source 0x801F380D @kind table */

/* @source 0x801F2CFC
 * @behavior Entry 2 of the target handler table at 0x801F37AC (the table the
 * payload publishes at 0x801F37D0). Reads the shared gate byte D_801490C7,
 * resets the shared status halfword D_801490A8 to 0xFFFF, and copies the two
 * bytes of the row the gate byte selects from the 0x801F380C row table into the
 * words at +0x18 and +0x1C of the record pointer held at D_80148208. Rows have a
 * byte stride of two, so the second copy takes the 0x801F380D byte of the same
 * row. The gate byte is read again for that second row; the original folds the
 * re-read into its own `lui`/`lb` pair instead of reusing the first value, which
 * the fixed-address accessor at that use site reproduces. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the row table sits in this payload's own
 * bin `tail` segment, so its two addresses are the displacements the original
 * folds into the `lbu` loads. The body is byte-identical to the slot-2 handler of
 * seven other overlays (emi/world04/area193/13 0x801F2D70,
 * emi/world04/area191/13 0x801F2F94, emi/world04/area187/13 0x801F2C04,
 * emi/world04/area154/13 0x801F2C04, emi/world04/area153/13 0x801F2C04,
 * emi/world02/area090/13 0x801F2C04, emi/world01/area053/13 0x801F2C04) apart
 * from that row address, and none of those rows carries source yet, so this file
 * is seeded here rather than cloned.
 */
void setStatusFFFFAndPublishRecordWords18And1CFromGateRowWorld04Area19213_801F2CFC(
    void) {
  u8 *record;

  record = D_80148208;
  D_801490A8 = 0xFFFF;
  *(u32 *)(record + 0x18) = D_801F380C[D_801490C7 * 2];
  *(u32 *)(record + 0x1C) = D_801F380D[PSX_REF(s8, 0x801490C7u) * 2];
}
