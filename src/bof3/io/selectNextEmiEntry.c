#include "bof3/core/slus_internal.h"

typedef struct EmiAudioEntry {
  u32 word;
  u8  unknown_04[8];
  u16 index;
  u16 state;
  u8  unknown_10[4];
} EmiAudioEntry;

extern u32           D_80146458;
extern u32           D_8014646C;
extern u8            D_80146481;
extern u8            D_80146482;
extern u8            D_80146483;
extern u8            D_80146484;
extern EmiAudioEntry D_80146780[];
extern s8            D_80148FC0[];

/* @behavior selects the next EMI entry, transfers its dispatch metadata into
 * loader state, releases any prior owner, and advances the loader step.
 * @source 0x80162790
 * @status partial
 * @match 68.18
 * @residual instruction schedule and register allocation only: 45/66
 * instructions with the original 264-byte size. The original keeps the
 * prior-owner load/store in source order around the loader-word load and
 * reloads D_80146483 for the release test; the canonical -O2 profile
 * re-schedules both and reuses the narrow local instead.
 */
void selectNextEmiEntry(void) {
  u8   unused_stack[64];
  u32* table_word;
  u16  entry_index;

  if (D_8014646C == 0) {
    u32 current_word;
    u8  prev_slot;

    prev_slot = D_80146481;
    table_word = &D_80146458;
    current_word = *table_word;
    D_80146484 = prev_slot;

    entry_index = D_80146780[current_word].index;
    D_80146483 = entry_index;

    *table_word = D_80146780[(u8)entry_index].word;
    D_80146780[(u8)entry_index].state = 0;

    if (D_80148FC0[D_80146483] != -1) {
      SsUtAllKeyOff(0);
      func_8016AD2C(D_80148FC0[D_80146482]);
      D_80148FC0[D_80146483] = -1;
    }
  }

  copyEmiTransferChunk();
  {
    u32* loader_step;
    loader_step = &D_8014646C;
    *loader_step = *loader_step + 1;
  }
  (void)unused_stack;
}
