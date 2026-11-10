#include "bof3/bof3.h"

/* Shared main-RAM flag bank the probe tests flag 0x77 of. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared front state this arm stamps and clears. */
extern u16 D_80144F54; /* @source 0x80144F54 @kind data */
extern u8 D_80146871;  /* @source 0x80146871 @kind data */
extern u8 D_80146880;  /* @source 0x80146880 @kind data */
extern u8 D_80146881;  /* @source 0x80146881 @kind data */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_80196070(void);

/* @source 0x801F3678
 * @behavior Guards the whole handler on the shared flag tester 0x8015B5D4
 *           evaluated over the main-RAM bank base D_80144F28 with flag id
 *           0x77: a zero return retires the handler without touching anything.
 *           A nonzero return stamps the shared front halfword D_80144F54 with
 *           0x1E0, clears the two front bytes D_80146880 and D_80146881,
 *           clears bit 0 of the state byte D_80146871, and finishes with the
 *           resident work-area reset 0x80196070, which clears the scratchpad
 *           game work area header. The 0x18-byte frame keeps only $ra at 0x10;
 *           the address of D_80146871 is materialized once into $a0 and reused
 *           for its load and its store, the load is hoisted above the three
 *           halfword/byte stores, and that store is the one scheduled into the
 *           0x80196070 jal delay slot, which leaves the just-written state
 *           byte in $a0 at the call.
 *
 * Name evidence: the probed flag is the `lui/addiu $a0, %hi(D_80144F28)` plus
 * `addiu $a1, $zero, 0x77` argument pair; the stamped halfword is the
 * `addiu $v0, $zero, 0x1E0` feeding `sh $v0, %lo(D_80144F54)`; the two cleared
 * bytes and the cleared bit 0 are the `sb $zero` pair and the `andi 0xFE`
 * read-modify-write of D_80146871; and the reset helper at 0x80196070 is the
 * already-lifted clearWorkFlags body. This is the same flag-0x77 front-state
 * reset shape as the reviewed sibling at emi/world04/area191/13 0x801F3BE0,
 * which sets bit 0 of the same state byte and increments D_80144F56 where this
 * body stamps D_80144F54 with 0x1E0 and clears bit 0 instead; that sibling
 * carries the wording for the bank cells, and no extracted payload ever loads
 * D_80144F54, so its role stays address-anchored.
 *
 * Shape: seeded from the original bytes. A payload scan over all 396 extracted
 * binaries found no twin at 0x801F3678's 0x60-byte body: 1 exact hit (this
 * range), and still 1 hit with every `j`/`jal` target masked and with every
 * immediate masked, so neither the clone nor the carve lever applies and the
 * range carries its own prologue and `jr $ra` epilogue at the payload's
 * function boundary (the `bin tail` segment starts at the next address,
 * 0x801F36D8).
 * @status exact
 * @match 100.00
 * @residual none
 */
void stampFrontHalfword1E0ClearFrontBytesAndStateBit0ThenResetWorkAreaWhenFlag77SetWorld04Area19213_801F3678(
    void) {
  if (func_8015B5D4((u32)&D_80144F28, 0x77) != 0) {
    u8 *state;

    state = &D_80146871;
    D_80144F54 = 0x1E0;
    D_80146880 = 0;
    D_80146881 = 0;
    *state &= 0xFE;
    func_80196070();
  }
}
