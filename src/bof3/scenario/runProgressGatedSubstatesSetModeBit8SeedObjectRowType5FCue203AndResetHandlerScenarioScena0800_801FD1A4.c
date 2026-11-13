#include "bof3/bof3.h"

extern u8 D_1F800000;
extern u8 D_80143FC8[];
extern u16 D_80146258;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;
extern u8 g_ScenarioProgress;

extern void dispatchSoundCue(u32 cue_id);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
s32 func_8019601C(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C601C(s32 arg0);

/* @source 0x801FD1A4
 * @behavior Overlay scenario handler selected by the shared primary state byte
 * D_80146874, driving its own sub-state byte D_80146875 through an eight-value
 * switch whose last value and default both leave the handler untouched.
 * Sub-state 0 waits for the shared progress byte g_ScenarioProgress to read 8,
 * runs func_801C601C(0), writes sub-state 1 and then raises bit 3 of the shared
 * mode halfword D_80146258. Sub-state 1 reaches the progress byte through one
 * address taken into a pointer, and while that byte still reads 0x14 it stores
 * its sum with 1, arms the shared countdown halfword D_80146876 with 0x78 and
 * advances to sub-state 2. Sub-state 2 decrements D_80146876 and, when the
 * unsigned halfword reaches zero, advances to sub-state 3. Sub-state 3 waits
 * for g_ScenarioProgress to read 0 and then runs the shared front-end startup
 * helper func_8015C058, dispatches the front selector context
 * func_8019FA28(0xD, 0x190000, 0x100000, 0x84), sets scenario flag 0xF through
 * func_8015B580(D_8014686C, 0xF) and returns the overlay to handler 0 and
 * sub-state 0. Sub-state 4 does nothing. Sub-state 5 runs func_801BE1B0(5) and
 * advances to sub-state 6. Sub-state 6 waits for g_ScenarioProgress to read 5,
 * claims an object index from func_8019601C, publishes it at scratchpad
 * 0x1F800000 and, unless that byte reads 0xFF, seeds the object 0x74-byte row
 * at 0x80143FC8 (+0 = 1, +5 = 0x5F, +1 = 6); it then arms D_80146876 with 0x1E
 * and advances to sub-state 7. Sub-state 7 decrements D_80146876 and, at zero,
 * clears g_ScenarioProgress, runs func_8015C058, dispatches sound cue 0x203,
 * sets scenario flag 0x10 through func_8015B580(D_8014686C, 0x10), dispatches
 * front selector context func_8019FA28(0xF, 0x198000, 0x510000, 1) and falls
 * into the same two stores that return the overlay to handler 0 and sub-state 0
 * that sub-state 3 branches to. It takes no arguments and returns nothing; the
 * 0x18-byte frame keeps $ra alone and every store to D_80146874, D_80146875 and
 * D_80146258 materialises the 0x8014 page in $at on its own, while the two
 * countdown and the sub-state 1 progress accesses take their address into a
 * local pointer. The eight-entry switch emits the `.rodata` jump table the
 * original links at 0x801F6D94 inside the overlay's leading table data, so
 * target.toml carries that address as a reviewed section placement for this
 * function. It is the immediate predecessor of
 * seedTwoObjectRowsAtProgress6ThenSetFlagBit14AndResetHandlerScenarioScena0800_801FD400
 * at 0x801FD400, which seeds two object rows of the same 0x80143FC8 table from
 * the same scratchpad index byte and shares the D_80146874/D_80146875 handler
 * pair.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runProgressGatedSubstatesSetModeBit8SeedObjectRowType5FCue203AndResetHandlerScenarioScena0800_801FD1A4(void) {
  u8* progress;
  u8 progress_value;
  u16* countdown;
  u16 countdown_value;
  u8 object_index;
  u8* record;

  switch (D_80146875) {
    case 0:
      if (g_ScenarioProgress != 8u) {
        break;
      }

      func_801C601C(0);
      D_80146875 = 1;
      D_80146258 |= 8u;
      break;

    case 1:
      progress = &g_ScenarioProgress;
      progress_value = *progress;
      if ((progress_value & 0xffu) != 0x14u) {
        break;
      }

      *progress = progress_value + 1;
      D_80146876 = 0x78;
      D_80146875 = 2;
      break;

    case 2:
      countdown = &D_80146876;
      countdown_value = *countdown - 1;
      *countdown = countdown_value;
      if (countdown_value != 0u) {
        break;
      }

      D_80146875 = 3;
      break;

    case 3:
      if (g_ScenarioProgress != 0u) {
        break;
      }

      func_8015C058();
      func_8019FA28(0xd, 0x190000, 0x100000, 0x84);
      func_8015B580(D_8014686C, 0xf);
      D_80146874 = 0;
      D_80146875 = 0;
      break;

    case 4:
      break;

    case 5:
      func_801BE1B0(5);
      D_80146875 = 6;
      break;

    case 6:
      if (g_ScenarioProgress != 5u) {
        break;
      }

      object_index = func_8019601C();
      D_1F800000 = object_index;
      if (object_index != 0xffu) {
        record = D_80143FC8 + (u32)object_index * 0x74u;
        record[0] = 1;
        record[5] = 0x5f;
        record[1] = 6;
      }

      D_80146876 = 0x1e;
      D_80146875 = 7;
      break;

    case 7:
      countdown = &D_80146876;
      countdown_value = *countdown - 1;
      *countdown = countdown_value;
      if (countdown_value != 0u) {
        break;
      }

      g_ScenarioProgress = 0;
      func_8015C058();
      dispatchSoundCue(0x203);
      func_8015B580(D_8014686C, 0x10);
      func_8019FA28(0xf, 0x198000, 0x510000, 1);
      D_80146874 = 0;
      D_80146875 = 0;
      break;
  }
}
