#include "bof3/world/area03004_internal.h"

/* @source 0x801E0124
 * @behavior AREA030 work-record entry-phase start (handler 0 of the 0x801E2348
 * table dispatched through scratch work-record byte 2): it clears the 32-bit
 * record word at 0x3C and queues cue 0x206 through func_8015DF18, then loads
 * the entry halfword at 0x12 of the D_801E2AB8 scale record selected by record
 * byte 0x06 through func_8014DD3C; next it clears bit 5 of record byte 0 and
 * zeroes record byte 0x2A, zeroes record byte 0x48 and loads entry 0 through
 * func_8014D6B8, and finally advances the phase byte 0x02 that selects the next
 * handler of the same table.
 * @status exact
 * @match 100.00
 * @residual none
 * One cursor variable per reload point is deliberate: a single variable reused
 * across the four scratch-cursor reads makes one quantity for the whole body and
 * collects all four reads into $v1, while the original gives the first and the
 * entry-0 group $v0 and the flag and phase groups $v1. Naming each read
 * separately reproduces the original register assignment.
 */
void resetWorkRecordAndRequestEntryWorld00Area03004_801E0124(void) {
  u8* cue;
  u8* flags;
  u8* entry;
  u8* phase;

  cue = D_1F800044;
  *(u32*)(cue + 0x3C) = 0;
  func_8015DF18(0x206);

  func_8014DD3C(D_801E2AB8[D_1F800044[6]].entry_12);

  flags = D_1F800044;
  flags[0] = (u8)(flags[0] & 0xDF);
  D_1F800044[0x2A] = 0;

  entry = D_1F800044;
  entry[0x48] = 0;
  func_8014D6B8(0);

  phase = D_1F800044;
  phase[2] = (u8)(phase[2] + 1u);
}
