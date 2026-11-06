#include "bof3/bof3.h"

/* Overlay-wide step cell group. Byte 0 is the step this overlay's handlers
 * advance by one or roll back by one as their commu request completes. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* Current commu request: mode byte plus its two parameter bytes. */
extern u8 D_801D4DF4; /* @source 0x801D4DF4 @kind data */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */
extern u8 D_801D4DFC; /* @source 0x801D4DFC @kind data */

/* Result code of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* @source 0x801D0C68
 * @behavior First entry of the handler run at 0x801D4BCC. Publishes commu
 * request mode 1 with both parameter bytes zero into 0x801D4DF4, 0x801D4DF8
 * and 0x801D4DFC, reports result code 0xB5 through the halfword at
 * 0x801D4E0C, and advances the overlay step byte at 0x801F294C by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D0C68(void) {
    u8 step;

    D_801D4DF4 = 1;
    step = D_801F294C;
    D_801D4DF8 = 0;
    D_801D4DFC = 0;
    D_801D4E0C = 0xB5;
    D_801F294C = step + 1;
}
