#include "bof3/bof3.h"

extern u8 D_80148331; /* @source 0x80148331 @kind data */
extern u8 D_80148332; /* @source 0x80148332 @kind data */
extern u8 D_80148333; /* @source 0x80148333 @kind data */
extern u8 D_80148330; /* @source 0x80148330 @kind data */
extern s16 D_80148334; /* @source 0x80148334 @kind data */
extern s16 D_80148336; /* @source 0x80148336 @kind data */
extern u8 D_8014833A; /* @source 0x8014833A @kind data */
extern u8 D_8014833B; /* @source 0x8014833B @kind data */
extern u8 D_8014833C; /* @source 0x8014833C @kind data */
extern u8 D_8014833D; /* @source 0x8014833D @kind data */
extern u8 D_80148338; /* @source 0x80148338 @kind data */
extern u8 D_80148339; /* @source 0x80148339 @kind data */
extern s16 D_80148340; /* @source 0x80148340 @kind data */
extern s16 D_80148342; /* @source 0x80148342 @kind data */
extern u8 D_80148355; /* @source 0x80148355 @kind data */
extern u8 D_80148356; /* @source 0x80148356 @kind data */
extern u8 D_80148357; /* @source 0x80148357 @kind data */
extern u8 D_80148354; /* @source 0x80148354 @kind data */

/* @source 0x801D16A4
 * @behavior Initializes the commu overlay's share of the shared UI work block:
 *           the mode/cursor bytes D_80148330 through D_80148333, the panel
 *           origin halfwords D_80148334/D_80148336, the mask bytes D_80148338
 *           through D_8014833D, the extent halfwords D_80148340/D_80148342 and
 *           the trailing bytes D_80148354 through D_80148357. It reads nothing
 *           and returns void through a bare `jr $ra`. The step handler
 *           0x801D0CF4 is its only reference, calling it on the screen-entry
 *           path just before arming D_801D1774 and advancing the step byte
 *           D_801F294D; 0x801D16A4 is absent from the overlay's handler table
 *           at 0x801D1750. The two repeated literals 2 and 1 stay in $v1/$a0
 *           across the whole body because they are written to both ends of the
 *           block, and the two halfword stores schedule ahead of the leading
 *           byte stores, so the byte stores are spelled first in source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeCommuUiState(void)
{
    D_80148331 = 2;
    D_80148332 = 0;
    D_80148333 = 2;
    D_80148330 = 1;
    D_80148334 = -0xAA;
    D_80148336 = 0x3E;
    D_8014833A = 0;
    D_8014833B = 0;
    D_8014833C = 0;
    D_8014833D = 0xFF;
    D_80148338 = 0;
    D_80148339 = 0;
    D_80148340 = 0;
    D_80148342 = 0;
    D_80148355 = 2;
    D_80148356 = 1;
    D_80148357 = 0;
    D_80148354 = 0;
}
