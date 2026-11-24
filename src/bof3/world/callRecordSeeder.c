#include "bof3/bof3.h"

void claimFixedStoreDistance(
    void);

/* @source 0x801F3D24
 * @behavior Thin overlay wrapper: calls the record-seeding helper at
 *           0x801F431C, which claims a front-end record at 0x80143FC8 and
 *           seeds handler 0x48, and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callRecordSeeder(void) {
  claimFixedStoreDistance();
}
