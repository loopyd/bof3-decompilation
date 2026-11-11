#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6A14
 * @behavior Runs the battle step while it waits for the queued local work
 * records to drain: it returns while any of the localBattlerCount 0x140-byte
 * record flag words at D_80145FB4 still carries the queued-slot flag 0x2000,
 * and when the D_801462E5 queue cursor differs from that count it steps the
 * battle state byte BATTLE_GLOBAL_BYTE_62E2 back by one and returns. With the
 * cursor matched and D_80146328 bit 0 clear it publishes each record through
 * the scratchpad work pointer and, for a record whose D_80145F10 status
 * halfword carries bit 0x800, copies that record's D_80145F20/D_80145F22
 * halfwords and D_80145F2E byte into D_80145F18/D_80145F1A/D_80145F1C before
 * submitting the 0xBFFF-masked status; every record is then submitted with the
 * 0xBF5F mask, refreshed through func_80196718, ranked through func_801DDFEC,
 * and the enemy-ready latch BATTLE_GLOBAL_BYTE_63CE is cleared. When no enemy
 * cue is pending it maps D_80146328 bit 0 to battle mode byte 0x801462E1 = 2
 * and bit 1 to 1, then runs the battle hook D_801463A4 registered by
 * func_801E320C, resets layout bank 0 through func_80161808, starts EMI stream
 * slot 1 or 2 through func_80161FDC for the mode byte it ended on, and clears
 * the battle state byte BATTLE_GLOBAL_BYTE_62E2.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the record copy group (D_80145F18/D_80145F1A/D_80145F1C from
 * D_80145F20/D_80145F22/D_80145F2E) is read and stored through plain u16/u8
 * views of the header's volatile byte arrays. With the volatile views the
 * group keeps its interleaved order and the 0xBFFF mask is scheduled ahead of
 * the stores, which leaves the 0xBF5F submit call's delay slot empty and
 * measured 145/157; the plain group measured 156/156 at the original 624
 * bytes. The 0x140 stride stays a named product of `index * 5` shifted by 6,
 * which keeps the product in the register the original uses; the single
 * `index * 0x140` product measured 115/157 with the product in a caller-saved
 * register, a copy across the masked submit call and 4 extra bytes. */
void submitBattlerRecordsAndStartPhaseStream(void) {
  u8  count;
  u8  index;
  u32 offset;
  u32 scaled;
  u8* record;
  u16 panel;
  u16 position;
  u16 status;
  u8  value;
  u16 flags;
  u8  mode_bits;

  count = *(u8*)&localBattlerCount;
  index = 0;
  if (count != 0) {
    do {
      if ((D_80145FB4[index].flags_00 & 0x2000u) != 0u) {
        return;
      }
      index += 1;
    } while (index < count);
  }
  if (D_801462E5 != *(u8*)&localBattlerCount) {
    BATTLE_GLOBAL_BYTE_62E2 -= 1;
    return;
  }
  if ((D_80146328 & 1u) == 0u) {
    index = 0;
    if ((D_80146328 & 1u) < D_801462E5) {
      do {
        scaled = (u32)index * 5u;
        offset = scaled << 6;
        flags = *(volatile u16*)(D_80145F10 + offset);
        record = (u8*)D_80145E90 + offset;
        battleWork = record;
        if ((flags & 0x800u) != 0u) {
          panel = *(u16*)(D_80145F20 + offset);
          position = *(u16*)(D_80145F22 + offset);
          status = *(volatile u16*)(D_80145F10 + offset);
          value = *(u8*)(D_80145F2E + offset);
          *(u16*)(D_80145F18 + offset) = panel;
          *(u16*)(D_80145F1A + offset) = position;
          *(u8*)(D_80145F1C + offset) = value;
          func_800A36F0(index, status & 0xBFFFu);
        }
        func_800A36F0(index, *(volatile u16*)(D_80145F10 + offset) & 0xBF5Fu);
        func_80196718(record);
        func_801DDFEC(index);
        index += 1;
      } while (index < localBattlerCount);
    }
    BATTLE_GLOBAL_BYTE_63CE = 0;
  }
  if (BATTLE_GLOBAL_BYTE_62EA == 0u) {
    mode_bits = D_80146328;
    if ((mode_bits & 1u) != 0u) {
      D_801462E1[0] = 2;
    }
    if ((mode_bits & 2u) != 0u) {
      D_801462E1[0] = 1;
    }
  }
  D_801463A4();
  func_80161808(0);
  if (D_801462E1[0] == 1u) {
    func_80161FDC(1);
  }
  if (D_801462E1[0] == 2u) {
    func_80161FDC(2);
  }
  BATTLE_GLOBAL_BYTE_62E2 = 0;
}
