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
 * @source 0x80162790 FUN_80162790
 */
void func_80162790(void) {
  u8   unused_stack[64];
  u32* table_word;

  if (D_8014646C == 0) {
    table_word = &D_80146458;
    D_80146484 = D_80146481;
    D_80146483 = D_80146780[*table_word].index;
    *table_word = D_80146780[D_80146483].word;
    D_80146780[D_80146483].state = 0;

    if (D_80148FC0[D_80146483] != -1) {
      func_8016debc(0);
      func_8016ad2c(D_80148FC0[D_80146482]);
      D_80148FC0[D_80146483] = -1;
    }
  }

  func_80162c14();
  {
    u32* loader_step;
    loader_step = &D_8014646C;
    *loader_step = *loader_step + 1;
  }
  (void)unused_stack;
}
