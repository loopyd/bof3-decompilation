#include "bof3/bof3.h"

/* Shared per-frame counter; its low two bits select one step of this overlay's
 * four-frame cycle.
 * @source 0x80143E6C @kind data
 */
extern s32 D_80143E6C;

/* Shared status byte that receives the mode constant 2.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared halfword counter this handler advances.
 * @source 0x8014932A @kind data
 */
extern u16 D_8014932A;

/* Four bytes 00 01 00 FF, that is 0, 1, 0 and -1 read as signed steps. The row
 * sits inside the reviewed data span D_801FC404 (0x801FC404 through 0x801FC4C3)
 * after the overlay's two code-pointer runs, and no image word points at it: it
 * is only read by this handler.
 * @source 0x801FC44C @kind table
 */
extern s8 D_801FC44C[];

/* @source 0x801F8800
 * @behavior Overlay step handler: it takes the low two bits of the shared
 * per-frame counter D_80143E6C as an index into the four-entry signed step row
 * at 0x801FC44C (0, 1, 0, -1), publishes mode 2 in the shared status byte
 * D_80149333, and adds that signed step times its unsigned byte argument to the
 * shared halfword counter D_8014932A, wrapping modulo 65536. The argument scales
 * the four-frame cycle: all three in-image callers of the address pass 4.
 * It reads only D_80143E6C, writes D_80149333 and D_8014932A, returns nothing
 * and uses no stack frame, being a leaf that keeps no saved register. Three
 * in-image jal sites reach it, at 0x801F81B8 and 0x801F8258 inside
 * func_801F7FB8 and at 0x801F9620 inside func_801F9320; each passes 4 in $a0
 * for a D_80146876 countdown tick, so the parameter is the per-tick step scale.
 * The $a0 truncation to eight bits is this body's own andi $a0, $a0, 0xFF, so the
 * declared width is the callee's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addSignedTableStepScaledByByteToHalfword8014932AScenarioScena1400_801F8800(
    u8 amount) {
    s32 step;

    step = D_801FC44C[D_80143E6C & 3];
    D_80149333 = 2;
    D_8014932A = (u16)(D_8014932A + step * amount);
}
