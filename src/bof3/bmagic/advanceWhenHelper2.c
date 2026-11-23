#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds no name at the
 * address, so the D_ spelling auto-binds with no map row and each access
 * re-materializes the cell address as a lui plus offset load (lui $v1,0x1F80 /
 * lw $v1,0x44($v1)), as the exact scratchpad lifts of this target do. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the lifts of this target read as a non-zero ready flag. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt,
 * so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801F014C
 * @behavior Entry 21 of the overlay handler code-pointer run that starts at
 *           0x801EEC04, the word the tabledata segment stores at 0x801EEC58
 *           between the entry at 0x801EEC54 and the entry 22 handler at
 *           0x801EEC5C. The payload holds that one pointer word and no jal or j
 *           word to this address, so the call site is the indirect dispatch
 *           through the copied run. Per frame it calls the shared readiness
 *           helper 0x8014DAEC and tests the byte that helper returns: while the
 *           helper reports not-ready the handler does nothing, and once it
 *           reports ready the handler loads the work object through the pointer
 *           cell 0x1F800044 and increments work byte 2, the handler index, with
 *           a byte-width add (lbu / addiu 1 / sb). The 0x18-byte frame only
 *           keeps $ra at 0x10 across the call. Nothing is returned and no other
 *           state is read or written. The 0x48-byte body is byte-identical to
 *           the span at 0x801EFB90 of this same payload, entry 13, which a
 *           payload cmp over all 0x48 bytes proved; that exact sibling supplied
 *           the statement order and the operand forms.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWhenHelper2(void) {
  if (func_8014DAEC() != 0u) {
    D_1F800044[2]++;
  }
}
