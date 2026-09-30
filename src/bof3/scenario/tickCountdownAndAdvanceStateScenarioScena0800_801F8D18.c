#include "bof3/bof3.h"

/* @source 0x801F8D18
 * @behavior Per-frame overlay handler: it decrements the countdown byte at
 * offset 2 of the object pointer it receives in $a0 and re-stores it; when
 * that decremented byte reads zero it re-arms the same byte to 0x20 and
 * increments the object's byte at offset 1, otherwise it leaves both bytes as
 * the decrement left them. It reads and writes no other memory, makes no call
 * and returns nothing, so its 0x34 bytes carry no frame and no saved register.
 * The address is entry 27 of this overlay's 51-entry per-frame handler run at
 * 0x801FE894, the table that dispatchWorkByte1HandlerScenarioScena0800_801F6F48
 * indexes with the scratch work object's byte 1, and it is also the eighth
 * entry of the sub-run at 0x801FE8E4 that func_801F7AD0 indexes with that
 * object's byte 2, so the byte it increments is the state byte those two
 * dispatchers select a handler with. Its bytes are the sibling shape of the two
 * handlers beside it in the same run: 0x801F8CDC also decrements byte 2 but
 * advances byte 3 by 6 and re-arms byte 2 from byte 4, and 0x801F8D4C also
 * decrements byte 2 but integrates a velocity word into a position word and
 * clears byte 0 when its countdown expires.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownAndAdvanceStateScenarioScena0800_801F8D18(u8* object) {
  u8 countdown;

  countdown = (u8)(object[2] - 1);
  object[2] = countdown;
  if (countdown == 0) {
    object[2] = 0x20;
    object[1]++;
  }
}
