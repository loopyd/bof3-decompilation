#include "bof3/bof3.h"

/* The four shared per-kind record runs this accessor selects between. They sit
 * below this overlay's load address, in the main executable, so the overlay
 * only reads them; each run holds one u16 at the same offset of a record and
 * the record widths are 18, 24, 22 and 20 bytes, which the index scaling below
 * spells as element strides of 9, 12, 11 and 10 halfwords. The sibling accessor
 * at 0x801DA9C4 in emi/etc/shop/00 shows the same four runs beside their
 * leading flag bytes, so the halfword each one holds is a record field rather
 * than the record head.
 */
extern u16 D_801C8974[]; /* @source 0x801C8974 @kind table */
extern u16 D_801C90F2[]; /* @source 0x801C90F2 @kind table */
extern u16 D_801C98B8[]; /* @source 0x801C98B8 @kind table */
extern u16 D_801C9E8E[]; /* @source 0x801C9E8E @kind table */

/* Body taken from the span at 0x801DA9C4 in emi/etc/shop/00, whose own row was
 * already instruction-exact. A payload cmp of the two 0xB4-byte spans found
 * them equal except for their three `j` words at offsets 0x44, 0x68 and 0x90,
 * each shifted by exactly the 0x4B00 offset delta between the spans, so the
 * two are one body and only the wording below is this target's own.
 */
/* @source 0x801D5EC4
 * @behavior Returns one halfword field of a shared per-kind record: both
 *           arguments are masked to bytes, the kind byte selects the run of the
 *           four that holds that record kind, and the index byte picks the
 *           record whose field the body loads. Kind 1 indexes the stride-12
 *           halfword run at 0x801C90F2, kind 2 the stride-11 run at
 *           0x801C98B8 and kind 3 the stride-10 run at 0x801C9E8E, while kind 0
 *           and every kind above 3 share the default arm that indexes the
 *           stride-9 run at 0x801C8974; the original emits that arm once, ahead
 *           of the two higher compares. The body creates no frame, saves no
 *           register and touches no memory besides the one halfword load, and
 *           no jal or table word of this payload names it, so it is an overlay
 *           entry reached from outside the payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
u16 readKindRecordValueShisu00_801D5EC4(u8 kind, u8 index)
{
    u16 value;

    switch (kind) {
    default:
    case 0:
        value = D_801C8974[index * 9];
        break;
    case 1:
        value = D_801C90F2[index * 12];
        break;
    case 2:
        value = D_801C98B8[index * 11];
        break;
    case 3:
        value = D_801C9E8E[index * 10];
        break;
    }
    return value;
}
