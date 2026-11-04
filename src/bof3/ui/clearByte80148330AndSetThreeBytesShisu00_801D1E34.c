#include "bof3/bof3.h"

extern u8 D_80148330; /* @source 0x80148330 @kind data */
extern u8 D_80148357; /* @source 0x80148357 @kind data */
extern u8 D_8014837B; /* @source 0x8014837B @kind data */
extern u8 D_8014839F; /* @source 0x8014839F @kind data */

/* @source 0x801D1E34
 * @behavior Clears byte 0x80148330 and sets bytes 0x80148357, 0x8014837B,
 * and 0x8014839F to one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearByte80148330AndSetThreeBytesShisu00_801D1E34(void)
{
    D_80148330 = 0;
    D_80148357 = 1;
    D_8014837B = 1;
    D_8014839F = 1;
}
