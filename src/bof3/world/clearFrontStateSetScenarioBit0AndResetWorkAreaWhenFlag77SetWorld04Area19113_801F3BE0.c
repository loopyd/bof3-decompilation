#include "bof3/bof3.h"

extern u8 D_80144F28;  /* @source 0x80144F28 @kind unknown */
extern u16 D_80144F54; /* @source 0x80144F54 @kind unknown */
extern u8 D_80144F56;  /* @source 0x80144F56 @kind unknown */
extern u8 D_80146871;  /* @source 0x80146871 @kind unknown */
extern u8 D_80146880;  /* @source 0x80146880 @kind unknown */
extern u8 D_80146881;  /* @source 0x80146881 @kind unknown */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_80196070(void);

/* @source 0x801F3BE0
 * @behavior Guards the whole handler on the shared flag tester 0x8015B5D4
 * evaluated over the main-RAM bank base D_80144F28 with flag id 0x77: a zero
 * return retires the handler without touching anything. A nonzero return
 * clears the halfword cell D_80144F54 and the two front-state bytes D_80146880
 * and D_80146881, sets bit 0 of the state byte D_80146871, increments the byte
 * cell D_80144F56 in the same bank as D_80144F54, and finishes with the
 * resident work-area reset 0x80196070, which clears the scratchpad game work
 * area header. The 0x18-byte frame keeps only $ra at 0x10, the count
 * D_80144F56 is read first and stored last, the address of D_80146871 is
 * materialized once into a register and reused for its load and its store,
 * and that store is the one scheduled into the 0x80196070 jal delay slot,
 * which leaves the just-written state byte in $a0 at the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearFrontStateSetScenarioBit0AndResetWorkAreaWhenFlag77SetWorld04Area19113_801F3BE0(void) {
  if (func_8015B5D4((u32)&D_80144F28, 0x77) != 0) {
    u8 *state;

    D_80144F56++;
    state = &D_80146871;
    D_80144F54 = 0;
    D_80146880 = 0;
    D_80146881 = 0;
    *state |= 1;
    func_80196070();
  }
}
