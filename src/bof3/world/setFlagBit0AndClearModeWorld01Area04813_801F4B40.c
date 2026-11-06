#include "bof3/bof3.h"

/* Shared world flag byte; this handler touches only its bit 0.
 * @source 0x80147CC8 @kind unknown
 */
extern u8 D_80147CC8;
/* Shared small mode byte: this overlay's handlers read it as an index scaled by
 * four into the pointer table 0x80148210, as a bound-tested value below 8, and
 * as the 16-step geometry scale of the entity at 0x80146884.
 * @source 0x8014686B @kind unknown
 */
extern u8 D_8014686B;

/* @source 0x801F4B40
 * @behavior Sets bit 0 of the shared world flag byte 0x80147CC8 and writes 0 to
 * the shared mode byte 0x8014686B. The flag byte is addressed once through a
 * pointer local, so its read and its write both go through that register; the
 * mode byte is a plain symbol store. Takes no arguments, returns nothing and
 * uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFlagBit0AndClearModeWorld01Area04813_801F4B40(void) {
  u8* flags;
  u8 value;

  flags = &D_80147CC8;
  value = *flags;
  D_8014686B = 0;
  *flags = value | 0x1;
}
