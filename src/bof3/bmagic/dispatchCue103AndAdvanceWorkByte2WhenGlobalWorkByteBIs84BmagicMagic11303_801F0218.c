#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 */

/* @source 0x801F0218
 * @behavior If the shared battle work record at 0x801EB4E0 has byte 0xB
 * equal to 0x84, dispatches sound cue 0x103, then reads the scratchpad
 * work-object pointer at 0x1F800044 and increments its byte 2 modulo 256.
 * Otherwise leaves state unchanged. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchCue103AndAdvanceWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801F0218(void) {
  if (D_801EB4E0[0xB] == 0x84) {
    func_8015DF18(0x103);
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
