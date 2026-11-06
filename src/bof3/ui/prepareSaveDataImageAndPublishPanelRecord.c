#include "bof3/ui/shop00_internal.h"

/* The original copies the 16-byte scenario-state header with `lwl`/`lwr` +
 * `swl`/`swr` word pairs, which is the code GCC 2.7.2 emits for a block move
 * whose type has alignment 1: both block-copied values below are byte
 * aggregates for that reason, not plain `u32` cells. */
typedef struct SaveHeaderBytes {
  u8 bytes[0x10];
} SaveHeaderBytes;

typedef struct SavePlayTimeBytes {
  u8 bytes[4];
} SavePlayTimeBytes;

/* @source 0x801D69E0
 * @behavior Save-data prepare routine of the shop/status save screen, called by
 *           the save-write handler func_801D51B8 and the load handler
 *           func_801D5894: it draws the shop header row through func_801D7080
 *           and func_801EF27C, copies the 16-byte scenario-state header
 *           D_80146870 into the head of the live save-data block D_801448D8 and
 *           publishes the save header fields: the scenario-progress bytes at
 *           +0x20..+0x23, the world id at +0x24, the game-mode record byte at
 *           +0x27, the player position words at +0x28/+0x2C, the cleared
 *           checksum halfword at +0x70 and nine halfwords at +0xCBC. It then
 *           rebuilds the save-slot panel record at +0xCA0 from the first party
 *           record at +0x90 and the play-time word at +0x6E8, replaces the
 *           first three party-order ids D_80144F5A with 0xB when an id equals
 *           the id 4 unless the low seven bits of the byte at +0x74C are 5 or
 *           0xC, stores the result of the flag test
 *           func_8015B5D4(&D_80144F28, 0x92) at +0x14 and republishes the
 *           world-flag halfword D_80146258 as a byte into D_80145025. Finally
 *           it copies the 0x10B0 live save bytes into the memory-card image
 *           buffer at 0x800C1A00, zero-fills the rest of its 0x1E00 bytes,
 *           stores the u16 byte-sum over the copied bytes at 0x800C1A70,
 *           extends the used-save count D_801E6078 when the selected slot
 *           D_801E6074 is the last one, and publishes the panel record for that
 *           slot through func_801D6CE4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void prepareSaveDataImageAndPublishPanelRecord(void) {
  u8* state;
  u8* record;
  u8* party;
  u8* p;
  u8* src;
  u8* dst;
  u32 sum;
  u32 flag;
  u16 i;

  func_801D7080();
  func_801EF27C(1, 0x13, D_80145AD4);
  *(SaveHeaderBytes*)D_801448D8 = *(SaveHeaderBytes*)&D_80146870;
  /* The header cells keep the symbol form: the original emits one `lui` plus
   * one store immediate per cell instead of indexing a base register. */
  D_80144948 = 0;
  D_801448F8 = g_ScenarioProgress;
  D_801448F9 = D_80146865;
  D_801448FA = D_80146866;
  D_801448FB = D_80146867;
  D_80144900 = D_80145EC4;
  D_80144904 = D_80145EC8;
  D_801448FC = D_80143F00;
  D_801448FF = D_80145E98[0];
  for (i = 0; i < 9; i++) {
    ((u16*)(D_801448D8 + 0xCBC))[i] = D_80145AB4[i];
  }
  /* The block base is materialized once here, after the halfword copy, and the
   * record, party-record and play-time accesses below are derived from it; the
   * earlier header stores above are the symbol form. */
  state = D_801448D8;
  record = state + 0xCA0;
  party = state + 0x90;
  strncpy((char*)record, (char*)party, 5);
  for (i = 0; i < 3; i++) {
    if (D_80144F5A[i] == 4) {
      /* The original keeps this cell address in its own register for the whole
       * loop (`addiu a3,s2,0x74C`); writing it as a state-relative subscript
       * folds the offset into the load instead, so the constant-address form is
       * the one that reproduces the hoisted address. */
      flag = D_801448D8[0x74C] & 0x7F;
      if ((flag != 5) && (flag != 0xC)) {
        /* Pointer-first add: `record + i` before the +5 displacement, matching
         * the original's `addu` operand order. */
        *(record + i + 5) = 0xB;
        continue;
      }
    }
    *(record + i + 5) = D_80144F5A[i];
  }
  /* The flag bank at save-data +0x650 is the block whose flag 0x92 is tested;
   * the original materializes its address for the call and derives the copy
   * source below as that address minus the member offset. */
  p = (u8*)&D_80144F28;
  record[8] = party[6];
  *(SavePlayTimeBytes*)(record + 0xC) = *(SavePlayTimeBytes*)(state + 0x6E8);
  record[9] = state[0x7A];
  record[0xA] = state[0x7B];
  *(u32*)(record + 0x10) = *(u32*)(party + 8);
  record[0x14] = func_8015B5D4((u32)p, 0x92);
  D_80145025 = D_80146258;
  sum = 0;
  dst = (u8*)0x800C1A00;
  src = p - 0x650;
  for (i = 0; i < 0x10B0; i++) {
    *dst = *src;
    dst++;
    /* The second read is a separate load in the original: the store through
     * `dst` and the read through `src` are pointer-based, so the compiler
     * cannot prove they do not alias and keeps both accesses. */
    sum += *src;
    src++;
  }
  for (; i < 0x1E00; i++) {
    *dst = 0;
    dst++;
  }
  D_800C1A70 = sum;
  if (D_801E6074 == D_801E6078) {
    D_801E6078 = D_801E6078 + 1;
  }
  func_801D6CE4(D_801E6074, D_801448D8);
}
