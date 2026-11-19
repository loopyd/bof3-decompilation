#include "bof3/bof3.h"

/* Shared gate halfword the body tests. No evidenced role exists for it in the
 * tree, so this source keeps it address-anchored. */
extern u16 D_80143F04; /* @source 0x80143F04 @kind data */

/* Shared world-flags halfword, bound at 0x80146258 by the shared map. Its
 * observed low bits carry the front-end record gate and the mode flags
 * 0x40/0x80/0x100/0x180/0x240/0x400; the role of the bit cleared here is
 * unknown, so the mask stays address-anchored as 0x2000. */
extern u16 D_80146258; /* @source 0x80146258 @kind data */

/* @source 0x801F2C38
 * @behavior When the shared gate halfword D_80143F04 reads 0x3C the body clears
 *           bit 0x2000 of the shared world-flags halfword D_80146258 with one
 *           halfword read-modify-write, and otherwise writes nothing: it loads
 *           D_80146258 through a folded lui/lhu base, leaves the load delay
 *           slot empty, ANDs the complement mask 0xDFFF into the loaded value
 *           and stores the low halfword back through a re-materialised lui $at
 *           base. Takes no arguments, returns nothing and keeps no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape: the gate follows the reviewed exact sibling
 *        setFlag42ClearFlag43ThenConsumeMapConditionRecordChainWorld02Area09413_801F3460,
 *        whose gate reads the same halfword with `lhu`, materialises its own
 *        constant in $v0 and branches with `bne`; the read-modify-write follows
 *        the reviewed exact sibling
 *        clearWorldFlags1010World04Area17013_801F4A9C, whose body is the same
 *        folded-base load, empty load-delay slot, AND and `lui $at` store. The
 *        52 bytes at 0x801F2C38 measured byte-identical (payload cmp) to the
 *        bytes at 0x801F2C38 of world01/area065/13, whose own row was still an
 *        `asm` boundary and so carried no source to clone. The only in-image
 *        reference to 0x801F2C38 is the word at 0x801F55D4 inside the overlay's
 *        script/data blob, and no `jal` immediate reaches it, so the blob
 *        publishes it as a script-invoked callback without a prototype and its
 *        entry roles stay structural.
 */
void clearWorldFlag2000WhenGateIs3cWorld02Area08713_801F2C38(void) {
  if (D_80143F04 == 0x3C) {
    D_80146258 &= 0xDFFF;
  }
}
