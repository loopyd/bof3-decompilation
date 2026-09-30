#include "bof3/bof3.h"

void func_801F8358(void);

/* @source 0x801F6E54
 * @behavior Overlay work-state handler for scratch work-object byte 2 = 0: it
 * calls the table-clearing helper at 0x801F8358, which zeroes the lead byte of
 * each of the 0x8 entries of the 0x1C-byte-stride record table at 0x800E4800
 * (the main-RAM work area this overlay shares with the world and scenario
 * overlays) and then zeroes the two flag bytes at 0x801FDFC4 and 0x801FDFC8,
 * the last two words of this payload, and afterwards re-arms the step: it
 * reloads the scratchpad work object published at pointer slot 0x1F800044 + 0x44
 * (the call clobbers the registers, so the cell is read again after it), stores
 * 0x40 into that object's 32-bit word at offset 0x0C and increments its byte at
 * offset 0x02 by one. Both written fields are the ones the following handlers of
 * the same run read: byte 0x02 is the index that func_801F6E10 (the dispatcher
 * at 0x801F6E10, which scales it by four and applies it to the handler table at
 * 0x801FDD3C) uses, and this function is that table's entry 0 (0x801F6E54 ==
 * 0x801FDD3C), so the increment moves the run on to its next entry; word 0x0C is
 * the countdown that the next entry, func_801F6E90 at 0x801F6E90, reads (its low
 * three bits select whether a free record is claimed and seeded this frame) and
 * decrements, incrementing byte 0x02 again once the decremented value reads
 * zero. It takes no arguments, returns nothing, reads only the pointer slot and
 * writes only the object's two fields plus what the callee owns; its 0x3C bytes
 * keep $ra across the single call, so the frame is 0x18 bytes and the call's
 * delay slot stays a nop (nothing is live into it). Its instruction sequence
 * matches the shape of clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0800_801F79B0
 * at 0x801F79B0 in emi/scenario/scena08/00 (same frame, call and byte-0x02
 * increment; that target's overlay stores its countdown as a halfword at 0x5A
 * instead of this one's word at 0x0C), a different target that owns its own
 * address, boundary, map row and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0700_801F6E54(void) {
  u8* work;

  func_801F8358();
  work = SPAD_PTR_SLOT(u8, 0x44);
  work[2]++;
  *(s32*)(work + 0xC) = 0x40;
}
