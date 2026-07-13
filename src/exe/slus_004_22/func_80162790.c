#include "internal.h"

typedef struct EmiAudioEntry {
  u32 word;
  u8  unknown_04[8];
  u16 index;
  u16 state;
  u8  unknown_10[4];
} EmiAudioEntry;

extern u32           DAT_80146458;
extern u32           DAT_8014646c;
extern u8            DAT_80146481;
extern u8            DAT_80146482;
extern u8            DAT_80146483;
extern u8            DAT_80146484;
extern EmiAudioEntry DAT_80146780[];
extern s8            DAT_80148fc0[];

/* @behavior selects the next EMI entry, transfers its dispatch metadata into
 * loader state, releases any prior owner, and advances the loader step.
 * @source 0x80162790 FUN_80162790
 */
void func_80162790(void) {
  u8 unused_stack[64];
  u32* table_word;

  if (DAT_8014646c == 0) {
    table_word = &DAT_80146458;
    DAT_80146484 = DAT_80146481;
    DAT_80146483 = DAT_80146780[*table_word].index;
    *table_word = DAT_80146780[DAT_80146483].word;
    DAT_80146780[DAT_80146483].state = 0;

    if (DAT_80148fc0[DAT_80146483] != -1) {
      func_8016debc(0);
      func_8016ad2c(DAT_80148fc0[DAT_80146482]);
      DAT_80148fc0[DAT_80146483] = -1;
    }
  }

  func_80162c14();
  {
    u32* loader_step;
    loader_step = &DAT_8014646c;
    *loader_step = *loader_step + 1;
  }
  (void)unused_stack;
}
