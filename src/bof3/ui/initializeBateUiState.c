#include "bof3/bof3.h"

extern u8 D_80144F5A[]; /* @source 0x80144F5A @kind data */
extern u8 D_80148330; /* @source 0x80148330 @kind data */
extern u8 D_80148331[]; /* @source 0x80148331 @kind data */
extern u8 D_80148332; /* @source 0x80148332 @kind data */
extern u8 D_80148333; /* @source 0x80148333 @kind data */
extern s16 D_80148334; /* @source 0x80148334 @kind data */
extern s16 D_80148336; /* @source 0x80148336 @kind data */
extern u8 D_80148338; /* @source 0x80148338 @kind data */
extern u8 D_80148339; /* @source 0x80148339 @kind data */
extern u8 D_8014833A; /* @source 0x8014833A @kind data */
extern u8 D_8014833B; /* @source 0x8014833B @kind data */
extern u8 D_8014833D; /* @source 0x8014833D @kind data */
extern s16 D_80148340; /* @source 0x80148340 @kind data */
extern u8* D_80148350; /* @source 0x80148350 @kind data */
extern u8 D_80148354; /* @source 0x80148354 @kind data */
extern u8 D_80148355; /* @source 0x80148355 @kind data */
extern u8 D_80148356; /* @source 0x80148356 @kind data */
extern u8 D_80148357; /* @source 0x80148357 @kind data */
extern s16 D_80148358; /* @source 0x80148358 @kind data */
extern s16 D_8014835A; /* @source 0x8014835A @kind data */
extern u8 D_8014835E; /* @source 0x8014835E @kind data */
extern u8 D_8014835F; /* @source 0x8014835F @kind data */
extern u8 D_80148360; /* @source 0x80148360 @kind data */
extern u8 D_80148361; /* @source 0x80148361 @kind data */
extern u8* D_80148374; /* @source 0x80148374 @kind data */
extern u8 D_80148378; /* @source 0x80148378 @kind data */
extern u8 D_80148379; /* @source 0x80148379 @kind data */
extern u8 D_8014837A; /* @source 0x8014837A @kind data */
extern u8 D_8014837B; /* @source 0x8014837B @kind data */
extern s16 D_8014837C; /* @source 0x8014837C @kind data */
extern s16 D_8014837E; /* @source 0x8014837E @kind data */
extern s16 D_80148388; /* @source 0x80148388 @kind data */
extern s16 D_8014838A; /* @source 0x8014838A @kind data */
extern u8 D_8014839C; /* @source 0x8014839C @kind data */
extern u8 D_8014839D; /* @source 0x8014839D @kind data */
extern u8 D_8014839E; /* @source 0x8014839E @kind data */
extern u8 D_8014839F; /* @source 0x8014839F @kind data */
extern s16 D_801483A0; /* @source 0x801483A0 @kind data */
extern s16 D_801483A2; /* @source 0x801483A2 @kind data */
extern s16 D_801483AC; /* @source 0x801483AC @kind data */
extern s16 D_801483AE; /* @source 0x801483AE @kind data */
extern u8 D_801483C0; /* @source 0x801483C0 @kind data */
extern u8 D_801483C1; /* @source 0x801483C1 @kind data */
extern u8 D_801483C2; /* @source 0x801483C2 @kind data */
extern u8 D_801D8EE0[]; /* @source 0x801D8EE0 @kind data */

/* @source 0x801D1C90
 * @behavior Initializes the shared BATE (masters) UI work block: the head state
 *           bytes D_80148330..D_80148333 and the halfword pairs
 *           D_80148334/D_80148336, the three panel records anchored at
 *           D_80148354, D_80148378 and D_8014839C, the cursor block
 *           D_80148338..D_80148340, and the record pointer D_801D8EE0
 *           published at D_80148350 and D_80148374. While scanning the first
 *           three shared party ids D_80144F5A it publishes the index of the
 *           first id that is zero or seven through the cell eleven bytes past
 *           D_80148331 (D_8014833C) and leaves that cell unchanged when no id
 *           matches.
 * @status partial
 * @match 77.17
 * @residual 98/127 insns at the original 508-byte size. The loop uses the
 *           original cursor and sentinel registers, but the sentinel load
 *           follows the cursor setup rather than preceding the base setup.
 *           Tail constant scheduling and allocation still differ, including
 *           the two -0x14 halfword stores moving ahead of the zero stores.
 */
void initializeBateUiState(void) {
  u8* base;
  u8* slot;
  u8 index;

  index = 0;
  base = D_80148331;
  slot = base + 11;
  base[0] = 2;
  D_80148332 = 2;
  D_80148333 = 2;
  D_80148330 = 1;
  D_80148334 = 0x140;
  D_80148336 = 0x3E;
  for (; index < 3; index++) {
    if (D_80144F5A[index] == 0 || D_80144F5A[index] == 7) {
      slot[0] = index;
      break;
    }
  }
  D_80148358 = -0xAA;
  D_8014835A = 0x3E;
  D_80148360 = 7;
  D_8014835F = 0xFF;
  slot = D_801D8EE0;
  D_80148350 = slot;
  D_80148374 = slot;
  D_8014837C = 0x14;
  D_8014838A = 0x118;
  D_801483A0 = 0xA4;
  D_80148339 = 0;
  D_8014833B = 0;
  D_8014833A = 0;
  D_80148338 = 0;
  D_8014833D = 0;
  D_80148340 = 0;
  D_80148355 = 2;
  D_80148356 = 3;
  D_80148357 = 2;
  D_80148354 = 1;
  D_8014835E = 0;
  D_80148361 = 1;
  D_80148379 = 2;
  D_8014837A = 4;
  D_8014837B = 2;
  D_80148378 = 1;
  D_8014837E = -0x14;
  D_80148388 = 0;
  D_8014839D = 2;
  D_8014839E = 4;
  D_8014839F = 3;
  D_8014839C = 1;
  D_801483A2 = -0x14;
  D_801483AC = 0;
  D_801483AE = 0x80;
  D_801483C1 = 2;
  D_801483C2 = 1;
  D_801483C0 = 0;
}
