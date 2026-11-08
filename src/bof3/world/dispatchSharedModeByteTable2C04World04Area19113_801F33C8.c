#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with lb,
 * so it is declared signed here while every arm stores it with a plain byte
 * store. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared flag byte and phase halfword the entry arms wait on. */
extern u8 D_80143BB0;  /* @source 0x80143BB0 @kind data */
extern u16 D_80143C40; /* @source 0x80143C40 @kind data */

/* Shared selector byte the record step clears and the shared flag byte the
 * release arms zero. */
extern u8 D_80145029; /* @source 0x80145029 @kind data */
extern u8 D_8014832E; /* @source 0x8014832E @kind data */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Shared main-RAM flag bank the release steps clear bits of. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared front state the record step resets. */
extern u16 D_80144F54; /* @source 0x80144F54 @kind data */
extern u8 D_80144F56;  /* @source 0x80144F56 @kind data */
extern u8 D_80146871;  /* @source 0x80146871 @kind data */
extern u8 D_80146880;  /* @source 0x80146880 @kind data */
extern u8 D_80146881;  /* @source 0x80146881 @kind data */

/* Shared front selector bundle the release steps request. */
extern u16 D_80145048; /* @source 0x80145048 @kind data */
extern u32 D_80145040; /* @source 0x80145040 @kind data */
extern u32 D_80145044; /* @source 0x80145044 @kind data */
extern u8 D_8014504A;  /* @source 0x8014504A @kind data */
extern u16 D_8014625A; /* @source 0x8014625A @kind data */

void func_80150224(s32 action_id);
void func_8014ECAC(u16 local_mode);
void func_80161D20(s32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_8015B580(void* flag_bank, u8 bit_index);
void func_8015B5A8(void* flag_bank, u8 bit_index);
void func_8015C058(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void resetFieldRecordPairThenFillWorkRecordsWorld04Area19113_801F3AB0(void);

/* @source 0x801F33C8
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay area-entry sequence, selecting the arm from the 31-slot
 *           jump table the payload keeps at 0x801F2C04. Modes 0, 2, 3, 4, 10,
 *           20 and 30 have bodies and every other slot returns untouched.
 *           Mode 0 requests the area action 0x6D through func_80150224, latches
 *           the shared phase byte 0x80143BB0 to 2 and advances the mode to 1.
 *           Mode 2 waits until that phase byte differs from 2, sets the shared
 *           front-end mode to 0 through func_8014ECAC and advances to 3. Mode 3
 *           waits until the shared halfword 0x80143C40 reads zero, clears the
 *           byte 0x8014832E, passes the shared byte 0x80145029 to the shared
 *           helper 0x80161D20, fills the overlay field records through
 *           0x801F3AB0, re-arms the shared frame timer through 0x801636A0 with
 *           the pair 0 and 1, and advances to 4. Mode 4 waits for the shared
 *           predicate 0x80163EA0, raises bit 0x82 of the shared bank through
 *           0x8015B580, advances to 10 and resets the shared front state: it
 *           increments the byte 0x80144F56, clears the halfword 0x80144F54 and
 *           the bytes 0x80146880 and 0x80146881, and raises bit 0 of the state
 *           byte 0x80146871. Mode 10 releases the overlay through 0x8015C058,
 *           requests the front selector context from the halfword 0x80145048
 *           with 0x80145040 and 0x80145044 as kind 4 through 0x8019FA28, clears
 *           the byte 0x8014504A, raises bit 0x40 of the halfword 0x8014625A,
 *           clears flag 0x77 of the shared bank through 0x8015B5A8 and drops the
 *           shared flag byte 0x801448EB and the mode byte to zero. Mode 20 only
 *           releases the overlay and clears that pair. Mode 30 does the same
 *           release and bank clear, requests the context 0x96 with 0x180000 and
 *           0x300000 as kind 1, clears the byte 0x8014504A, raises bit 0x40 of
 *           the halfword 0x8014625A, clears the shared flag and mode bytes and
 *           clears bit 0 of the state byte 0x80146871.
 *           Both release arms reach the shared flag bank 0x80144F28 through a
 *           pointer to the neighbouring bundle cells, which the original also
 *           uses to compute the bank base: mode 10 keeps the address of the
 *           selector halfword 0x80145048 live across the context call and
 *           subtracts 0x120 from it, mode 30 does the same with the flag byte
 *           0x8014504A and 0x122, so the base is one register add and not a
 *           second symbol materialisation. The comparison is exact for the 31
 *           table words as well: the queue of mode bodies the payload keeps at
 *           0x801F2C04 is reproduced by the switch, checked through the
 *           reviewed rodata placement of this function.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C04World04Area19113_801F33C8(void) {
  switch (D_801448EC) {
  case 0:
    func_80150224(0x6D);
    D_80143BB0 = 2;
    D_801448EC = 1;
    break;
  case 2:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8014ECAC(0);
    D_801448EC = 3;
    break;
  case 3:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    func_80161D20(D_80145029);
    resetFieldRecordPairThenFillWorkRecordsWorld04Area19113_801F3AB0();
    func_801636A0(0, 1);
    D_801448EC = 4;
    break;
  case 4:
    if ((u8)func_80163EA0() == 0) {
      break;
    }
    func_8015B580(&D_80144F28, 0x82);
    D_801448EC = 10;
    {
      u8* state;

      D_80144F56++;
      state = &D_80146871;
      D_80144F54 = 0;
      D_80146880 = 0;
      D_80146881 = 0;
      *state |= 1;
    }
    break;
  case 10:
    {
      u16* selector;

      if (D_80143BB0 == 2) {
        break;
      }
      func_8015C058();
      selector = &D_80145048;
      func_8019FA28(*selector, D_80145040, D_80145044, 4);
      D_8014504A = 0;
      D_8014625A |= 0x40;
      func_8015B5A8((u8*)selector - 0x120, 0x77);
      D_801448EB = 0;
      D_801448EC = 0;
    }
    break;
  case 20:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    D_801448EC = 0;
    break;
  case 30:
    {
      u8* flag;
      u8* state;

      if (D_80143BB0 == 2) {
        break;
      }
      func_8015C058();
      func_8019FA28(0x96, 0x180000, 0x300000, 1);
      flag = &D_8014504A;
      D_8014625A |= 0x40;
      *flag = 0;
      func_8015B5A8(flag - 0x122, 0x77);
      state = &D_80146871;
      D_801448EB = 0;
      D_801448EC = 0;
      *state &= 0xFE;
    }
    break;
  }
}
