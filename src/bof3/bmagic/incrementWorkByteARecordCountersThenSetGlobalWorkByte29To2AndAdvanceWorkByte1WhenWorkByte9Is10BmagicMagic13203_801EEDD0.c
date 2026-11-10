#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014637C; /* @source 0x8014637C @kind data */

typedef struct BmagicMagic13203CounterRecord {
  u8 counter_00;
  u8 counter_01;
  u8 counter_02;
  u8 unk_03[9];
} BmagicMagic13203CounterRecord;

extern BmagicMagic13203CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/* @source 0x801EEDD0
 * @behavior Increments the first three bytes of the 12-byte record at
 *           0x80145BD6 selected by work byte 0xA, reloading the scratchpad
 *           work-pointer cell for each counter. Increments work byte 9 and,
 *           when its byte-width result equals 0x10, stores 2 in byte 0x29
 *           of the object published at 0x8014637C, then reloads the scratchpad
 *           work pointer and increments work byte 1. Takes no arguments and
 *           returns nothing. The remaining record fields and the global
 *           object's role are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteARecordCountersThenSetGlobalWorkByte29To2AndAdvanceWorkByte1WhenWorkByte9Is10BmagicMagic13203_801EEDD0(void) {
  D_80145BD6[g_battle_work[0xA]].counter_00++;
  D_80145BD6[g_battle_work[0xA]].counter_01++;
  D_80145BD6[g_battle_work[0xA]].counter_02++;
  if (++g_battle_work[9] == 0x10) {
    D_8014637C[0x29] = 2;
    g_battle_work[1]++;
  }
}
