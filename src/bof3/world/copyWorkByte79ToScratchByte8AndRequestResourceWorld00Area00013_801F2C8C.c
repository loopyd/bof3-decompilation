#include "bof3/bof3.h"

/* Scratch work-record pointer cell in the shared scratchpad, outside this
 * overlay's image. Both accesses re-read it: a store through the loaded pointer
 * may alias the cell, which is why the original materialises the cell address
 * with its own `lui`/`lw` pair at each of the two accesses instead of reusing
 * the address in a register. The cell is volatile so the second read is a real
 * reload.
 * @source 0x1F800044 @kind data
 */
extern u8* volatile D_1F800044;

/* Current work-entity pointer cell in main RAM, outside this overlay's image.
 * The body reaches only the entity byte at +0x79 through it, so this target
 * declares the byte view and keeps the cell address anchored.
 * @source 0x80146884 @kind unknown
 */
extern u8* D_80146884;

/* Resident resource helper in the main executable image, outside this overlay:
 * it takes the work record's byte 8 as its resource id. Only a prototype is
 * declared here; the generated undefined_funcs_auto.txt row binds the address.
 */
extern void func_8015BAC4(u8 resource_id);

/* @source 0x801F2C8C
 * @behavior Overlay handler: reads the byte at +0x79 of the current work-entity
 *           record published through the pointer cell 0x80146884 and stores it
 *           into byte 8 of the scratch work record published at the scratchpad
 *           pointer cell 0x1F800044, then re-reads that scratch cell and passes
 *           its byte 8 to the resident resource helper func_8015BAC4
 *           (0x8015BAC4) as the resource id. The original materialises the
 *           0x1F800044 cell twice, because the store through the loaded
 *           pointer may alias the cell itself, so the cell keeps its named
 *           symbol form and each access re-reads it. The fixed-address
 *           accessor spelling measured first cached the cell address in one
 *           register (`lui`/`ori`) and hoisted the frame setup, 14/19
 *           instructions; the named cell emits the original's own `lui`/`lw`
 *           pair per access and matched. The payload names this span only as a
 *           word of the handler pointer run at 0x801F3FB8 (the run's word at
 *           0x801F3FF0), so the overlay dispatcher reaches it indirectly and
 *           the parameter roles stay structural. Byte 8 of the scratch record
 *           is the byte the same target's sibling
 *           armWorkWithScratchByte8AndSetByte7To2... (0x801F2CD8) publishes as
 *           a helper argument too, and the +0x79 byte of the work record keeps
 *           the WorkByte79 spelling other world lifts use for that anchored
 *           offset, whose role no lifted body establishes. Takes no arguments
 *           and returns nothing; the 0x18-byte frame only saves $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkByte79ToScratchByte8AndRequestResourceWorld00Area00013_801F2C8C(void) {
  D_1F800044[8] = D_80146884[0x79];
  func_8015BAC4(D_1F800044[8]);
}
