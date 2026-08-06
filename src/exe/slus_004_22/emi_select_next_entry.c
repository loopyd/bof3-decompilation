#include "internal.h"

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
 */
void emi_select_next_entry(void) {
  u8   unused_stack[64];
  u32* table_word;
  u8   next_index;
  u16  entry_index;

  if (D_8014646C == 0) {
    u32 current_word;
    u16 raw_index;
    u8  prev_slot;

    table_word = &D_80146458;
    current_word = *table_word;
    /* MATCHING_AID: hoisting the entry-index lhu before the D_80146481 copy
     * reproduces the original prologue schedule (lui/addiu a0, lbu, lw, sb).
     * Exhausted: statement reordering and volatile qualifiers left the copy
     * store after the table chain (asm-diff first=+0x000c). Permuter-found;
     * remove if the allocator emits the original order without it. */
    entry_index = D_80146780[current_word].index;
    prev_slot = D_80146481;
    D_80146484 = prev_slot;
    /* MATCHING_AID: barrier() pins the sb to D_80146484 ahead of the
     * D_80146780 index chain; without it GCC sinks the store past the lhu
     * (byte-match DIFFER). Remove if codegen keeps the store first. */
    barrier();
    raw_index = entry_index;
    D_80146483 = raw_index;
    next_index = (u8)raw_index;
    /* MATCHING_AID: barrier() keeps the sb to D_80146483 directly after the
     * lhu result (original fills no delay slot there); the separate
     * next_index local reproduces the original andi v0,0xff narrowing.
     * Remove if the original sb placement survives without it. */
    barrier();
    *table_word = D_80146780[next_index].word;
    D_80146780[next_index].state = 0;
    /* MATCHING_AID: barrier() forces the original reload of D_80146483
     * (lui/lbu) after the word/state stores before the D_80148FC0 test;
     * without it GCC reuses the cached register. Remove if a declaration
     * change reproduces the reload. */
    barrier();

    if (D_80148FC0[D_80146483] != -1) {
      SsUtAllKeyOff(0);
      func_8016AD2C(D_80148FC0[D_80146482]);
      D_80148FC0[D_80146483] = -1;
    }
  }

  emi_copy_transfer_chunk();
  {
    u32* loader_step;
    loader_step = &D_8014646C;
    *loader_step = *loader_step + 1;
  }
  (void)unused_stack;
}
