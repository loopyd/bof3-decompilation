#include "bof3/bof3.h"

/* Shared front-end effect-busy halfword at 0x80143C40 (bound as effectBusy by
 * emi/etc/game/00); the front-end transition wait loop reads the same word. */
extern u16 D_80143C40;

/* Shared sub-state byte at 0x80146875, bound by the shared map. The store
 * re-materialises $at for its own lui/sb pair. */
extern u8 D_80146875;

/* Shared work-flag reset helper at 0x80196070 (clearWorkFlags in
 * emi/etc/game/00). It sits outside this overlay's payload, so only its
 * prototype is declared; it takes no arguments and returns nothing. */
void func_80196070(void);

/* @source 0x801F3374
 * @behavior Overlay step handler: while the shared effect-busy halfword
 * D_80143C40 reads zero it stores the sub-state 0x14 into the shared sub-state
 * byte D_80146875 and then runs the shared func_80196070 work-flag reset; when
 * the effect is busy it does neither. The busy word is loaded once ahead of the
 * frame setup, the constant reaches the store through $v0, and the 0x18-byte
 * frame only keeps $ra at +0x10 across the call whose argumentless jal keeps
 * the nop delay slot. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate14AndClearWorkFlagsWhenEffectIdleWorld04Area17213_801F3374(
    void) {
  if (D_80143C40 == 0) {
    D_80146875 = 0x14;
    func_80196070();
  }
}
