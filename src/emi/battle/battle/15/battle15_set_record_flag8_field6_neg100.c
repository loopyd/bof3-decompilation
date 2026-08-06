#include "internal.h"

/* @behavior initializes two adjacent battle-state fields through D_801463A0.
 * @source 0x8009E224
 */
void battle15_set_record_flag8_field6_neg100(void) {
  *((volatile u8*)D_801463A0 + 8) = 2;
  (*(s16*)((u32)D_801463A0 + 6)) = -0x64;
}
