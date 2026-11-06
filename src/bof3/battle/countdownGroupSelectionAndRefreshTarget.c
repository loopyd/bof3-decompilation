#include "bof3/battle/battle03_internal.h"

/* @source 0x801EB494 @kind table */
extern u8 D_801EB494[];
void func_8014D6B8(u8 request);

/* @source 0x801E6FB0
 * @behavior When battle global halfword 0x801462E8 carries bit 0x400 and the
 * current work record byte +5 is the global progress byte 0x80146374, or fight
 * stage 0x80146375 reads 4 with 0x801463C0 reading 0x97 or 0xE1, increments
 * scratchpad byte +1 and returns. Otherwise it counts down scratch byte +0x0A:
 * on expiry it takes the next enabled 0x58 group bit from func_801E8FA8, clears
 * the queued slot bytes when that index is 0xFF, and otherwise arms scratch
 * byte +0x27 to 0xFF and byte +0x0A to 0x3C and requests the D_801EB494 table
 * byte of that index, publishing the index at scratch byte +0x0B when it
 * changed. It then clears scratch byte +1 when the selected record's 0x58 group
 * bits are clear, derives the queued work position with func_801E8DD8, runs
 * func_8014D978 and func_8014D4E0, and clears scratch byte +1 again when the
 * selected battler is unavailable while work byte +0 has bit 0 clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownGroupSelectionAndRefreshTarget(void) {
  Battle03LocalWork* work;
  u8 selector;
  u8 index;
  u16 group_flags;

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x400u) != 0u
      && (D_801EB4E0->unk_05 == BATTLE_GLOBAL_BYTE_6374
          || (D_80146375 == 4u
              && (D_801463C0 == 0x97u || D_801463C0 == 0xe1u)))) {
    D_1F800044->unk_01++;
    return;
  }

  if (((u8*)D_1F800044)[0x0a] == 0u) {
    index = func_801E8FA8();
    if (index == 0xffu) {
      clearQueuedSlotBytes();
      return;
    }
    ((u8*)D_1F800044)[0x27] = 0xff;
    ((u8*)D_1F800044)[0x0a] = 0x3c;
    if (((u8*)D_1F800044)[0x0b] != index) {
      func_8014D6B8(D_801EB494[index]);
      ((u8*)D_1F800044)[0x0b] = index;
    }
  } else {
    ((u8*)D_1F800044)[0x0a]--;
  }

  work = D_801EB4E0;
  selector = work->unk_05;
  if (selector < 3u) {
    group_flags = D_80145E90[selector].unk_80 & 0x58u;
  } else {
    group_flags = D_801EB630[selector - 3u].unk_82 & 0x58u;
  }
  if (group_flags == 0u) {
    ((u8*)D_1F800044)[1] = 0;
  }

  func_801E8DD8();
  func_8014D978();
  func_8014D4E0();

  if (isBattlerInactiveOrFlag4000(D_801EB4E0->unk_05) != 0u
      && (D_801EB4E0->flags_00 & 0x01u) == 0u) {
    ((u8*)D_1F800044)[1] = 0;
  }
}
